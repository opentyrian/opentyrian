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
#include "nortsong.h"

#include "file.h"
#include "joystick.h"
#include "keyboard.h"
#include "loudness.h"
#include "musmast.h"
#include "opentyr.h"
#include "params.h"
#include "sndmast.h"
#include "vga256d.h"

#include "SDL.h"

JE_boolean notYetLoadedSound = true;

JE_word frameCountMax;

JE_byte *digiFx[SAMPLE_COUNT] = { NULL }; /* [1..soundnum + 9] */
JE_word fxSize[SAMPLE_COUNT]; /* [1..soundnum + 9] */

JE_word tyrMusicVolume, fxVolume;
JE_word fxPlayVol;
JE_word tempVolume;

// The period of the x86 programmable interval timer in milliseconds.
static const float pitPeriod = (12.0f / 14318180.0f) * 1000.0f;

static Uint16 delaySpeed = 0x4300;
static float delayPeriod = 0x4300 * ((12.0f / 14318180.0f) * 1000.0f);

static Uint32 target = 0;
static Uint32 target2 = 0;

void setDelay(int delay)  // FKA NortSong.frameCount
{
	target = SDL_GetTicks() + delay * delayPeriod;
}

void setDelay2(int delay)  // FKA NortSong.frameCount2
{
	target2 = SDL_GetTicks() + delay * delayPeriod;
}

Uint32 getDelayTicks(void)  // FKA NortSong.frameCount
{
	Sint32 delay = target - SDL_GetTicks();
	return MAX(0, delay);
}

Uint32 getDelayTicks2(void)  // FKA NortSong.frameCount2
{
	Sint32 delay = target2 - SDL_GetTicks();
	return MAX(0, delay);
}

void wait_delay(void)
{
	Sint32 delay = target - SDL_GetTicks();
	if (delay > 0)
		SDL_Delay(delay);
}

void service_wait_delay(void)
{
	for (; ; )
	{
		service_SDL_events(false);

		Sint32 delay = target - SDL_GetTicks();
		if (delay <= 0)
			return;

		SDL_Delay(MIN(delay, SDL_POLL_INTERVAL));
	}
}

void wait_delayorinput(void)
{
	for (; ; )
	{
		service_SDL_events(false);
		poll_joysticks();

		if (newkey || mousedown || joydown)
		{
			newkey = false;
			return;
		}

		Sint32 delay = target - SDL_GetTicks();
		if (delay <= 0)
			return;

		SDL_Delay(MIN(delay, SDL_POLL_INTERVAL));
	}
}

void JE_loadSndFile(const char *effects_sndfile, const char *voices_sndfile)
{
	JE_byte y, z;
	JE_longint templ;
	JE_longint sndPos[2][SAMPLE_COUNT + 1];
	JE_word sndNum;

	FILE *fi;
	
	/* SYN: Loading offsets into TYRIAN.SND */
	fi = dir_fopen_die(data_dir(), effects_sndfile, "rb");

	fread_u16_die(&sndNum, 1, fi);

	assert(sndNum < COUNTOF(sndPos[0]) - 1);
	fread_s32_die(sndPos[0], sndNum, fi);
	fseek(fi, 0, SEEK_END);
	sndPos[0][sndNum] = ftell(fi); /* Store file size */

	for (z = 0; z < sndNum; z++)
	{
		fseek(fi, sndPos[0][z], SEEK_SET);
		fxSize[z] = (sndPos[0][z+1] - sndPos[0][z]); /* Store sample sizes */
		free(digiFx[z]);
		digiFx[z] = malloc(fxSize[z]);
		fread_u8_die(digiFx[z], fxSize[z], fi); /* JE: Load sample to buffer */
	}

	fclose(fi);

	/* SYN: Loading offsets into VOICES.SND */
	fi = dir_fopen_die(data_dir(), voices_sndfile, "rb");
	
	fread_u16_die(&sndNum, 1, fi);

	assert(sndNum < COUNTOF(sndPos[1]) - 1);
	fread_s32_die(sndPos[1], sndNum, fi);
	fseek(fi, 0, SEEK_END);
	sndPos[1][sndNum] = ftell(fi); /* Store file size */

	z = SAMPLE_COUNT - 9;

	for (y = 0; y < sndNum; y++)
	{
		fseek(fi, sndPos[1][y], SEEK_SET);

		templ = (sndPos[1][y+1] - sndPos[1][y]) - 100; /* SYN: I'm not entirely sure what's going on here. */
		if (templ < 1)
			templ = 1;
		fxSize[z + y] = templ; /* Store sample sizes */
		free(digiFx[z + y]);
		digiFx[z + y] = malloc(fxSize[z + y]);
		fread_u8_die(digiFx[z + y], fxSize[z + y], fi); /* JE: Load sample to buffer */
	}

	fclose(fi);

	notYetLoadedSound = false;

}

void JE_playSampleNum(JE_byte samplenum)
{
	JE_multiSamplePlay(digiFx[samplenum-1], fxSize[samplenum-1], 0, fxPlayVol);
}

void JE_calcFXVol(void) // TODO: not sure *exactly* what this does
{
	fxPlayVol = (fxVolume - 1) >> 5;
}

void setDelaySpeed(Uint16 speed)  // FKA NortSong.speed and NortSong.setTimerInt
{
	delaySpeed = speed;
	delayPeriod = speed * pitPeriod;
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
	
	JE_calcFXVol();
	
	set_volume(*music, *sample);
}
