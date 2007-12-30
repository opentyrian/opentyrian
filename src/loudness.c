/* vim: set noet:
 *
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Development Team
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
#include "opentyr.h"
#include "loudness.h"

#include "fm_synth.h"
#include "lds_play.h"
#include "params.h"


/* SYN: These are externally accessible variables: */
JE_MusicType musicData;
JE_boolean repeated;
JE_boolean playing;

float sample_volume = 0.4f;
float music_volume = 0.6f;

SDL_mutex *soundmutex = NULL;

/* SYN: These shouldn't be used outside this file. Hands off! */
SAMPLE_TYPE *channel_buffer[SFX_CHANNELS];
SAMPLE_TYPE *channel_pos[SFX_CHANNELS];
Uint32 channel_len[SFX_CHANNELS];
Uint8 channel_vol[SFX_CHANNELS];
int sound_init_state = false;
int freq = 11025 * OUTPUT_QUALITY;

bool music_playing = false;


void audio_cb(void *userdata, unsigned char *feedme, int howmuch);

/* SYN: The arguments to this function are probably meaningless now */
void JE_initialize(JE_word soundblaster, JE_word midi, JE_boolean mixenable, JE_byte sberror, JE_byte midierror)
{
	SDL_AudioSpec plz, got;
	int i = 0;

	if (SDL_InitSubSystem(SDL_INIT_AUDIO))
	{
		printf("Failed to initialize audio: %s\n", SDL_GetError());
		noSound = true;
		return;
	}

	sound_init_state = true;

	soundmutex = SDL_CreateMutex();

	if (soundmutex == NULL)
	{
		printf("Couldn't create mutex! Oh noes!\n");
		exit(-1);
	}

	for (i = 0; i < SFX_CHANNELS; i++)
	{
		channel_buffer[i] = channel_pos[i] = NULL;
		channel_len[i] = 0;
	}

	opl_init();

	plz.freq = freq;
	plz.format = AUDIO_S16SYS;
	plz.channels = 1;
	plz.samples = 512;
	plz.callback = audio_cb;
	plz.userdata = soundmutex;

	printf("\tRequested SDL frequency: %d; SDL buffer size: %d\n", plz.freq, plz.samples);

	if ( SDL_OpenAudio(&plz, &got) < 0 )
	{
		printf("\tWARNING: Failed to initialize SDL audio. Bailing out.\n");
		exit(1);
	}

	printf("\tObtained  SDL frequency: %d; SDL buffer size: %d\n", got.freq, got.samples);

	SDL_PauseAudio(0);
}

void audio_cb(void *userdata, unsigned char *sdl_buffer, int howmuch)
{
	static long ct = 0;

	SDL_mutex *mut = (SDL_mutex *) userdata;

	/* Making sure that we don't mess with sound buffers when someone else is using them! */
	if (SDL_mutexP(mut) == -1)
	{
		printf("Couldn't lock mutex! Argh!\n");
		exit(-1);
	}

	SAMPLE_TYPE *feedme = (SAMPLE_TYPE *) sdl_buffer;

	if (music_playing)
	{
		/* SYN: Simulate the fm synth chip */
		SAMPLE_TYPE *music_pos = feedme;
		long remaining = howmuch / BYTES_PER_SAMPLE;
		while (remaining > 0)
		{
			while (ct < 0)
			{
				ct += freq;
				lds_update(); /* SYN: Do I need to use the return value for anything here? */
			}
			/* SYN: Okay, about the calculations below. I still don't 100% get what's going on, but...
			- freq is samples/time as output by SDL.
			- REFRESH is how often the play proc would have been called in Tyrian. Standard speed is
			70Hz, which is the default value of 70.0f
			- ct represents the margin between play time (representing # of samples) and tick speed of
			the songs (70Hz by default). It keeps track of which one is ahead, because they don't
			synch perfectly. */
	
			/* set i to smaller of data requested by SDL and a value calculated from the refresh rate */
			long i = (long)((ct / REFRESH) + 4) & ~3;
			i = (i > remaining) ? remaining : i; /* i should now equal the number of samples we get */
			opl_update((short *)music_pos, i);
			music_pos += i;
			remaining -= i;
			ct -= (long)(REFRESH * i);
		}
	
		/* Reduce the music volume. */
		int qu = howmuch / BYTES_PER_SAMPLE;
		for (int smp = 0; smp < qu; smp++)
		{
			feedme[smp] *= music_volume;
		}
	}

	/* SYN: Mix sound channels and shove into audio buffer */
	for (int ch = 0; ch < SFX_CHANNELS; ch++)
	{
		float volume = sample_volume * (channel_vol[ch] / 8.0f);
		
		/* SYN: Don't copy more data than is in the channel! */
		int qu = (howmuch > channel_len[ch] ? channel_len[ch] : howmuch) / BYTES_PER_SAMPLE;
		for (int smp = 0; smp < qu; smp++)
		{
			long clip = (long)feedme[smp] + (long)(channel_pos[ch][smp] * volume);
			feedme[smp] = (clip > 0x7fff) ? 0x7fff : (clip <= -0x8000) ? -0x8000 : (short)clip;
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

	SDL_mutexV(mut); /* release mutex */
}

void JE_deinitialize( void )
{
	/* SYN: TODO: Clean up any other audio stuff, if necessary. This should only be called when we're quitting. */
	opl_deinit();
	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void JE_play( void )
{
	/* SYN: This proc isn't necessary, because filling the buffer is handled in the SDL callback function.*/
}

/* SYN: selectSong is called with 0 to disable the current song. Calling it with 1 will start the current song if not playing,
   or restart it if it is. */
void JE_selectSong( JE_word value )
{
	if (noSound)
		return;

	/* TODO: The mutex'd region could possibly be smaller, but I wanted to keep it in this file. */

	/* Making sure that we don't mess with sound buffers when someone else is using them! */
	if (SDL_mutexP(soundmutex) == -1)
	{
		printf("Couldn't lock mutex! Argh!\n");
		exit(-1);
	}

	switch (value)
	{
		case 0:
			music_playing = false;
			break;
		case 1:
		case 2:
			lds_load((JE_byte *) musicData); /* Load song */
			music_playing = true;
			break;
		default:
			printf("JE_selectSong: fading TODO!\n");
			/* TODO: Finish this FADING function! */
			break;
	}

	SDL_mutexV(soundmutex); /* release mutex */
}

void JE_samplePlay(JE_word addlo, JE_word addhi, JE_word size, JE_word freq)
{
	/* SYN: I don't think this function is used. */
	STUB();
}

void JE_bigSamplePlay(JE_word addlo, JE_word addhi, JE_word size, JE_word freq)
{
	/* SYN: I don't think this function is used. */
	STUB();
}

/* Call with 0x1-0x100 for music volume, and 0x10 to 0xf0 for sample volume. */
/* SYN: Either I'm misunderstanding Andreas's comments, or the information in them is inaccurate. */
void JE_setVol(JE_word volume, JE_word sample)
{
	/* printf("JE_setVol: music: %d, sample: %d\n", volume, sample); */
	
	if (volume > 0)
		music_volume = volume * (float)(0.6 / 256.0);
	sample_volume = sample * (float)(0.4 / 256.0);
}

JE_word JE_getVol( void )
{
	STUB();
	return 0;
}

JE_word JE_getSampleVol( void )
{
	STUB();
	return 0;
}

void JE_multiSampleInit(JE_word addlo, JE_word addhi, JE_word dmalo, JE_word dmahi)
{
	/* SYN: I don't know if this function should do anything else. For now, it just checks to see if sound has
	   been initialized and, if not, calls the main initialize function. */

	if (!sound_init_state)
	{
		JE_initialize(0, 0, 0, 0, 0);
	}
}

void JE_multiSampleMix( void )
{
	/* SYN: This proc isn't necessary, because the mixing is handled in the SDL callback function.*/
}

void JE_multiSamplePlay(JE_byte *buffer, JE_word size, JE_byte chan, JE_byte vol)
{
	if (noSound)
		return;
	
	/* Making sure that we don't mess with sound buffers when someone else is using them! */
	if (SDL_mutexP(soundmutex) == -1)
	{
		printf("Couldn't lock mutex! Argh!\n");
		exit(-1);
	}

	free(channel_buffer[chan]);

	channel_len[chan] = size * BYTES_PER_SAMPLE * SAMPLE_SCALING;
	channel_buffer[chan] = malloc(channel_len[chan]);
	channel_pos[chan] = channel_buffer[chan];
	channel_vol[chan] = vol + 1;

	for (int i = 0; i < size; i++)
	{
		for (int ex = 0; ex < SAMPLE_SCALING; ex++)
		{
			channel_buffer[chan][(i * SAMPLE_SCALING) + ex] = ((SAMPLE_TYPE) ((Sint8) buffer[i]) << 8);
		}
	}

	SDL_mutexV(soundmutex); /* release mutex */
}

