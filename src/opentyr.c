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
#include "config.h"
#include "editship.h"
#include "episodes.h"
#include "file.h"
#include "fonthand.h"
#include "helptext.h"
#include "joystick.h"
#include "jukebox.h"
#include "keyboard.h"
#include "loudness.h"
#include "mainint.h"
#include "mtrand.h"
#include "musmast.h"
#include "network.h"
#include "newshape.h"
#include "nortsong.h"
#include "nortvars.h"
#include "opentyr.h"
#include "params.h"
#include "picload.h"
#include "scroller.h"
#include "setup.h"
#include "tyrian2.h"
#include "xmas.h"
#include "varz.h"
#include "vga256d.h"
#include "video.h"
#include "video_scale.h"

#include "SDL.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


const JE_byte shapereorderlist[7] = {1, 2, 5, 0, 3, 4, 6};

const char *opentyrian_str = "OpenTyrian",
           *opentyrian_version = "Classic revision " SVN_REV;
const char *opentyrian_menu_items[] =
{
	"About OpenTyrian",
	"Toggle Fullscreen",
	"Scaler: None",
	/* "Play Destruct", */
	"Jukebox",
	"Return to Main Menu"
};

/* zero-terminated strncpy */
char *strnztcpy( char *to, const char *from, size_t count )
{
	to[count] = '\0';
	return strncpy(to, from, count);
}

void opentyrian_menu( void )
{
	int sel = 0;
	const int maxSel = COUNTOF(opentyrian_menu_items) - 1;
	bool quit = false, fade_in = true;
	
	int temp_scaler = scaler;
	char buffer[100];
	
	JE_fadeBlack(10);
	JE_loadPic(13, false);

	JE_outTextAdjust(JE_fontCenter(opentyrian_str, FONT_SHAPES), 5, opentyrian_str, 15, -3, FONT_SHAPES, false);

	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

	JE_showVGA();

	play_song(36); // A Field for Mag
	
	do
	{
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
		
		for (int i = 0; i <= maxSel; i++)
		{
			const char *text = opentyrian_menu_items[i];
			
			if (i == 2) /* Scaler */
			{
				snprintf(buffer, sizeof(buffer), "Scaler: %s", scalers[temp_scaler].name);
				text = buffer;
			}
			
			JE_outTextAdjust(JE_fontCenter(text, SMALL_FONT_SHAPES),
			                 (i != maxSel) ? (i * 16 + 32) : 118, text,
			                 15, (i != sel ? -4 : -2), SMALL_FONT_SHAPES, true);
		}
		
		JE_showVGA();
		
		if (fade_in)
		{
			fade_in = false;
			JE_fadeColor(20);
			wait_noinput(true, false, false);
		}

		tempW = 0;
		JE_textMenuWait(&tempW, false);

		if (newkey)
		{
			switch (lastkey_sym)
			{
				case SDLK_UP:
					sel--;
					if (sel < 0)
					{
						sel = maxSel;
					}
					JE_playSampleNum(S_CURSOR);
					break;
				case SDLK_DOWN:
					sel++;
					if (sel > maxSel)
					{
						sel = 0;
					}
					JE_playSampleNum(S_CURSOR);
					break;
				case SDLK_LEFT:
					if (sel == 2)
					{
						do {
							if (temp_scaler == 0)
								temp_scaler = COUNTOF(scalers);
							temp_scaler--;
						} while ((display_surface->format->BitsPerPixel == 32 && scalers[temp_scaler].scaler32 == NULL) ||
						         (display_surface->format->BitsPerPixel == 16 && scalers[temp_scaler].scaler16 == NULL));
						if (display_surface->format->BitsPerPixel == 8)
							temp_scaler = 0;
						else
							JE_playSampleNum(S_CURSOR);
					}
					break;
				case SDLK_RIGHT:
					if (sel == 2)
					{
						do {
							temp_scaler++;
							if (temp_scaler == COUNTOF(scalers))
								temp_scaler = 0;
						} while ((display_surface->format->BitsPerPixel == 32 && scalers[temp_scaler].scaler32 == NULL) ||
						         (display_surface->format->BitsPerPixel == 16 && scalers[temp_scaler].scaler16 == NULL));
						if (display_surface->format->BitsPerPixel == 8)
							temp_scaler = 0;
						else
							JE_playSampleNum(S_CURSOR);
					}
					break;
				case SDLK_RETURN:
					switch (sel)
					{
						case 0: /* About */
							JE_playSampleNum(S_SELECT);
							
							scroller_sine(about_text);
							
							memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
							JE_showVGA();
							fade_in = true;
							break;
						case 1: /* Fullscreen */
							JE_playSampleNum(S_SELECT);
							
							fullscreen_enabled = !fullscreen_enabled;
							reinit_video();
							break;
						case 2: /* Scaler */
							JE_playSampleNum(S_SELECT);
							
							if (scaler != temp_scaler)
							{
								scaler = temp_scaler;
								reinit_video();
							}
							break;
						case 3: /* Jukebox */
							JE_playSampleNum(S_SELECT);
							
							JE_fadeBlack(10);
							jukebox();
							
							memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
							JE_showVGA();
							fade_in = true;
							break;
						default: /* Return to main menu */
							quit = true;
							JE_playSampleNum(S_SPRING);
							break;
					}
					break;
				case SDLK_ESCAPE:
					quit = true;
					JE_playSampleNum(S_SPRING);
					break;
				default:
					break;
			}
		}
	} while (!quit);
}

int main( int argc, char *argv[] )
{
	mt_srand(time(NULL));

	printf("\nWelcome to... >> %s %s <<\n\n", opentyrian_str, opentyrian_version);

	printf("Copyright (C) 2007-2009 The OpenTyrian Development Team\n\n");

	printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	printf("This is free software, and you are welcome to redistribute it\n");
	printf("under certain conditions.  See the file GPL.txt for details.\n\n");
	
	if (SDL_Init(0))
	{
		printf("Failed to initialize SDL: %s\n", SDL_GetError());
		return -1;
	}
	
	JE_loadConfiguration();

	JE_paramCheck(argc, argv);

	JE_scanForEpisodes();

	init_video();
	init_keyboard();
	init_joysticks();
	printf("assuming mouse detected\n"); // SDL can't tell us if there isn't one
	
	xmas |= xmas_time();
	if (xmas && (!dir_file_exists(data_dir(), "tyrianc.shp") || !dir_file_exists(data_dir(), "voicesc.snd")))
	{
		xmas = false;
		
		fprintf(stderr, "warning: Christmas is missing.\n");
	}
	
	JE_loadPals();
	JE_loadMainShapeTables(xmas ? "tyrianc.shp" : "tyrian.shp");
	
	tempScreenSeg = VGAScreen;
	if (xmas && !xmas_prompt())
	{
		xmas = false;
		
		free_main_shape_tables();
		JE_loadMainShapeTables("tyrian.shp");
	}
	
	
	/* Default Options */
	youAreCheating = false;
	smoothScroll = true;
	loadDestruct = false;
	
	if (!audio_disabled)
	{
		printf("initializing SDL audio...\n");
		
		init_audio();
		
		load_music();
		
		JE_loadSndFile("tyrian.snd", xmas ? "voicesc.snd" : "voices.snd");
	}
	else
	{
		printf("audio disabled\n");
	}
	
	if (record_demo)
		printf("demo recording enabled (input limited to keyboard)\n");

	megaData1 = malloc(sizeof(*megaData1));
	megaData2 = malloc(sizeof(*megaData2));
	megaData3 = malloc(sizeof(*megaData3));

	JE_loadExtraShapes();  /*Editship*/

	JE_loadHelpText();
	/*debuginfo("Help text complete");*/
	
	if (isNetworkGame)
	{
		if (network_init())
		{
			network_tyrian_halt(3, false);
		}
	}
	
	JE_main();
	
	// typically we don't get here, see JE_tyrianHalt()
	deinit_video();
	deinit_joysticks();
	
	return 0;
}

// kate: tab-width 4; vim: set noet:
