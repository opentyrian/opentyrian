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

#include <stdlib.h>

float music_volume = 0, sample_volume = 0;

bool music_stopped = true;
unsigned int song_playing = 0;

bool audio_disabled = false, music_disabled = false, samples_disabled = false;

/* SYN: These shouldn't be used outside this file. Hands off! */
FILE *music_file = NULL;
Uint32 *song_offset;
Uint16 song_count = 0;


SAMPLE_TYPE *channel_buffer[SFX_CHANNELS] = { NULL };
SAMPLE_TYPE *channel_pos[SFX_CHANNELS] = { NULL };
Uint32 channel_len[SFX_CHANNELS] = { 0 };
Uint8 channel_vol[SFX_CHANNELS];

int sound_init_state = false;

static SDL_AudioDeviceID audio_device = 0;

void audio_cb( void *user_data, unsigned char *sdl_buffer, int bytes_needed );

void load_song( unsigned int song_num );

bool init_audio( void )
{
	if (audio_disabled)
		return false;
	
	SDL_AudioSpec ask, got;
	
	ask.freq = SAMPLE_RATE;
	ask.format = (BYTES_PER_SAMPLE == 2) ? AUDIO_S16SYS : AUDIO_S8;
	ask.channels = 1;
	ask.samples = 256 * OUTPUT_QUALITY; // ~23 ms
	ask.callback = audio_cb;
	
	if (SDL_InitSubSystem(SDL_INIT_AUDIO))
	{
		fprintf(stderr, "error: failed to initialize SDL audio: %s\n", SDL_GetError());
		audio_disabled = true;
		return false;
	}
	
	audio_device = SDL_OpenAudioDevice(/*device*/ NULL, /*iscapture*/ 0, &ask, &got, /*allowed_changes*/ 0);

	if (audio_device == 0)
	{
		fprintf(stderr, "error: SDL failed to open audio device: %s\n", SDL_GetError());
		audio_disabled = true;
		return false;
	}
	
	opl_init();
	
	SDL_PauseAudioDevice(audio_device, 0); // unpause
	
	return true;
}

void audio_cb( void *user_data, unsigned char *sdl_buffer, int bytes_needed )
{
	(void)user_data;
	
	static long ct = 0;
	
	SAMPLE_TYPE *const feedme = (SAMPLE_TYPE *)sdl_buffer;
	const int samples_needed = bytes_needed / BYTES_PER_SAMPLE;

	if (!music_disabled && !music_stopped)
	{
		/* SYN: Simulate the fm synth chip */
		SAMPLE_TYPE *music_pos = feedme;
		long remaining = samples_needed;
		while (remaining > 0)
		{
			while (ct < 0)
			{
				ct += SAMPLE_RATE;
				lds_update(); /* SYN: Do I need to use the return value for anything here? */
			}
			/* SYN: Okay, about the calculations below. I still don't 100% get what's going on, but...
			- SAMPLE_RATE is samples/time as output by SDL.
			- REFRESH is how often the play proc would have been called in Tyrian. Standard speed is
			70Hz, which is the default value of 70.0f
			- ct represents the margin between play time (representing # of samples) and tick speed of
			the songs (70Hz by default). It keeps track of which one is ahead, because they don't
			synch perfectly. */
			
			/* set i to smaller of data requested by SDL and a value calculated from the refresh rate */
			long i = (long)((ct / REFRESH) + 4) & ~3;
			i = (i > remaining) ? remaining : i; /* i should now equal the number of samples we get */
			opl_update((SAMPLE_TYPE *)music_pos, i);
			music_pos += i;
			remaining -= i;
			ct -= (long)(REFRESH * i);
		}
		
		/* Reduce the music volume. */
		for (int smp = 0; smp < samples_needed; smp++)
		{
			feedme[smp] *= music_volume;
		}
	}
	else
	{
		for (int smp = 0; smp < samples_needed; smp++)
		{
			feedme[smp] = 0;
		}
	}
	
	if (!samples_disabled)
	{
		/* SYN: Mix sound channels and shove into audio buffer */
		for (int ch = 0; ch < SFX_CHANNELS; ch++)
		{
			float volume = sample_volume * (channel_vol[ch] / (float)SFX_CHANNELS);
			
			/* SYN: Don't copy more data than is in the channel! */
			const unsigned int qu = ((unsigned)bytes_needed > channel_len[ch] ? channel_len[ch] : (unsigned)bytes_needed) / BYTES_PER_SAMPLE;
			for (unsigned int smp = 0; smp < qu; smp++)
			{
#if (BYTES_PER_SAMPLE == 2)
				Sint32 clip = (Sint32)feedme[smp] + (Sint32)(channel_pos[ch][smp] * volume);
				feedme[smp] = (clip > 0x7fff) ? 0x7fff : (clip <= -0x8000) ? -0x8000 : (Sint16)clip;
#else  /* BYTES_PER_SAMPLE */
				Sint16 clip = (Sint16)feedme[smp] + (Sint16)(channel_pos[ch][smp] * volume);
				feedme[smp] = (clip > 0x7f) ? 0x7f : (clip <= -0x80) ? -0x80 : (Sint8)clip;
#endif  /* BYTES_PER_SAMPLE */
			}
			
			channel_pos[ch] += qu;
			channel_len[ch] -= qu * BYTES_PER_SAMPLE;
			
			/* SYN: If we've emptied a channel buffer, let's free the memory and clear the channel. */
			if (channel_len[ch] == 0)
			{
				free(channel_buffer[ch]);
				channel_buffer[ch] = channel_pos[ch] = NULL;
			}
		}
	}
}

void deinit_audio( void )
{
	if (audio_disabled)
		return;
	
	if (audio_device != 0)
	{
		SDL_PauseAudioDevice(audio_device, 1); // pause
		SDL_CloseAudioDevice(audio_device);
		audio_device = 0;
	}
	
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	
	for (unsigned int i = 0; i < SFX_CHANNELS; i++)
	{
		free(channel_buffer[i]);
		channel_buffer[i] = channel_pos[i] = NULL;
		channel_len[i] = 0;
	}
	
	lds_free();
}


void load_music( void )
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

void load_song( unsigned int song_num )
{
	if (audio_disabled)
		return;
	
	SDL_LockAudioDevice(audio_device);
	
	if (song_num < song_count)
	{
		unsigned int song_size = song_offset[song_num + 1] - song_offset[song_num];
		lds_load(music_file, song_offset[song_num], song_size);
	}
	else
	{
		fprintf(stderr, "warning: failed to load song %d\n", song_num + 1);
	}
	
	SDL_UnlockAudioDevice(audio_device);
}

void play_song( unsigned int song_num )
{
	if (song_num != song_playing)
	{
		load_song(song_num);
		song_playing = song_num;
	}
	
	music_stopped = false;
}

void restart_song( void )
{
	unsigned int temp = song_playing;
	song_playing = -1;
	play_song(temp);
}

void stop_song( void )
{
	music_stopped = true;
}

void fade_song( void )
{
	/* STUB: we have no implementation of this to port */
}

void set_volume( unsigned int music, unsigned int sample )
{
	music_volume = music * (1.5f / 255.0f);
	sample_volume = sample * (1.0f / 255.0f);
}

void JE_multiSamplePlay(JE_byte *buffer, JE_word size, JE_byte chan, JE_byte vol)
{
	if (audio_disabled || samples_disabled)
		return;
	
	SDL_LockAudioDevice(audio_device);
	
	free(channel_buffer[chan]);
	
	channel_len[chan] = size * BYTES_PER_SAMPLE * SAMPLE_SCALING;
	channel_buffer[chan] = malloc(channel_len[chan]);
	channel_pos[chan] = channel_buffer[chan];
	channel_vol[chan] = vol + 1;

	for (int i = 0; i < size; i++)
	{
		for (int ex = 0; ex < SAMPLE_SCALING; ex++)
		{
#if (BYTES_PER_SAMPLE == 2)
			channel_buffer[chan][(i * SAMPLE_SCALING) + ex] = (Sint8)buffer[i] << 8;
#else  /* BYTES_PER_SAMPLE */
			channel_buffer[chan][(i * SAMPLE_SCALING) + ex] = (Sint8)buffer[i];
#endif  /* BYTES_PER_SAMPLE */
		}
	}

	SDL_UnlockAudioDevice(audio_device);
}

