/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) The OpenTyrian Development Team
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
#include "nortsong.h"

#include "file.h"
#include "logging.h"
#include "loudness.h"
#include "opentyr.h"
#include "sndmast.h"

#include "SDL.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

JE_word frameCountMax;

Sint16 *soundSamples[SOUND_COUNT] = { NULL }; /* [1..soundnum + 9] */  // FKA digiFx
size_t soundSampleCount[SOUND_COUNT] = { 0 }; /* [1..soundnum + 9] */  // FKA fxSize

JE_word tyrMusicVolume, fxVolume;
const JE_word fxPlayVol = 4;
JE_word tempVolume;

// The frequency of the x86 programmable interval timer is (315 / 88 / 3) MHz.
// The PIT was configured to generate an interrupt every `speed` cycles, which
// decremented `frameCount`.

static Uint16 frameSpeed = 0x4300;

// Fixed point UQ6.10 in milliseconds.
static Uint16 framePeriod = ((Uint64)0x4300 << 10) * 1000 * 88 * 3 / 315000000;

// Fixed point UQ22.10 in milliseconds.
static Uint32 frameCountEnd = 0;
static Uint32 frameCount2End = 0;

void setFrameSpeed(Uint16 speed)  // FKA NortSong.speed and NortSong.setTimerInt
{
	frameSpeed = speed;
	framePeriod = ((Uint64)speed << 10) * 1000 * 88 * 3 / 315000000;

	Uint32 now = SDL_GetTicks() << 10;
	frameCountEnd = now;
}

void setFrameCount(JE_word frameCount)  // FKA NortSong.frameCount
{
	// Keep the partial timer period that has already elapsed.
	Uint32 now = SDL_GetTicks() << 10;
	Sint32 diff = now - frameCountEnd;
	if (diff >= framePeriod)
		frameCountEnd = now - (Uint32)diff % framePeriod;
	else if (-diff >= framePeriod)
		frameCountEnd = now + (Uint32)-diff % framePeriod;

	frameCountEnd += frameCount * framePeriod;
}

void setFrameCount2(JE_word frameCount2)  // FKA NortSong.frameCount2
{
	// Keep the partial timer period that has already elapsed.
	Uint32 now = SDL_GetTicks() << 10;
	Sint32 diff = now - frameCount2End;
	if (diff >= framePeriod)
		frameCount2End = now - (Uint32)diff % framePeriod;
	else if (-diff >= framePeriod)
		frameCount2End = now + (Uint32)-diff % framePeriod;

	frameCount2End += frameCount2 * framePeriod;
}

Uint32 getFrameCountTicks(void)
{
	const Uint32 half = 1 << 9;
	Uint32 now = SDL_GetTicks() << 10;
	Sint32 diff = frameCountEnd - now;
	return diff >= 0 ? ((Uint32)diff + half) >> 10 : 0;
}

Uint32 getFrameCount2Ticks(void)
{
	const Uint32 half = 1 << 9;
	Uint32 now = SDL_GetTicks() << 10;
	Sint32 diff = frameCount2End - now;
	return diff >= 0 ? ((Uint32)diff + half) >> 10 : 0;
}

void delayUntilElapsed(void)
{
	const Uint32 half = 1 << 9;
	Uint32 now = SDL_GetTicks() << 10;
	Sint32 diff = frameCountEnd - now;
	if (diff >= 0)
		SDL_Delay(((Uint32)diff + half) >> 10);
}

static void loadSounds(size_t soundsOffset, size_t soundsCount, const char *filename, bool trim, SDL_AudioCVT *cvt)
{
	File file = dataFileOpen(filename, "rb");
	if (file.error)
	{
		logFatal("Failed to open file '%s': %s", filename, fileGetError(&file));
		exit(EXIT_FAILURE);
	}

	size_t maxSize = 0;

	// Read number of sounds.
	Uint16 count = fileReadU16(&file);
	assert(count == soundsCount);
	count = MIN(count, soundsCount);

	size_t positionsCount = count + 1;
	long *positions = malloc(sizeof *positions * positionsCount);

	// Read positions of sounds.
	for (size_t i = 0; i < count; ++i)
		positions[i] = fileReadU32(&file);

	positions[count] = fileGetLength(&file);

	for (size_t i = 0; i < count; ++i)
	{
		long position = positions[i];
		long endPosition = positions[i + 1];
		size_t size = endPosition > position ? endPosition - position : 0;

		// Voice sounds have some bad data at the end.
		if (trim)
			size = size >= 100 ? size - 100 : 0;

		maxSize = MAX(maxSize, size);
	}

	cvt->buf = malloc(maxSize * cvt->len_mult);

	for (size_t i = 0; i < count; ++i)
	{
		long position = positions[i];
		long endPosition = positions[i + 1];
		size_t size = endPosition > position ? endPosition - position : 0;

		// Voice sounds have some bad data at the end.
		if (trim)
			size = size >= 100 ? size - 100 : 0;

		assert(size <= maxSize);

		fileSetPosition(&file, position);

		fileReadExactly(&file, cvt->buf, size);
		cvt->len = size;

		if (SDL_ConvertAudio(cvt) != 0)
		{
			logError("Failed to convert audio: %s", SDL_GetError());
			continue;
		}

		soundSamples[soundsOffset + i] = malloc(cvt->len_cvt);
		memcpy(soundSamples[soundsOffset + i], cvt->buf, cvt->len_cvt);
		soundSampleCount[soundsOffset + i] = cvt->len_cvt / sizeof (Sint16);
	}

	free(cvt->buf);

	free(positions);

	if (file.error)
		logError("Failed to read from file '%s': %s", filename, fileGetError(&file));

	fileClose(&file);
}

void loadSndFile(bool xmas)
{
	for (size_t i = 0; i < COUNTOF(soundSamples); ++i)
	{
		free(soundSamples[i]);
		soundSamples[i] = NULL;

		soundSampleCount[i] = 0;
	}

	// Build converter to output sample format and rate.
	SDL_AudioCVT cvt;
	if (SDL_BuildAudioCVT(&cvt, AUDIO_S8, 1, 11025, AUDIO_S16SYS, 1, audioSampleRate) < 0)
	{
		logError("Failed to build audio converter: %s", SDL_GetError());
		return;
	}

	const char *sfxFilename = "tyrian.snd";
	loadSounds(0, SFX_COUNT, sfxFilename, false, &cvt);

	const char *voiceFilename = xmas ? "voicesc.snd" : "voices.snd";
	loadSounds(SFX_COUNT, VOICE_COUNT, voiceFilename, true, &cvt);
}

void JE_playSampleNum(JE_byte samplenum)
{
	multiSamplePlay(soundSamples[samplenum-1], soundSampleCount[samplenum-1], 0, fxPlayVol);
}

void JE_changeVolume(JE_word *music, int music_delta, JE_word *sample, int sample_delta)
{
	int music_temp = *music + music_delta,
	    sample_temp = *sample + sample_delta;
	
	if (music_delta)
	{
		if (music_temp > 255)
		{
			music_temp = 255;
			JE_playSampleNum(S_CLINK);
		}
		else if (music_temp < 0)
		{
			music_temp = 0;
			JE_playSampleNum(S_CLINK);
		}
	}
	
	if (sample_delta)
	{
		if (sample_temp > 255)
		{
			sample_temp = 255;
			JE_playSampleNum(S_CLINK);
		}
		else if (sample_temp < 0)
		{
			sample_temp = 0;
			JE_playSampleNum(S_CLINK);
		}
	}
	
	*music = music_temp;
	*sample = sample_temp;
	
	set_volume(*music, *sample);
}
