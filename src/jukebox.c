/* 
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
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
#include "opentyr.h"
#include "jukebox.h"

#include "fonthand.h"
#include "joystick.h"
#include "keyboard.h"
#include "lds_play.h"
#include "loudness.h"
#include "mtrand.h"
#include "newshape.h"
#include "nortsong.h"
#include "palette.h"
#include "starlib.h"
#include "vga256d.h"
#include "video.h"

void jukebox( void )
{
	bool hide_text = false, quit = false;
	
	bool fade_looped_songs = true, fading = false;
	bool fx = false, stopped = false;
	
	int fx_num = 0;
	
	SDL_FillRect(VGAScreenSeg, NULL, 0);
	JE_showVGA();
	JE_updateColorsFast(vga_palette); //JE_fadeColor(10);
	
	JE_starlib_init();
	
	int fade_volume = tyrMusicVolume;
	
	while (!quit)
	{
		if (!stopped && !audio_disabled)
		{
			if (songlooped && fade_looped_songs)
				fading = true;
			
			if (fading)
			{
				if (fade_volume > 5)
				{
					fade_volume -= 2;
				}
				else
				{
					fade_volume = tyrMusicVolume;
					
					fading = false;
				}
				
				set_volume(fade_volume, fxVolume);
			}
			
			if (!playing || (songlooped && fade_looped_songs && !fading))
				play_song(mt_rand() % MUSIC_NUM);
		}
		
		setdelay(1);
		
		push_joysticks_as_keyboard();
		service_SDL_events(true);
		
		SDL_FillRect(VGAScreenSeg, NULL, 0);
		
		// starlib input needs to be rewritten
		JE_starlib_main();
		
		if (!hide_text)
		{
			char tempStr[60];
			
			tempScreenSeg = VGAScreenSeg;
			
			if (fx)
				sprintf(tempStr, "%d %s", fx_num + 1, soundTitle[fx_num]);
			else
				sprintf(tempStr, "%d %s", song_playing + 1, musicTitle[song_playing]);
			JE_outText(JE_fontCenter(tempStr, TINY_FONT), 190, tempStr, 1, 4);
			
			strcpy(tempStr, "Press ESC to quit the jukebox.");
			JE_outText(JE_fontCenter(tempStr, TINY_FONT), 170, tempStr, 1, 0);
			
			strcpy(tempStr, "Arrow keys change the song being played.");
			JE_outText(JE_fontCenter(tempStr, TINY_FONT), 180, tempStr, 1, 0);
		}
		
		JE_showVGA();
		
		wait_delay();
		
		// quit on mouse click
		Uint16 x, y;
		if (JE_mousePosition(&x, &y) > 0)
			quit = true;
		
		if (newkey)
		{
			switch (lastkey_sym)
			{
			case SDLK_ESCAPE: // quit jukebox
			case SDLK_q:
				quit = true;
				break;
			case SDLK_SPACE:
				hide_text = !hide_text;
				break;
				
			case SDLK_f:
				fading = !fading;
				break;
			case SDLK_n:
				fade_looped_songs = !fade_looped_songs;
				break;
			
			case SDLK_SLASH: // switch to sfx mode
				fx = !fx;
				break;
			case SDLK_COMMA:
				if (fx && --fx_num < 0)
					fx_num = SOUND_NUM + 9 - 1;
				break;
			case SDLK_PERIOD:
				if (fx && ++fx_num >= SOUND_NUM + 9)
					fx_num = 0;
				break;
			case SDLK_SEMICOLON:
				if (fx)
					JE_playSampleNum(fx_num + 1);
				break;
				
			case SDLK_LEFT:
			case SDLK_UP:
				play_song((song_playing > 0 ? song_playing : MUSIC_NUM) - 1);
				stopped = false;
				break;
			case SDLK_RETURN:
			case SDLK_RIGHT:
			case SDLK_DOWN:
				play_song((song_playing + 1) % MUSIC_NUM);
				stopped = false;
				break;
			case SDLK_s: // stop song
				stop_song();
				stopped = true;
				break;
			case SDLK_r: // restart song
				restart_song();
				stopped = false;
				break;
				
			default:
				break;
			}
		}
	}
	
	set_volume(tyrMusicVolume, fxVolume);
	
	JE_updateColorsFast(black); //JE_fadeBlack(10);
}

// kate: tab-width 4; vim: set noet:
