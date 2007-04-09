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
Uint8 *final_audio_buffer;
Uint8 *audio_pos;

/* SYN: These are temporary while I get the bare bones audio working. */
Uint8 *tmp_buffer;
Uint8 *tmp_pos;
Uint32 tmp_len;

void audio_cb(void *userdata, Uint8 *feedme, int howmuch);

/* SYN: The arguments to this function are probably meaningless now */
void JE_initialize(JE_word soundblaster, JE_word midi, JE_boolean mixenable, JE_byte sberror, JE_byte midierror)
{
    SDL_AudioSpec plz;
	
	tmp_len = 0;
	tmp_pos = NULL;
	tmp_buffer = NULL;
	
    plz.freq = 11025;
    plz.format = AUDIO_S8;
    plz.channels = 1;
    plz.samples = 512;
    plz.callback = audio_cb;
    plz.userdata = NULL;

    if ( SDL_OpenAudio(&plz, NULL) < 0 ) 
	{
        printf("WARNING: Failed to initialize audio. Bailing out.\n");
        exit(1);
    }
    SDL_PauseAudio(0);
}

void audio_cb(void *userdata, Uint8 *feedme, int howmuch)
{
	if (tmp_len == 0) 
	{
		return;
	}
		
	howmuch = ( (Uint32) howmuch > tmp_len ? (int) tmp_len : howmuch);
	SDL_MixAudio(feedme, tmp_pos, howmuch, SDL_MIX_MAXVOLUME);
	tmp_pos += howmuch;
	tmp_len -= howmuch;
}

void JE_deinitialize( void )
{
	/* TODO: Stub function, need to fill in */
}

void JE_play( void )
{
	/* TODO: Stub function, need to fill in */
}

/* SYN: selectSong is called with 0 to disable the current song. Calling it with 1 will start the current song if not playing,
   or restart it if it is. */
void JE_selectSong( JE_word value )
{
	/* TODO: Stub function, need to fill in */
}

void JE_samplePlay(JE_word addlo, JE_word addhi, JE_word size, JE_word freq)
{
	/* TODO: Stub function, need to fill in */
}

void JE_bigSamplePlay(JE_word addlo, JE_word addhi, JE_word size, JE_word freq)
{
	/* TODO: Stub function, need to fill in */
}

/* Call with 0x1-0x100 for music volume, and 0x10 to 0xf0 for sample volume. */
void JE_setVol(JE_word volume, JE_word sample)
{
	/* TODO: Stub function, need to fill in */
}

JE_word JE_getVol( void )
{
	/* TODO: Stub function, need to fill in */
	return 0;
}

JE_word JE_getSampleVol( void )
{
	/* TODO: Stub function, need to fill in */
	return 0;
}

void JE_multiSampleInit(JE_word addlo, JE_word addhi, JE_word dmalo, JE_word dmahi)
{
	/* TODO: Stub function, need to fill in */
}

void JE_multiSampleMix( void )
{
	/* TODO: Stub function, need to fill in */
}

void JE_multiSamplePlay(JE_byte *buffer, JE_word size, JE_byte chan, JE_byte vol)
{
	int i; 
	
	tmp_len = size;
	tmp_buffer = malloc(tmp_len);
	tmp_pos = tmp_buffer;
	
	for (i = 0; i < size; i++)
	{
		tmp_buffer[i] = (Uint8) (((signed char) buffer[i]) / 5);
	}
}

