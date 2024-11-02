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
#include "mainint.h"

#include "backgrnd.h"
#include "config.h"
#include "editship.h"
#include "episodes.h"
#include "file.h"
#include "font.h"
#include "fonthand.h"
#include "helptext.h"
#include "helptext.h"
#include "joystick.h"
#include "keyboard.h"
#include "lds_play.h"
#include "loudness.h"
#include "menus.h"
#include "mouse.h"
#include "mtrand.h"
#include "network.h"
#include "nortsong.h"
#include "nortvars.h"
#include "opentyr.h"
#include "palette.h"
#include "params.h"
#include "pcxmast.h"
#include "picload.h"
#include "player.h"
#include "shots.h"
#include "sndmast.h"
#include "sprite.h"
#include "varz.h"
#include "vga256d.h"
#include "video.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

bool button[4];

#define MAX_PAGE 8
#define TOPICS 6
const JE_byte topicStart[TOPICS] = { 0, 1, 2, 3, 7, 255 };

JE_shortint constantLastX;
JE_word textErase;
JE_word upgradeCost;
JE_word downgradeCost;
JE_boolean performSave;
JE_boolean jumpSection;
JE_boolean useLastBank; /* See if I want to use the last 16 colors for DisplayText */

bool pause_pressed = false, ingamemenu_pressed = false;

/* Draws a message at the bottom text window on the playing screen */
void JE_drawTextWindow(const char *text)
{
	if (textErase > 0) // erase current text
		blit_sprite(VGAScreenSeg, 16, 189, OPTION_SHAPES, 36);  // in-game text area

	textErase = 100;
	JE_outText(VGAScreenSeg, 20, 190, text, 0, 4);
}

void JE_outCharGlow(JE_word x, JE_word y, const char *s)
{
	JE_integer maxloc, loc, z;
	JE_shortint glowcol[60]; /* [1..60] */
	JE_shortint glowcolc[60]; /* [1..60] */
	JE_word textloc[60]; /* [1..60] */
	JE_byte bank;

	setDelay2(1);

	bank = (warningRed) ? 7 : ((useLastBank) ? 15 : 14);

	if (s[0] == '\0')
		return;

	if (frameCountMax == 0)
	{
		JE_textShade(VGAScreen, x, y, s, bank, 0, PART_SHADE);
		JE_showVGA();
	}
	else
	{
		maxloc = strlen(s);
		for (z = 0; z < 60; z++)
		{
			glowcol[z] = -8;
			glowcolc[z] = 1;
		}

		loc = x;
		for (z = 0; z < maxloc; z++)
		{
			textloc[z] = loc;

			int sprite_id = font_ascii[(unsigned char)s[z]];

			if (s[z] == ' ')
				loc += 6;
			else if (sprite_id != -1)
				loc += sprite(TINY_FONT, sprite_id)->width + 1;
		}

		for (loc = 0; (unsigned)loc < strlen(s) + 28; loc++)
		{
			if (!ESCPressed)
			{
				setDelay(frameCountMax);

				NETWORK_KEEP_ALIVE();

				int sprite_id = -1;

				for (z = loc - 28; z <= loc; z++)
				{
					if (z >= 0 && z < maxloc)
					{
						sprite_id = font_ascii[(unsigned char)s[z]];

						if (sprite_id != -1)
						{
							blit_sprite_hv(VGAScreen, textloc[z], y, TINY_FONT, sprite_id, bank, glowcol[z]);

							glowcol[z] += glowcolc[z];
							if (glowcol[z] > 9)
								glowcolc[z] = -1;
						}
					}
				}
				if (sprite_id != -1 && --z < maxloc)
					blit_sprite_dark(VGAScreen, textloc[z] + 1, y + 1, TINY_FONT, sprite_id, true);

				if (JE_anyButton())
					frameCountMax = 0;

				do
				{
					if (levelWarningDisplay)
						JE_updateWarning(VGAScreen);

					SDL_Delay(16);
				} while (!(getDelayTicks() == 0 || ESCPressed));

				JE_showVGA();
			}
		}
	}
}

void JE_drawPortConfigButtons(void) // rear weapon pattern indicator
{
	if (twoPlayerMode)
		return;

	if (player[0].weapon_mode == 1)
	{
		blit_sprite(VGAScreenSeg, 285, 44, OPTION_SHAPES, 18);  // lit
		blit_sprite(VGAScreenSeg, 302, 44, OPTION_SHAPES, 19);  // unlit
	}
	else // == 2
	{
		blit_sprite(VGAScreenSeg, 285, 44, OPTION_SHAPES, 19);  // unlit
		blit_sprite(VGAScreenSeg, 302, 44, OPTION_SHAPES, 18);  // lit
	}
}

static bool helpSystemPage(Uint8 *topic, bool *restart);

void JE_helpSystem(JE_byte startTopic)
{
	if (shopSpriteSheet.data == NULL)
		JE_loadCompShapes(&shopSpriteSheet, '1');  // need mouse pointer sprites

	Uint8 topic = startTopic;

	bool restart = true;

	const size_t menuItemsCount = COUNTOF(topicName) - 1;
	size_t selectedIndex = 0;

	const int xCenter = 320 / 2;
	const int yMenuHeader = 30;
	const int yMenuItems = 60;
	const int dyMenuItems = 20;
	const int hMenuItem = 13;
	int wMenuItem[COUNTOF(topicName) - 1] = { 0 };

	for (; ; )
	{
		if (restart)
		{
			play_song(SONG_MAPVIEW);

			JE_loadPic(VGAScreen2, 2, false);
		}

		if (topic > 1)
		{
			if (!helpSystemPage(&topic, &restart))
				return;

			selectedIndex = (size_t)topic - 1;
			topic = 1;
			continue;
		}

		// Restore background.
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, (size_t)VGAScreen->pitch * VGAScreen->h);

		// Draw header.
		draw_font_hv_shadow(VGAScreen, xCenter, yMenuHeader, topicName[0], large_font, centered, 15, -3, false, 2);

		// Draw menu items.
		for (size_t i = 0; i < menuItemsCount; ++i)
		{
			const char *const text = topicName[i + 1];

			wMenuItem[i] = JE_textWidth(text, normal_font);
			const int y = yMenuItems + dyMenuItems * i;

			const bool selected = i == selectedIndex;

			draw_font_hv_shadow(VGAScreen, xCenter, y, text, normal_font, centered, 15, -3 + (selected ? 2 : 0), false, 2);
		}

		mouseCursor = MOUSE_POINTER_NORMAL;

		if (restart)
		{
			fade_palette(colors, 10, 0, 255);

			restart = false;
		}

		service_SDL_events(true);

		JE_mouseStart();
		JE_showVGA();
		JE_mouseReplace();

		bool mouseMoved = false;
		do
		{
			SDL_Delay(16);

			Uint16 oldMouseX = mouse_x;
			Uint16 oldMouseY = mouse_y;

			push_joysticks_as_keyboard();
			service_SDL_events(false);

			mouseMoved = mouse_x != oldMouseX || mouse_y != oldMouseY;
		} while (!(newkey || newmouse || mouseMoved));

		// Handle interaction.

		bool action = false;
		bool done = false;

		if (mouseMoved || newmouse)
		{
			// Find menu item that was hovered or clicked.
			for (size_t i = 0; i < menuItemsCount; ++i)
			{
				const int xMenuItem = xCenter - wMenuItem[i] / 2;
				if (mouse_x >= xMenuItem && mouse_x < xMenuItem + wMenuItem[i])
				{
					const int yMenuItem = yMenuItems + dyMenuItems * i;
					if (mouse_y >= yMenuItem && mouse_y < yMenuItem + hMenuItem)
					{
						if (selectedIndex != i)
						{
							JE_playSampleNum(S_CURSOR);

							selectedIndex = i;
						}

						if (newmouse && lastmouse_but == SDL_BUTTON_LEFT &&
						    lastmouse_x >= xMenuItem && lastmouse_x < xMenuItem + wMenuItem[i] &&
						    lastmouse_y >= yMenuItem && lastmouse_y < yMenuItem + hMenuItem)
						{
							action = true;
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

				done = true;
			}
		}
		else if (newkey)
		{
			switch (lastkey_scan)
			{
			case SDL_SCANCODE_UP:
			{
				JE_playSampleNum(S_CURSOR);

				selectedIndex = selectedIndex == 0
					? menuItemsCount - 1
					: selectedIndex - 1;
				break;
			}
			case SDL_SCANCODE_DOWN:
			{
				JE_playSampleNum(S_CURSOR);

				selectedIndex = selectedIndex == menuItemsCount - 1
					? 0
					: selectedIndex + 1;
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

				done = true;
				break;
			}
			default:
				break;
			}
		}

		if (action)
		{
			JE_playSampleNum(S_SELECT);

			topic = selectedIndex + 2;

			if (selectedIndex == menuItemsCount - 1)
				done = true;
		}

		if (done)
		{
			fade_black(15);

			return;
		}
	}
}

static bool helpSystemPage(Uint8 *topic, bool *restart)
{
	Uint8 page = topicStart[*topic - 1];

	const int xCenter = 320 / 2;

	for (; ; )
	{
		if (page == 0)
		{
			*topic = 1;
			return true;
		}
		else if (page > MAX_PAGE)
		{
			*topic = COUNTOF(topicName) - 1;
			return true;
		}

		for (Uint8 temp = 0; temp < COUNTOF(topicName); ++temp)
		{
			if (topicStart[temp] <= page)
				*topic = temp + 1;
			else
				break;
		}

		// Restore background.
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, (size_t)VGAScreen->pitch * VGAScreen->h);

		fill_rectangle_wh(VGAScreen, 0, 192, 320, 8, 0);

		const char *const text = topicName[*topic - 1];

		// Draw header.
		draw_font_hv_shadow(VGAScreen, xCenter, 1, text, normal_font, centered, 15, -3, false, 2);

		// Draw footer.
		JE_char buffer[128];

		snprintf(buffer, sizeof buffer, "%s %d", miscText[24], page - topicStart[*topic - 1] + 1);
		draw_font_hv(VGAScreen, 10, 192, buffer, small_font, left_aligned, 13, 5);

		snprintf(buffer, sizeof buffer, "%s %d of %d", miscText[25], page, MAX_PAGE);
		draw_font_hv(VGAScreen, 320 - 10, 192, buffer, small_font, right_aligned, 13, 5);

		// Draw text.

		helpBoxBrightness = 3;
		verticalHeight = 8;

		switch (page)
		{
		case 1: /* One-Player Menu */
			JE_HBox(VGAScreen, 10,  20,  2, 60);
			JE_HBox(VGAScreen, 10,  50,  5, 60);
			JE_HBox(VGAScreen, 10,  80, 21, 60);
			JE_HBox(VGAScreen, 10, 110,  1, 60);
			JE_HBox(VGAScreen, 10, 140, 28, 60);
			break;
		case 2: /* Two-Player Menu */
			JE_HBox(VGAScreen, 10,  20,  1, 60);
			JE_HBox(VGAScreen, 10,  60,  2, 60);
			JE_HBox(VGAScreen, 10, 100, 21, 60);
			JE_HBox(VGAScreen, 10, 140, 28, 60);
			break;
		case 3: /* Upgrade Ship */
			JE_HBox(VGAScreen, 10,  20,  5, 60);
			JE_HBox(VGAScreen, 10,  70,  6, 60);
			JE_HBox(VGAScreen, 10, 110,  7, 60);
			break;
		case 4:
			JE_HBox(VGAScreen, 10,  20,  8, 60);
			JE_HBox(VGAScreen, 10,  55,  9, 60);
			JE_HBox(VGAScreen, 10,  87, 10, 60);
			JE_HBox(VGAScreen, 10, 120, 11, 60);
			JE_HBox(VGAScreen, 10, 170, 13, 60);
			break;
		case 5:
			JE_HBox(VGAScreen, 10,  20, 14, 60);
			JE_HBox(VGAScreen, 10,  80, 15, 60);
			JE_HBox(VGAScreen, 10, 120, 16, 60);
			break;
		case 6:
			JE_HBox(VGAScreen, 10,  20, 17, 60);
			JE_HBox(VGAScreen, 10,  40, 18, 60);
			JE_HBox(VGAScreen, 10, 130, 20, 60);
			break;
		case 7: /* Options */
			JE_HBox(VGAScreen, 10,  20, 21, 60);
			JE_HBox(VGAScreen, 10,  70, 22, 60);
			JE_HBox(VGAScreen, 10, 110, 23, 60);
			JE_HBox(VGAScreen, 10, 140, 24, 60);
			break;
		case 8:
			JE_HBox(VGAScreen, 10,  20, 25, 60);
			JE_HBox(VGAScreen, 10,  60, 26, 60);
			JE_HBox(VGAScreen, 10, 100, 27, 60);
			JE_HBox(VGAScreen, 10, 140, 28, 60);
			JE_HBox(VGAScreen, 10, 170, 29, 60);
			break;
		}

		helpBoxBrightness = 1;
		verticalHeight = 7;

		if (*restart)
		{
			fade_palette(colors, 10, 0, 255);

			*restart = false;
		}

		do
		{
			mouseCursor = mouse_x < xCenter ? MOUSE_POINTER_LEFT : MOUSE_POINTER_RIGHT;

			service_SDL_events(true);

			JE_mouseStart();
			JE_showVGA();
			JE_mouseReplace();

			SDL_Delay(16);

			push_joysticks_as_keyboard();
			service_SDL_events(false);
		} while (!(newkey || newmouse));

		// Handle interaction.

		bool done = false;

		if (newmouse)
		{
			switch (lastmouse_but)
			{
			case SDL_BUTTON_LEFT:
			{
				JE_playSampleNum(S_CURSOR);

				if (mouse_x < xCenter)
					page -= 1;
				else
					page += 1;
				break;
			}
			case SDL_BUTTON_RIGHT:
			{
				JE_playSampleNum(S_SPRING);

				done = true;
				break;
			}
			default:
				break;
			}
		}
		else if (newkey)
		{
			switch (lastkey_scan)
			{
			case SDL_SCANCODE_LEFT:
			{
				JE_playSampleNum(S_CURSOR);

				page -= 1;
				break;
			}
			case SDL_SCANCODE_RIGHT:
			case SDL_SCANCODE_SPACE:
			case SDL_SCANCODE_RETURN:
			{
				JE_playSampleNum(S_CURSOR);

				page += 1;
				break;
			}
			case SDL_SCANCODE_ESCAPE:
			{
				JE_playSampleNum(S_SPRING);

				done = true;
				break;
			}
			default:
				break;
			}
		}

		if (done)
		{
			fade_black(15);

			return false;
		}
	}
}

// cost to upgrade a weapon power from power-1 (where power == 0 indicates an unupgraded weapon)
long weapon_upgrade_cost(long base_cost, unsigned int power)
{
	assert(power <= 11);

	unsigned int temp = 0;

	// 0 1 3 6 10 15 21 29 ...
	for (; power > 0; power--)
		temp += power;

	return base_cost * temp;
}

ulong JE_getCost(JE_byte itemType, JE_word itemNum)
{
	long cost = 0;

	switch (itemType)
	{
	case 2:
		cost = (itemNum > 90) ? 100 : ships[itemNum].cost;
		break;
	case 3:
	case 4:
		cost = weaponPort[itemNum].cost;

		const uint port = itemType - 3,
		           item_power = player[0].items.weapon[port].power - 1;

		downgradeCost = weapon_upgrade_cost(cost, item_power);
		upgradeCost = weapon_upgrade_cost(cost, item_power + 1);
		break;
	case 5:
		cost = shields[itemNum].cost;
		break;
	case 6:
		cost = powerSys[itemNum].cost;
		break;
	case 7:
	case 8:
		cost = options[itemNum].cost;
		break;
	}

	return cost;
}

bool JE_loadScreen(void)
{
	if (shopSpriteSheet.data == NULL)
		JE_loadCompShapes(&shopSpriteSheet, '1');  // need mouse pointer and arrow sprites

	bool restart = true;

	size_t playersIndex = 0;
	const size_t menuItemsCount = 12;
	size_t selectedIndex = 0;

	const int xCenter = 320 / 2;
	const int yMenuHeader = 5;
	const int xMenuItem = 10;
	const int xMenuItemName = xMenuItem;
	const int xMenuItemLastLevel = 120;
	const int xMenuItemEpisode = 250;
	const int wMenuItem = 300;
	const int yMenuItems = 30;
	const int dyMenuItems = 13;
	const int hMenuItem = 8;
	const int xLeftControl = 83;
	const int xRightControl = 213;
	const int wControl = 24;
	const int yControls = 179;

	for (; ; )
	{
		if (restart)
		{
			JE_loadPic(VGAScreen2, 2, false);
			fill_rectangle_wh(VGAScreen2, 0, 192, 320, 8, 0);
		}

		// Restore background.
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, (size_t)VGAScreen->pitch * VGAScreen->h);

		// Draw header.
		draw_font_hv_shadow(VGAScreen, xCenter, yMenuHeader, miscText[38 + playersIndex], large_font, centered, 15, -3, false, 2);

		// Draw menu items.

		for (size_t i = 0; i < menuItemsCount; ++i)
		{
			const int y = yMenuItems + dyMenuItems * i;

			const bool selected = i == selectedIndex;

			if (i == menuItemsCount - 1)
			{
				JE_textShade(VGAScreen, xMenuItemName, y, miscText[33], 13, selected ? 6 : 2, FULL_SHADE);
				continue;
			}

			const JE_SaveFileType *const saveFile = &saveFiles[playersIndex * 11 + i];

			const bool disabled = saveFile->level == 0;

			char buffer[22];

			if (disabled)
			{
				JE_textShade(VGAScreen, xMenuItemName, y, miscText[2], 13, selected ? 6 : 0, FULL_SHADE);

				snprintf(buffer, sizeof buffer, "%s -----", miscTextB[2]);
				JE_textShade(VGAScreen, xMenuItemLastLevel, y, buffer, 5, selected ? 6 : 0, FULL_SHADE);
			}
			else
			{
				JE_textShade(VGAScreen, xMenuItemName, y, saveFile->name, 13, selected ? 6 : 2, FULL_SHADE);

				snprintf(buffer, sizeof buffer, "%s %s", miscTextB[2], saveFile->levelName);
				JE_textShade(VGAScreen, xMenuItemLastLevel, y, buffer, 5, selected ? 6 : 2, FULL_SHADE);

				snprintf(buffer, sizeof buffer, "%s %u", miscTextB[1], saveFile->episode);
				JE_textShade(VGAScreen, xMenuItemEpisode, y, buffer, 5, selected ? 6 : 2, FULL_SHADE);
			}
		}

		// Draw paging controls.

		const bool leftControlVisible = playersIndex > 0;
		const bool rightControlVisible = playersIndex < 1;

		if (leftControlVisible)
			blit_sprite2x2(VGAScreen, xLeftControl, yControls, shopSpriteSheet, 279);

		if (rightControlVisible)
			blit_sprite2x2(VGAScreen, xRightControl, yControls, shopSpriteSheet, 281);

		helpBoxColor = 15;
		JE_helpBox(VGAScreen, 103, 182, miscText[55], 25);

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
		do
		{
			SDL_Delay(16);

			Uint16 oldMouseX = mouse_x;
			Uint16 oldMouseY = mouse_y;

			push_joysticks_as_keyboard();
			service_SDL_events(false);

			mouseMoved = mouse_x != oldMouseX || mouse_y != oldMouseY;
		} while (!(newkey || newmouse || mouseMoved));

		// Handle interaction.

		bool leftAction = false;
		bool rightAction = false;
		bool action = false;
		bool done = false;

		if (mouseMoved || newmouse)
		{
			if (leftControlVisible &&
			    mouse_y >= yControls &&
			    mouse_x >= xLeftControl &&
			    mouse_x < xLeftControl + wControl)
			{
				if (newmouse && lastmouse_but == SDL_BUTTON_LEFT)
				{
					JE_playSampleNum(S_CURSOR);

					leftAction = true;
				}
			}
			else if (rightControlVisible &&
			         mouse_y >= yControls &&
			         mouse_x >= xRightControl &&
			         mouse_x < xRightControl + wControl)
			{
				if (newmouse && lastmouse_but == SDL_BUTTON_LEFT)
				{
					JE_playSampleNum(S_CURSOR);

					rightAction = true;
				}
			}
			else
			{
				// Find menu item that was hovered or clicked.
				if (mouse_x >= xMenuItem && mouse_x < xMenuItem + wMenuItem)
				{
					for (size_t i = 0; i < menuItemsCount; ++i)
					{
						const int yMenuItem = yMenuItems + dyMenuItems * i;
						if (mouse_y >= yMenuItem && mouse_y < yMenuItem + hMenuItem)
						{
							if (selectedIndex != i)
							{
								JE_playSampleNum(S_CURSOR);

								selectedIndex = i;
							}

							if (newmouse && lastmouse_but == SDL_BUTTON_LEFT &&
							    lastmouse_x >= xMenuItem && lastmouse_x < xMenuItem + wMenuItem &&
							    lastmouse_y >= yMenuItem && lastmouse_y < yMenuItem + hMenuItem)
							{
								action = true;
							}

							break;
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

				done = true;
			}
		}
		else if (newkey)
		{
			switch (lastkey_scan)
			{
			case SDL_SCANCODE_LEFT:
			{
				JE_playSampleNum(S_CURSOR);

				leftAction = true;
				break;
			}
			case SDL_SCANCODE_RIGHT:
			{
				JE_playSampleNum(S_CURSOR);

				rightAction = true;
				break;
			}
			case SDL_SCANCODE_UP:
			{
				JE_playSampleNum(S_CURSOR);

				selectedIndex = selectedIndex == 0
					? menuItemsCount - 1
					: selectedIndex - 1;
				break;
			}
			case SDL_SCANCODE_DOWN:
			{
				JE_playSampleNum(S_CURSOR);

				selectedIndex = selectedIndex == menuItemsCount - 1
					? 0
					: selectedIndex + 1;
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

				done = true;
				break;
			}
			default:
				break;
			}
		}

		if (leftAction)
		{
			playersIndex = playersIndex == 0 ? 1 : 0;
		}
		else if (rightAction)
		{
			playersIndex = playersIndex == 1 ? 0 : 1;
		}
		else if (action)
		{
			if (selectedIndex == menuItemsCount - 1)  // "Exit to Main Menu"
			{
				JE_playSampleNum(S_SELECT);

				done = true;
			}
			else
			{
				const size_t saveFileIndex = playersIndex * 11 + selectedIndex;

				if (saveFiles[saveFileIndex].level == 0)  // "EMPTY SLOT"
				{
					JE_playSampleNum(S_CLINK);
				}
				else
				{
					JE_playSampleNum(S_SELECT);

					performSave = false;
					JE_operation(saveFileIndex + 1);

					fade_black(15);

					return gameLoaded;
				}
			}
		}

		if (done)
		{
			fade_black(15);

			return false;
		}
	}
}

ulong JE_totalScore(const Player *this_player)
{
	ulong temp = this_player->cash;

	temp += JE_getValue(2, this_player->items.ship);
	temp += JE_getValue(3, this_player->items.weapon[FRONT_WEAPON].id);
	temp += JE_getValue(4, this_player->items.weapon[REAR_WEAPON].id);
	temp += JE_getValue(5, this_player->items.shield);
	temp += JE_getValue(6, this_player->items.generator);
	temp += JE_getValue(7, this_player->items.sidekick[LEFT_SIDEKICK]);
	temp += JE_getValue(8, this_player->items.sidekick[RIGHT_SIDEKICK]);

	return temp;
}

JE_longint JE_getValue(JE_byte itemType, JE_word itemNum)
{
	long value = 0;

	switch (itemType)
	{
	case 2:
		value = ships[itemNum].cost;
		break;
	case 3:
	case 4:;
		const long base_value = weaponPort[itemNum].cost;

		// if two-player, use first player's front and second player's rear weapon
		const uint port = itemType - 3;
		const uint item_power = player[twoPlayerMode ? port : 0].items.weapon[port].power - 1;

		value = base_value;
		for (unsigned int i = 1; i <= item_power; ++i)
			value += weapon_upgrade_cost(base_value, i);
		break;
	case 5:
		value = shields[itemNum].cost;
		break;
	case 6:
		value = powerSys[itemNum].cost;
		break;
	case 7:
	case 8:
		value = options[itemNum].cost;
		break;
	}

	return value;
}

void JE_nextEpisode(void)
{
	strcpy(lastLevelName, "Completed");

	if (episodeNum == initial_episode_num && !gameHasRepeated && episodeNum != EPISODE_AVAILABLE &&
	    !isNetworkGame && !constantPlay)
	{
		JE_highScoreCheck();
	}

	unsigned int newEpisode = JE_findNextEpisode();

	if (jumpBackToEpisode1)
	{
		if (episodeNum > 2 &&
			!constantPlay)
		{
			JE_playCredits();
		}

		// randomly give player the SuperCarrot
		if ((mt_rand() % 6) == 0)
		{
			player[0].items.ship = 2;                      // SuperCarrot
			player[0].items.weapon[FRONT_WEAPON].id = 23;  // Banana Blast
			player[0].items.weapon[REAR_WEAPON].id = 24;   // Banana Blast Rear

			for (uint i = 0; i < COUNTOF(player[0].items.weapon); ++i)
				player[0].items.weapon[i].power = 1;

			player[1].items.weapon[REAR_WEAPON].id = 24;   // Banana Blast Rear

			player[0].last_items = player[0].items;
		}
	}

	if (newEpisode != episodeNum)
		JE_initEpisode(newEpisode);

	gameLoaded = true;
	mainLevel = FIRST_LEVEL;
	saveLevel = FIRST_LEVEL;

	play_song(26);

	JE_clr256(VGAScreen);
	memcpy(colors, palettes[6-1], sizeof(colors));

	JE_dString(VGAScreen, JE_fontCenter(episode_name[episodeNum], SMALL_FONT_SHAPES), 130, episode_name[episodeNum], SMALL_FONT_SHAPES);
	JE_dString(VGAScreen, JE_fontCenter(miscText[5-1], SMALL_FONT_SHAPES), 185, miscText[5-1], SMALL_FONT_SHAPES);

	JE_showVGA();
	fade_palette(colors, 15, 0, 255);

	JE_wipeKey();
	if (!constantPlay)
	{
		do
		{
			NETWORK_KEEP_ALIVE();

			SDL_Delay(16);
		} while (!JE_anyButton());
	}

	fade_black(15);
}

void JE_initPlayerData(void)
{
	/* JE: New Game Items/Data */

	player[0].items.ship = 1;                     // USP Talon
	player[0].items.weapon[FRONT_WEAPON].id = 1;  // Pulse Cannon
	player[0].items.weapon[REAR_WEAPON].id = 0;   // None
	player[0].items.shield = 4;                   // Gencore High Energy Shield
	player[0].items.generator = 2;                // Advanced MR-12
	for (uint i = 0; i < COUNTOF(player[0].items.sidekick); ++i)
		player[0].items.sidekick[i] = 0;          // None
	player[0].items.special = 0;                  // None

	player[0].last_items = player[0].items;

	player[1].items = player[0].items;
	player[1].items.weapon[REAR_WEAPON].id = 15;  // Vulcan Cannon
	player[1].items.sidekick_level = 101;         // 101, 102, 103
	player[1].items.sidekick_series = 0;          // None

	gameHasRepeated = false;
	onePlayerAction = false;
	superArcadeMode = SA_NONE;
	superTyrian = false;
	twoPlayerMode = false;

	secretHint = (mt_rand() % 3) + 1;

	for (uint p = 0; p < COUNTOF(player); ++p)
	{
		for (uint i = 0; i < COUNTOF(player->items.weapon); ++i)
		{
			player[p].items.weapon[i].power = 1;
		}

		player[p].weapon_mode = 1;
		player[p].armor = ships[player[p].items.ship].dmg;

		player[p].is_dragonwing = (p == 1);
		player[p].lives = &player[p].items.weapon[p].power;

	}

	mainLevel = FIRST_LEVEL;
	saveLevel = FIRST_LEVEL;

	strcpy(lastLevelName, miscText[19]);
}

void JE_sortHighScores(void)
{
	JE_byte x;

	temp = 0;
	for (x = 0; x < 6; x++)
	{
		JE_sort();
		temp += 3;
	}
}

void JE_highScoreScreen(void)
{
	if (shopSpriteSheet.data == NULL)
		JE_loadCompShapes(&shopSpriteSheet, '1');  // need mouse pointer and arrow sprites

	bool restart = true;

	size_t episodeIndex = 0;
	// Save file only has space for scores for 3 episodes.
	const size_t episodeCount = 3;

	const int xCenter = 320 / 2;
	const int yMenuHeader = 3;
	const int yEpisodeHeader = 30;
	const int xLeftControl = 83;
	const int xRightControl = 213;
	const int wControl = 24;
	const int yControls = 179;

	for (; ; )
	{
		if (restart)
		{
			JE_loadPic(VGAScreen2, 2, false);
			fill_rectangle_wh(VGAScreen2, 0, 192, 320, 8, 0);

			// Draw header.
			draw_font_hv_shadow(VGAScreen2, xCenter, yMenuHeader, miscText[50], large_font, centered, 15, -3, false, 2);
		}

		// Restore background and header.
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, (size_t)VGAScreen->pitch * VGAScreen->h);

		const bool disabled = !episodeAvail[episodeIndex];

		// Draw episode header.
		draw_font_hv_shadow(VGAScreen, xCenter, yEpisodeHeader, episode_name[episodeIndex + 1], normal_font, centered, 15, -3 + (disabled ? -4 : 0), false, 2);

		char buffer[29];

		// Draw 1-player scores.

		draw_font_hv_shadow(VGAScreen, xCenter, 55, miscText[46], normal_font, centered, 15, -3, false, 2);

		for (Uint8 i = 0; i < 3; ++i)
		{
			const int y = 75 + 10 * i;

			const JE_SaveFileType *const saveFile = &saveFiles[episodeIndex * 6 + i];
			const int rank = MIN(saveFile->highScoreDiff, COUNTOF(difficultyNameB) - 1);

			snprintf(buffer, sizeof buffer, "~#%d:~  %d", i + 1, saveFile->highScore1);
			JE_textShade(VGAScreen, 20, y, buffer, 15, 0, FULL_SHADE);
			JE_textShade(VGAScreen, 110, y, saveFile->highScoreName, 15, 2, FULL_SHADE);
			JE_textShade(VGAScreen, 250, y, difficultyNameB[rank], 15, rank + (rank == 0 ? 0 : -1), FULL_SHADE);
		}

		// Draw 2-player scores.

		draw_font_hv_shadow(VGAScreen, xCenter, 120, miscText[47], normal_font, centered, 15, -3, false, 2);

		for (Uint8 i = 0; i < 3; ++i)
		{
			const int y = 135 + 10 * i;

			const JE_SaveFileType *const saveFile = &saveFiles[episodeIndex * 6 + 3 + i];
			const int rank = MIN(saveFile->highScoreDiff, COUNTOF(difficultyNameB) - 1);

			snprintf(buffer, sizeof buffer, "~#%d:~  %d", i + 1, saveFile->highScore1);
			JE_textShade(VGAScreen, 20, y, buffer, 15, 0, FULL_SHADE);
			JE_textShade(VGAScreen, 110, y, saveFile->highScoreName, 15, 2, FULL_SHADE);
			JE_textShade(VGAScreen, 250, y, difficultyNameB[rank], 15, rank + (rank == 0 ? 0 : -1), FULL_SHADE);
		}

		// Draw paging controls.

		const bool leftControlVisible = episodeIndex > 0;
		const bool rightControlVisible = episodeIndex < episodeCount - 1;

		if (leftControlVisible)
			blit_sprite2x2(VGAScreen, xLeftControl, yControls, shopSpriteSheet, 279);

		if (rightControlVisible)
			blit_sprite2x2(VGAScreen, xRightControl, yControls, shopSpriteSheet, 281);

		helpBoxColor = 15;
		JE_helpBox(VGAScreen, 103, 182, miscText[56], 25);

		if (restart)
		{
			mouseCursor = MOUSE_POINTER_NORMAL;

			fade_palette(colors, 10, 0, 255);

			restart = false;
		}

		do
		{
			service_SDL_events(true);

			JE_mouseStart();
			JE_showVGA();
			JE_mouseReplace();

			SDL_Delay(16);

			push_joysticks_as_keyboard();
			service_SDL_events(false);
		} while (!(newkey || newmouse));

		// Handle interaction.

		bool leftAction = false;
		bool rightAction = false;
		bool done = false;

		if (newmouse)
		{
			switch (lastmouse_but)
			{
			case SDL_BUTTON_LEFT:
			{
				if (leftControlVisible &&
				    mouse_y >= yControls &&
				    mouse_x >= xLeftControl &&
				    mouse_x < xLeftControl + wControl)
				{
					JE_playSampleNum(S_CURSOR);

					leftAction = true;
				}
				else if (rightControlVisible &&
				         mouse_y >= yControls &&
				         mouse_x >= xRightControl &&
				         mouse_x < xRightControl + wControl)
				{
					JE_playSampleNum(S_CURSOR);

					rightAction = true;
				}
				break;
			}
			case SDL_BUTTON_RIGHT:
			{
				JE_playSampleNum(S_SPRING);

				done = true;
				break;
			}
			default:
				break;
			}
		}
		else if (newkey)
		{
			switch (lastkey_scan)
			{
			case SDL_SCANCODE_LEFT:
			{
				JE_playSampleNum(S_CURSOR);

				leftAction = true;
				break;
			}
			case SDL_SCANCODE_RIGHT:
			{
				JE_playSampleNum(S_CURSOR);

				rightAction = true;
				break;
			}
			case SDL_SCANCODE_SPACE:
			case SDL_SCANCODE_RETURN:
			case SDL_SCANCODE_ESCAPE:
			{
				JE_playSampleNum(S_SPRING);

				done = true;
				break;
			}
			default:
				break;
			}
		}

		if (leftAction)
		{
			episodeIndex = episodeIndex == 0
				? episodeCount - 1
				: episodeIndex - 1;
		}
		else if (rightAction)
		{
			episodeIndex = episodeIndex == episodeCount - 1
				? 0
				: episodeIndex + 1;
		}

		if (done)
		{
			fade_black(15);

			return;
		}
	}
}

void JE_gammaCorrect_func(JE_byte *col, JE_real r)
{
	int temp = roundf(*col * r);
	if (temp > 255)
	{
		temp = 255;
	}
	*col = temp;
}

void JE_gammaCorrect(Palette *colorBuffer, JE_byte gamma)
{
	int x;
	JE_real r = 1 + (JE_real)gamma / 10;

	for (x = 0; x < 256; x++)
	{
		JE_gammaCorrect_func(&(*colorBuffer)[x].r, r);
		JE_gammaCorrect_func(&(*colorBuffer)[x].g, r);
		JE_gammaCorrect_func(&(*colorBuffer)[x].b, r);
	}
}

JE_boolean JE_gammaCheck(void)
{
	bool temp = keysactive[SDL_SCANCODE_F11] != 0;
	if (temp)
	{
		keysactive[SDL_SCANCODE_F11] = false;
		newkey = false;
		gammaCorrection = (gammaCorrection + 1) % 4;
		memcpy(colors, palettes[pcxpal[3-1]], sizeof(colors));
		JE_gammaCorrect(&colors, gammaCorrection);
		set_palette(colors, 0, 255);
	}
	return temp;
}

void JE_doInGameSetup(void)
{
	mouseSetRelative(false);

	haltGame = false;

#ifdef WITH_NETWORK
	if (isNetworkGame)
	{
		network_prepare(PACKET_GAME_MENU);
		network_send(4);  // PACKET_GAME_MENU

		while (true)
		{
			service_SDL_events(false);

			if (packet_in[0] && SDLNet_Read16(&packet_in[0]->data[0]) == PACKET_GAME_MENU)
			{
				network_update();
				break;
			}

			network_update();
			network_check();

			SDL_Delay(16);
		}
	}
#endif

	if (yourInGameMenuRequest)
	{
		if (JE_inGameSetup())
		{
			reallyEndLevel = true;
			playerEndLevel = true;
		}
		quitRequested = false;

		keysactive[SDL_SCANCODE_ESCAPE] = false;

#ifdef WITH_NETWORK
		if (isNetworkGame)
		{
			if (!playerEndLevel)
			{
				network_prepare(PACKET_WAITING);
				network_send(4);  // PACKET_WAITING
			}
			else
			{
				network_prepare(PACKET_GAME_QUIT);
				network_send(4);  // PACKET_GAMEQUIT
			}
		}
#endif
	}

#ifdef WITH_NETWORK
	if (isNetworkGame)
	{
		SDL_Surface *temp_surface = VGAScreen;
		VGAScreen = VGAScreenSeg; /* side-effect of game_screen */

		if (!yourInGameMenuRequest)
		{
			JE_barShade(VGAScreen, 3, 60, 257, 80); /*Help Box*/
			JE_barShade(VGAScreen, 5, 62, 255, 78);
			JE_dString(VGAScreen, 10, 65, "Other player in options menu.", SMALL_FONT_SHAPES);
			JE_showVGA();

			while (true)
			{
				service_SDL_events(false);
				JE_showVGA();

				if (packet_in[0])
				{
					if (SDLNet_Read16(&packet_in[0]->data[0]) == PACKET_WAITING)
					{
						network_check();
						break;
					}
					else if (SDLNet_Read16(&packet_in[0]->data[0]) == PACKET_GAME_QUIT)
					{
						reallyEndLevel = true;
						playerEndLevel = true;

						network_check();
						break;
					}
				}

				network_update();
				network_check();

				SDL_Delay(16);
			}
		}
		else
		{
			/*
			JE_barShade(3, 160, 257, 180); /-*Help Box*-/
			JE_barShade(5, 162, 255, 178);
			tempScreenSeg = VGAScreen;
			JE_dString(VGAScreen, 10, 165, "Waiting for other player.", SMALL_FONT_SHAPES);
			JE_showVGA();
			*/
		}

		while (!network_is_sync())
		{
			service_SDL_events(false);

			network_check();
			SDL_Delay(16);
		}

		VGAScreen = temp_surface; /* side-effect of game_screen */
	}
#endif

	yourInGameMenuRequest = false;

	//skipStarShowVGA = true;

	mouseSetRelative(true);
}

JE_boolean JE_inGameSetup(void)
{
	bool result = false;

	SDL_Surface *temp_surface = VGAScreen;
	VGAScreen = VGAScreenSeg; /* side-effect of game_screen */

	enum MenuItemIndex
	{
		MENU_ITEM_MUSIC_VOLUME = 0,
		MENU_ITEM_EFFECTS_VOLUME,
		MENU_ITEM_DETAIL_LEVEL,
		MENU_ITEM_GAME_SPEED,
		MENU_ITEM_RETURN_TO_GAME,
		MENU_ITEM_QUIT,
	};

	const size_t helpIndexes[] = { 14, 14, 27, 28, 25, 26 };

	if (shopSpriteSheet.data == NULL)
		JE_loadCompShapes(&shopSpriteSheet, '1');  // need mouse pointer sprites

	bool restart = true;

	const size_t menuItemsCount = COUNTOF(inGameText);
	size_t selectedIndex = MENU_ITEM_MUSIC_VOLUME;

	const int yMenuItems = 20;
	const int dyMenuItems = 20;
	const int xMenuItem = 10;
	const int xMenuItemName = xMenuItem;
	const int wMenuItemName = 110;
	const int xMenuItemValue = xMenuItemName + wMenuItemName;
	const int wMenuItemValue = 90;
	const int wMenuItem = wMenuItemName + wMenuItemValue;
	const int hMenuItem = 13;

	for (bool done = false; !done; )
	{
		if (restart)
		{
			// Main box
			JE_barShade(VGAScreen, 3, 13, 217, 137);
			JE_barShade(VGAScreen, 5, 15, 215, 135);

			// Help box
			JE_barShade(VGAScreen, 3, 143, 257, 157);
			JE_barShade(VGAScreen, 5, 145, 255, 155);

			memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

			mouseCursor = MOUSE_POINTER_NORMAL;

			restart = false;
		}

		// Restore background.
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, (size_t)VGAScreen->pitch * VGAScreen->h);

		// Draw menu items.
		for (size_t i = 0; i < menuItemsCount; ++i)
		{
			const int y = yMenuItems + dyMenuItems * i;

			const char *const name = inGameText[i];

			const bool selected = i == selectedIndex;

			draw_font_hv_shadow(VGAScreen, xMenuItemName, y, name, normal_font, left_aligned, 15, -4 + (selected ? 2 : 0), false, 2);

			switch (i)
			{
			case MENU_ITEM_MUSIC_VOLUME:
			{
				JE_barDrawShadow(VGAScreen, xMenuItemValue, y, 1, music_disabled ? 12 : 16, (tyrMusicVolume + 6) / 12, 3, 13);
				break;
			}
			case MENU_ITEM_EFFECTS_VOLUME:
			{
				JE_barDrawShadow(VGAScreen, xMenuItemValue, y, 1, samples_disabled ? 12 : 16, (fxVolume + 6) / 12, 3, 13);
				break;
			}
			case MENU_ITEM_DETAIL_LEVEL:
			{
				draw_font_hv_shadow(VGAScreen, xMenuItemValue, y, detailLevel[processorType-1], normal_font, left_aligned, 15, -4 + (selected ? 2 : 0), false, 2);
				break;
			}
			case MENU_ITEM_GAME_SPEED:
			{
				draw_font_hv_shadow(VGAScreen, xMenuItemValue, y, gameSpeedText[gameSpeed-1], normal_font, left_aligned, 15, -4 + (selected ? 2 : 0), false, 2);
				break;
			}
			}
		}

		// Draw help text.
		JE_outTextAdjust(VGAScreen, 10, 147, mainMenuHelp[helpIndexes[selectedIndex]], 14, 6, TINY_FONT, true);

		service_SDL_events(true);

		JE_mouseStart();
		JE_showVGA();
		JE_mouseReplace();

		bool mouseMoved = false;
		do
		{
			SDL_Delay(16);

			Uint16 oldMouseX = mouse_x;
			Uint16 oldMouseY = mouse_y;

			push_joysticks_as_keyboard();
			service_SDL_events(false);

			NETWORK_KEEP_ALIVE();

			mouseMoved = mouse_x != oldMouseX || mouse_y != oldMouseY;
		} while (!(newkey || newmouse || mouseMoved));

		// Handle interaction.

		bool action = false;
		bool leftAction = false;
		bool rightAction = false;

		if (mouseMoved || newmouse)
		{
			// Find menu item that was hovered or clicked.
			if (mouse_x >= xMenuItem && mouse_x < xMenuItem + wMenuItem)
			{
				for (size_t i = 0; i < menuItemsCount; ++i)
				{
					const int yMenuItem = yMenuItems + dyMenuItems * i;
					if (mouse_y >= yMenuItem && mouse_y < yMenuItem + hMenuItem)
					{
						if (selectedIndex != i)
						{
							JE_playSampleNum(S_CURSOR);

							selectedIndex = i;
						}

						if (newmouse && lastmouse_but == SDL_BUTTON_LEFT &&
						    lastmouse_x >= xMenuItem && lastmouse_x < xMenuItem + wMenuItem &&
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
								switch (i)
								{
								case MENU_ITEM_MUSIC_VOLUME:
								{
									JE_playSampleNum(S_CURSOR);

									const int w = ((255 + 6) / 12) * (3 + 1) - 1;

									int value = (lastmouse_x - xMenuItemValue) * 255 / (w - 1);
									tyrMusicVolume = MIN(MAX(0, value), 255);

									set_volume(tyrMusicVolume, fxVolume);
									break;
								}
								case MENU_ITEM_EFFECTS_VOLUME:
								{
									const int w = ((255 + 6) / 12) * (3 + 1) - 1;

									int value = (lastmouse_x - xMenuItemValue) * 255 / (w - 1);
									fxVolume = MIN(MAX(0, value), 255);

									set_volume(tyrMusicVolume, fxVolume);

									JE_playSampleNum(S_CURSOR);
									break;
								}
								case MENU_ITEM_DETAIL_LEVEL:
								case MENU_ITEM_GAME_SPEED:
								{
									rightAction = true;
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

				done = true;
			}
		}
		else if (newkey)
		{
			switch (lastkey_scan)
			{
			case SDL_SCANCODE_UP:
			{
				JE_playSampleNum(S_CURSOR);

				selectedIndex = selectedIndex == 0
					? menuItemsCount - 1
					: selectedIndex - 1;
				break;
			}
			case SDL_SCANCODE_DOWN:
			{
				JE_playSampleNum(S_CURSOR);

				selectedIndex = selectedIndex == menuItemsCount - 1
					? 0
					: selectedIndex + 1;
				break;
			}
			case SDL_SCANCODE_LEFT:
			{
				leftAction = true;
				break;
			}
			case SDL_SCANCODE_RIGHT:
			{
				rightAction = true;
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

				done = true;
				break;
			}
			case SDL_SCANCODE_W:
			{
				if (selectedIndex == MENU_ITEM_DETAIL_LEVEL)
				{
					processorType = 6;
					JE_initProcessorType();
				}
				break;
			}
			default:
				break;
			}
		}

		if (action)
		{
			switch (selectedIndex)
			{
			case MENU_ITEM_MUSIC_VOLUME:
			{
				JE_playSampleNum(S_SELECT);

				music_disabled = !music_disabled;
				break;
			}
			case MENU_ITEM_EFFECTS_VOLUME:
			{
				samples_disabled = !samples_disabled;

				JE_playSampleNum(S_SELECT);
				break;
			}
			case MENU_ITEM_RETURN_TO_GAME:
			{
				JE_playSampleNum(S_SELECT);

				done = true;
				break;
			}
			case MENU_ITEM_QUIT:
			{
				JE_playSampleNum(S_SELECT);

				if (constantPlay)
					JE_tyrianHalt(0);

				if (isNetworkGame)
				{
					/*Tell other computer to exit*/
					haltGame = true;
					playerEndLevel = true;
				}

				result = true;
				done = true;
				break;
			}
			default:
				break;
			}
		}
		else if (leftAction)
		{
			switch (selectedIndex)
			{
			case MENU_ITEM_MUSIC_VOLUME:
			{
				JE_playSampleNum(S_CURSOR);

				JE_changeVolume(&tyrMusicVolume, -12, &fxVolume, 0);
				break;
			}
			case MENU_ITEM_EFFECTS_VOLUME:
			{
				JE_changeVolume(&tyrMusicVolume, 0, &fxVolume, -12);

				JE_playSampleNum(S_CURSOR);
				break;
			}
			case MENU_ITEM_DETAIL_LEVEL:
			{
				JE_playSampleNum(S_CURSOR);

				processorType = processorType > 1 ? processorType - 1 : 4;
				JE_initProcessorType();
				JE_setNewGameSpeed();
				break;
			}
			case MENU_ITEM_GAME_SPEED:
			{
				JE_playSampleNum(S_CURSOR);

				gameSpeed = gameSpeed > 1 ? gameSpeed - 1 : 5;
				JE_initProcessorType();
				JE_setNewGameSpeed();
				break;
			}
			default:
				break;
			}
		}
		else if (rightAction)
		{
			switch (selectedIndex)
			{
			case MENU_ITEM_MUSIC_VOLUME:
			{
				JE_playSampleNum(S_CURSOR);

				JE_changeVolume(&tyrMusicVolume, 12, &fxVolume, 0);
				break;
			}
			case MENU_ITEM_EFFECTS_VOLUME:
			{
				JE_changeVolume(&tyrMusicVolume, 0, &fxVolume, 12);

				JE_playSampleNum(S_CURSOR);
				break;
			}
			case MENU_ITEM_DETAIL_LEVEL:
			{
				JE_playSampleNum(S_CURSOR);

				processorType = processorType < 4 ? processorType + 1 : 1;
				JE_initProcessorType();
				JE_setNewGameSpeed();
				break;
			}
			case MENU_ITEM_GAME_SPEED:
			{
				JE_playSampleNum(S_CURSOR);

				gameSpeed = gameSpeed < 5 ? gameSpeed + 1 : 1;
				JE_initProcessorType();
				JE_setNewGameSpeed();
				break;
			}
			default:
				break;
			}
		}
	}

	VGAScreen = temp_surface; /* side-effect of game_screen */

	return result;
}

void JE_inGameHelp(void)
{
	SDL_Surface *temp_surface = VGAScreen;
	VGAScreen = VGAScreenSeg; /* side-effect of game_screen */

	//tempScreenSeg = VGAScreenSeg;

	JE_clearKeyboard();
	JE_wipeKey();

	JE_barShade(VGAScreen, 1, 1, 262, 182); /*Main Box*/
	JE_barShade(VGAScreen, 3, 3, 260, 180);
	JE_barShade(VGAScreen, 5, 5, 258, 178);
	JE_barShade(VGAScreen, 7, 7, 256, 176);
	fill_rectangle_xy(VGAScreen, 9, 9, 254, 174, 0);

	if (twoPlayerMode)  // Two-Player Help
	{
		helpBoxColor = 3;
		helpBoxBrightness = 3;
		JE_HBox(VGAScreen, 20,  4, 36, 50);

		// weapon help
		blit_sprite(VGAScreenSeg, 2, 21, OPTION_SHAPES, 43);
		helpBoxColor = 5;
		helpBoxBrightness = 3;
		JE_HBox(VGAScreen, 55, 20, 37, 40);

		// sidekick help
		blit_sprite(VGAScreenSeg, 5, 36, OPTION_SHAPES, 41);
		helpBoxColor = 5;
		helpBoxBrightness = 3;
		JE_HBox(VGAScreen, 40, 43, 34, 44);

		// shield/armor help
		blit_sprite(VGAScreenSeg, 2, 79, OPTION_SHAPES, 42);
		helpBoxColor = 5;
		helpBoxBrightness = 3;
		JE_HBox(VGAScreen, 54, 84, 35, 40);

		helpBoxColor = 5;
		helpBoxBrightness = 3;
		JE_HBox(VGAScreen, 5, 126, 38, 55);
		helpBoxColor = 5;
		helpBoxBrightness = 3;
		JE_HBox(VGAScreen, 5, 160, 39, 55);
	}
	else
	{
		// power bar help
		blit_sprite(VGAScreenSeg, 15, 5, OPTION_SHAPES, 40);
		helpBoxColor = 5;
		helpBoxBrightness = 3;
		JE_HBox(VGAScreen, 40, 10, 31, 45);

		// weapon help
		blit_sprite(VGAScreenSeg, 5, 37, OPTION_SHAPES, 39);
		helpBoxColor = 5;
		helpBoxBrightness = 3;
		JE_HBox(VGAScreen, 40, 40, 32, 44);
		helpBoxColor = 5;
		helpBoxBrightness = 3;
		JE_HBox(VGAScreen, 40, 60, 33, 44);

		// sidekick help
		blit_sprite(VGAScreenSeg, 5, 98, OPTION_SHAPES, 41);
		helpBoxColor = 5;
		helpBoxBrightness = 3;
		JE_HBox(VGAScreen, 40, 103, 34, 44);

		// shield/armor help
		blit_sprite(VGAScreenSeg, 2, 138, OPTION_SHAPES, 42);
		helpBoxColor = 5;
		helpBoxBrightness = 3;
		JE_HBox(VGAScreen, 54, 143, 35, 40);
	}

	// "press a key"
	blit_sprite(VGAScreenSeg, 16, 189, OPTION_SHAPES, 36);  // in-game text area
	JE_outText(VGAScreenSeg, 120 - JE_textWidth(miscText[5-1], TINY_FONT) / 2 + 20, 190, miscText[5-1], 0, 4);

	do
	{
		service_SDL_events(true);

		JE_mouseStart();
		JE_showVGA();
		JE_mouseReplace();

		SDL_Delay(16);

		push_joysticks_as_keyboard();
		service_SDL_events(false);

		NETWORK_KEEP_ALIVE();
	} while (!(newkey || newmouse));

	textErase = 1;

	VGAScreen = temp_surface;
}

void JE_highScoreCheck(void)
{
	if (shopSpriteSheet.data == NULL)
		JE_loadCompShapes(&shopSpriteSheet, '1');  // need mouse pointer sprite

	Sint32 temp_score;

	for (int temp_p = 0; temp_p < (twoPlayerMode ? 2 : 1); ++temp_p)
	{
		JE_sortHighScores();

		int p = temp_p;

		if (twoPlayerMode)
		{
			// ask for the highest scorer first
			if (player[0].cash < player[1].cash)
				p = (temp_p == 0) ? 1 : 0;

			temp_score = (p == 0) ? player[0].cash : player[1].cash;
		}
		else
		{
			// single player highscore includes cost of upgrades
			temp_score = JE_totalScore(&player[0]);
		}

		int slot;
		const int first_slot = (initial_episode_num - 1) * 6 + (twoPlayerMode ? 3 : 0),
		          slot_limit = first_slot + 3;

		for (slot = first_slot; slot < slot_limit; ++slot)
		{
			if (temp_score > saveFiles[slot].highScore1)
				break;
		}

		// did you get a high score?
		if (slot < slot_limit)
		{
			// shift down old scores
			for (int i = slot_limit - 1; i > slot; --i)
			{
				saveFiles[i].highScore1 = saveFiles[i - 1].highScore1;
				strcpy(saveFiles[i].highScoreName, saveFiles[i - 1].highScoreName);
			}

			wait_noinput(false, true, false);

			JE_clr256(VGAScreen);
			JE_showVGA();
			memcpy(colors, palettes[0], sizeof(colors));

			play_song(33);

			{
				/* Enter Thy name */

				JE_byte flash = 8 * 16 + 10;
				JE_boolean fadein = true;
				JE_boolean quit = false, cancel = false;
				char stemp[30], tempstr[30];
				char buffer[256];

				strcpy(stemp, "                             ");
				temp = 0;

				JE_barShade(VGAScreen, 65, 55, 255, 155);

				do
				{
					service_SDL_events(true);

					JE_dString(VGAScreen, JE_fontCenter(miscText[51], FONT_SHAPES), 3, miscText[51], FONT_SHAPES);

					temp3 = twoPlayerMode ? 58 + p : 53;

					JE_dString(VGAScreen, JE_fontCenter(miscText[temp3-1], SMALL_FONT_SHAPES), 30, miscText[temp3-1], SMALL_FONT_SHAPES);

					blit_sprite(VGAScreenSeg, 50, 50, OPTION_SHAPES, 35);  // message box

					if (twoPlayerMode)
					{
						sprintf(buffer, "%s %s", miscText[48 + p], miscText[53]);
						JE_textShade(VGAScreen, 60, 55, buffer, 11, 4, FULL_SHADE);
					}
					else
					{
						JE_textShade(VGAScreen, 60, 55, miscText[53], 11, 4, FULL_SHADE);
					}

					sprintf(buffer, "%s %d", miscText[37], temp_score);
					JE_textShade(VGAScreen, 70, 70, buffer, 11, 4, FULL_SHADE);

					do
					{
						flash = (flash == 8 * 16 + 10) ? 8 * 16 + 2 : 8 * 16 + 10;
						temp3 = (temp3 == 6) ? 2 : 6;

						strncpy(tempstr, stemp, temp);
						tempstr[temp] = '\0';
						JE_outText(VGAScreen, 65, 89, tempstr, 8, 3);
						tempW = 65 + JE_textWidth(tempstr, TINY_FONT);
						JE_barShade(VGAScreen, tempW + 2, 90, tempW + 6, 95);
						fill_rectangle_xy(VGAScreen, tempW + 1, 89, tempW + 5, 94, flash);

						for (int i = 0; i < 14; i++)
						{
							setDelay(1);

							JE_mouseStart();
							JE_showVGA();
							if (fadein)
							{
								fade_palette(colors, 15, 0, 255);
								fadein = false;
							}
							JE_mouseReplace();

							push_joysticks_as_keyboard();
							service_wait_delay();

							if (newkey || newmouse)
								break;
						}

					} while (!newkey && !newmouse && !new_text);

					if (!playing)
						play_song(31);

					if (mouseButton > 0)
					{
						if (mouseX > 56 && mouseX < 142 && mouseY > 123 && mouseY < 149)
						{
							quit = true;
						}
						else if (mouseX > 151 && mouseX < 237 && mouseY > 123 && mouseY < 149)
						{
							quit = true;
							cancel = true;
						}
					}
					else if (new_text)
					{
						for (size_t ti = 0U; last_text[ti] != '\0'; ++ti)
						{
							const char c = (unsigned char)last_text[ti] <= 127U ? toupper(last_text[ti]) : 0;
							if ((c == ' ' || font_ascii[(unsigned char)c] != -1) &&
							    temp < 28)
							{
								stemp[temp] = c;
								temp += 1;
							}
						}
					}
					else if (newkey)
					{
						switch (lastkey_scan)
						{
							case SDL_SCANCODE_BACKSPACE:
							case SDL_SCANCODE_DELETE:
								if (temp)
								{
									temp--;
									stemp[temp] = ' ';
								}
								break;
							case SDL_SCANCODE_ESCAPE:
								quit = true;
								cancel = true;
								break;
							case SDL_SCANCODE_RETURN:
								quit = true;
								break;
							default:
								break;
						}
					}
				} while (!quit);

				if (!cancel)
				{
					saveFiles[slot].highScore1 = temp_score;
					strcpy(saveFiles[slot].highScoreName, stemp);
					saveFiles[slot].highScoreDiff = difficultyLevel;
				}

				fade_black(15);
				JE_loadPic(VGAScreen, 2, false);

				JE_dString(VGAScreen, JE_fontCenter(miscText[50], FONT_SHAPES), 10, miscText[50], FONT_SHAPES);
				JE_dString(VGAScreen, JE_fontCenter(episode_name[episodeNum], SMALL_FONT_SHAPES), 35, episode_name[episodeNum], SMALL_FONT_SHAPES);

				for (int i = first_slot; i < slot_limit; ++i)
				{
					if (i != slot)
					{
						sprintf(buffer, "~#%d:~  %d", (i - first_slot + 1), saveFiles[i].highScore1);
						JE_textShade(VGAScreen,  20, ((i - first_slot + 1) * 12) + 65, buffer, 15, 0, FULL_SHADE);
						JE_textShade(VGAScreen, 150, ((i - first_slot + 1) * 12) + 65, saveFiles[i].highScoreName, 15, 2, FULL_SHADE);
					}
				}

				JE_showVGA();

				fade_palette(colors, 15, 0, 255);

				sprintf(buffer, "~#%d:~  %d", (slot - first_slot + 1), saveFiles[slot].highScore1);

				frameCountMax = 6;
				textGlowFont = TINY_FONT;

				textGlowBrightness = 10;
				JE_outTextGlow(VGAScreenSeg,  20, (slot - first_slot + 1) * 12 + 65, buffer);
				textGlowBrightness = 10;
				JE_outTextGlow(VGAScreenSeg, 150, (slot - first_slot + 1) * 12 + 65, saveFiles[slot].highScoreName);
				textGlowBrightness = 10;
				JE_outTextGlow(VGAScreenSeg, JE_fontCenter(miscText[4], TINY_FONT), 180, miscText[4]);

				JE_showVGA();

				if (frameCountMax != 0)
					wait_input(true, true, true);

				fade_black(15);
			}

		}
	}
}

// increases game difficulty based on player's total score / total of players' scores
void adjust_difficulty(void)
{
	const float score_multiplier[10] =
	{
		0,     // Wimp  (doesn't exist)
		0.4f,  // Easy
		0.8f,  // Normal
		1.3f,  // Hard
		1.6f,  // Impossible
		2,     // Insanity
		2,     // Suicide
		3,     // Maniacal
		3,     // Zinglon
		3,     // Nortaneous
	};

	assert(initialDifficulty > 0 && initialDifficulty < 10);

	const ulong score = twoPlayerMode ? (player[0].cash + player[1].cash) : JE_totalScore(&player[0]),
	            adjusted_score = roundf(score * score_multiplier[initialDifficulty]);

	uint new_difficulty = 0;

	if (twoPlayerMode)
	{
		if (adjusted_score < 10000)
			new_difficulty = DIFFICULTY_EASY;
		else if (adjusted_score < 20000)
			new_difficulty = DIFFICULTY_NORMAL;
		else if (adjusted_score < 50000)
			new_difficulty = DIFFICULTY_HARD;
		else if (adjusted_score < 80000)
			new_difficulty = DIFFICULTY_IMPOSSIBLE;
		else if (adjusted_score < 125000)
			new_difficulty = DIFFICULTY_INSANITY;
		else if (adjusted_score < 200000)
			new_difficulty = DIFFICULTY_SUICIDE;
		else if (adjusted_score < 400000)
			new_difficulty = DIFFICULTY_MANIACAL;
		else if (adjusted_score < 600000)
			new_difficulty = DIFFICULTY_ZINGLON;
		else
			new_difficulty = DIFFICULTY_NORTANEOUS;
	}
	else
	{
		if (adjusted_score < 40000)
			new_difficulty = DIFFICULTY_EASY;
		else if (adjusted_score < 70000)
			new_difficulty = DIFFICULTY_NORMAL;
		else if (adjusted_score < 150000)
			new_difficulty = DIFFICULTY_HARD;
		else if (adjusted_score < 300000)
			new_difficulty = DIFFICULTY_IMPOSSIBLE;
		else if (adjusted_score < 600000)
			new_difficulty = DIFFICULTY_INSANITY;
		else if (adjusted_score < 1000000)
			new_difficulty = DIFFICULTY_SUICIDE;
		else if (adjusted_score < 2000000)
			new_difficulty = DIFFICULTY_MANIACAL;
		else if (adjusted_score < 3000000)
			new_difficulty = DIFFICULTY_ZINGLON;
		else
			new_difficulty = DIFFICULTY_NORTANEOUS;
	}

	difficultyLevel = MAX((unsigned)difficultyLevel, new_difficulty);
}

bool load_next_demo(void)
{
	if (++demo_num > 5)
		demo_num = 1;

	char demo_filename[9];
	snprintf(demo_filename, sizeof(demo_filename), "demo.%d", demo_num);
	demo_file = dir_fopen_die(data_dir(), demo_filename, "rb"); // TODO: only play demos from existing file (instead of dying)

	difficultyLevel = DIFFICULTY_NORMAL;
	bonusLevelCurrent = false;

	Uint8 temp;
	fread_u8_die(&temp, 1, demo_file);
	JE_initEpisode(temp);

	fread_die(levelName, 1, 10, demo_file);
	levelName[10] = '\0';

	fread_u8_die(&lvlFileNum, 1, demo_file);

	fread_u8_die(&player[0].items.weapon[FRONT_WEAPON].id,  1, demo_file);
	fread_u8_die(&player[0].items.weapon[REAR_WEAPON].id,   1, demo_file);
	fread_u8_die(&player[0].items.super_arcade_mode,        1, demo_file);
	fread_u8_die(&player[0].items.sidekick[LEFT_SIDEKICK],  1, demo_file);
	fread_u8_die(&player[0].items.sidekick[RIGHT_SIDEKICK], 1, demo_file);
	fread_u8_die(&player[0].items.generator,                1, demo_file);

	fread_u8_die(&player[0].items.sidekick_level,           1, demo_file); // could probably ignore
	fread_u8_die(&player[0].items.sidekick_series,          1, demo_file); // could probably ignore

	fread_u8_die(&initial_episode_num,                      1, demo_file); // could probably ignore

	fread_u8_die(&player[0].items.shield,                   1, demo_file);
	fread_u8_die(&player[0].items.special,                  1, demo_file);
	fread_u8_die(&player[0].items.ship,                     1, demo_file);

	for (uint i = 0; i < 2; ++i)
		fread_u8_die(&player[0].items.weapon[i].power,      1, demo_file);

	Uint8 unused[3];
	fread_u8_die(unused, 3, demo_file);

	fread_u8_die(&levelSong, 1, demo_file);

	demo_keys = 0;

	Uint8 temp2[2] = { 0, 0 };
	fread_u8(temp2, 2, demo_file);
	demo_keys_wait = (temp2[0] << 8) | temp2[1];

	printf("loaded demo '%s'\n", demo_filename);

	return true;
}

bool replay_demo_keys(void)
{
	while (demo_keys_wait == 0)
	{
		demo_keys = 0;
		fread_u8(&demo_keys, 1, demo_file);

		Uint8 temp2[2] = { 0, 0 };
		fread_u8(temp2, 2, demo_file);
		demo_keys_wait = (temp2[0] << 8) | temp2[1];

		if (feof(demo_file))
		{
			// no more keys
			return false;
		}
	}

	demo_keys_wait--;

	if (demo_keys & (1 << 0))
		player[0].y -= CURRENT_KEY_SPEED;
	if (demo_keys & (1 << 1))
		player[0].y += CURRENT_KEY_SPEED;

	if (demo_keys & (1 << 2))
		player[0].x -= CURRENT_KEY_SPEED;
	if (demo_keys & (1 << 3))
		player[0].x += CURRENT_KEY_SPEED;

	button[0] = (bool)(demo_keys & (1 << 4));
	button[3] = (bool)(demo_keys & (1 << 5));
	button[1] = (bool)(demo_keys & (1 << 6));
	button[2] = (bool)(demo_keys & (1 << 7));

	return true;
}

/*Street Fighter codes*/
void JE_SFCodes(JE_byte playerNum_, JE_integer PX_, JE_integer PY_, JE_integer mouseX_, JE_integer mouseY_)
{
	JE_byte temp, temp2, temp3, temp4, temp5;

	uint ship = player[playerNum_-1].items.ship;

	/*Get direction*/
	if (playerNum_ == 2 && ship < 15)
	{
		ship = 0;
	}

	if (ship < 15)
	{

		temp2 = (mouseY_ > PY_) +    /*UP*/
		        (mouseY_ < PY_) +    /*DOWN*/
		        (PX_ < mouseX_) +    /*LEFT*/
		        (PX_ > mouseX_);     /*RIGHT*/
		temp = (mouseY_ > PY_) * 1 + /*UP*/
		       (mouseY_ < PY_) * 2 + /*DOWN*/
		       (PX_ < mouseX_) * 3 + /*LEFT*/
		       (PX_ > mouseX_) * 4;  /*RIGHT*/

		if (temp == 0) // no direction being pressed
		{
			if (!button[0]) // if fire button is released
			{
				temp = 9;
				temp2 = 1;
			}
			else
			{
				temp2 = 0;
				temp = 99;
			}
		}

		if (temp2 == 1) // if exactly one direction pressed or fire button is released
		{
			temp += button[0] * 4;

			temp3 = superTyrian ? 21 : 3;
			for (temp2 = 0; temp2 < temp3; temp2++)
			{

				/*Use SuperTyrian ShipCombos or not?*/
				temp5 = superTyrian ? shipCombosB[temp2] : shipCombos[ship][temp2];

				// temp5 == selected combo in ship
				if (temp5 == 0) /* combo doesn't exists */
				{
					// mark twiddles as cancelled/finished
					SFCurrentCode[playerNum_-1][temp2] = 0;
				}
				else
				{
					// get next combo key
					temp4 = keyboardCombos[temp5-1][SFCurrentCode[playerNum_-1][temp2]];

					// correct key
					if (temp4 == temp)
					{
						SFCurrentCode[playerNum_-1][temp2]++;

						temp4 = keyboardCombos[temp5-1][SFCurrentCode[playerNum_-1][temp2]];
						if (temp4 > 100 && temp4 <= 100 + SPECIAL_NUM)
						{
							SFCurrentCode[playerNum_-1][temp2] = 0;
							SFExecuted[playerNum_-1] = temp4 - 100;
						}
					}
					else
					{
						if ((temp != 9) &&
						    (temp4 - 1) % 4 != (temp - 1) % 4 &&
						    (SFCurrentCode[playerNum_-1][temp2] == 0 ||
						     keyboardCombos[temp5-1][SFCurrentCode[playerNum_-1][temp2]-1] != temp))
						{
							SFCurrentCode[playerNum_-1][temp2] = 0;
						}
					}
				}
			}
		}

	}
}

void JE_sort(void)
{
	JE_byte a, b;

	for (a = 0; a < 2; a++)
	{
		for (b = a + 1; b < 3; b++)
		{
			if (saveFiles[temp + a].highScore1 < saveFiles[temp + b].highScore1)
			{
				JE_longint tempLI;
				char tempStr[30];
				JE_byte tempByte;

				tempLI = saveFiles[temp + a].highScore1;
				saveFiles[temp + a].highScore1 = saveFiles[temp + b].highScore1;
				saveFiles[temp + b].highScore1 = tempLI;

				strcpy(tempStr, saveFiles[temp + a].highScoreName);
				strcpy(saveFiles[temp + a].highScoreName, saveFiles[temp + b].highScoreName);
				strcpy(saveFiles[temp + b].highScoreName, tempStr);

				tempByte = saveFiles[temp + a].highScoreDiff;
				saveFiles[temp + a].highScoreDiff = saveFiles[temp + b].highScoreDiff;
				saveFiles[temp + b].highScoreDiff = tempByte;
			}
		}
	}
}

void JE_playCredits(void)
{
	enum { lines_max = 131 };
	enum { line_max_length = 65 };

	char credstr[lines_max][line_max_length + 1];

	int lines = 0;

	JE_byte currentpic = 0, fade = 0;
	JE_shortint fadechg = 1;
	JE_byte currentship = 0;
	JE_integer shipx = 0, shipxwait = 0;
	JE_shortint shipxc = 0, shipxca = 0;

	load_sprites_file(EXTRA_SHAPES, "estsc.shp");

	setDelay2(1000);

	play_song(8);

	// load credits text
	FILE *f = dir_fopen_die(data_dir(), "tyrian.cdt", "rb");
	for (lines = 0; lines < lines_max; ++lines)
	{
		read_encrypted_pascal_string(credstr[lines], sizeof(credstr[lines]), f);
	}
	fclose(f);

	memcpy(colors, palettes[6-1], sizeof(colors));
	JE_clr256(VGAScreen);
	JE_showVGA();
	fade_palette(colors, 2, 0, 255);

	//tempScreenSeg = VGAScreenSeg;

	const int ticks_max = lines * 20 * 3;
	for (int ticks = 0; ticks < ticks_max; ++ticks)
	{
		setDelay(1);
		JE_clr256(VGAScreen);

		blit_sprite_hv(VGAScreenSeg, 319 - sprite(EXTRA_SHAPES, currentpic)->width, 100 - (sprite(EXTRA_SHAPES, currentpic)->height / 2), EXTRA_SHAPES, currentpic, 0x0, fade - 15);

		fade += fadechg;
		if (fade == 0 && fadechg == -1)
		{
			fadechg = 1;
			++currentpic;
			if (currentpic >= sprite_table[EXTRA_SHAPES].count)
				currentpic = 0;
		}
		if (fade == 15)
			fadechg = 0;

		if (getDelayTicks2() == 0)
		{
			fadechg = -1;
			setDelay2(900);
		}

		if (ticks % 200 == 0)
		{
			currentship = (mt_rand() % 11) + 1;
			shipxwait = (mt_rand() % 80) + 10;
			if ((mt_rand() % 2) == 1)
			{
				shipx = 1;
				shipxc = 0;
				shipxca = 1;
			}
			else
			{
				shipx = 900;
				shipxc = 0;
				shipxca = -1;
			}
		}

		shipxwait--;
		if (shipxwait == 0)
		{
			if (shipx == 1 || shipx == 900)
				shipxc = 0;
			shipxca = -shipxca;
			shipxwait = (mt_rand() % 40) + 15;
		}
		shipxc += shipxca;
		shipx += shipxc;
		if (shipx < 1)
		{
			shipx = 1;
			shipxwait = 1;
		}
		if (shipx > 900)
		{
			shipx = 900;
			shipxwait = 1;
		}
		int tmp_unknown = shipxc * shipxc;
		if (450 + tmp_unknown < 0 || 450 + tmp_unknown > 900)
		{
			if (shipxca < 0 && shipxc < 0)
				shipxwait = 1;
			if (shipxca > 0 && shipxc > 0)
				shipxwait = 1;
		}

		uint ship_sprite = ships[currentship].shipgraphic;
		if (shipxc < -10)
			ship_sprite -= (shipxc < -20) ? 4 : 2;
		else if (shipxc > 10)
			ship_sprite += (shipxc > 20) ? 4 : 2;

		blit_sprite2x2(VGAScreen, shipx / 40, 184 - (ticks % 200), spriteSheet9, ship_sprite);

		const int bottom_line = (ticks / 3) / 20;
		int y = 20 - ((ticks / 3) % 20);

		for (int line = bottom_line - 10; line < bottom_line; ++line)
		{
			if (line >= 0 && line < lines_max)
			{
				if (strcmp(&credstr[line][0], ".") != 0 && strlen(credstr[line]))
				{
					const Uint8 color = credstr[line][0] - 65;
					const char *text = &credstr[line][1];

					const int x = 110 - JE_textWidth(text, SMALL_FONT_SHAPES) / 2;

					JE_outTextAdjust(VGAScreen, x + abs((y / 18) % 4 - 2) - 1, y - 1, text, color, -8, SMALL_FONT_SHAPES, false);
					JE_outTextAdjust(VGAScreen, x,                             y,     text, color, -2, SMALL_FONT_SHAPES, false);
				}
			}

			y += 20;
		}

		fill_rectangle_xy(VGAScreen, 0,  0, 319, 10, 0);
		fill_rectangle_xy(VGAScreen, 0, 190, 319, 199, 0);

		if (currentpic == sprite_table[EXTRA_SHAPES].count - 1)
			JE_outTextAdjust(VGAScreen, 5, 180, miscText[54], 2, -2, SMALL_FONT_SHAPES, false);  // levels-in-episode

		if (bottom_line == lines_max - 8)
			fade_song();

		if (ticks == ticks_max - 1)
		{
			--ticks;
			play_song(9);
		}

		NETWORK_KEEP_ALIVE();

		JE_showVGA();

		wait_delay();

		if (JE_anyButton())
			break;
	}

	fade_black(10);

	free_sprites(EXTRA_SHAPES);
}

void JE_endLevelAni(void)
{
	JE_word x, y;
	JE_byte temp;
	char tempStr[256];

	Sint8 i;

	if (!constantPlay)
	{
		// grant shipedit privileges

		// special
		if (player[0].items.special < 21)
			saveTemp[SAVE_FILES_SIZE + 81 + player[0].items.special] = 1;

		for (uint p = 0; p < COUNTOF(player); ++p)
		{
			// front, rear
			for (uint i = 0; i < COUNTOF(player[p].items.weapon); ++i)
				saveTemp[SAVE_FILES_SIZE + player[p].items.weapon[i].id] = 1;

			// options
			for (uint i = 0; i < COUNTOF(player[p].items.sidekick); ++i)
				saveTemp[SAVE_FILES_SIZE + 51 + player[p].items.sidekick[i]] = 1;
		}
	}

	adjust_difficulty();

	player[0].last_items = player[0].items;
	strcpy(lastLevelName, levelName);

	JE_wipeKey();
	frameCountMax = 4;
	textGlowFont = SMALL_FONT_SHAPES;

	SDL_Color white = { 255, 255, 255 };
	set_colors(white, 254, 254);

	if (!levelTimer || levelTimerCountdown > 0 || !(episodeNum == 4))
		JE_playSampleNum(V_LEVEL_END);
	else
		play_song(21);

	if (bonusLevel)
	{
		JE_outTextGlow(VGAScreenSeg, 20, 20, miscText[17-1]);
	}
	else if (all_players_alive())
	{
		sprintf(tempStr, "%s %s", miscText[27-1], levelName); // "Completed"
		JE_outTextGlow(VGAScreenSeg, 20, 20, tempStr);
	}
	else
	{
		sprintf(tempStr, "%s %s", miscText[62-1], levelName); // "Exiting"
		JE_outTextGlow(VGAScreenSeg, 20, 20, tempStr);
	}

	if (twoPlayerMode)
	{
		for (uint i = 0; i < 2; ++i)
		{
			snprintf(tempStr, sizeof(tempStr), "%s %lu", miscText[40 + i], player[i].cash);
			JE_outTextGlow(VGAScreenSeg, 30, 50 + 20 * i, tempStr);
		}
	}
	else
	{
		sprintf(tempStr, "%s %lu", miscText[28-1], player[0].cash);
		JE_outTextGlow(VGAScreenSeg, 30, 50, tempStr);
	}

	temp = (totalEnemy == 0) ? 0 : roundf(enemyKilled * 100 / totalEnemy);
	sprintf(tempStr, "%s %d%%", miscText[63-1], temp);
	JE_outTextGlow(VGAScreenSeg, 40, 90, tempStr);

	if (!constantPlay)
		editorLevel += temp / 5;

	if (!onePlayerAction && !twoPlayerMode)
	{
		JE_outTextGlow(VGAScreenSeg, 30, 120, miscText[4-1]);   /*Cubes*/

		if (cubeMax > 0)
		{
			if (cubeMax > 4)
				cubeMax = 4;

			if (frameCountMax != 0)
				frameCountMax = 1;

			for (temp = 1; temp <= cubeMax; temp++)
			{
				NETWORK_KEEP_ALIVE();

				JE_playSampleNum(S_ITEM);
				x = 20 + 30 * temp;
				y = 135;
				JE_drawCube(VGAScreenSeg, x, y, 9, 0);
				JE_showVGA();

				for (i = -15; i <= 10; i++)
				{
					setDelay(frameCountMax);

					blit_sprite_hv(VGAScreenSeg, x, y, OPTION_SHAPES, 25, 0x9, i);

					if (JE_anyButton())
						frameCountMax = 0;

					JE_showVGA();

					wait_delay();
				}
				for (i = 10; i >= 0; i--)
				{
					setDelay(frameCountMax);

					blit_sprite_hv(VGAScreenSeg, x, y, OPTION_SHAPES, 25, 0x9, i);

					if (JE_anyButton())
						frameCountMax = 0;

					JE_showVGA();

					wait_delay();
				}
			}
		}
		else
		{
			JE_outTextGlow(VGAScreenSeg, 50, 135, miscText[15-1]);
		}

	}

	if (frameCountMax != 0)
	{
		frameCountMax = 6;
		temp = 1;
	}
	else
	{
		temp = 0;
	}
	temp2 = twoPlayerMode ? 150 : 160;
	JE_outTextGlow(VGAScreenSeg, 90, temp2, miscText[5-1]);

	if (!constantPlay)
	{
		do
		{
			setDelay(1);

			NETWORK_KEEP_ALIVE();

			wait_delay();
		} while (!(JE_anyButton() || (frameCountMax == 0 && temp == 1)));
	}

	wait_noinput(false, false, true); // TODO: should up the joystick repeat temporarily instead

	fade_black(15);
	JE_clr256(VGAScreen);
}

void JE_drawCube(SDL_Surface * screen, JE_word x, JE_word y, JE_byte filter, JE_byte brightness)
{
	blit_sprite_dark(screen, x + 4, y + 4, OPTION_SHAPES, 25, false);
	blit_sprite_dark(screen, x + 3, y + 3, OPTION_SHAPES, 25, false);
	blit_sprite_hv(screen, x, y, OPTION_SHAPES, 25, filter, brightness);
}

void JE_handleChat(void)
{
	// STUB(); Annoying piece of crap =P
}

bool str_pop_int(char *str, int *val)
{
	bool success = false;

	char buf[256];
	assert(strlen(str) < sizeof(buf));

	// grab the value from str
	char *end;
	*val = strtol(str, &end, 10);

	if (end != str)
	{
		success = true;

		// shift the rest to the beginning
		strcpy(buf, end);
		strcpy(str, buf);
	}

	return success;
}

void JE_operation(JE_byte slot)
{
	JE_byte flash;
	char stemp[21];
	char tempStr[51];

	if (!performSave)
	{
		if (saveFiles[slot-1].level > 0)
		{
			gameJustLoaded = true;
			JE_loadGame(slot);
			gameLoaded = true;
		}
	}
	else if (slot % 11 != 0)
	{
		strcpy(stemp, "              ");
		memcpy(stemp, saveFiles[slot-1].name, strlen(saveFiles[slot-1].name));
		temp = strlen(stemp);
		while (stemp[temp-1] == ' ' && --temp);

		flash = 8 * 16 + 10;

		wait_noinput(false, true, false);

		JE_barShade(VGAScreen, 65, 55, 255, 155);

		bool quit = false;
		while (!quit)
		{
			service_SDL_events(true);

			blit_sprite(VGAScreen, 50, 50, OPTION_SHAPES, 35);  // message box

			JE_textShade(VGAScreen, 60, 55, miscText[1-1], 11, 4, DARKEN);
			JE_textShade(VGAScreen, 70, 70, levelName, 11, 4, DARKEN);

			do
			{
				flash = (flash == 8 * 16 + 10) ? 8 * 16 + 2 : 8 * 16 + 10;
				temp3 = (temp3 == 6) ? 2 : 6;

				strcpy(tempStr, miscText[2-1]);
				strncat(tempStr, stemp, temp);
				JE_outText(VGAScreen, 65, 89, tempStr, 8, 3);
				tempW = 65 + JE_textWidth(tempStr, TINY_FONT);
				JE_barShade(VGAScreen, tempW + 2, 90, tempW + 6, 95);
				fill_rectangle_xy(VGAScreen, tempW + 1, 89, tempW + 5, 94, flash);

				int text_x = 54 + 45 - (JE_textWidth(miscText[9], FONT_SHAPES) / 2);
				JE_outTextAdjust(VGAScreen, text_x, 128, miscText[9], 15, -5, FONT_SHAPES, true);

				text_x = 149 + 45 - (JE_textWidth(miscText[10], FONT_SHAPES) / 2);
				JE_outTextAdjust(VGAScreen, text_x, 128, miscText[10], 15, -5, FONT_SHAPES, true);

				for (int i = 0; i < 14; i++)
				{
					setDelay(1);

					push_joysticks_as_keyboard();
					service_wait_delay();

					JE_mouseStart();
					JE_showVGA();
					JE_mouseReplace();

					if (newkey || newmouse || new_text)
						break;
				}
			} while (!newkey && !newmouse && !new_text);

			if (mouseButton > 0)
			{
				if (lastmouse_x > 56 && lastmouse_x < 142 && lastmouse_y > 123 && lastmouse_y < 149)
				{
					quit = true;
					JE_saveGame(slot, stemp);
					JE_playSampleNum(S_SELECT);
				}
				else if (lastmouse_x > 151 && lastmouse_x < 237 && lastmouse_y > 123 && lastmouse_y < 149)
				{
					quit = true;
					JE_playSampleNum(S_SPRING);
				}
			}
			else if (new_text)
			{
				for (size_t ti = 0U; last_text[ti] != '\0'; ++ti)
				{
					const char c = (unsigned char)last_text[ti] <= 127U ? toupper(last_text[ti]) : 0;
					if ((c == ' ' || font_ascii[(unsigned char)c] != -1) &&
					    temp < 14)
					{
						JE_playSampleNum(S_CURSOR);
						stemp[temp] = c;
						temp += 1;
					}
				}
			}
			else if (newkey)
			{
				switch (lastkey_scan)
				{
					case SDL_SCANCODE_BACKSPACE:
					case SDL_SCANCODE_DELETE:
						if (temp)
						{
							temp--;
							stemp[temp] = ' ';
							JE_playSampleNum(S_CLICK);
						}
						break;
					case SDL_SCANCODE_ESCAPE:
						quit = true;
						JE_playSampleNum(S_SPRING);
						break;
					case SDL_SCANCODE_RETURN:
						quit = true;
						JE_saveGame(slot, stemp);
						JE_playSampleNum(S_SELECT);
						break;
					default:
						break;
				}
			}
		}
	}

	wait_noinput(false, true, false);
}

void JE_inGameDisplays(void)
{
	char stemp[21];
	char tempstr[256];

	for (uint i = 0; i < ((twoPlayerMode && !galagaMode) ? 2 : 1); ++i)
	{
		snprintf(tempstr, sizeof(tempstr), "%lu", player[i].cash);
		JE_textShade(VGAScreen, 30 + 200 * i, 175, tempstr, 2, 4, FULL_SHADE);
	}

	/*Special Weapon?*/
	if (player[0].items.special > 0)
		blit_sprite2x2(VGAScreen, 25, 1, spriteSheet10, special[player[0].items.special].itemgraphic);

	/*Lives Left*/
	if (onePlayerAction || twoPlayerMode)
	{
		for (int temp = 0; temp < (onePlayerAction ? 1 : 2); temp++)
		{
			const uint extra_lives = *player[temp].lives - 1;

			int y = (temp == 0 && player[0].items.special > 0) ? 35 : 15;
			tempW = (temp == 0) ? 30: 270;

			if (extra_lives >= 5)
			{
				blit_sprite2(VGAScreen, tempW, y, spriteSheet9, 285);
				tempW = (temp == 0) ? 45 : 250;
				sprintf(tempstr, "%d", extra_lives);
				JE_textShade(VGAScreen, tempW, y + 3, tempstr, 15, 1, FULL_SHADE);
			}
			else if (extra_lives >= 1)
			{
				for (uint i = 0; i < extra_lives; ++i)
				{
					blit_sprite2(VGAScreen, tempW, y, spriteSheet9, 285);

					tempW += (temp == 0) ? 12 : -12;
				}
			}

			strcpy(stemp, (temp == 0) ? miscText[49-1] : miscText[50-1]);
			if (isNetworkGame)
			{
				strcpy(stemp, JE_getName(temp+1));
			}

			tempW = (temp == 0) ? 28 : (285 - JE_textWidth(stemp, TINY_FONT));
			JE_textShade(VGAScreen, tempW, y - 7, stemp, 2, 6, FULL_SHADE);
		}
	}

	/*Super Bombs!!*/
	for (uint i = 0; i < COUNTOF(player); ++i)
	{
		int x = (i == 0) ? 30 : 270;

		for (uint j = player[i].superbombs; j > 0; --j)
		{
			blit_sprite2(VGAScreen, x, 160, spriteSheet9, 304);
			x += (i == 0) ? 12 : -12;
		}
	}

	if (youAreCheating)
	{
		JE_outText(VGAScreen, 90, 170, "Cheaters always prosper.", 3, 4);
	}
}

void JE_mainKeyboardInput(void)
{
	JE_gammaCheck();

	/* { Network Request Commands } */

	if (!isNetworkGame)
	{
		/* { Edited Ships } for Player 1 */
		if (extraAvail && keysactive[SDL_SCANCODE_TAB] && !isNetworkGame && !superTyrian)
		{
			for (x = SDL_SCANCODE_1; x <= SDL_SCANCODE_0; x++)
			{
				if (keysactive[x])
				{
					int z = x - SDL_SCANCODE_1 + 1;
					player[0].items.ship = 90 + z;                     /*Ships*/
					z = (z - 1) * 15;
					player[0].items.weapon[FRONT_WEAPON].id = extraShips[z + 1];
					player[0].items.weapon[REAR_WEAPON].id = extraShips[z + 2];
					player[0].items.special = extraShips[z + 3];
					player[0].items.sidekick[LEFT_SIDEKICK] = extraShips[z + 4];
					player[0].items.sidekick[RIGHT_SIDEKICK] = extraShips[z + 5];
					player[0].items.generator = extraShips[z + 6];
					/*Armor*/
					player[0].items.shield = extraShips[z + 8];
					memset(shotMultiPos, 0, sizeof(shotMultiPos));

					if (player[0].weapon_mode > JE_portConfigs())
						player[0].weapon_mode = 1;

					tempW = player[0].armor;
					JE_getShipInfo();
					if (player[0].armor > tempW && editShip1)
						player[0].armor = tempW;
					else
						editShip1 = true;

					SDL_Surface *temp_surface = VGAScreen;
					VGAScreen = VGAScreenSeg;
					JE_wipeShieldArmorBars();
					JE_drawArmor();
					JE_drawShield();
					VGAScreen = temp_surface;
					JE_drawOptions();

					keysactive[x] = false;
				}
			}
		}

		/* for Player 2 */
		if (extraAvail && keysactive[SDL_SCANCODE_CAPSLOCK] && !isNetworkGame && !superTyrian)
		{
			for (x = SDL_SCANCODE_1; x <= SDL_SCANCODE_0; x++)
			{
				if (keysactive[x])
				{
					int z = x - SDL_SCANCODE_1 + 1;
					player[1].items.ship = 90 + z;
					z = (z - 1) * 15;
					player[1].items.weapon[FRONT_WEAPON].id = extraShips[z + 1];
					player[1].items.weapon[REAR_WEAPON].id = extraShips[z + 2];
					player[1].items.special = extraShips[z + 3];
					player[1].items.sidekick[LEFT_SIDEKICK] = extraShips[z + 4];
					player[1].items.sidekick[RIGHT_SIDEKICK] = extraShips[z + 5];
					player[1].items.generator = extraShips[z + 6];
					/*Armor*/
					player[1].items.shield = extraShips[z + 8];
					memset(shotMultiPos, 0, sizeof(shotMultiPos));

					if (player[1].weapon_mode > JE_portConfigs())
						player[1].weapon_mode = 1;

					tempW = player[1].armor;
					JE_getShipInfo();
					if (player[1].armor > tempW && editShip2)
						player[1].armor = tempW;
					else
						editShip2 = true;

					SDL_Surface *temp_surface = VGAScreen;
					VGAScreen = VGAScreenSeg;
					JE_wipeShieldArmorBars();
					JE_drawArmor();
					JE_drawShield();
					VGAScreen = temp_surface;
					JE_drawOptions();

					keysactive[x] = false;
				}
			}
		}
	}

	/* { In-Game Help } */
	if (keysactive[SDL_SCANCODE_F1])
	{
		if (isNetworkGame)
		{
			helpRequest = true;
		}
		else
		{
			JE_inGameHelp();
			skipStarShowVGA = true;
		}
	}

	/* {!Activate Nort Ship!} */
	if (keysactive[SDL_SCANCODE_F2] && keysactive[SDL_SCANCODE_F4] && keysactive[SDL_SCANCODE_F6] && keysactive[SDL_SCANCODE_F7] &&
	    keysactive[SDL_SCANCODE_F9] && keysactive[SDL_SCANCODE_BACKSLASH] && keysactive[SDL_SCANCODE_SLASH])
	{
		if (isNetworkGame)
		{
			nortShipRequest = true;
		}
		else
		{
			player[0].items.ship = 12;                     // Nort Ship
			player[0].items.special = 13;                  // Astral Zone
			player[0].items.weapon[FRONT_WEAPON].id = 36;  // NortShip Super Pulse
			player[0].items.weapon[REAR_WEAPON].id = 37;   // NortShip Spreader
			shipGr = 1;
		}
	}

	/* {Cheating} */
	if (!isNetworkGame && !twoPlayerMode && !superTyrian && superArcadeMode == SA_NONE)
	{
		if (keysactive[SDL_SCANCODE_F2] && keysactive[SDL_SCANCODE_F3] && keysactive[SDL_SCANCODE_F6])
		{
			youAreCheating = !youAreCheating;
			keysactive[SDL_SCANCODE_F2] = false;
		}

		if (keysactive[SDL_SCANCODE_F2] && keysactive[SDL_SCANCODE_F3] && (keysactive[SDL_SCANCODE_F4] || keysactive[SDL_SCANCODE_F5]))
		{
			for (uint i = 0; i < COUNTOF(player); ++i)
				player[i].armor = 0;

			youAreCheating = !youAreCheating;
			JE_drawTextWindow(miscText[63-1]);
		}

		if (constantPlay && keysactive[SDL_SCANCODE_C])
		{
			youAreCheating = !youAreCheating;
			keysactive[SDL_SCANCODE_C] = false;
		}
	}

	if (superTyrian)
	{
		youAreCheating = false;
	}

	/* {Personal Commands} */

	/* {DEBUG} */
	if (keysactive[SDL_SCANCODE_F10] && keysactive[SDL_SCANCODE_BACKSPACE])
	{
		keysactive[SDL_SCANCODE_F10] = false;
		debug = !debug;

		debugHist = 1;
		debugHistCount = 1;

		/* YKS: clock ticks since midnight replaced by SDL_GetTicks */
		lastDebugTime = SDL_GetTicks();
	}

	/* {CHEAT-SKIP LEVEL} */
	if (keysactive[SDL_SCANCODE_F2] && keysactive[SDL_SCANCODE_F6] && (keysactive[SDL_SCANCODE_F7] || keysactive[SDL_SCANCODE_F8]) && !keysactive[SDL_SCANCODE_F9] &&
	    !superTyrian && superArcadeMode == SA_NONE)
	{
		if (isNetworkGame)
		{
			skipLevelRequest = true;
		}
		else
		{
			levelTimer = true;
			levelTimerCountdown = 0;
			endLevel = true;
			levelEnd = 40;
		}
	}

	/* pause game */
	pause_pressed = pause_pressed || keysactive[SDL_SCANCODE_P];

	/* in-game setup */
	ingamemenu_pressed = ingamemenu_pressed || keysactive[SDL_SCANCODE_ESCAPE];

	if (keysactive[SDL_SCANCODE_BACKSPACE])
	{
		/* toggle screenshot pause */
		if (keysactive[SDL_SCANCODE_NUMLOCKCLEAR])
			superPause = !superPause;

		/* {SMOOTHIES} */
		if (keysactive[SDL_SCANCODE_F12] && keysactive[SDL_SCANCODE_SCROLLLOCK])
		{
			for (temp = SDL_SCANCODE_2; temp <= SDL_SCANCODE_9; temp++)
				if (keysactive[temp])
					smoothies[temp-SDL_SCANCODE_2] = !smoothies[temp-SDL_SCANCODE_2];
			if (keysactive[SDL_SCANCODE_0])
				smoothies[8] = !smoothies[8];
		}
		else

		/* {CYCLE THROUGH FILTER COLORS} */
		if (keysactive[SDL_SCANCODE_MINUS])
		{
			if (levelFilter == -99)
			{
				levelFilter = 0;
			}
			else
			{
				levelFilter++;
				if (levelFilter == 16)
					levelFilter = -99;
			}
		}
		else

		/* {HYPER-SPEED} */
		if (keysactive[SDL_SCANCODE_1])
		{
			fastPlay++;
			if (fastPlay > 2)
				fastPlay = 0;
			keysactive[SDL_SCANCODE_1] = false;
			JE_setNewGameSpeed();
		}

		/* {IN-GAME RANDOM MUSIC SELECTION} */
		if (keysactive[SDL_SCANCODE_SCROLLLOCK])
			play_song(mt_rand() % MUSIC_NUM);
	}
}

void JE_pauseGame(void)
{
	mouseSetRelative(false);

	JE_boolean done = false;
	JE_word mouseX, mouseY;

	SDL_Surface *temp_surface = VGAScreen;
	VGAScreen = VGAScreenSeg; /* side-effect of game_screen */

	//tempScreenSeg = VGAScreenSeg; // sega000
	if (!superPause)
	{
		JE_dString(VGAScreenSeg, 120, 90, miscText[22], FONT_SHAPES);

		VGAScreen = VGAScreenSeg;
		JE_showVGA();
	}

	set_volume(tyrMusicVolume / 2, fxVolume);

#ifdef WITH_NETWORK
	if (isNetworkGame)
	{
		network_prepare(PACKET_GAME_PAUSE);
		network_send(4);  // PACKET_GAME_PAUSE

		while (true)
		{
			service_SDL_events(false);

			if (packet_in[0] && SDLNet_Read16(&packet_in[0]->data[0]) == PACKET_GAME_PAUSE)
			{
				network_update();
				break;
			}

			network_update();
			network_check();

			SDL_Delay(16);
		}
	}
#endif

	wait_noinput(false, false, true); // TODO: should up the joystick repeat temporarily instead

	do
	{
		setDelay(2);

		push_joysticks_as_keyboard();
		service_SDL_events(true);

		if ((newkey && lastkey_scan != SDL_SCANCODE_LCTRL && lastkey_scan != SDL_SCANCODE_RCTRL && lastkey_scan != SDL_SCANCODE_LALT && lastkey_scan != SDL_SCANCODE_RALT) ||
		    JE_mousePosition(&mouseX, &mouseY) > 0)
		{
#ifdef WITH_NETWORK
			if (isNetworkGame)
			{
				network_prepare(PACKET_WAITING);
				network_send(4);  // PACKET_WAITING
			}
#endif
			done = true;
		}

#ifdef WITH_NETWORK
		if (isNetworkGame)
		{
			network_check();

			if (packet_in[0] && SDLNet_Read16(&packet_in[0]->data[0]) == PACKET_WAITING)
			{
				network_check();

				done = true;
			}
		}
#endif

		wait_delay();
	} while (!done);

#ifdef WITH_NETWORK
	if (isNetworkGame)
	{
		while (!network_is_sync())
		{
			service_SDL_events(false);

			network_check();
			SDL_Delay(16);
		}
	}
#endif

	set_volume(tyrMusicVolume, fxVolume);

	//skipStarShowVGA = true;

	VGAScreen = temp_surface; /* side-effect of game_screen */

	mouseSetRelative(true);
}

void JE_playerMovement(Player *this_player,
                       JE_byte inputDevice,
                       JE_byte playerNum_,
                       JE_word shipGr_,
                       Sprite2_array *shipGrPtr_,
                       JE_word *mouseX_, JE_word *mouseY_)
{
	JE_integer mouseXC, mouseYC;
	JE_integer accelXC, accelYC;

	if (playerNum_ == 2 || !twoPlayerMode)
	{
		tempW = weaponPort[this_player->items.weapon[REAR_WEAPON].id].opnum;

		if (this_player->weapon_mode > tempW)
			this_player->weapon_mode = 1;
	}

#ifdef WITH_NETWORK
	if (isNetworkGame && thisPlayerNum == playerNum_)
	{
		network_state_prepare();
		memset(&packet_state_out[0]->data[4], 0, 10);
	}
#endif

redo:

	if (isNetworkGame)
	{
		inputDevice = 0;
	}

	mouseXC = 0;
	mouseYC = 0;
	accelXC = 0;
	accelYC = 0;

	bool link_gun_analog = false;
	float link_gun_angle = 0;

	/* Draw Player */
	if (!this_player->is_alive)
	{
		if (this_player->exploding_ticks > 0)
		{
			--this_player->exploding_ticks;

			if (levelEndFxWait > 0)
			{
				levelEndFxWait--;
			}
			else
			{
				levelEndFxWait = (mt_rand() % 6) + 3;
				if ((mt_rand() % 3) == 1)
					soundQueue[6] = S_EXPLOSION_9;
				else
					soundQueue[5] = S_EXPLOSION_11;
			}

			int explosion_x = this_player->x + (mt_rand() % 32) - 16;
			int explosion_y = this_player->y + (mt_rand() % 32) - 16;
			JE_setupExplosionLarge(false, 0, explosion_x, explosion_y + 7);
			JE_setupExplosionLarge(false, 0, this_player->x, this_player->y + 7);

			if (levelEnd > 0)
				levelEnd--;
		}
		else
		{
			if (twoPlayerMode || onePlayerAction)  // if arcade mode
			{
				if (*this_player->lives > 1)  // respawn if any extra lives
				{
					--(*this_player->lives);

					reallyEndLevel = false;
					shotMultiPos[playerNum_-1] = 0;
					calc_purple_balls_needed(this_player);
					twoPlayerLinked = false;
					if (galagaMode)
						twoPlayerMode = false;
					this_player->y = 160;
					this_player->invulnerable_ticks = 100;
					this_player->is_alive = true;
					endLevel = false;

					if (galagaMode || episodeNum == 4)
						this_player->armor = this_player->initial_armor;
					else
						this_player->armor = this_player->initial_armor / 2;

					if (galagaMode)
						this_player->shield = 0;
					else
						this_player->shield = this_player->shield_max / 2;

					VGAScreen = VGAScreenSeg; /* side-effect of game_screen */
					JE_drawArmor();
					JE_drawShield();
					VGAScreen = game_screen; /* side-effect of game_screen */
					goto redo;
				}
				else
				{
					if (galagaMode)
						twoPlayerMode = false;
					if (allPlayersGone && isNetworkGame)
						reallyEndLevel = true;
				}

			}
		}
	}
	else if (constantDie)
	{
		// finished exploding?  start dying again
		if (this_player->exploding_ticks == 0)
		{
			this_player->shield = 0;

			if (this_player->armor > 0)
			{
				--this_player->armor;
			}
			else
			{
				this_player->is_alive = false;
				this_player->exploding_ticks = 60;
				levelEnd = 40;
			}

			JE_wipeShieldArmorBars();
			VGAScreen = VGAScreenSeg; /* side-effect of game_screen */
			JE_drawArmor();
			VGAScreen = game_screen; /* side-effect of game_screen */

			// as if instant death weren't enough, player also gets infinite lives in order to enjoy an infinite number of deaths -_-
			if (*player[0].lives < 11)
				++(*player[0].lives);
		}
	}

	if (!this_player->is_alive)
	{
		explosionFollowAmountX = explosionFollowAmountY = 0;
		return;
	}

	if (!endLevel)
	{
		*mouseX_ = this_player->x;
		*mouseY_ = this_player->y;
		button[1-1] = false;
		button[2-1] = false;
		button[3-1] = false;
		button[4-1] = false;

		/* --- Movement Routine Beginning --- */

		if (!isNetworkGame || playerNum_ == thisPlayerNum)
		{
			if (endLevel)
			{
				this_player->y -= 2;
			}
			else
			{
				if (record_demo || play_demo)
					inputDevice = 1;  // keyboard is required device for demo recording

				// demo playback input
				if (play_demo)
				{
					if (!replay_demo_keys())
					{
						endLevel = true;
						levelEnd = 40;
					}
				}

				/* joystick input */
				if ((inputDevice == 0 || inputDevice >= 3) && joysticks > 0)
				{
					int j = inputDevice  == 0 ? 0 : inputDevice - 3;
					int j_max = inputDevice == 0 ? joysticks : inputDevice - 3 + 1;
					for (; j < j_max; j++)
					{
						poll_joystick(j);

						if (joystick[j].analog)
						{
							mouseXC += joystick_axis_reduce(j, joystick[j].x);
							mouseYC += joystick_axis_reduce(j, joystick[j].y);

							link_gun_analog = joystick_analog_angle(j, &link_gun_angle);
						}
						else
						{
							this_player->x += (joystick[j].direction[3] ? -CURRENT_KEY_SPEED : 0) + (joystick[j].direction[1] ? CURRENT_KEY_SPEED : 0);
							this_player->y += (joystick[j].direction[0] ? -CURRENT_KEY_SPEED : 0) + (joystick[j].direction[2] ? CURRENT_KEY_SPEED : 0);
						}

						button[0] |= joystick[j].action[0];
						button[1] |= joystick[j].action[2];
						button[2] |= joystick[j].action[3];
						button[3] |= joystick[j].action_pressed[1];

						ingamemenu_pressed |= joystick[j].action_pressed[4];
						pause_pressed |= joystick[j].action_pressed[5];
					}
				}

				service_SDL_events(false);

				/* mouse input */
				if ((inputDevice == 0 || inputDevice == 2) && has_mouse)
				{
					button[0] |= mouse_pressed[0];
					button[1] |= mouse_pressed[1];
					button[2] |= mouse_has_three_buttons ? mouse_pressed[2] : mouse_pressed[1];

					Sint32 mouseXR;
					Sint32 mouseYR;
					mouseGetRelativePosition(&mouseXR, &mouseYR);
					mouseXC += mouseXR;
					mouseYC += mouseYR;
				}

				/* keyboard input */
				if ((inputDevice == 0 || inputDevice == 1) && !play_demo)
				{
					if (keysactive[keySettings[KEY_SETTING_UP]])
						this_player->y -= CURRENT_KEY_SPEED;
					if (keysactive[keySettings[KEY_SETTING_DOWN]])
						this_player->y += CURRENT_KEY_SPEED;

					if (keysactive[keySettings[KEY_SETTING_LEFT]])
						this_player->x -= CURRENT_KEY_SPEED;
					if (keysactive[keySettings[KEY_SETTING_RIGHT]])
						this_player->x += CURRENT_KEY_SPEED;

					button[0] = button[0] || keysactive[keySettings[KEY_SETTING_FIRE]];
					button[3] = button[3] || keysactive[keySettings[KEY_SETTING_CHANGE_FIRE]];
					button[1] = button[1] || keysactive[keySettings[KEY_SETTING_LEFT_SIDEKICK]];
					button[2] = button[2] || keysactive[keySettings[KEY_SETTING_RIGHT_SIDEKICK]];

					if (constantPlay)
					{
						for (unsigned int i = 0; i < 4; i++)
							button[i] = true;

						++this_player->y;
						this_player->x += constantLastX;
					}

					// TODO: check if demo recording still works
					if (record_demo)
					{
						bool new_input = false;

						for (unsigned int i = 0; i < 8; i++)
						{
							bool temp = demo_keys & (1 << i);
							if (temp != keysactive[keySettings[i]])
								new_input = true;
						}

						demo_keys_wait++;

						if (new_input)
						{
							Uint8 temp2[2] = { demo_keys_wait >> 8, demo_keys_wait };
							fwrite_u8(temp2, 2, demo_file);

							demo_keys = 0;
							for (unsigned int i = 0; i < 8; i++)
								demo_keys |= keysactive[keySettings[i]] ? (1 << i) : 0;

							fwrite_u8(&demo_keys, 1, demo_file);

							demo_keys_wait = 0;
						}
					}
				}

				if (smoothies[9-1])
				{
					*mouseY_ = this_player->y - (*mouseY_ - this_player->y);
					mouseYC = -mouseYC;
				}

				accelXC += this_player->x - *mouseX_;
				accelYC += this_player->y - *mouseY_;

				if (mouseXC > 30)
					mouseXC = 30;
				else if (mouseXC < -30)
					mouseXC = -30;
				if (mouseYC > 30)
					mouseYC = 30;
				else if (mouseYC < -30)
					mouseYC = -30;

				if (mouseXC > 0)
					this_player->x += (mouseXC + 3) / 4;
				else if (mouseXC < 0)
					this_player->x += (mouseXC - 3) / 4;
				if (mouseYC > 0)
					this_player->y += (mouseYC + 3) / 4;
				else if (mouseYC < 0)
					this_player->y += (mouseYC - 3) / 4;

				if (mouseXC > 3)
					accelXC++;
				else if (mouseXC < -2)
					accelXC--;
				if (mouseYC > 2)
					accelYC++;
				else if (mouseYC < -2)
					accelYC--;

			}   /*endLevel*/

#ifdef WITH_NETWORK
			if (isNetworkGame && playerNum_ == thisPlayerNum)
			{
				Uint16 buttons = 0;
				for (int i = 4 - 1; i >= 0; i--)
				{
					buttons <<= 1;
					buttons |= button[i];
				}

				SDLNet_Write16(this_player->x - *mouseX_, &packet_state_out[0]->data[4]);
				SDLNet_Write16(this_player->y - *mouseY_, &packet_state_out[0]->data[6]);
				SDLNet_Write16(accelXC,                   &packet_state_out[0]->data[8]);
				SDLNet_Write16(accelYC,                   &packet_state_out[0]->data[10]);
				SDLNet_Write16(buttons,                   &packet_state_out[0]->data[12]);

				this_player->x = *mouseX_;
				this_player->y = *mouseY_;

				button[0] = false;
				button[1] = false;
				button[2] = false;
				button[3] = false;

				accelXC = 0;
				accelYC = 0;
			}
#endif
		}  /*isNetworkGame*/

		/* --- Movement Routine Ending --- */

		moveOk = true;

#ifdef WITH_NETWORK
		if (isNetworkGame && !network_state_is_reset())
		{
			if (playerNum_ != thisPlayerNum)
			{
				if (thisPlayerNum == 2)
					difficultyLevel = SDLNet_Read16(&packet_state_in[0]->data[16]);

				Uint16 buttons = SDLNet_Read16(&packet_state_in[0]->data[12]);
				for (int i = 0; i < 4; i++)
				{
					button[i] = buttons & 1;
					buttons >>= 1;
				}

				this_player->x += (Sint16)SDLNet_Read16(&packet_state_in[0]->data[4]);
				this_player->y += (Sint16)SDLNet_Read16(&packet_state_in[0]->data[6]);
				accelXC = (Sint16)SDLNet_Read16(&packet_state_in[0]->data[8]);
				accelYC = (Sint16)SDLNet_Read16(&packet_state_in[0]->data[10]);
			}
			else
			{
				Uint16 buttons = SDLNet_Read16(&packet_state_out[network_delay]->data[12]);
				for (int i = 0; i < 4; i++)
				{
					button[i] = buttons & 1;
					buttons >>= 1;
				}

				this_player->x += (Sint16)SDLNet_Read16(&packet_state_out[network_delay]->data[4]);
				this_player->y += (Sint16)SDLNet_Read16(&packet_state_out[network_delay]->data[6]);
				accelXC = (Sint16)SDLNet_Read16(&packet_state_out[network_delay]->data[8]);
				accelYC = (Sint16)SDLNet_Read16(&packet_state_out[network_delay]->data[10]);
			}
		}
#endif

		/*Street-Fighter codes*/
		JE_SFCodes(playerNum_, this_player->x, this_player->y, *mouseX_, *mouseY_);

		if (moveOk)
		{
			/* END OF MOVEMENT ROUTINES */

			/*Linking Routines*/

			if (twoPlayerMode && !twoPlayerLinked && this_player->x == *mouseX_ && this_player->y == *mouseY_ &&
			    abs(player[0].x - player[1].x) < 8 && abs(player[0].y - player[1].y) < 8 &&
			    player[0].is_alive && player[1].is_alive && !galagaMode)
			{
				twoPlayerLinked = true;
			}

			if (playerNum_ == 1 && (button[3-1] || button[2-1]) && !galagaMode)
				twoPlayerLinked = false;

			if (twoPlayerMode && twoPlayerLinked && playerNum_ == 2 &&
			    (this_player->x != *mouseX_ || this_player->y != *mouseY_))
			{
				if (button[0])
				{
					if (link_gun_analog)
					{
						linkGunDirec = link_gun_angle;
					}
					else
					{
						JE_real tempR;

						if (abs(this_player->x - *mouseX_) > abs(this_player->y - *mouseY_))
							tempR = (this_player->x - *mouseX_ > 0) ? M_PI_2 : (M_PI + M_PI_2);
						else
							tempR = (this_player->y - *mouseY_ > 0) ? 0 : M_PI;

						if (fabsf(linkGunDirec - tempR) < 0.3f)
							linkGunDirec = tempR;
						else if (linkGunDirec < tempR && linkGunDirec - tempR > -3.24f)
							linkGunDirec += 0.2f;
						else if (linkGunDirec - tempR < M_PI)
							linkGunDirec -= 0.2f;
						else
							linkGunDirec += 0.2f;
					}

					if (linkGunDirec >= (2 * M_PI))
						linkGunDirec -= (2 * M_PI);
					else if (linkGunDirec < 0)
						linkGunDirec += (2 * M_PI);
				}
				else if (!galagaMode)
				{
					twoPlayerLinked = false;
				}
			}
		}
	}

	if (levelEnd > 0 && all_players_dead())
		reallyEndLevel = true;

	/* End Level Fade-Out */
	if (this_player->is_alive && endLevel)
	{
		if (levelEnd == 0)
		{
			reallyEndLevel = true;
		}
		else
		{
			this_player->y -= levelEndWarp;
			if (this_player->y < -200)
				reallyEndLevel = true;

			int trail_spacing = 1;
			int trail_y = this_player->y;
			int num_trails = abs(41 - levelEnd);
			if (num_trails > 20)
				num_trails = 20;

			for (int i = 0; i < num_trails; i++)
			{
				trail_y += trail_spacing;
				trail_spacing++;
			}

			for (int i = 1; i < num_trails; i++)
			{
				trail_y -= trail_spacing;
				trail_spacing--;

				if (trail_y > 0 && trail_y < 170)
				{
					if (shipGr_ == 0)
					{
						blit_sprite2x2(VGAScreen, this_player->x - 17, trail_y - 7, *shipGrPtr_, 13);
						blit_sprite2x2(VGAScreen, this_player->x + 7 , trail_y - 7, *shipGrPtr_, 51);
					}
					else if (shipGr_ == 1)
					{
						blit_sprite2x2(VGAScreen, this_player->x - 17, trail_y - 7, *shipGrPtr_, 220);
						blit_sprite2x2(VGAScreen, this_player->x + 7 , trail_y - 7, *shipGrPtr_, 222);
					}
					else
					{
						blit_sprite2x2(VGAScreen, this_player->x - 5, trail_y - 7, *shipGrPtr_, shipGr_);
					}
				}
			}
		}
	}

	if (play_demo)
		JE_dString(VGAScreen, 115, 10, miscText[7], SMALL_FONT_SHAPES); // insert coin

	if (this_player->is_alive && !endLevel)
	{
		if (!twoPlayerLinked || playerNum_ < 2)
		{
			if (!twoPlayerMode || shipGr2 != 0)  // if not dragonwing
			{
				if (this_player->sidekick[LEFT_SIDEKICK].style == 0)
				{
					this_player->sidekick[LEFT_SIDEKICK].x = *mouseX_ - 14;
					this_player->sidekick[LEFT_SIDEKICK].y = *mouseY_;
				}

				if (this_player->sidekick[RIGHT_SIDEKICK].style == 0)
				{
					this_player->sidekick[RIGHT_SIDEKICK].x = *mouseX_ + 16;
					this_player->sidekick[RIGHT_SIDEKICK].y = *mouseY_;
				}
			}

			if (this_player->x_friction_ticks > 0)
			{
				--this_player->x_friction_ticks;
			}
			else
			{
				this_player->x_friction_ticks = 1;

				if (this_player->x_velocity < 0)
					++this_player->x_velocity;
				else if (this_player->x_velocity > 0)
					--this_player->x_velocity;
			}

			if (this_player->y_friction_ticks > 0)
			{
				--this_player->y_friction_ticks;
			}
			else
			{
				this_player->y_friction_ticks = 2;

				if (this_player->y_velocity < 0)
					++this_player->y_velocity;
				else if (this_player->y_velocity > 0)
					--this_player->y_velocity;
			}

			this_player->x_velocity += accelXC;
			this_player->y_velocity += accelYC;

			this_player->x_velocity = MIN(MAX(-4, this_player->x_velocity), 4);
			this_player->y_velocity = MIN(MAX(-4, this_player->y_velocity), 4);

			this_player->x += this_player->x_velocity;
			this_player->y += this_player->y_velocity;

			// if player moved, add new ship x, y history entry
			if (this_player->x - *mouseX_ != 0 || this_player->y - *mouseY_ != 0)
			{
				for (uint i = 1; i < COUNTOF(player->old_x); ++i)
				{
					this_player->old_x[i - 1] = this_player->old_x[i];
					this_player->old_y[i - 1] = this_player->old_y[i];
				}
				this_player->old_x[COUNTOF(player->old_x) - 1] = this_player->x;
				this_player->old_y[COUNTOF(player->old_x) - 1] = this_player->y;
			}
		}
		else  /*twoPlayerLinked*/
		{
			if (shipGr_ == 0)
				this_player->x = player[0].x - 1;
			else
				this_player->x = player[0].x;
			this_player->y = player[0].y + 8;

			this_player->x_velocity = player[0].x_velocity;
			this_player->y_velocity = 4;

			// turret direction marker/shield
			shotMultiPos[SHOT_MISC] = 0;
			b = player_shot_create(0, SHOT_MISC, this_player->x + 1 + roundf(sinf(linkGunDirec + 0.2f) * 26), this_player->y + roundf(cosf(linkGunDirec + 0.2f) * 26), *mouseX_, *mouseY_, 148, playerNum_);
			shotMultiPos[SHOT_MISC] = 0;
			b = player_shot_create(0, SHOT_MISC, this_player->x + 1 + roundf(sinf(linkGunDirec - 0.2f) * 26), this_player->y + roundf(cosf(linkGunDirec - 0.2f) * 26), *mouseX_, *mouseY_, 148, playerNum_);
			shotMultiPos[SHOT_MISC] = 0;
			b = player_shot_create(0, SHOT_MISC, this_player->x + 1 + roundf(sinf(linkGunDirec) * 26), this_player->y + roundf(cosf(linkGunDirec) * 26), *mouseX_, *mouseY_, 147, playerNum_);

			if (shotRepeat[SHOT_REAR] > 0)
			{
				--shotRepeat[SHOT_REAR];
			}
			else if (button[1-1])
			{
				shotMultiPos[SHOT_REAR] = 0;
				b = player_shot_create(0, SHOT_REAR, this_player->x + 1 + roundf(sinf(linkGunDirec) * 20), this_player->y + roundf(cosf(linkGunDirec) * 20), *mouseX_, *mouseY_, linkGunWeapons[this_player->items.weapon[REAR_WEAPON].id-1], playerNum_);
				player_shot_set_direction(b, this_player->items.weapon[REAR_WEAPON].id, linkGunDirec);
			}
		}
	}

	if (!endLevel)
	{
		if (this_player->x > 256)
		{
			this_player->x = 256;
			constantLastX = -constantLastX;
		}
		if (this_player->x < 40)
		{
			this_player->x = 40;
			constantLastX = -constantLastX;
		}

		if (isNetworkGame && playerNum_ == 1)
		{
			if (this_player->y > 154)
				this_player->y = 154;
		}
		else
		{
			if (this_player->y > 160)
				this_player->y = 160;
		}

		if (this_player->y < 10)
			this_player->y = 10;

		// Determines the ship banking sprite to display, depending on horizontal velocity and acceleration
		int ship_banking = this_player->x_velocity / 2 + (this_player->x - *mouseX_) / 6;
		ship_banking = MAX(-2, MIN(ship_banking, 2));

		int ship_sprite = ship_banking * 2 + shipGr_;

		explosionFollowAmountX = this_player->x - this_player->last_x_explosion_follow;
		explosionFollowAmountY = this_player->y - this_player->last_y_explosion_follow;

		if (explosionFollowAmountY < 0)
			explosionFollowAmountY = 0;

		this_player->last_x_explosion_follow = this_player->x;
		this_player->last_y_explosion_follow = this_player->y;

		if (shipGr_ == 0)
		{
			if (background2)
			{
				blit_sprite2x2_darken(VGAScreen, this_player->x - 17 - mapX2Ofs + 30, this_player->y - 7 + shadowYDist, *shipGrPtr_, ship_sprite + 13);
				blit_sprite2x2_darken(VGAScreen, this_player->x + 7 - mapX2Ofs + 30, this_player->y - 7 + shadowYDist, *shipGrPtr_, ship_sprite + 51);
				if (superWild)
				{
					blit_sprite2x2_darken(VGAScreen, this_player->x - 16 - mapX2Ofs + 30, this_player->y - 7 + shadowYDist, *shipGrPtr_, ship_sprite + 13);
					blit_sprite2x2_darken(VGAScreen, this_player->x + 6 - mapX2Ofs + 30, this_player->y - 7 + shadowYDist, *shipGrPtr_, ship_sprite + 51);
				}
			}
		}
		else if (shipGr_ == 1)
		{
			if (background2)
			{
				blit_sprite2x2_darken(VGAScreen, this_player->x - 17 - mapX2Ofs + 30, this_player->y - 7 + shadowYDist, *shipGrPtr_, 220);
				blit_sprite2x2_darken(VGAScreen, this_player->x + 7 - mapX2Ofs + 30, this_player->y - 7 + shadowYDist, *shipGrPtr_, 222);
			}
		}
		else
		{
			if (background2)
			{
				blit_sprite2x2_darken(VGAScreen, this_player->x - 5 - mapX2Ofs + 30, this_player->y - 7 + shadowYDist, *shipGrPtr_, ship_sprite);
				if (superWild)
				{
					blit_sprite2x2_darken(VGAScreen, this_player->x - 4 - mapX2Ofs + 30, this_player->y - 7 + shadowYDist, *shipGrPtr_, ship_sprite);
				}
			}
		}

		if (this_player->invulnerable_ticks > 0)
		{
			--this_player->invulnerable_ticks;

			if (shipGr_ == 0)
			{
				blit_sprite2x2_blend(VGAScreen, this_player->x - 17, this_player->y - 7, *shipGrPtr_, ship_sprite + 13);
				blit_sprite2x2_blend(VGAScreen, this_player->x + 7 , this_player->y - 7, *shipGrPtr_, ship_sprite + 51);
			}
			else if (shipGr_ == 1)
			{
				blit_sprite2x2_blend(VGAScreen, this_player->x - 17, this_player->y - 7, *shipGrPtr_, 220);
				blit_sprite2x2_blend(VGAScreen, this_player->x + 7 , this_player->y - 7, *shipGrPtr_, 222);
			}
			else
				blit_sprite2x2_blend(VGAScreen, this_player->x - 5, this_player->y - 7, *shipGrPtr_, ship_sprite);
		}
		else
		{
			if (shipGr_ == 0)
			{
				blit_sprite2x2(VGAScreen, this_player->x - 17, this_player->y - 7, *shipGrPtr_, ship_sprite + 13);
				blit_sprite2x2(VGAScreen, this_player->x + 7, this_player->y - 7, *shipGrPtr_, ship_sprite + 51);
			}
			else if (shipGr_ == 1)
			{
				blit_sprite2x2(VGAScreen, this_player->x - 17, this_player->y - 7, *shipGrPtr_, 220);
				blit_sprite2x2(VGAScreen, this_player->x + 7, this_player->y - 7, *shipGrPtr_, 222);

				int ship_banking = 0;
				switch (ship_sprite)
				{
				case 5:
					blit_sprite2(VGAScreen, this_player->x - 17, this_player->y + 7, *shipGrPtr_, 40);
					tempW = this_player->x - 7;
					ship_banking = -2;
					break;
				case 3:
					blit_sprite2(VGAScreen, this_player->x - 17, this_player->y + 7, *shipGrPtr_, 39);
					tempW = this_player->x - 7;
					ship_banking = -1;
					break;
				case 1:
					ship_banking = 0;
					break;
				case -1:
					blit_sprite2(VGAScreen, this_player->x + 19, this_player->y + 7, *shipGrPtr_, 58);
					tempW = this_player->x + 9;
					ship_banking = 1;
					break;
				case -3:
					blit_sprite2(VGAScreen, this_player->x + 19, this_player->y + 7, *shipGrPtr_, 59);
					tempW = this_player->x + 9;
					ship_banking = 2;
					break;
				}
				if (ship_banking != 0)  // NortSparks
				{
					if (shotRepeat[SHOT_NORTSPARKS] > 0)
					{
						--shotRepeat[SHOT_NORTSPARKS];
					}
					else
					{
						b = player_shot_create(0, SHOT_NORTSPARKS, tempW + (mt_rand() % 8) - 4, this_player->y + (mt_rand() % 8) - 4, *mouseX_, *mouseY_, 671, 1);
						shotRepeat[SHOT_NORTSPARKS] = abs(ship_banking) - 1;
					}
				}
			}
			else
			{
				blit_sprite2x2(VGAScreen, this_player->x - 5, this_player->y - 7, *shipGrPtr_, ship_sprite);
			}
		}

		/*Options Location*/
		if (playerNum_ == 2 && shipGr_ == 0)  // if dragonwing
		{
			if (this_player->sidekick[LEFT_SIDEKICK].style == 0)
			{
				this_player->sidekick[LEFT_SIDEKICK].x = this_player->x - 14 + ship_banking * 2;
				this_player->sidekick[LEFT_SIDEKICK].y = this_player->y;
			}

			if (this_player->sidekick[RIGHT_SIDEKICK].style == 0)
			{
				this_player->sidekick[RIGHT_SIDEKICK].x = this_player->x + 17 + ship_banking * 2;
				this_player->sidekick[RIGHT_SIDEKICK].y = this_player->y;
			}
		}
	}  // !endLevel

	if (moveOk)
	{
		if (this_player->is_alive)
		{
			if (!endLevel)
			{
				this_player->delta_x_shot_move = this_player->x - this_player->last_x_shot_move;
				this_player->delta_y_shot_move = this_player->y - this_player->last_y_shot_move;

				/* PLAYER SHOT Change */
				if (button[4-1])
				{
					portConfigChange = true;
					if (portConfigDone)
					{
						shotMultiPos[SHOT_REAR] = 0;

						if (superArcadeMode != SA_NONE && superArcadeMode <= SA_NORTSHIPZ)
						{
							shotMultiPos[SHOT_SPECIAL] = 0;
							shotMultiPos[SHOT_SPECIAL2] = 0;
							if (player[0].items.special == SASpecialWeapon[superArcadeMode-1])
							{
								player[0].items.special = SASpecialWeaponB[superArcadeMode-1];
								this_player->weapon_mode = 2;
							}
							else
							{
								player[0].items.special = SASpecialWeapon[superArcadeMode-1];
								this_player->weapon_mode = 1;
							}
						}
						else if (++this_player->weapon_mode > JE_portConfigs())
							this_player->weapon_mode = 1;

						JE_drawPortConfigButtons();
						portConfigDone = false;
					}
				}

				/* PLAYER SHOT Creation */

				/*SpecialShot*/
				if (!galagaMode)
					JE_doSpecialShot(playerNum_, &this_player->armor, &this_player->shield);

				/*Normal Main Weapons*/
				if (!(twoPlayerLinked && playerNum_ == 2))
				{
					int min, max;

					if (!twoPlayerMode)
						min = 1, max = 2;
					else
						min = max = playerNum_;

					for (temp = min - 1; temp < max; temp++)
					{
						const uint item = this_player->items.weapon[temp].id;

						if (item > 0)
						{
							if (shotRepeat[temp] > 0)
							{
								--shotRepeat[temp];
							}
							else if (button[1-1])
							{
								const uint item_power = galagaMode ? 0 : this_player->items.weapon[temp].power - 1,
								           item_mode = (temp == REAR_WEAPON) ? this_player->weapon_mode - 1 : 0;

								b = player_shot_create(item, temp, this_player->x, this_player->y, *mouseX_, *mouseY_, weaponPort[item].op[item_mode][item_power], playerNum_);
							}
						}
					}
				}

				/*Super Charge Weapons*/
				if (playerNum_ == 2)
				{

					if (!twoPlayerLinked)
						blit_sprite2(VGAScreen, this_player->x + (shipGr_ == 0) + 1, this_player->y - 13, spriteSheet10, 77 + chargeLevel + chargeGr * 19);

					if (chargeGrWait > 0)
					{
						chargeGrWait--;
					}
					else
					{
						chargeGr++;
						if (chargeGr == 4)
							chargeGr = 0;
						chargeGrWait = 3;
					}

					if (chargeLevel > 0)
					{
						fill_rectangle_xy(VGAScreenSeg, 269, 107 + (chargeLevel - 1) * 3, 275, 108 + (chargeLevel - 1) * 3, 193);
					}

					if (chargeWait > 0)
					{
						chargeWait--;
					}
					else
					{
						if (chargeLevel < chargeMax)
							chargeLevel++;

						chargeWait = 28 - this_player->items.weapon[REAR_WEAPON].power * 2;
						if (difficultyLevel > DIFFICULTY_HARD)
							chargeWait -= 5;
					}

					if (chargeLevel > 0)
						fill_rectangle_xy(VGAScreenSeg, 269, 107 + (chargeLevel - 1) * 3, 275, 108 + (chargeLevel - 1) * 3, 204);

					if (shotRepeat[SHOT_P2_CHARGE] > 0)
					{
						--shotRepeat[SHOT_P2_CHARGE];
					}
					else if (button[1-1] && (!twoPlayerLinked || chargeLevel > 0))
					{
						shotMultiPos[SHOT_P2_CHARGE] = 0;
						b = player_shot_create(16, SHOT_P2_CHARGE, this_player->x, this_player->y, *mouseX_, *mouseY_, chargeGunWeapons[player[1].items.weapon[REAR_WEAPON].id-1] + chargeLevel, playerNum_);

						if (chargeLevel > 0)
							fill_rectangle_xy(VGAScreenSeg, 269, 107 + (chargeLevel - 1) * 3, 275, 108 + (chargeLevel - 1) * 3, 193);

						chargeLevel = 0;
						chargeWait = 30 - this_player->items.weapon[REAR_WEAPON].power * 2;
					}
				}

				/*SUPER BOMB*/
				temp = playerNum_;
				if (temp == 0)
					temp = 1;  /*Get whether player 1 or 2*/

				if (player[temp-1].superbombs > 0)
				{
					if (shotRepeat[SHOT_P1_SUPERBOMB + temp-1] > 0)
					{
						--shotRepeat[SHOT_P1_SUPERBOMB + temp-1];
					}
					else if (button[3-1] || button[2-1])
					{
						--player[temp-1].superbombs;
						shotMultiPos[SHOT_P1_SUPERBOMB + temp-1] = 0;
						b = player_shot_create(16, SHOT_P1_SUPERBOMB + temp-1, this_player->x, this_player->y, *mouseX_, *mouseY_, 535, playerNum_);
					}
				}

				// sidekicks

				if (this_player->sidekick[LEFT_SIDEKICK].style == 4 && this_player->sidekick[RIGHT_SIDEKICK].style == 4)
					optionSatelliteRotate += 0.2f;
				else if (this_player->sidekick[LEFT_SIDEKICK].style == 4 || this_player->sidekick[RIGHT_SIDEKICK].style == 4)
					optionSatelliteRotate += 0.15f;

				switch (this_player->sidekick[LEFT_SIDEKICK].style)
				{
				case 1:  // trailing
				case 3:
					this_player->sidekick[LEFT_SIDEKICK].x = this_player->old_x[COUNTOF(player->old_x) / 2 - 1];
					this_player->sidekick[LEFT_SIDEKICK].y = this_player->old_y[COUNTOF(player->old_x) / 2 - 1];
					break;
				case 2:  // front-mounted
					this_player->sidekick[LEFT_SIDEKICK].x = this_player->x;
					this_player->sidekick[LEFT_SIDEKICK].y = MAX(10, this_player->y - 20);
					break;
				case 4:  // orbiting
					this_player->sidekick[LEFT_SIDEKICK].x = this_player->x + roundf(sinf(optionSatelliteRotate) * 20);
					this_player->sidekick[LEFT_SIDEKICK].y = this_player->y + roundf(cosf(optionSatelliteRotate) * 20);
					break;
				}

				switch (this_player->sidekick[RIGHT_SIDEKICK].style)
				{
				case 4:  // orbiting
					this_player->sidekick[RIGHT_SIDEKICK].x = this_player->x - roundf(sinf(optionSatelliteRotate) * 20);
					this_player->sidekick[RIGHT_SIDEKICK].y = this_player->y - roundf(cosf(optionSatelliteRotate) * 20);
					break;
				case 1:  // trailing
				case 3:
					this_player->sidekick[RIGHT_SIDEKICK].x = this_player->old_x[0];
					this_player->sidekick[RIGHT_SIDEKICK].y = this_player->old_y[0];
					break;
				case 2:  // front-mounted
					if (!optionAttachmentLinked)
					{
						this_player->sidekick[RIGHT_SIDEKICK].y += optionAttachmentMove / 2;
						if (optionAttachmentMove >= -2)
						{
							if (optionAttachmentReturn)
								temp = 2;
							else
								temp = 0;

							if (this_player->sidekick[RIGHT_SIDEKICK].y > (this_player->y - 20) + 5)
							{
								temp = 2;
								optionAttachmentMove -= 1 + optionAttachmentReturn;
							}
							else if (this_player->sidekick[RIGHT_SIDEKICK].y > (this_player->y - 20) - 0)
							{
								temp = 3;
								if (optionAttachmentMove > 0)
									optionAttachmentMove--;
								else
									optionAttachmentMove++;
							}
							else if (this_player->sidekick[RIGHT_SIDEKICK].y > (this_player->y - 20) - 5)
							{
								temp = 2;
								optionAttachmentMove++;
							}
							else if (optionAttachmentMove < 2 + optionAttachmentReturn * 4)
							{
								optionAttachmentMove += 1 + optionAttachmentReturn;
							}

							if (optionAttachmentReturn)
								temp = temp * 2;
							if (abs(this_player->sidekick[RIGHT_SIDEKICK].x - this_player->x) < temp)
								temp = 1;

							if (this_player->sidekick[RIGHT_SIDEKICK].x > this_player->x)
								this_player->sidekick[RIGHT_SIDEKICK].x -= temp;
							else if (this_player->sidekick[RIGHT_SIDEKICK].x < this_player->x)
								this_player->sidekick[RIGHT_SIDEKICK].x += temp;

							if (abs(this_player->sidekick[RIGHT_SIDEKICK].y - (this_player->y - 20)) + abs(this_player->sidekick[RIGHT_SIDEKICK].x - this_player->x) < 8)
							{
								optionAttachmentLinked = true;
								soundQueue[2] = S_CLINK;
							}

							if (button[3-1])
								optionAttachmentReturn = true;
						}
						else  // sidekick needs to catch up to player
						{
							optionAttachmentMove += 1 + optionAttachmentReturn;
							JE_setupExplosion(this_player->sidekick[RIGHT_SIDEKICK].x + 1, this_player->sidekick[RIGHT_SIDEKICK].y + 10, 0, 0, false, false);
						}
					}
					else
					{
						this_player->sidekick[RIGHT_SIDEKICK].x = this_player->x;
						this_player->sidekick[RIGHT_SIDEKICK].y = this_player->y - 20;
						if (button[3-1])
						{
							optionAttachmentLinked = false;
							optionAttachmentReturn = false;
							optionAttachmentMove = -20;
							soundQueue[3] = S_WEAPON_26;
						}
					}

					if (this_player->sidekick[RIGHT_SIDEKICK].y < 10)
						this_player->sidekick[RIGHT_SIDEKICK].y = 10;
					break;
				}

				if (playerNum_ == 2 || !twoPlayerMode)  // if player has sidekicks
				{
					for (uint i = 0; i < COUNTOF(player->items.sidekick); ++i)
					{
						uint shot_i = (i == 0) ? SHOT_LEFT_SIDEKICK : SHOT_RIGHT_SIDEKICK;

						JE_OptionType *this_option = &options[this_player->items.sidekick[i]];

						// fire/refill sidekick
						if (this_option->wport > 0)
						{
							if (shotRepeat[shot_i] > 0)
							{
								--shotRepeat[shot_i];
							}
							else
							{
								const int ammo_max = this_player->sidekick[i].ammo_max;

								if (ammo_max > 0)  // sidekick has limited ammo
								{
									if (this_player->sidekick[i].ammo_refill_ticks > 0)
									{
										--this_player->sidekick[i].ammo_refill_ticks;
									}
									else  // refill one ammo
									{
										this_player->sidekick[i].ammo_refill_ticks = this_player->sidekick[i].ammo_refill_ticks_max;

										if (this_player->sidekick[i].ammo < ammo_max)
											++this_player->sidekick[i].ammo;

										// draw sidekick refill ammo gauge
										const int y = hud_sidekick_y[twoPlayerMode ? 1 : 0][i] + 13;
										draw_segmented_gauge(VGAScreenSeg, 284, y, 112, 2, 2, MAX(1, ammo_max / 10), this_player->sidekick[i].ammo);
									}

									if (button[1 + i] && this_player->sidekick[i].ammo > 0)
									{
										b = player_shot_create(this_option->wport, shot_i, this_player->sidekick[i].x, this_player->sidekick[i].y, *mouseX_, *mouseY_, this_option->wpnum + this_player->sidekick[i].charge, playerNum_);

										--this_player->sidekick[i].ammo;
										if (this_player->sidekick[i].charge > 0)
										{
											shotMultiPos[shot_i] = 0;
											this_player->sidekick[i].charge = 0;
										}
										this_player->sidekick[i].charge_ticks = 20;
										this_player->sidekick[i].animation_enabled = true;

										// draw sidekick discharge ammo gauge
										const int y = hud_sidekick_y[twoPlayerMode ? 1 : 0][i] + 13;
										fill_rectangle_xy(VGAScreenSeg, 284, y, 312, y + 2, 0);
										draw_segmented_gauge(VGAScreenSeg, 284, y, 112, 2, 2, MAX(1, ammo_max / 10), this_player->sidekick[i].ammo);
									}
								}
								else  // has infinite ammo
								{
									if (button[0] || button[1 + i])
									{
										b = player_shot_create(this_option->wport, shot_i, this_player->sidekick[i].x, this_player->sidekick[i].y, *mouseX_, *mouseY_, this_option->wpnum + this_player->sidekick[i].charge, playerNum_);

										if (this_player->sidekick[i].charge > 0)
										{
											shotMultiPos[shot_i] = 0;
											this_player->sidekick[i].charge = 0;
										}
										this_player->sidekick[i].charge_ticks = 20;
										this_player->sidekick[i].animation_enabled = true;
									}
								}
							}
						}
					}
				}  // end of if player has sidekicks
			}  // !endLevel
		} // this_player->is_alive
	} // moveOK

	// draw sidekicks
	if ((playerNum_ == 2 || !twoPlayerMode) && !endLevel)
	{
		for (uint i = 0; i < COUNTOF(this_player->sidekick); ++i)
		{
			JE_OptionType *this_option = &options[this_player->items.sidekick[i]];

			if (this_option->option > 0)
			{
				if (this_player->sidekick[i].animation_enabled)
				{
					if (++this_player->sidekick[i].animation_frame >= this_option->ani)
					{
						this_player->sidekick[i].animation_frame = 0;
						this_player->sidekick[i].animation_enabled = (this_option->option == 1);
					}
				}

				const int x = this_player->sidekick[i].x,
				          y = this_player->sidekick[i].y;
				const uint sprite = this_option->gr[this_player->sidekick[i].animation_frame] + this_player->sidekick[i].charge;

				if (this_player->sidekick[i].style == 1 || this_player->sidekick[i].style == 2)
					blit_sprite2x2(VGAScreen, x - 6, y, spriteSheet10, sprite);
				else
					blit_sprite2(VGAScreen, x, y, spriteSheet9, sprite);
			}

			if (--this_player->sidekick[i].charge_ticks == 0)
			{
				if (this_player->sidekick[i].charge < this_option->pwr)
					++this_player->sidekick[i].charge;
				this_player->sidekick[i].charge_ticks = 20;
			}
		}
	}
}

void JE_mainGamePlayerFunctions(void)
{
	/*PLAYER MOVEMENT/MOUSE ROUTINES*/

	if (endLevel && levelEnd > 0)
	{
		levelEnd--;
		levelEndWarp++;
	}

	/*Reset Street-Fighter commands*/
	memset(SFExecuted, 0, sizeof(SFExecuted));

	portConfigChange = false;

	if (twoPlayerMode)
	{
		JE_playerMovement(&player[0],
		                  !galagaMode ? inputDevice[0] : 0, 1, shipGr, shipGrPtr,
		                  &mouseX, &mouseY);
		JE_playerMovement(&player[1],
		                  !galagaMode ? inputDevice[1] : 0, 2, shipGr2, shipGr2ptr,
		                  &mouseXB, &mouseYB);
	}
	else
	{
		JE_playerMovement(&player[0],
		                  0, 1, shipGr, shipGrPtr,
		                  &mouseX, &mouseY);
	}

	/* == Parallax Map Scrolling == */
	JE_word tempX;
	if (twoPlayerMode)
		tempX = (player[0].x + player[1].x) / 2;
	else
		tempX = player[0].x;

	tempW = floorf((float)(260 - (tempX - 36)) / (260 - 36) * (24 * 3) - 1);
	mapX3Ofs   = tempW;
	mapX3Pos   = mapX3Ofs % 24;
	mapX3bpPos = 1 - (mapX3Ofs / 24);

	mapX2Ofs   = (tempW * 2) / 3;
	mapX2Pos   = mapX2Ofs % 24;
	mapX2bpPos = 1 - (mapX2Ofs / 24);

	oldMapXOfs = mapXOfs;
	mapXOfs    = mapX2Ofs / 2;
	mapXPos    = mapXOfs % 24;
	mapXbpPos  = 1 - (mapXOfs / 24);

	if (background3x1)
	{
		mapX3Ofs = mapXOfs;
		mapX3Pos = mapXPos;
		mapX3bpPos = mapXbpPos - 1;
	}
}

const char *JE_getName(JE_byte pnum)
{
	if (pnum == thisPlayerNum && network_player_name[0] != '\0')
		return network_player_name;
	else if (network_opponent_name[0] != '\0')
		return network_opponent_name;

	return miscText[47 + pnum];
}

void JE_playerCollide(Player *this_player, JE_byte playerNum_)
{
	char tempStr[256];

	for (int z = 0; z < 100; z++)
	{
		if (enemyAvail[z] != 1)
		{
			int enemy_screen_x = enemy[z].ex + enemy[z].mapoffset;

			if (abs(this_player->x - enemy_screen_x) < 12 && abs(this_player->y - enemy[z].ey) < 14)
			{   /*Collide*/
				int evalue = enemy[z].evalue;
				if (evalue > 29999)
				{
					if (evalue == 30000)  // spawn dragonwing in galaga mode, otherwise just a purple ball
					{
						this_player->cash += 100;

						if (!galagaMode)
						{
							handle_got_purple_ball(this_player);
						}
						else
						{
							// spawn the dragonwing?
							if (twoPlayerMode)
								this_player->cash += 2400;
							twoPlayerMode = true;
							twoPlayerLinked = true;
							player[1].items.weapon[REAR_WEAPON].power = 1;
							player[1].armor = 10;
							player[1].is_alive = true;
						}
						enemyAvail[z] = 1;
						soundQueue[7] = S_POWERUP;
					}
					else if (superArcadeMode != SA_NONE && evalue > 30000)
					{
						shotMultiPos[SHOT_FRONT] = 0;
						shotRepeat[SHOT_FRONT] = 10;

						tempW = SAWeapon[superArcadeMode-1][evalue - 30000-1];

						// if picked up already-owned weapon, power weapon up
						if (tempW == player[0].items.weapon[FRONT_WEAPON].id)
						{
							this_player->cash += 1000;
							power_up_weapon(this_player, FRONT_WEAPON);
						}
						// else weapon also gives purple ball
						else
						{
							handle_got_purple_ball(this_player);
						}

						player[0].items.weapon[FRONT_WEAPON].id = tempW;
						this_player->cash += 200;
						soundQueue[7] = S_POWERUP;
						enemyAvail[z] = 1;
					}
					else if (evalue > 32100)
					{
						if (playerNum_ == 1)
						{
							this_player->cash += 250;
							player[0].items.special = evalue - 32100;
							shotMultiPos[SHOT_SPECIAL] = 0;
							shotRepeat[SHOT_SPECIAL] = 10;
							shotMultiPos[SHOT_SPECIAL2] = 0;
							shotRepeat[SHOT_SPECIAL2] = 0;

							if (isNetworkGame)
								snprintf(tempStr, sizeof tempStr, "%s %s %s", JE_getName(1), miscTextB[4-1], special[evalue - 32100].name);
							else if (twoPlayerMode)
								sprintf(tempStr, "%s %s", miscText[43-1], special[evalue - 32100].name);
							else
								sprintf(tempStr, "%s %s", miscText[64-1], special[evalue - 32100].name);
							JE_drawTextWindow(tempStr);
							soundQueue[7] = S_POWERUP;
							enemyAvail[z] = 1;
						}
					}
					else if (evalue > 32000)
					{
						if (playerNum_ == 2)
						{
							enemyAvail[z] = 1;
							if (isNetworkGame)
								snprintf(tempStr, sizeof tempStr, "%s %s %s", JE_getName(2), miscTextB[4-1], options[evalue - 32000].name);
							else
								sprintf(tempStr, "%s %s", miscText[44-1], options[evalue - 32000].name);
							JE_drawTextWindow(tempStr);

							// if picked up a different sidekick than player already has, then reset sidekicks to least powerful, else power them up
							if (evalue - 32000u != player[1].items.sidekick_series)
							{
								player[1].items.sidekick_series = evalue - 32000;
								player[1].items.sidekick_level = 101;
							}
							else if (player[1].items.sidekick_level < 103)
							{
								++player[1].items.sidekick_level;
							}

							uint temp = player[1].items.sidekick_level - 100 - 1;
							for (uint i = 0; i < COUNTOF(player[1].items.sidekick); ++i)
								player[1].items.sidekick[i] = optionSelect[player[1].items.sidekick_series][temp][i];

							shotMultiPos[SHOT_LEFT_SIDEKICK] = 0;
							shotMultiPos[SHOT_RIGHT_SIDEKICK] = 0;
							JE_drawOptions();
							soundQueue[7] = S_POWERUP;
						}
						else if (onePlayerAction)
						{
							enemyAvail[z] = 1;
							sprintf(tempStr, "%s %s", miscText[64-1], options[evalue - 32000].name);
							JE_drawTextWindow(tempStr);

							for (uint i = 0; i < COUNTOF(player[0].items.sidekick); ++i)
								player[0].items.sidekick[i] = evalue - 32000;
							shotMultiPos[SHOT_LEFT_SIDEKICK] = 0;
							shotMultiPos[SHOT_RIGHT_SIDEKICK] = 0;

							JE_drawOptions();
							soundQueue[7] = S_POWERUP;
						}
						if (enemyAvail[z] == 1)
							this_player->cash += 250;
					}
					else if (evalue > 31000)
					{
						this_player->cash += 250;
						if (playerNum_ == 2)
						{
							if (isNetworkGame)
								snprintf(tempStr, sizeof tempStr, "%s %s %s", JE_getName(2), miscTextB[4-1], weaponPort[evalue - 31000].name);
							else
								sprintf(tempStr, "%s %s", miscText[44-1], weaponPort[evalue - 31000].name);
							JE_drawTextWindow(tempStr);
							player[1].items.weapon[REAR_WEAPON].id = evalue - 31000;
							shotMultiPos[SHOT_REAR] = 0;
							enemyAvail[z] = 1;
							soundQueue[7] = S_POWERUP;
						}
						else if (onePlayerAction)
						{
							sprintf(tempStr, "%s %s", miscText[64-1], weaponPort[evalue - 31000].name);
							JE_drawTextWindow(tempStr);
							player[0].items.weapon[REAR_WEAPON].id = evalue - 31000;
							shotMultiPos[SHOT_REAR] = 0;
							enemyAvail[z] = 1;
							soundQueue[7] = S_POWERUP;

							if (player[0].items.weapon[REAR_WEAPON].power == 0)  // does this ever happen?
								player[0].items.weapon[REAR_WEAPON].power = 1;
						}
					}
					else if (evalue > 30000)
					{
						if (playerNum_ == 1 && twoPlayerMode)
						{
							if (isNetworkGame)
								snprintf(tempStr, sizeof tempStr, "%s %s %s", JE_getName(1), miscTextB[4-1], weaponPort[evalue - 30000].name);
							else
								sprintf(tempStr, "%s %s", miscText[43-1], weaponPort[evalue - 30000].name);
							JE_drawTextWindow(tempStr);
							player[0].items.weapon[FRONT_WEAPON].id = evalue - 30000;
							shotMultiPos[SHOT_FRONT] = 0;
							enemyAvail[z] = 1;
							soundQueue[7] = S_POWERUP;
						}
						else if (onePlayerAction)
						{
							sprintf(tempStr, "%s %s", miscText[64-1], weaponPort[evalue - 30000].name);
							JE_drawTextWindow(tempStr);
							player[0].items.weapon[FRONT_WEAPON].id = evalue - 30000;
							shotMultiPos[SHOT_FRONT] = 0;
							enemyAvail[z] = 1;
							soundQueue[7] = S_POWERUP;
						}

						if (enemyAvail[z] == 1)
						{
							player[0].items.special = specialArcadeWeapon[evalue - 30000-1];
							if (player[0].items.special > 0)
							{
								shotMultiPos[SHOT_SPECIAL] = 0;
								shotRepeat[SHOT_SPECIAL] = 0;
								shotMultiPos[SHOT_SPECIAL2] = 0;
								shotRepeat[SHOT_SPECIAL2] = 0;
							}
							this_player->cash += 250;
						}

					}
				}
				else if (evalue > 20000)
				{
					if (twoPlayerLinked)
					{
						// share the armor evenly between linked players
						for (uint i = 0; i < COUNTOF(player); ++i)
						{
							player[i].armor += (evalue - 20000) / COUNTOF(player);
							if (player[i].armor > 28)
								player[i].armor = 28;
						}
					}
					else
					{
						this_player->armor += evalue - 20000;
						if (this_player->armor > 28)
							this_player->armor = 28;
					}
					enemyAvail[z] = 1;
					VGAScreen = VGAScreenSeg; /* side-effect of game_screen */
					JE_drawArmor();
					VGAScreen = game_screen; /* side-effect of game_screen */
					soundQueue[7] = S_POWERUP;
				}
				else if (evalue > 10000 && enemyAvail[z] == 2)
				{
					if (!bonusLevel)
					{
						play_song(30);  /*Zanac*/
						bonusLevel = true;
						nextLevel = evalue - 10000;
						enemyAvail[z] = 1;
						displayTime = 150;
					}
				}
				else if (enemy[z].scoreitem)
				{
					enemyAvail[z] = 1;
					soundQueue[7] = S_ITEM;
					if (evalue == 1)
					{
						cubeMax++;
						soundQueue[3] = V_DATA_CUBE;
					}
					else if (evalue == -1)  // got front weapon powerup
					{
						if (isNetworkGame)
							snprintf(tempStr, sizeof tempStr, "%s %s %s", JE_getName(1), miscTextB[4-1], miscText[45-1]);
						else if (twoPlayerMode)
							sprintf(tempStr, "%s %s", miscText[43-1], miscText[45-1]);
						else
							strcpy(tempStr, miscText[45-1]);
						JE_drawTextWindow(tempStr);

						power_up_weapon(&player[0], FRONT_WEAPON);
						soundQueue[7] = S_POWERUP;
					}
					else if (evalue == -2)  // got rear weapon powerup
					{
						if (isNetworkGame)
							snprintf(tempStr, sizeof tempStr, "%s %s %s", JE_getName(2), miscTextB[4-1], miscText[46-1]);
						else if (twoPlayerMode)
							sprintf(tempStr, "%s %s", miscText[44-1], miscText[46-1]);
						else
							strcpy(tempStr, miscText[46-1]);
						JE_drawTextWindow(tempStr);

						power_up_weapon(twoPlayerMode ? &player[1] : &player[0], REAR_WEAPON);
						soundQueue[7] = S_POWERUP;
					}
					else if (evalue == -3)
					{
						// picked up orbiting asteroid killer
						shotMultiPos[SHOT_MISC] = 0;
						b = player_shot_create(0, SHOT_MISC, this_player->x, this_player->y, mouseX, mouseY, 104, playerNum_);
						shotAvail[z] = 0;
					}
					else if (evalue == -4)
					{
						if (player[playerNum_-1].superbombs < 10)
							++player[playerNum_-1].superbombs;
					}
					else if (evalue == -5)
					{
						player[0].items.weapon[FRONT_WEAPON].id = 25;  // HOT DOG!
						player[0].items.weapon[REAR_WEAPON].id = 26;
						player[1].items.weapon[REAR_WEAPON].id = 26;

						player[0].last_items = player[0].items;

						for (uint i = 0; i < COUNTOF(player); ++i)
							player[i].weapon_mode = 1;

						memset(shotMultiPos, 0, sizeof(shotMultiPos));
					}
					else if (twoPlayerLinked)
					{
						// players get equal share of pick-up cash when linked
						for (uint i = 0; i < COUNTOF(player); ++i)
							player[i].cash += evalue / COUNTOF(player);
					}
					else
					{
						this_player->cash += evalue;
					}
					JE_setupExplosion(enemy_screen_x, enemy[z].ey, 0, enemyDat[enemy[z].enemytype].explosiontype, true, false);
				}
				else if (this_player->invulnerable_ticks == 0 && enemyAvail[z] == 0 &&
				         (enemyDat[enemy[z].enemytype].explosiontype & 1) == 0) // explosiontype & 1 == 0: not ground enemy
				{
					int armorleft = enemy[z].armorleft;
					if (armorleft > damageRate)
						armorleft = damageRate;

					JE_playerDamage(armorleft, this_player);

					// player ship gets push-back from collision
					if (enemy[z].armorleft > 0)
					{
						this_player->x_velocity += (enemy[z].exc * enemy[z].armorleft) / 2;
						this_player->y_velocity += (enemy[z].eyc * enemy[z].armorleft) / 2;
					}

					int armorleft2 = enemy[z].armorleft;
					if (armorleft2 == 255)
						armorleft2 = 30000;

					temp = enemy[z].linknum;
					if (temp == 0)
						temp = 255;

					b = z;

					if (armorleft2 > armorleft)
					{
						// damage enemy
						if (enemy[z].armorleft != 255)
							enemy[z].armorleft -= armorleft;
						soundQueue[5] = S_ENEMY_HIT;
					}
					else
					{
						// kill enemy
						for (temp2 = 0; temp2 < 100; temp2++)
						{
							if (enemyAvail[temp2] != 1)
							{
								temp3 = enemy[temp2].linknum;
								if (temp2 == b ||
									(temp != 255 &&
									 (temp == temp3 || temp - 100 == temp3 ||
									  (temp3 > 40 && temp3 / 20 == temp / 20 && temp3 <= temp))))
								{
									int enemy_screen_x = enemy[temp2].ex + enemy[temp2].mapoffset;

									enemy[temp2].linknum = 0;

									enemyAvail[temp2] = 1;

									if (enemyDat[enemy[temp2].enemytype].esize == 1)
									{
										JE_setupExplosionLarge(enemy[temp2].enemyground, enemy[temp2].explonum, enemy_screen_x, enemy[temp2].ey);
										soundQueue[6] = S_EXPLOSION_9;
									}
									else
									{
										JE_setupExplosion(enemy_screen_x, enemy[temp2].ey, 0, 1, false, false);
										soundQueue[5] = S_EXPLOSION_4;
									}
								}
							}
						}
						enemyAvail[z] = 1;
					}
				}
			}

		}
	}
}
