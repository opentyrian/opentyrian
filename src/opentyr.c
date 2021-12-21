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
#include "opentyr.h"

#include "config.h"
#include "destruct.h"
#include "editship.h"
#include "episodes.h"
#include "file.h"
#include "font.h"
#include "helptext.h"
#include "joystick.h"
#include "jukebox.h"
#include "keyboard.h"
#include "loudness.h"
#include "mainint.h"
#include "mtrand.h"
#include "musmast.h"
#include "network.h"
#include "nortsong.h"
#include "opentyrian_version.h"
#include "params.h"
#include "picload.h"
#include "scroller.h"
#include "setup.h"
#include "sprite.h"
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
#include <string.h>
#include <time.h>

const char *opentyrian_str = "OpenTyrian";
const char *opentyrian_version = OPENTYRIAN_VERSION;

void opentyrian_menu( void )
{
	typedef enum
	{
		MENU_ABOUT = 0,
		MENU_FULLSCREEN,
		MENU_SCALER,
		MENU_SCALING_MODE,
		// MENU_DESTRUCT,
		MENU_JUKEBOX,
		MENU_RETURN,
		MenuOptions_MAX
	} MenuOptions;

	static const char *menu_items[] =
	{
		"About OpenTyrian",
		NULL,  // "Windowed" or "Fullscreen: Display %d"
		NULL,  // "Scaler: %s"
		NULL,  // "Scaling Mode: %s"
		// "Play Destruct",
		"Jukebox",
		"Return to Main Menu",
	};
	bool menu_items_disabled[] =
	{
		false,
		false,
		false,
		false,
		// false,
		false,
		false,
	};
	
	assert(COUNTOF(menu_items) == MenuOptions_MAX);
	assert(COUNTOF(menu_items_disabled) == MenuOptions_MAX);

	fade_black(10);
	JE_loadPic(VGAScreen, 13, false);

	draw_font_hv(VGAScreen, VGAScreen->w / 2, 5, opentyrian_str, large_font, centered, 15, -3);

	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

	JE_showVGA();

	play_song(36); // A Field for Mag

	MenuOptions sel = 0;

	int temp_fullscreen_display = fullscreen_display;
	uint temp_scaler = scaler;
	int temp_scaling_mode = scaling_mode;

	bool fade_in = true, quit = false;
	do
	{
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);

		for (MenuOptions i = 0; i < MenuOptions_MAX; i++)
		{
			const char *text = menu_items[i];
			char buffer[100];

			if (i == MENU_FULLSCREEN)
			{
				if (temp_fullscreen_display == -1)
				{
					text = "Windowed";
				}
				else
				{
					snprintf(buffer, sizeof(buffer), "Fullscreen: Display %d", temp_fullscreen_display + 1);
					text = buffer;
				}
			}
			else if (i == MENU_SCALER)
			{
				snprintf(buffer, sizeof(buffer), "Scaler: %s", scalers[temp_scaler].name);
				text = buffer;
			}
			else if (i == MENU_SCALING_MODE)
			{
				snprintf(buffer, sizeof(buffer), "Scaling Mode: %s", scaling_mode_names[temp_scaling_mode]);
				text = buffer;
			}

			int y = i != MENU_RETURN ? i * 16 + 32 : 118;
			draw_font_hv(VGAScreen, VGAScreen->w / 2, y, text, normal_font, centered, 15, menu_items_disabled[i] ? -8 : i != sel ? -4 : -2);
		}

		if (sel == MENU_FULLSCREEN || sel == MENU_SCALER || sel == MENU_SCALING_MODE) {
			draw_font_hv_shadow(VGAScreen, VGAScreen->w / 2, 190, "Change option with Left/Right keys then press Enter.",
					small_font, centered, 15, 2, true, 1);
		}

		JE_showVGA();

		if (fade_in)
		{
			fade_in = false;
			fade_palette(colors, 20, 0, 255);
			wait_noinput(true, false, false);
		}

		tempW = 0;
		JE_textMenuWait(&tempW, false);

		if (newkey)
		{
			switch (lastkey_scan)
			{
			case SDL_SCANCODE_UP:
				do
				{
					if (sel-- == 0)
						sel = MenuOptions_MAX - 1;
				}
				while (menu_items_disabled[sel]);
				
				JE_playSampleNum(S_CURSOR);
				break;
			case SDL_SCANCODE_DOWN:
				do
				{
					if (++sel >= MenuOptions_MAX)
						sel = 0;
				}
				while (menu_items_disabled[sel]);
				
				JE_playSampleNum(S_CURSOR);
				break;
				
			case SDL_SCANCODE_LEFT:
				if (sel == MENU_FULLSCREEN)
				{
					if (temp_fullscreen_display == -1)
						temp_fullscreen_display = SDL_GetNumVideoDisplays();
					temp_fullscreen_display--;

					JE_playSampleNum(S_CURSOR);
				}
				else if (sel == MENU_SCALER)
				{
					if (temp_scaler == 0)
						temp_scaler = scalers_count;
					temp_scaler--;
					
					JE_playSampleNum(S_CURSOR);
				}
				else if (sel == MENU_SCALING_MODE)
				{
					if (temp_scaling_mode == 0)
						temp_scaling_mode = ScalingMode_MAX;
					temp_scaling_mode--;
					
					JE_playSampleNum(S_CURSOR);
				}
				break;
			case SDL_SCANCODE_RIGHT:
				if (sel == MENU_FULLSCREEN)
				{
					temp_fullscreen_display++;
					if (temp_fullscreen_display == SDL_GetNumVideoDisplays())
						temp_fullscreen_display = -1;

					JE_playSampleNum(S_CURSOR);
				}
				else if (sel == MENU_SCALER)
				{
					temp_scaler++;
					if (temp_scaler == scalers_count)
						temp_scaler = 0;
					
					JE_playSampleNum(S_CURSOR);
				}
				else if (sel == MENU_SCALING_MODE)
				{
					temp_scaling_mode++;
					if (temp_scaling_mode == ScalingMode_MAX)
						temp_scaling_mode = 0;
					
					JE_playSampleNum(S_CURSOR);
				}
				break;
				
			case SDL_SCANCODE_RETURN:
				switch (sel)
				{
				case MENU_ABOUT:
					JE_playSampleNum(S_SELECT);

					scroller_sine(about_text);

					memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
					JE_showVGA();
					fade_in = true;
					break;
					
				case MENU_FULLSCREEN:
					JE_playSampleNum(S_SELECT);

					reinit_fullscreen(temp_fullscreen_display);
					break;
					
				case MENU_SCALER:
					JE_playSampleNum(S_SELECT);

					if (scaler != temp_scaler)
					{
						if (!init_scaler(temp_scaler) &&   // try new scaler
							!init_scaler(scaler))          // revert on fail
						{
							exit(EXIT_FAILURE);
						}
					}
					break;
					
				case MENU_SCALING_MODE:
					JE_playSampleNum(S_SELECT);
					scaling_mode = temp_scaling_mode;
					break;

				case MENU_JUKEBOX:
					JE_playSampleNum(S_SELECT);

					fade_black(10);
					jukebox();

					memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
					JE_showVGA();
					fade_in = true;
					break;
					
				case MENU_RETURN:
					quit = true;
					JE_playSampleNum(S_SPRING);
					break;
					
				case MenuOptions_MAX:
					assert(false);
					break;
				}
				break;
				
			case SDL_SCANCODE_ESCAPE:
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

	printf("Copyright (C) 2007-2013 The OpenTyrian Development Team\n\n");

	printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	printf("This is free software, and you are welcome to redistribute it\n");
	printf("under certain conditions.  See the file COPYING for details.\n\n");

	if (SDL_Init(0))
	{
		printf("Failed to initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	JE_loadConfiguration();

	xmas = xmas_time();  // arg handler may override

	JE_paramCheck(argc, argv);

	JE_scanForEpisodes();

	init_video();
	init_keyboard();
	init_joysticks();
	printf("assuming mouse detected\n"); // SDL can't tell us if there isn't one

	if (xmas && (!dir_file_exists(data_dir(), "tyrianc.shp") || !dir_file_exists(data_dir(), "voicesc.snd")))
	{
		xmas = false;

		fprintf(stderr, "warning: Christmas is missing.\n");
	}

	JE_loadPals();
	JE_loadMainShapeTables(xmas ? "tyrianc.shp" : "tyrian.shp");

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

	JE_loadExtraShapes();  /*Editship*/

	JE_loadHelpText();
	/*debuginfo("Help text complete");*/

	if (isNetworkGame)
	{
#ifdef WITH_NETWORK
		if (network_init())
		{
			network_tyrian_halt(3, false);
		}
#else
		fprintf(stderr, "OpenTyrian was compiled without networking support.");
		JE_tyrianHalt(5);
#endif
	}

#ifdef NDEBUG
	if (!isNetworkGame)
		intro_logos();
#endif

	for (; ; )
	{
		JE_initPlayerData();
		JE_sortHighScores();

		if (JE_titleScreen(true))
			break;  // user quit from title screen

		if (loadDestruct)
		{
			JE_destructGame();
			loadDestruct = false;
		}
		else
		{
			JE_main();
		}
	}

	JE_tyrianHalt(0);

	return 0;
}

