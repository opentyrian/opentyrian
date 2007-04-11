/* vim: set noet:
 *
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Team
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

#define NO_EXTERNS
#include "loudness.h"
#undef NO_EXTERNS

/* TODO: Audio isn't really "working" yet. It makes noises but I still neeed to build
   the mixer and full audio system back here. Also need to add the music. So yeah, I
   know this code sucks right now. Ignore it, please. */


/* SYN: These are externally accessible variables: */
JE_MusicType musicData;
JE_boolean repeated;
JE_boolean playing;

/* SYN: These shouldn't be used outside this file. Hands off! */
signed char *channel_buffer [SFX_CHANNELS]; /* SYN: I'm not sure what Tyrian actually does for sound effect channels... */
signed char *channel_pos [SFX_CHANNELS];
Uint32 channel_len [SFX_CHANNELS];
int sound_init_state = FALSE;

void audio_cb(void *userdata, unsigned char *feedme, int howmuch);

/* SYN: The arguments to this function are probably meaningless now */
void JE_initialize(JE_word soundblaster, JE_word midi, JE_boolean mixenable, JE_byte sberror, JE_byte midierror)
{
    SDL_AudioSpec plz;
	int i = 0;

	sound_init_state = TRUE;
	
	/*final_audio_buffer = NULL;
	audio_pos = NULL;*/
	for (i = 0; i < SFX_CHANNELS + 1; i++)
	{
		channel_buffer[i] = channel_pos[i] = NULL;
		channel_len[i] = 0;
	}
	
    plz.freq = 11025;
    plz.format = AUDIO_S8;
    plz.channels = 1;
    plz.samples = 512;
    plz.callback = audio_cb;
    plz.userdata = NULL;

    if ( SDL_OpenAudio(&plz, NULL) < 0 ) 
	{
        printf("WARNING: Failed to initialize SDL audio. Bailing out.\n");
        exit(1);
    }
    SDL_PauseAudio(0);
}

void audio_cb(void *userdata, unsigned char *feedme, int howmuch)
{
	int ch, smp, qu;
	
	/* SYN: Mix sound channels and shove into audio buffer */
	for (ch = 0; ch < SFX_CHANNELS; ch++) {
		
		/* SYN: Don't copy more data than is in the channel! */
		qu = ( (Uint32) howmuch > channel_len[ch] ? (int) channel_len[ch] : howmuch);
		
		for (smp = 0; smp < qu; smp++)
		{
			feedme[smp] = ((signed char) feedme[smp] + (channel_pos[ch][smp] / VOLUME_SCALING ));
		}
		
		channel_pos[ch] += qu;
		channel_len[ch] -= qu;
		
		/* SYN: If we've emptied a channel buffer, let's free the memory and clear the channel. */
		if (channel_len == 0)
		{
			free(channel_buffer[ch]);
			channel_buffer[ch] = channel_pos[ch] = NULL;
		}
	}
}

void JE_deinitialize( void )
{
	/* SYN: TODO: Clean up any other audio stuff, if necessary. This should only be called when we're quitting. */
	SDL_CloseAudio();
}

void JE_play( void )
{
	/* SYN: This proc isn't necessary, because filling the buffer is handled in the SDL callback function.*/
}

/* SYN: selectSong is called with 0 to disable the current song. Calling it with 1 will start the current song if not playing,
   or restart it if it is. */
void JE_selectSong( JE_word value )
{
	STUB(JE_selectSong);
}

void JE_samplePlay(JE_word addlo, JE_word addhi, JE_word size, JE_word freq)
{
	/* SYN: I don't think this function is used. */
	STUB(JE_samplePlay);
}

void JE_bigSamplePlay(JE_word addlo, JE_word addhi, JE_word size, JE_word freq)
{
	/* SYN: I don't think this function is used. */
	STUB(JE_bigSamplePlay);
}

/* Call with 0x1-0x100 for music volume, and 0x10 to 0xf0 for sample volume. */
/* SYN: Either I'm misunderstanding Andreas's comments, or the information in them is inaccurate. */
void JE_setVol(JE_word volume, JE_word sample)
{
	STUB(JE_setVol);
}

JE_word JE_getVol( void )
{
	STUB(JE_getVol);
	return 0;
}

JE_word JE_getSampleVol( void )
{
	STUB(JE_getSampleVol);
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
	int i; 
	double v = 1;
	/* v = (vol - 0x100) / ((double) 0xe00); */ /* SYN: Convert Loudness vol to fraction) */
	
	if (channel_buffer[chan] != NULL)
	{
		/* SYN: Something is already playing on this channel, so remove it */
		free(channel_buffer[chan]);
	}
	
	channel_len[chan] = size;
	channel_buffer[chan] = malloc(size);
	channel_pos[chan] = channel_buffer[chan];
	
	for (i = 0; i < size; i++)
	{
		channel_buffer[chan][i] = ((signed char) buffer[i]) * v;
	}
}

