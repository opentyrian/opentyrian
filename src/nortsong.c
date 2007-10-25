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

#include "error.h"
#include "joystick.h"
#include "keyboard.h"
#include "loudness.h"
#include "musmast.h"
#include "params.h"
#include "sndmast.h"
#include "vga256d.h"

#define NO_EXTERNS
#include "nortsong.h"
#undef NO_EXTERNS

#include "SDL.h"


JE_word w1;
JE_AweType * awe_data;
/*JE_word tempw;*/
JE_word w2;
JE_byte sberror;
JE_byte sysintcount;
JE_byte sbint;
JE_AweType * awe_code;
void * oldvector;
JE_byte midiport;
JE_byte sysintwait;
JE_word sbport;
JE_DigiMixType * digimix;
JE_byte midierror;
JE_longint address;
JE_word intcount;
JE_word dspversion;
const char hexa[17] = "0123456789ABCDEF";

Uint32 target, target2;
JE_boolean mixEnable = false;
JE_boolean notYetLoadedSound = true;
JE_boolean notYetLoadedMusic = true;

JE_SongPosType songPos;

JE_byte soundEffects = 1; /* TODO: Give this a real value, figure out what they mean. */

JE_word frameCount, frameCount2, frameCountMax;

JE_byte currentSong = 0;
JE_byte jConfigure = 0;

JE_byte soundActive = true; /* I'm not sure if these two are ever false. */
JE_byte musicActive = true; /* TODO: Make sure these get proper values. */

JE_byte *digiFx[SOUND_NUM + 9]; /* [1..soundnum + 9] */
JE_word fxSize[SOUND_NUM + 9]; /* [1..soundnum + 9] */
int allocd_digifx = false;


JE_word fxVolume = 128; /* Default value, should be loaded from config */
JE_word fxPlayVol = (128 - 1) >> 5; /* Same result as calling calcFXVol with default value of fxvolume*/

JE_word tempVolume;
JE_word tyrMusicVolume;

float jasondelay = 1000.0f / (1193180.0f / 0x4300);

void setdelay( JE_byte delay )
{
	target = (delay << 4)+SDL_GetTicks(); /* delay << 4 == delay * 16 */
}

void setjasondelay( int delay )
{
	target = SDL_GetTicks() + delay * jasondelay;
}

void setjasondelay2( int delay )
{
	target2 = SDL_GetTicks() + delay * jasondelay;
}

int delaycount( void )
{
	return (SDL_GetTicks() < target ? target - SDL_GetTicks() : 0);
}

int delaycount2( void )
{
	return (SDL_GetTicks() < target2 ? target2 - SDL_GetTicks() : 0);
}

void wait_delay( void )
{
	Uint32 ticks;

	while ((ticks = SDL_GetTicks()) < target)
	{
		if (ticks % SDL_POLL_INTERVAL == 0)
		{
			service_SDL_events(false);
		}
	}
}

void wait_delayorinput( JE_boolean keyboard, JE_boolean mouse, JE_boolean joystick )
{
	Uint32 ticks;

	newkey = newmouse = false;
	service_SDL_events(false);
	while ((ticks = SDL_GetTicks()) < target && !(keydown || !keyboard) && !(mousedown || !mouse) && !(button[0] || !joystick))
	{
		if (ticks % SDL_POLL_INTERVAL == 0)
		{
			if (joystick)
			{
				JE_joystick2();
			}
			service_SDL_events(false);
		}
	}
}

void JE_loadSong( JE_word songnum )
{
	JE_word x;
	FILE *fi, *test;

	JE_resetFile(&fi, "music.mus");

	if (notYetLoadedMusic)
	{
		/* SYN: We're loading offsets into MUSIC.MUS for each song here. */
		notYetLoadedMusic = false;
		efread(&x, sizeof(x), 1, fi);
		efread(songPos, sizeof(JE_longint), sizeof(songPos) / sizeof(JE_longint), fi); /* SYN: reads long int (i.e. 4) * MUSICNUM */
		fseek(fi, 0, SEEK_END);
		songPos[MUSIC_NUM] = ftell(fi); /* Store file size */
	}

	/* SYN: Now move to the start of the song we want, and load the number of bytes given by the
	   difference in offsets between it and the next song. */
	fseek(fi, songPos[songnum - 1], SEEK_SET);
	efread(&musicData, 1, songPos[songnum] - songPos[songnum - 1], fi);

	/* currentSong = songnum; */

	fclose(fi);
}

void JE_loadSndFile( void )
{
	FILE *fi;
	JE_byte y, z;
	JE_word x;
	JE_longint templ;
	JE_longint sndPos[2][SOUND_NUM + 1]; /* Reindexed by -1, dammit Jason */
	JE_word sndNum;

	/* SYN: Loading offsets into TYRIAN.SND */
	JE_resetFile(&fi, "tyrian.snd");
	efread(&sndNum, sizeof(sndNum), 1, fi);

	for (x = 0; x < sndNum; x++)
	{
		efread(&sndPos[0][x], sizeof(sndPos[0][x]), 1, fi);
	}
	fseek(fi, 0, SEEK_END);
	sndPos[1][sndNum] = ftell(fi); /* Store file size */

	for (z = 0; z < sndNum; z++)
	{
		fseek(fi, sndPos[0][z], SEEK_SET);
		fxSize[z] = (sndPos[0][z+1] - sndPos[0][z]); /* Store sample sizes */
		if (allocd_digifx) free(digiFx[z]);
		digiFx[z] = malloc(fxSize[z]);
		efread(digiFx[z], 1, fxSize[z], fi); /* JE: Load sample to buffer */
	}
	allocd_digifx = true;

	fclose(fi);

	/* SYN: Loading offsets into VOICES.SND */
	if (tyrianXmas)
	{
		JE_resetFile(&fi, "voicesc.snd");
	} else {
		JE_resetFile(&fi, "voices.snd");
	}
	efread(&sndNum, sizeof(sndNum), 1, fi);

	for (x = 0; x < sndNum; x++)
	{
		efread(&sndPos[1][x], sizeof(sndPos[1][x]), 1, fi);
	}
	fseek(fi, 0, SEEK_END);
	sndPos[1][sndNum] = ftell(fi); /* Store file size */

	z = SOUND_NUM;

	for (y = 0; y < sndNum; y++)
	{
		fseek(fi, sndPos[1][y], SEEK_SET);

		templ = (sndPos[1][y+1] - sndPos[1][y]) - 100; /* SYN: I'm not entirely sure what's going on here. */
		if (templ < 1) templ = 1;
		fxSize[z + y] = templ; /* Store sample sizes */
		digiFx[z + y] = malloc(fxSize[z + y]);
		efread(digiFx[z + y], 1, fxSize[z + y], fi); /* JE: Load sample to buffer */
	}

	fclose(fi);

	notYetLoadedSound = false;

}

void JE_playSong ( JE_word songnum )
{
	/* If sound is disabled, bail out */
	if (noSound)
	{
		return;
	}


	#ifndef NDEBUG
	printf("Loading song number %d...\n", songnum);
	#endif
	if (songnum == 0) /* SYN: Trying to play song 0 was doing strange things D: */
	{
		JE_stopSong();
		currentSong = 0;
		return;
	}
	if (currentSong != songnum && musicActive) /* Original also checked for midiport > 1 */
	{
		 /*ASM
		 IN   al, $21
		 push ax
		 OR   al, 3
		 out  $21, al
		 END;*/

		 JE_stopSong();
		currentSong = songnum;
		 JE_loadSong (songnum);
		 repeated = false;
		 playing = true;
		 JE_selectSong (1);
		 /* JE_waitRetrace(); */

		 /*ASM
		 mov al, $36
		 out $43, al
		 mov ax, speed
		 out $40, al
		 mov al, ah
		 out $40, al
		 pop ax
		 out $21, al
		 END;*/
	}
}

void JE_stopSong( void )
{
	JE_selectSong(0);
}

void JE_restartSong( void )
{
	JE_selectSong(2);
}

void JE_playSampleNum( JE_byte samplenum )
{
	if (soundEffects > 0 && soundActive)
	{
		/* SYN: Reindexing by -1 because of Jason's arrays starting at 1. Dammit. */
		JE_multiSamplePlay( digiFx[samplenum-1], fxSize[samplenum-1], 0, fxPlayVol );
	}
}

void JE_calcFXVol( void )
{
	fxPlayVol = (fxVolume - 1) >> 5;
	printf("JE_calcFXVol: in:0x%x out:0x%x\n", fxVolume, fxPlayVol);
}

void JE_setTimerInt( void )
{
	jasondelay = 1000.0 / (1193180.0 / speed);
}

void JE_resetTimerInt( void )
{
	jasondelay = 1000.0 / (1193180.0 / 0x4300);
}

void JE_timerInt( void )
{
	STUB();
}

void JE_changeVolume( JE_word *temp, JE_integer change, JE_word *fxvol, JE_integer fxchange )
{
	if (change != 0)
	{
		if (*temp + change > 254)
		{
			*temp = 256 - change;
			JE_playSampleNum(WRONG);
		}
		*temp += change;
		if (*temp < 16)
		{
			*temp = 16;
			JE_playSampleNum(WRONG);
		}
	}

	if (fxchange != 0)
	{
		if (*fxvol + fxchange > 254)
		{
			*fxvol = 256 - fxchange;
			JE_playSampleNum(WRONG);
		}
		*fxvol += fxchange;
		if (*fxvol < 16)
		{
			*fxvol = 16;
			JE_playSampleNum(WRONG);
		}
	}

	JE_calcFXVol();
	JE_setVol(*temp, fxPlayVol);
}

void JE_waitFrameCount( void )
{
	/* TODO: I'm not sure how long this function should wait. Leaving it blank for now, it doesn't seem to hurt much... */
}
