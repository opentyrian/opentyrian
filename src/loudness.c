/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "loudness.h"

#include "file.h"
#include "lds_play.h"
#include "nortsong.h"
#include "opentyr.h"
#include "params.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT_QUALITY 4  // 44.1 kHz

int audioSampleRate = 0;

bool music_stopped = true;
unsigned int song_playing = 0;

bool audio_disabled = false, music_disabled = false, samples_disabled = false;

static SDL_AudioDeviceID audioDevice = 0;

static Uint8 musicVolume = 255;
static Uint8 sampleVolume = 255;

static const float volumeRange = 30.0f;  // dB

// Fixed point Q20.12; needs to be able to store (10 * INT16_MIN/MAX)
static Sint32 volumeFactorTable[256];
#define TO_FIXED(x) ((Sint32)((x) * (1 << 12)))
#define FIXED_TO_INT(x) ((Sint32)((x) >> 12))

// Twice the Loudness update rate (in updates/second).  In Tyrian, Loudness
// updates were performed at the same rate as the game timer, which varied
// depending on the game speed (~69.57 Hz at most game speeds).  We don't have
// the same limitations, so we'll keep the update rate constant, but we do want
// to stick to integer math, so we'll update at 69.5 Hz.
static const int ldsUpdate2Rate = 139;  // 69.5 * 2

static int samplesPerLdsUpdate;
static int samplesPerLdsUpdateFrac;

static int samplesUntilLdsUpdate = 0;
static int samplesUntilLdsUpdateFrac = 0;

static FILE *music_file = NULL;
static Uint32 *song_offset;
static Uint16 song_count = 0;

#define CHANNEL_COUNT 8
static const Sint16 *channelSamples[CHANNEL_COUNT];
static size_t channelSampleCount[CHANNEL_COUNT] = { 0 };
static Uint8 channelVolume[CHANNEL_COUNT];
#define CHANNEL_VOLUME_LEVELS 8

static void audioCallback(void *userdata, Uint8 *stream, int size);

static void load_song(unsigned int song_num);

bool init_audio(void)
{
	if (audio_disabled)
		return false;

	SDL_AudioSpec ask, got;

	ask.freq = 11025 * OUTPUT_QUALITY;
	ask.format = AUDIO_S16SYS;
	ask.channels = 1;
	ask.samples = 256 * OUTPUT_QUALITY; // ~23 ms
	ask.callback = audioCallback;

	if (SDL_InitSubSystem(SDL_INIT_AUDIO))
	{
		fprintf(stderr, "error: failed to initialize SDL audio: %s\n", SDL_GetError());
		audio_disabled = true;
		return false;
	}

	int allowedChanges = SDL_AUDIO_ALLOW_FREQUENCY_CHANGE;
#if SDL_VERSION_ATLEAST(2, 0, 9)
	allowedChanges |= SDL_AUDIO_ALLOW_SAMPLES_CHANGE;
#endif
	audioDevice = SDL_OpenAudioDevice(/*device*/ NULL, /*iscapture*/ 0, &ask, &got, allowedChanges);

	if (audioDevice == 0)
	{
		fprintf(stderr, "error: SDL failed to open audio device: %s\n", SDL_GetError());
		audio_disabled = true;
		return false;
	}

	audioSampleRate = got.freq;

	samplesPerLdsUpdate = 2 * (audioSampleRate / ldsUpdate2Rate);
	samplesPerLdsUpdateFrac = 2 * (audioSampleRate % ldsUpdate2Rate);

	volumeFactorTable[0] = 0;
	for (size_t i = 1; i < 256; ++i)
		volumeFactorTable[i] = TO_FIXED(powf(10, (255 - i) * (-volumeRange / (20.0f * 255))));

	opl_init();

	SDL_PauseAudioDevice(audioDevice, 0); // unpause

	return true;
}

static void audioCallback(void *userdata, Uint8 *stream, int size)
{
	(void)userdata;

	Sint16 *const samples = (Sint16 *)stream;
	const int samplesCount = size / sizeof (Sint16);

	if (!music_disabled && !music_stopped)
	{
		Sint16 *remaining = samples;
		int remainingCount = samplesCount;
		while (remainingCount > 0)
		{
			if (samplesUntilLdsUpdate == 0)
			{
				lds_update();

				// The number of samples that should be produced per Loudness
				// update is not an integer, but we can only produce an integer
				// number of samples, so we accumulate the fractional samples
				// until it amounts to a whole sample.
				samplesUntilLdsUpdate += samplesPerLdsUpdate;
				samplesUntilLdsUpdateFrac += samplesPerLdsUpdateFrac;
				if (samplesUntilLdsUpdateFrac >= ldsUpdate2Rate)
				{
					samplesUntilLdsUpdate += 1;
					samplesUntilLdsUpdateFrac -= ldsUpdate2Rate;
				}
			}

			int count = MIN(samplesUntilLdsUpdate, remainingCount);

			opl_update(remaining, count);

			remaining += count;
			remainingCount -= count;

			samplesUntilLdsUpdate -= count;
		}
	}
	else
	{
		for (int i = 0; i < samplesCount; ++i)
			samples[i] = 0;
	}

	Sint32 musicVolumeFactor = volumeFactorTable[musicVolume];
	musicVolumeFactor *= 2;  // OPL emulator is too quiet

	if (samples_disabled && !music_disabled)
	{
		// Mix music
		Sint16 *remaining = samples;
		int remainingCount = samplesCount;
		while (remainingCount > 0)
		{
			Sint32 sample = *remaining * musicVolumeFactor;

			sample = FIXED_TO_INT(sample);
			*remaining = MIN(MAX(INT16_MIN, sample), INT16_MAX);

			remaining += 1;
			remainingCount -= 1;
		}
	}
	else if (!samples_disabled)
	{
		Sint32 sampleVolumeFactor = volumeFactorTable[sampleVolume];
		Sint32 sampleVolumeFactors[CHANNEL_VOLUME_LEVELS];
		for (int i = 0; i < CHANNEL_VOLUME_LEVELS; ++i)
			sampleVolumeFactors[i] = sampleVolumeFactor * (i + 1) / CHANNEL_VOLUME_LEVELS;

		// Mix music and channels
		Sint16 *remaining = samples;
		int remainingCount = samplesCount;
		while (remainingCount > 0)
		{
			Sint32 sample = *remaining * musicVolumeFactor;

			for (size_t i = 0; i < CHANNEL_COUNT; ++i)
			{
				if (channelSampleCount[i] > 0)
				{
					sample += *channelSamples[i] * sampleVolumeFactors[channelVolume[i]];

					channelSamples[i] += 1;
					channelSampleCount[i] -= 1;
				}
			}

			sample = FIXED_TO_INT(sample);
			*remaining = MIN(MAX(INT16_MIN, sample), INT16_MAX);

			remaining += 1;
			remainingCount -= 1;
		}
	}
}

void deinit_audio(void)
{
	if (audio_disabled)
		return;

	if (audioDevice != 0)
	{
		SDL_PauseAudioDevice(audioDevice, 1); // pause
		SDL_CloseAudioDevice(audioDevice);
		audioDevice = 0;
	}

	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	memset(channelSampleCount, 0, sizeof channelSampleCount);

	lds_free();
}

void load_music(void)  // FKA NortSong.loadSong
{
	if (music_file == NULL)
	{
		music_file = dir_fopen_die(data_dir(), "music.mus", "rb");

		fread_u16_die(&song_count, 1, music_file);

		song_offset = malloc((song_count + 1) * sizeof(*song_offset));

		fread_u32_die(song_offset, song_count, music_file);

		song_offset[song_count] = ftell_eof(music_file);
	}
}

static void load_song(unsigned int song_num)  // FKA NortSong.loadSong
{
	if (song_num < song_count)
	{
		unsigned int song_size = song_offset[song_num + 1] - song_offset[song_num];
		lds_load(music_file, song_offset[song_num], song_size);
	}
	else
	{
		fprintf(stderr, "warning: failed to load song %d\n", song_num + 1);
	}
}

void play_song(unsigned int song_num)  // FKA NortSong.playSong
{
	if (audio_disabled)
		return;

	if (song_num != song_playing)
	{
		SDL_LockAudioDevice(audioDevice);

		music_stopped = true;

		SDL_UnlockAudioDevice(audioDevice);

		load_song(song_num);

		song_playing = song_num;
	}

	SDL_LockAudioDevice(audioDevice);

	music_stopped = false;

	SDL_UnlockAudioDevice(audioDevice);
}

void restart_song(void)  // FKA Player.selectSong(1)
{
	if (audio_disabled)
		return;

	SDL_LockAudioDevice(audioDevice);

	lds_rewind();

	music_stopped = false;

	SDL_UnlockAudioDevice(audioDevice);
}

void stop_song(void)  // FKA Player.selectSong(0)
{
	if (audio_disabled)
		return;

	SDL_LockAudioDevice(audioDevice);

	music_stopped = true;

	SDL_UnlockAudioDevice(audioDevice);
}

void fade_song(void)  // FKA Player.selectSong($C001)
{
	if (audio_disabled)
		return;

	SDL_LockAudioDevice(audioDevice);

	lds_fade(1);

	SDL_UnlockAudioDevice(audioDevice);
}

void set_volume(Uint8 musicVolume_, Uint8 sampleVolume_)  // FKA NortSong.setVol and Player.setVol
{
	if (audio_disabled)
		return;

	SDL_LockAudioDevice(audioDevice);

	musicVolume = musicVolume_;
	sampleVolume = sampleVolume_;

	SDL_UnlockAudioDevice(audioDevice);
}

void multiSamplePlay(const Sint16 *samples, size_t sampleCount, Uint8 chan, Uint8 vol)  // FKA Player.multiSamplePlay
{
	assert(chan < CHANNEL_COUNT);
	assert(vol < CHANNEL_VOLUME_LEVELS);

	if (audio_disabled || samples_disabled)
		return;

	SDL_LockAudioDevice(audioDevice);

	channelSamples[chan] = samples;
	channelSampleCount[chan] = sampleCount;
	channelVolume[chan] = vol;

	SDL_UnlockAudioDevice(audioDevice);
}
