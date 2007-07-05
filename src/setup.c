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

#include "config.h"
#include "fonthand.h"
#include "joystick.h"
#include "jukebox.h"
#include "keyboard.h"
#include "loudness.h"
#include "network.h"
#include "newshape.h"
#include "nortsong.h"
#include "nortvars.h"
#include "starfade.h"
#include "starlib.h"
#include "varz.h"
#include "vga256d.h"

#define NO_EXTERNS
#include "setup.h"
#undef NO_EXTERNS

#include "SDL.h"


JE_boolean volumeActive = TRUE;
JE_boolean fx;
JE_word fxNum;
JE_boolean repeatedFade, continuousPlay;

void JE_textMenuWait( JE_word *waitTime, JE_boolean doGamma )
{
	JE_setMousePosition(160, 100);

	do
	{
		JE_showVGA();

		service_SDL_events(TRUE);

		inputDetected = newkey | mousedown | JE_joystickNotHeld();

		if (lastkey_sym == SDLK_SPACE)
		{
			lastkey_sym = SDLK_RETURN;
		}

		if (mousedown)
		{
			newkey = TRUE;
			lastkey_sym = SDLK_RETURN;
		}

#ifdef NDEBUG
		if (mouseInstalled)
		{
			if (abs(mouse_y - 100) > 10)
			{
				inputDetected = TRUE;
				if (mouse_y - 100 < 0)
				{
					lastkey_sym = SDLK_UP;
				} else {
					lastkey_sym = SDLK_DOWN;
				}
				newkey = TRUE;
			}
			if (abs(mouse_x - 160) > 10)
			{
				inputDetected = TRUE;
				if (mouse_x - 160 < 0)
				{
					lastkey_sym = SDLK_LEFT;
				} else {
					lastkey_sym = SDLK_RIGHT;
				}
				newkey = TRUE;
			}
		}
#endif

		SDL_Delay(1); /* <MXD> attempt non-processor-based wait, implement a real delay later */
		if (*waitTime > 0)
		{
			(*waitTime)--;
		}
	} while (!(inputDetected || *waitTime == 1 || haltGame || netQuit));
}


void JE_jukeboxGo( void )
{
	JE_boolean weirdMusic, weirdCurrent;
	JE_byte weirdSpeed = 0;
	char tempStr[64];

	JE_byte lastSong;
	JE_byte tempVolume;
	JE_boolean youStopped, drawText, quit, fade;


	weirdMusic = FALSE;
	weirdCurrent = TRUE;
	drawText = TRUE;

	fx = FALSE;
	fxNum = 1;

	lastSong = currentJukeboxSong;

	/*JE_fadeBlack(10);
	JE_initVGA256();
	JE_fadeColor(10);*/

	JE_initVGA256();
	JE_starlib_init();

	quit = FALSE;
	fade = FALSE;
	repeatedFade = FALSE;

	tempVolume = tyrMusicVolume;
	youStopped = FALSE;

	JE_wipeKey();

	do
	{
		tempScreenSeg = VGAScreen; /*sega000*/

		if (weirdMusic) /* TODO: Not sure what this is about, figure it out */
		{
			/*
			IF framecount2 = 0 THEN
			BEGIN
				framecount2 := weirdspeed;

				IF weirdcurrent THEN
					nortsong.setvol (tempvolume SHR 1, FXvolume)
				ELSE
					nortsong.setvol (tempvolume, FXvolume);

				weirdcurrent := NOT weirdcurrent;
			END;
			*/
		}

		if (repeated && !repeatedFade)
		{
		  fade = TRUE;
		  repeatedFade = TRUE;
		}

		if ( ( (repeated && !fade) || !playing) && !youStopped)
		{
		  currentSong = ( rand() % MUSIC_NUM );
		  JE_playNewSong();
		}

		setdelay(1);
		service_SDL_events(TRUE);

		JE_starlib_main();

		if (lastSong != currentSong)
		{
		  lastSong = currentSong;
		  JE_bar(50, 190, 250, 198, 0); /* vga256c.BAR (50, 190, 250, 198, 0); */
		}

		if (drawText)
		{
			/* TODO: Put in actual song titles here */
			if (fx)
			{
				sprintf(tempStr, "%d %s", fxNum, soundTitle[fxNum - 1]);
				JE_bar(50, 190, 250, 198, 0); /* vga256c.BAR (50, 190, 250, 198, 0); */
				JE_outText(JE_fontCenter(tempStr, TINY_FONT), 190, tempStr, 1, 4);
			} else {
				sprintf(tempStr, "%d %s", currentJukeboxSong, musicTitle[currentJukeboxSong - 1]);
				JE_outText(JE_fontCenter(tempStr, TINY_FONT), 190, tempStr, 1, 4);
			}
		}

		if (drawText)
		{
			tempScreenSeg = VGAScreen; /*sega000*/
			JE_outText(JE_fontCenter("Press ESC to quit the jukebox.", TINY_FONT), 170, "Press ESC to quit the jukebox.", 1, 0);
			tempScreenSeg = VGAScreen; /*sega000*/
			JE_outText(JE_fontCenter("Arrow keys change the song being played.", TINY_FONT), 180, "Arrow keys change the song being played.", 1, 0);
		}
		wait_delay();

		JE_showVGA();

		if (fade)
		{
			if (volumeActive)
			{
				if (tempVolume > 5)
				{
					tempVolume -= 2;
					JE_setVol(tempVolume, fxVolume);
				} else {
					fade = FALSE;
				}
			}
			else if (speed < 0xE000)
			{
				speed += 0x800;
			} else {
				speed = 0xE000;
				fade = FALSE;
			}
			JE_resetTimerInt();
			JE_setTimerInt();
		}

		/* TODO: Make this stuff work */
		/*
      joystick2;
      IF (mouseposition (x, y) > 0) OR button [1] THEN
        BEGIN
          quit := TRUE;
          wipekey;
        END;
      */

		JE_showVGA();

		if (newkey) {
			JE_newSpeed();
			switch (lastkey_sym)
			{
			case SDLK_ESCAPE: /* quit jukebox */
			case SDLK_q:
				quit = TRUE;
				break;
			case SDLK_r: /* restart song */
				JE_jukebox_selectSong(1);
				break;
			case SDLK_n: /* toggle continuous play */
				continuousPlay = !continuousPlay;
				break;
			case SDLK_v:
				volumeActive = !volumeActive;
				break;
			case SDLK_t: /* No idea what this is doing -- possibly resetting to default speed? */
				speed = 0x4300;
				JE_resetTimerInt();
				JE_setTimerInt();
				break;
			case SDLK_f:
				fade = !fade;
				break;
			case SDLK_COMMA: /* dec sound effect */
				fxNum = (fxNum - 1 < 1) ? SOUND_NUM + 9 : fxNum - 1;
				break;
			case SDLK_PERIOD: /* inc sound effect */
				fxNum = (fxNum + 1 > SOUND_NUM + 9) ? 1 : fxNum + 1;
				break;
			case SDLK_SLASH: /* switch to sfx mode */
				fx = !fx;
				break;
			case SDLK_SEMICOLON:
				JE_playSampleNum(fxNum);
				break;
			case SDLK_RETURN:
				currentJukeboxSong++;
				JE_playNewSong();
				youStopped = FALSE;
				break;
			case SDLK_s:
				JE_jukebox_selectSong(0);
				youStopped = TRUE;
				break;
			case SDLK_w:
				if (!weirdMusic)
				{
					weirdMusic = TRUE;
					weirdSpeed = 10;
				}
				else if (weirdSpeed > 1)
				{
					weirdSpeed--;
				}
				else
				{
					weirdMusic = FALSE;
					if (!fade)
					{
						JE_setVol(tempVolume, fxVolume);
					}
				}
				break;
			case SDLK_SPACE:
				drawText = !drawText;
				if (!drawText)
				{
					JE_bar(30, 170, 270, 198, 0);
				}
				break;
			case SDLK_LEFT:
			case SDLK_UP:
				currentJukeboxSong--;
				JE_playNewSong();
				youStopped = FALSE;
				break;
			case SDLK_RIGHT:
			case SDLK_DOWN:
				currentJukeboxSong++;
				JE_playNewSong();
				youStopped = FALSE;
				break;
			default:
				break;
			}
		}
	} while (!quit);

	JE_fadeBlack(10);
	JE_setVol(255, fxVolume);
}

void JE_newSpeed( void )
{
	/* TODO: Figure out what to do with this function. The timing system has changed a lot. */
		/*
          vga256d.speed := $5300 - starlib.speed * $800;
          IF starlib.speed > 5 THEN
            vga256d.speed := $1000;
          resettimerint;
          nortsong.settimerint;
        */
}

void JE_playNewSong( void )
{
	currentJukeboxSong = (currentJukeboxSong > MUSIC_NUM) ? 1 : (currentJukeboxSong < 1) ? MUSIC_NUM : currentJukeboxSong;
	JE_playSong(currentJukeboxSong);
	playing = TRUE;
	repeatedFade = FALSE;
	tempVolume = tyrMusicVolume;
	JE_setVol(tempVolume, fxVolume);
}



/* TODO */
