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
#include "fonthand.h"
#include "helptext.h"
#include "joystick.h"
#include "jukebox.h"
#include "keyboard.h"
#include "loudness.h"
#include "mainint.h"
#include "mouse.h"
#include "mtrand.h"
#include "network.h"
#include "nortsong.h"
#include "nortvars.h"
#include "opentyrian_version.h"
#include "palette.h"
#include "params.h"
#include "picload.h"
#include "sprite.h"
#include "tyrian2.h"
#include "varz.h"
#include "vga256d.h"
#include "video.h"
#include "video_scale.h"
#include "xmas.h"

#include "SDL.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *opentyrian_str = "OpenTyrian";
const char *opentyrian_version = OPENTYRIAN_VERSION;

static size_t getDisplayPickerItemsCount(void)
{
	return 1 + (size_t)SDL_GetNumVideoDisplays();
}

static const char *getDisplayPickerItem(size_t i, char *buffer, size_t bufferSize)
{
	if (i == 0)
		return "Window";

	snprintf(buffer, bufferSize, "Display %d", (int)i);
	return buffer;
}

static size_t getScalerPickerItemsCount(void)
{
	return (size_t)scalers_count;
}

static const char *getScalerPickerItem(size_t i, char *buffer, size_t bufferSize)
{
	(void)buffer, (void)bufferSize;

	return scalers[i].name;
}

static size_t getScalingModePickerItemsCount(void)
{
	return (size_t)ScalingMode_MAX;
}

static const char *getScalingModePickerItem(size_t i, char *buffer, size_t bufferSize)
{
	(void)buffer, (void)bufferSize;

	return scaling_mode_names[i];
}

void setupMenu(void)
{
	typedef enum
	{
		MENU_ITEM_NONE = 0,
		MENU_ITEM_DONE,
		MENU_ITEM_GRAPHICS,
		MENU_ITEM_SOUND,
		MENU_ITEM_JUKEBOX,
		MENU_ITEM_DESTRUCT,
		MENU_ITEM_DISPLAY,
		MENU_ITEM_SCALER,
		MENU_ITEM_SCALING_MODE,
		MENU_ITEM_MUSIC_VOLUME,
		MENU_ITEM_SOUND_VOLUME,
	} MenuItemId;

	typedef enum
	{
		MENU_NONE = 0,
		MENU_SETUP,
		MENU_GRAPHICS,
		MENU_SOUND,
	} MenuId;

	typedef struct
	{
		MenuItemId id;
		const char *name;
		const char *description;
		size_t (*getPickerItemsCount)(void);
		const char *(*getPickerItem)(size_t i, char *buffer, size_t bufferSize);
	} MenuItem;

	typedef struct
	{
		const char *header;
		const MenuItem items[6];
	} Menu;

	static const Menu menus[] = {
		[MENU_SETUP] = {
			.header = "Setup",
			.items = {
				{ MENU_ITEM_GRAPHICS, "Graphics...", "Change the graphics settings." },
				{ MENU_ITEM_SOUND, "Sound...", "Change the sound settings." },
				{ MENU_ITEM_JUKEBOX, "Jukebox", "Listen to the music of Tyrian." },
				// { MENU_ITEM_DESTRUCT, "Destruct", "Play a bonus mini-game." },
				{ MENU_ITEM_DONE, "Done", "Return to the main menu." },
				{ -1 }
			},
		},
		[MENU_GRAPHICS] = {
			.header = "Graphics",
			.items = {
				{ MENU_ITEM_DISPLAY, "Display:", "Change the display mode.", getDisplayPickerItemsCount, getDisplayPickerItem },
				{ MENU_ITEM_SCALER, "Scaler:", "Change the pixel art scaling algorithm.", getScalerPickerItemsCount, getScalerPickerItem },
				{ MENU_ITEM_SCALING_MODE, "Scaling Mode:", "Change the scaling mode.", getScalingModePickerItemsCount, getScalingModePickerItem },
				{ MENU_ITEM_DONE, "Done", "Return to the previous menu." },
				{ -1 }
			},
		},
		[MENU_SOUND] = {
			.header = "Sound",
			.items = {
				{ MENU_ITEM_MUSIC_VOLUME, "Music Volume", "Change volume with the left/right arrow keys." },
				{ MENU_ITEM_SOUND_VOLUME, "Sound Volume", "Change volume with the left/right arrow keys." },
				{ MENU_ITEM_DONE, "Done", "Return to the previous menu." },
				{ -1 }
			},
		},
	};

	char buffer[100];

	if (shopSpriteSheet.data == NULL)
		JE_loadCompShapes(&shopSpriteSheet, '1');  // need mouse pointer sprites

	bool restart = true;

	MenuId menuParents[COUNTOF(menus)] = { MENU_NONE };
	size_t selectedMenuItemIndexes[COUNTOF(menus)] = { 0 };
	MenuId currentMenu = MENU_SETUP;
	MenuItemId currentPicker = MENU_ITEM_NONE;
	size_t pickerSelectedIndex = 0;

	const int xCenter = 320 / 2;
	const int yMenuHeader = 4;
	const int xMenuItem = 45;
	const int xMenuItemName = xMenuItem;
	const int wMenuItemName = 135;
	const int xMenuItemValue = xMenuItemName + wMenuItemName;
	const int wMenuItemValue = 95;
	const int wMenuItem = wMenuItemName + wMenuItemValue;
	const int yMenuItems = 37;
	const int dyMenuItems = 21;
	const int hMenuItem = 13;

	for (; ; )
	{
		if (restart)
		{
			JE_loadPic(VGAScreen2, 2, false);
			fill_rectangle_wh(VGAScreen2, 0, 192, 320, 8, 0);
		}

		// Restore background.
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, (size_t)VGAScreen->pitch * VGAScreen->h);

		const Menu *menu = &menus[currentMenu];

		// Draw header.
		draw_font_hv_shadow(VGAScreen, xCenter, yMenuHeader, menu->header, large_font, centered, 15, -3, false, 2);

		int yPicker = 0;
		const int dyPickerItem = 15;
		const int dyPickerItemPadding = 2;
		const int hPickerItem = dyPickerItem - dyPickerItemPadding;

		size_t *const selectedMenuItemIndex = &selectedMenuItemIndexes[currentMenu];
		const MenuItem *const menuItems = menu->items;

		// Draw menu items.

		size_t menuItemsCount = 0;
		for (size_t i = 0; menuItems[i].id != (MenuItemId)-1; ++i)
		{
			menuItemsCount += 1;

			const MenuItem *const menuItem = &menuItems[i];

			const int y = yMenuItems + dyMenuItems * i;

			const bool selected = i == *selectedMenuItemIndex;
			const bool disabled = currentPicker != MENU_ITEM_NONE && !selected;

			if (selected)
				yPicker = y;

			const char *const name = menuItem->name;

			draw_font_hv_shadow(VGAScreen, xMenuItemName, y, name, normal_font, left_aligned, 15, -3 + (selected ? 2 : 0) + (disabled ? -4 : 0), false, 2);

			switch (menuItem->id)
			{
			case MENU_ITEM_DISPLAY:;
				const char *value = "Window";
				if (fullscreen_display >= 0)
				{
					snprintf(buffer, sizeof(buffer), "Display %d", fullscreen_display + 1);
					value = buffer;
				}

				draw_font_hv_shadow(VGAScreen, xMenuItemValue, y, value, normal_font, left_aligned, 15, -3 + (selected ? 2 : 0) + (disabled ? -4 : 0), false, 2);
				break;

			case MENU_ITEM_SCALER:
				draw_font_hv_shadow(VGAScreen, xMenuItemValue, y, scalers[scaler].name, normal_font, left_aligned, 15, -3 + (selected ? 2 : 0) + (disabled ? -4 : 0), false, 2);
				break;

			case MENU_ITEM_SCALING_MODE:
				draw_font_hv_shadow(VGAScreen, xMenuItemValue, y, scaling_mode_names[scaling_mode], normal_font, left_aligned, 15, -3 + (selected ? 2 : 0) + (disabled ? -4 : 0), false, 2);
				break;

			case MENU_ITEM_MUSIC_VOLUME:
				JE_barDrawShadow(VGAScreen, xMenuItemValue, y, 1, music_disabled ? 170 : 174, (tyrMusicVolume + 4) / 8, 2, 10);
				JE_rectangle(VGAScreen, xMenuItemValue - 2, y - 2, xMenuItemValue + 96, y + 11, 242);
				break;

			case MENU_ITEM_SOUND_VOLUME:
				JE_barDrawShadow(VGAScreen, xMenuItemValue, y, 1, samples_disabled ? 170 : 174, (fxVolume + 4) / 8, 2, 10);
				JE_rectangle(VGAScreen, xMenuItemValue - 2, y - 2, xMenuItemValue + 96, y + 11, 242);
				break;

			default:
				break;
			}
		}

		// Draw status text.
		JE_textShade(VGAScreen, xMenuItemName, 190, menuItems[*selectedMenuItemIndex].description, 15, 4, PART_SHADE);

		// Draw picker box and items.

		if (currentPicker != MENU_ITEM_NONE)
		{
			const MenuItem *selectedMenuItem = &menuItems[*selectedMenuItemIndex];
			const size_t pickerItemsCount = selectedMenuItem->getPickerItemsCount();

			const int hPicker = dyPickerItem * pickerItemsCount - dyPickerItemPadding;
			yPicker = MIN(yPicker, 200 - 10 - (hPicker + 5 + 2));

			JE_rectangle(VGAScreen, xMenuItemValue - 5, yPicker- 3, xMenuItemValue + wMenuItemValue + 5 - 1, yPicker + hPicker + 3 - 1, 248);
			JE_rectangle(VGAScreen, xMenuItemValue - 4, yPicker- 4, xMenuItemValue + wMenuItemValue + 4 - 1, yPicker + hPicker + 4 - 1, 250);
			JE_rectangle(VGAScreen, xMenuItemValue - 3, yPicker- 5, xMenuItemValue + wMenuItemValue + 3 - 1, yPicker + hPicker + 5 - 1, 248);
			fill_rectangle_wh(VGAScreen, xMenuItemValue - 2, yPicker - 2, wMenuItemValue + 2 + 2, hPicker + 2 + 2, 224);

			for (size_t i = 0; i < pickerItemsCount; ++i)
			{
				const int y = yPicker + dyPickerItem * (int)i;

				const bool selected = i == pickerSelectedIndex;

				const char *value = selectedMenuItem->getPickerItem(i, buffer, sizeof buffer);

				draw_font_hv_shadow(VGAScreen, xMenuItemValue, y, value, normal_font, left_aligned, 15, -3 + (selected ? 2 : 0), false, 2);
			}
		}

		if (restart)
		{
			mouseCursor = MOUSE_POINTER_NORMAL;

			fade_palette(colors, 10, 0, 255);

			restart = false;
		}

		service_SDL_events(true);

		JE_mouseStart();
		JE_showVGA();
		JE_mouseReplace();

		bool mouseMoved = false;
		int oldFullscreenDisplay = fullscreen_display;
		do
		{
			SDL_Delay(16);

			Uint16 oldMouseX = mouse_x;
			Uint16 oldMouseY = mouse_y;

			push_joysticks_as_keyboard();
			service_SDL_events(false);

			mouseMoved = mouse_x != oldMouseX || mouse_y != oldMouseY;
		} while (!(newkey || newmouse || mouseMoved || fullscreen_display != oldFullscreenDisplay));

		if (currentPicker == MENU_ITEM_NONE)
		{
			// Handle menu item interaction.

			bool action = false;

			if (mouseMoved || newmouse)
			{
				// Find menu item name or value that was hovered or clicked.
				if (mouse_x >= xMenuItem && mouse_x < xMenuItem + wMenuItem)
				{
					for (size_t i = 0; i < menuItemsCount; ++i)
					{
						const int yMenuItem = yMenuItems + dyMenuItems * i;
						if (mouse_y >= yMenuItem && mouse_y < yMenuItem + hMenuItem)
						{
							if (*selectedMenuItemIndex != i)
							{
								JE_playSampleNum(S_CURSOR);

								*selectedMenuItemIndex = i;
							}

							if (newmouse && lastmouse_but == SDL_BUTTON_LEFT &&
							    lastmouse_y >= yMenuItem && lastmouse_y < yMenuItem + hMenuItem)
							{
								// Act on menu item via name.
								if (lastmouse_x >= xMenuItemName && lastmouse_x < xMenuItemName + wMenuItemName)
								{
									action = true;
								}

								// Act on menu item via value.
								else if (lastmouse_x >= xMenuItemValue && lastmouse_x < xMenuItemValue + wMenuItemValue)
								{
									switch (menuItems[*selectedMenuItemIndex].id)
									{
									case MENU_ITEM_DISPLAY:
									case MENU_ITEM_SCALER:
									case MENU_ITEM_SCALING_MODE:
									{
										action = true;
										break;
									}
									case MENU_ITEM_MUSIC_VOLUME:
									{
										JE_playSampleNum(S_CURSOR);

										int value = (lastmouse_x - xMenuItemValue) * 255 / (wMenuItemValue - 1);
										tyrMusicVolume = MIN(MAX(0, value), 255);

										set_volume(tyrMusicVolume, fxVolume);
										break;
									}
									case MENU_ITEM_SOUND_VOLUME:
									{
										int value = (lastmouse_x - xMenuItemValue) * 255 / (wMenuItemValue - 1);
										fxVolume = MIN(MAX(0, value), 255);

										set_volume(tyrMusicVolume, fxVolume);

										JE_playSampleNum(S_CURSOR);
										break;
									}
									default:
										break;
									}
								}
							}

							break;
						}
					}
				}
			}

			if (newmouse)
			{
				if (lastmouse_but == SDL_BUTTON_RIGHT)
				{
					JE_playSampleNum(S_SPRING);

					currentMenu = menuParents[currentMenu];
				}
			}
			else if (newkey)
			{
				switch (lastkey_scan)
				{
				case SDL_SCANCODE_UP:
				{
					JE_playSampleNum(S_CURSOR);

					*selectedMenuItemIndex = *selectedMenuItemIndex == 0
						? menuItemsCount - 1
						: *selectedMenuItemIndex - 1;
					break;
				}
				case SDL_SCANCODE_DOWN:
				{
					JE_playSampleNum(S_CURSOR);

					*selectedMenuItemIndex = *selectedMenuItemIndex == menuItemsCount - 1
						? 0
						: *selectedMenuItemIndex + 1;
					break;
				}
				case SDL_SCANCODE_LEFT:
				{
					switch (menuItems[*selectedMenuItemIndex].id)
					{
					case MENU_ITEM_MUSIC_VOLUME:
					{
						JE_playSampleNum(S_CURSOR);

						JE_changeVolume(&tyrMusicVolume, -8, &fxVolume, 0);
						break;
					}
					case MENU_ITEM_SOUND_VOLUME:
					{
						JE_changeVolume(&tyrMusicVolume, 0, &fxVolume, -8);

						JE_playSampleNum(S_CURSOR);
						break;
					}
					default:
						break;
					}
					break;
				}
				case SDL_SCANCODE_RIGHT:
				{
					switch (menuItems[*selectedMenuItemIndex].id)
					{
					case MENU_ITEM_MUSIC_VOLUME:
					{
						JE_playSampleNum(S_CURSOR);

						JE_changeVolume(&tyrMusicVolume, 8, &fxVolume, 0);
						break;
					}
					case MENU_ITEM_SOUND_VOLUME:
					{
						JE_changeVolume(&tyrMusicVolume, 0, &fxVolume, 8);

						JE_playSampleNum(S_CURSOR);
						break;
					}
					default:
						break;
					}
					break;
				}
				case SDL_SCANCODE_SPACE:
				case SDL_SCANCODE_RETURN:
				{
					action = true;
					break;
				}
				case SDL_SCANCODE_ESCAPE:
				{
					JE_playSampleNum(S_SPRING);

					currentMenu = menuParents[currentMenu];
					break;
				}
				default:
					break;
				}
			}

			if (action)
			{
				const MenuItemId selectedMenuItemId = menuItems[*selectedMenuItemIndex].id;

				switch (selectedMenuItemId)
				{
				case MENU_ITEM_DONE:
				{
					JE_playSampleNum(S_SELECT);

					currentMenu = menuParents[currentMenu];
					break;
				}
				case MENU_ITEM_GRAPHICS:
				{
					JE_playSampleNum(S_SELECT);

					menuParents[MENU_GRAPHICS] = currentMenu;
					currentMenu = MENU_GRAPHICS;
					selectedMenuItemIndexes[currentMenu] = 0;
					break;
				}
				case MENU_ITEM_SOUND:
				{
					JE_playSampleNum(S_SELECT);

					menuParents[MENU_SOUND] = currentMenu;
					currentMenu = MENU_SOUND;
					selectedMenuItemIndexes[currentMenu] = 0;
					break;
				}
				case MENU_ITEM_JUKEBOX:
				{
					JE_playSampleNum(S_SELECT);

					fade_black(10);

					jukebox();

					restart = true;
					break;
				}
				case MENU_ITEM_DESTRUCT:
				{
					JE_playSampleNum(S_SELECT);

					fade_black(10);

					JE_destructGame();

					restart = true;
					break;
				}
				case MENU_ITEM_DISPLAY:
				{
					JE_playSampleNum(S_CLICK);

					currentPicker = selectedMenuItemId;
					pickerSelectedIndex = (size_t)(fullscreen_display + 1);
					break;
				}
				case MENU_ITEM_SCALER:
				{
					JE_playSampleNum(S_CLICK);

					currentPicker = selectedMenuItemId;
					pickerSelectedIndex = scaler;
					break;
				}
				case MENU_ITEM_SCALING_MODE:
				{
					JE_playSampleNum(S_CLICK);

					currentPicker = selectedMenuItemId;
					pickerSelectedIndex = scaling_mode;
					break;
				}
				case MENU_ITEM_MUSIC_VOLUME:
				{
					JE_playSampleNum(S_CLICK);

					music_disabled = !music_disabled;
					if (!music_disabled)
						restart_song();
					break;
				}
				case MENU_ITEM_SOUND_VOLUME:
				{
					samples_disabled = !samples_disabled;

					JE_playSampleNum(S_CLICK);
					break;
				}
				default:
					break;
				}
			}

			if (currentMenu == MENU_NONE)
			{
				fade_black(10);

				return;
			}
		}
		else
		{
			const MenuItem *selectedMenuItem = &menuItems[*selectedMenuItemIndex];

			// Handle picker interaction.

			bool action = false;

			if (mouseMoved || newmouse)
			{
				const size_t pickerItemsCount = selectedMenuItem->getPickerItemsCount();

				// Find picker item that was hovered clicked.
				if (mouse_x >= xMenuItemValue && mouse_x < xMenuItemValue + wMenuItemValue)
				{
					for (size_t i = 0; i < pickerItemsCount; ++i)
					{
						const int yPickerItem = yPicker + dyPickerItem * i;

						if (mouse_y >= yPickerItem && mouse_y < yPickerItem + hPickerItem)
						{
							if (pickerSelectedIndex != i)
							{
								JE_playSampleNum(S_CURSOR);

								pickerSelectedIndex = i;
							}

							// Act on picker item.
							if (newmouse && lastmouse_but == SDL_BUTTON_LEFT &&
							    lastmouse_x >= xMenuItemValue && lastmouse_y < xMenuItemValue + wMenuItemName &&
							    lastmouse_y >= yPickerItem && lastmouse_y < yPickerItem + hPickerItem)
							{
								action = true;
							}
						}
					}
				}
			}

			if (newmouse)
			{
				if (lastmouse_but == SDL_BUTTON_RIGHT)
				{
					JE_playSampleNum(S_SPRING);

					currentPicker = MENU_ITEM_NONE;
				}
			}
			else if (newkey)
			{
				switch (lastkey_scan)
				{
				case SDL_SCANCODE_UP:
				{
					JE_playSampleNum(S_CURSOR);

					const size_t pickerItemsCount = selectedMenuItem->getPickerItemsCount();

					pickerSelectedIndex = pickerSelectedIndex == 0
						? pickerItemsCount - 1
						: pickerSelectedIndex - 1;
					break;
				}
				case SDL_SCANCODE_DOWN:
				{
					JE_playSampleNum(S_CURSOR);

					const size_t pickerItemsCount = selectedMenuItem->getPickerItemsCount();

					pickerSelectedIndex = pickerSelectedIndex == pickerItemsCount - 1
						? 0
						: pickerSelectedIndex + 1;
					break;
				}
				case SDL_SCANCODE_SPACE:
				case SDL_SCANCODE_RETURN:
				{
					action = true;
					break;
				}
				case SDL_SCANCODE_ESCAPE:
				{
					JE_playSampleNum(S_SPRING);

					currentPicker = MENU_ITEM_NONE;
					break;
				}
				default:
					break;
				}
			}

			if (action)
			{
				JE_playSampleNum(S_CLICK);

				switch (selectedMenuItem->id)
				{
				case MENU_ITEM_DISPLAY:
				{
					if ((int)pickerSelectedIndex - 1 != fullscreen_display)
						reinit_fullscreen((int)pickerSelectedIndex - 1);
					break;
				}
				case MENU_ITEM_SCALER:
				{
					if (pickerSelectedIndex != scaler)
					{
						const int oldScaler = scaler;
						if (!init_scaler(pickerSelectedIndex) &&  // try new scaler
							!init_scaler(oldScaler))              // revert on fail
						{
							exit(EXIT_FAILURE);
						}
					}
					break;
				}
				case MENU_ITEM_SCALING_MODE:
				{
					scaling_mode = pickerSelectedIndex;
					break;
				}
				default:
					break;
				}

				currentPicker = MENU_ITEM_NONE;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	mt_srand(time(NULL));

	printf("\nWelcome to... >> %s %s <<\n\n", opentyrian_str, opentyrian_version);

	printf("Copyright (C) 2022 The OpenTyrian Development Team\n\n");

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

		loadSndFile(xmas);
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

		play_demo = false;
		stopped_demo = false;

		gameLoaded = false;
		jumpSection = false;

#ifdef WITH_NETWORK
		if (isNetworkGame)
		{
			networkStartScreen();
		}
		else
#endif
		{
			if (!titleScreen())
			{
				// Player quit from title screen.
				break;
			}
		}

		if (loadDestruct)
		{
			JE_destructGame();

			loadDestruct = false;
		}
		else
		{
			JE_main();

			if (trentWin)
			{
				// Player beat SuperTyrian.
				break;
			}
		}
	}

	JE_tyrianHalt(0);

	return 0;
}
