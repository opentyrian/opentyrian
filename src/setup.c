/* 
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
#include "setup.h"

#include "config.h"
#include "fonthand.h"
#include "joystick.h"
#include "jukebox.h"
#include "keyboard.h"
#include "loudness.h"
#include "mainint.h"
#include "mtrand.h"
#include "network.h"
#include "newshape.h"
#include "nortsong.h"
#include "nortvars.h"
#include "palette.h"
#include "params.h"
#include "starlib.h"
#include "varz.h"
#include "vga256d.h"
#include "video.h"

#include "SDL.h"


JE_boolean volumeActive = true;
JE_boolean fx;
JE_word fxNum;
JE_boolean repeatedFade, continuousPlay;

void JE_textMenuWait( JE_word *waitTime, JE_boolean doGamma )
{
	set_mouse_position(160, 100);
	
	do
	{
		JE_showVGA();
		
		push_joysticks_as_keyboard();
		service_SDL_events(true);
		
		inputDetected = newkey | mousedown;
		
		if (lastkey_sym == SDLK_SPACE)
		{
			lastkey_sym = SDLK_RETURN;
		}
		
		if (mousedown)
		{
			newkey = true;
			lastkey_sym = SDLK_RETURN;
		}
		
		if (mouseInstalled && input_grabbed)
		{
			if (abs(mouse_y - 100) > 10)
			{
				inputDetected = true;
				if (mouse_y - 100 < 0)
				{
					lastkey_sym = SDLK_UP;
				} else {
					lastkey_sym = SDLK_DOWN;
				}
				newkey = true;
			}
			if (abs(mouse_x - 160) > 10)
			{
				inputDetected = true;
				if (mouse_x - 160 < 0)
				{
					lastkey_sym = SDLK_LEFT;
				} else {
					lastkey_sym = SDLK_RIGHT;
				}
				newkey = true;
			}
		}
		
		NETWORK_KEEP_ALIVE();
		
		SDL_Delay(16);
		
		if (*waitTime > 0)
		{
			(*waitTime)--;
		}
	} while (!(inputDetected || *waitTime == 1 || haltGame));
}


void JE_jukeboxGo( void )
{
	JE_boolean weirdMusic, weirdCurrent;
	JE_byte weirdSpeed = 0;
	char tempStr[64];

	JE_byte lastSong;
	JE_byte tempVolume;
	JE_boolean youStopped, drawText, quit, fade;


	weirdMusic = false;
	weirdCurrent = true;
	drawText = true;

	fx = false;
	fxNum = 1;

	lastSong = currentJukeboxSong;

	JE_fadeBlack(10);
	SDL_FillRect(VGAScreenSeg, NULL, 0x0);
	JE_showVGA();
	JE_updateColorsFast(vga_palette); //JE_fadeColor(10);

	JE_starlib_init();

	quit = false;
	fade = false;
	repeatedFade = false;

	tempVolume = tyrMusicVolume;
	youStopped = false;

	JE_wipeKey();

	do
	{
		tempScreenSeg = VGAScreenSeg;

		if (weirdMusic)
		{
			if (delaycount2() == 0)
			{
				setjasondelay2(weirdSpeed);
				
				if (weirdCurrent)
				{
					JE_setVol(tempVolume / 2, fxVolume);
				} else {
					JE_setVol(tempVolume, fxVolume);
				}
				
				weirdCurrent = !weirdCurrent;
			}
		}

		if (repeated && !repeatedFade)
		{
			fade = true;
			repeatedFade = true;
		}

		if ( ( (repeated && !fade) || !playing) && !youStopped)
		{
			currentJukeboxSong = ( mt_rand() % MUSIC_NUM );
			JE_playNewSong();
		}

		setdelay(1);
		
		push_joysticks_as_keyboard();
		service_SDL_events(true);

		JE_starlib_main();

		if (lastSong != currentJukeboxSong)
		{
			lastSong = currentJukeboxSong;
			JE_bar(50, 190, 250, 198, 0);
		}

		if (drawText)
		{
			tempScreenSeg = VGAScreenSeg;
			if (fx)
			{
				sprintf(tempStr, "%d %s", fxNum, soundTitle[fxNum - 1]);
				JE_bar(50, 190, 250, 198, 0);
				JE_outText(JE_fontCenter(tempStr, TINY_FONT), 190, tempStr, 1, 4);
			} else {
				sprintf(tempStr, "%d %s", currentJukeboxSong, musicTitle[currentJukeboxSong - 1]);
				JE_outText(JE_fontCenter(tempStr, TINY_FONT), 190, tempStr, 1, 4);
			}

			tempScreenSeg = VGAScreenSeg;
			JE_outText(JE_fontCenter("Press ESC to quit the jukebox.", TINY_FONT), 170, "Press ESC to quit the jukebox.", 1, 0);
			tempScreenSeg = VGAScreenSeg;
			JE_outText(JE_fontCenter("Arrow keys change the song being played.", TINY_FONT), 180, "Arrow keys change the song being played.", 1, 0);
		}

		JE_showVGA();
		
		wait_delay();

		if (fade)
		{
			if (volumeActive)
			{
				if (tempVolume > 5)
				{
					tempVolume -= 2;
					JE_setVol(tempVolume, fxVolume);
				} else {
					fade = false;
				}
			}
			else if (speed < 0xE000)
			{
				speed += 0x800;
			} else {
				speed = 0xE000;
				fade = false;
			}
			JE_resetTimerInt();
			JE_setTimerInt();
		}

		if (JE_mousePosition(&x, &y) > 0 || button[0])
		{
			quit = true;
			JE_wipeKey();
		}

		if (newkey)
		{
			JE_newSpeed();
			switch (lastkey_sym)
			{
			case SDLK_ESCAPE: /* quit jukebox */
			case SDLK_q:
				quit = true;
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
				youStopped = false;
				break;
			case SDLK_s:
				JE_jukebox_selectSong(0);
				youStopped = true;
				break;
			case SDLK_w:
				if (!weirdMusic)
				{
					weirdMusic = true;
					weirdSpeed = 10;
				}
				else if (weirdSpeed > 1)
				{
					weirdSpeed--;
				}
				else
				{
					weirdMusic = false;
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
				youStopped = false;
				break;
			case SDLK_RIGHT:
			case SDLK_DOWN:
				currentJukeboxSong++;
				JE_playNewSong();
				youStopped = false;
				break;
			default:
				break;
			}
		}
	} while (!quit);
	
	JE_updateColorsFast(black); //JE_fadeBlack(10);
	JE_setVol(255, fxVolume);
}

void JE_newSpeed( void )
{
	/* deprecated; changed music tempo to match star speed */
}

void JE_playNewSong( void )
{
	currentJukeboxSong = (currentJukeboxSong > MUSIC_NUM) ? 1 : (currentJukeboxSong < 1) ? MUSIC_NUM : currentJukeboxSong;
	JE_playSong(currentJukeboxSong);
	playing = true;
	repeatedFade = false;
	tempVolume = tyrMusicVolume;
	JE_setVol(tempVolume, fxVolume);
}

// kate: tab-width 4; vim: set noet:
