/* vim: set noet:
 *
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
#include "scroller.h"

#include "fonthand.h"
#include "joystick.h"
#include "jukebox.h"
#include "keyboard.h"
#include "newshape.h"
#include "nortsong.h"
#include "starfade.h"
#include "varz.h"
#include "vga256d.h"


const struct about_text_type about_text[] =
{
	{0x30, "----- ~OpenTyrian~ -----"},
	{0x00, ""},
	{0x00, ""},
	{0x00, "This is a test of the credits"},
	{0x00, "scroller."},
	{0x00, ""},
	{0x00, ""},
	{0x00, "If this had been an ~actual~ scroller"},
	{0x00, "credit text would be in place"},
	{0x00, "of this message."},
	{0x00, ""},
	{0x00, ""},
	{0x30, "----- ~Developers~ -----"}, /* in alphabetical order */
	{0x00, ""},
	{0x03, "MindlessXD"},
	{0x06, "mwolson"},
	{0x04, "syntaxglitch"},
	{0x07, "yuriks"},
	{0x00, ""},
	{0x00, ""},
	{0x05, "Thanks to the ~AdPlug~ team for donating"},
	{0x05, "the FM synthesis and LDS loading code."},
	{0x00, ""},
	{0x00, ""},
	{0x32, "And a big thanks to ~Jason Emery~"},
	{0x32, "for making all this possible."},
	{0x00, ""},
	{0x00, ""},
	{0x00, "This is line color test ~0~."},
	{0x01, "This is line color test ~1~."},
	{0x02, "This is line color test ~2~."},
	{0x03, "This is line color test ~3~."},
	{0x04, "This is line color test ~4~."},
	{0x05, "This is line color test ~5~."},
	{0x06, "This is line color test ~6~."},
	{0x07, "This is line color test ~7~."},
	{0x08, "This is line color test ~8~."},
	{0x09, "This is line color test ~9~."},
	{0x0a, "This is line color test ~A~."},
	{0x0b, "This is line color test ~B~."},
	{0x0c, "This is line color test ~C~."},
	{0x0d, "This is line color test ~D~."},
	{0x0e, "This is line color test ~E~."},
	{0x0f, "This is line color test ~F~."},
	{0x00, ""},
	{0x00, ""},
	{0x00, ""},
	{0x00, "Press a key to leave."},
	{0x00, NULL}
};

#define LINE_HEIGHT 15

#define MAX_BEER 5
#define BEER_SHAPE 241

struct coin_def_type {
	int shape_num;
	int frame_count;
	bool reverse_anim;
};

#define MAX_COINS 20
struct coin_def_type coin_defs[] =
{
	{1, 6}, {7, 6}, {20, 6}, {26, 6}, // Coins
	{14, 5, true}, {32, 5, true}, {51, 5, true} // Gems
};

/* Text is an array of strings terminated by a NULL */
void scroller_sine( const struct about_text_type text[] )
{
	bool ale = rand() % 2;

	int visible_lines = surface_height / LINE_HEIGHT + 1;
	int current_line = -visible_lines;
	int y = 0;
	bool fade_in = true;
	
	struct coin_type { int x, y, vel, type, cur_frame; bool backwards; } coins[MAX_COINS];
	struct { int x, y, ay, vx, vy; } beer[MAX_BEER];

	if (ale)
	{
		memset(beer, 0, sizeof(beer));
	} else {
		for (int i = 0; i < MAX_COINS; i++)
		{
			coins[i].x = rand() % (surface_width - 12);
			coins[i].y = rand() % (surface_height - 20 - 14);
			
			coins[i].vel = (rand() % 4) + 1;
			coins[i].type = rand() % COUNTOF(coin_defs);
			coins[i].cur_frame = rand() % coin_defs[coins[i].type].frame_count;
			coins[i].backwards = false;
		}
	}
	
	JE_fadeBlack(10);
	
	wait_noinput(true, true, joystick_installed);
	
	currentJukeboxSong = 41; /* BEER! =D */
	JE_playSong(currentJukeboxSong);

	while (!JE_anyButton())
	{
		setdelay(3);

		JE_clr256();

		if (!ale)
		{
			for (int i = 0; i < MAX_COINS/2; i++)
			{
				struct coin_type *coin = &coins[i];
				JE_drawShape2(coin->x, coin->y, coin_defs[coin->type].shape_num + coin->cur_frame, eShapes5);
			}
		}

		for (int i = 0; i < visible_lines; i++)
		{
			if (current_line + i >= 0)
			{
				if (text[current_line + i].text == NULL)
				{
					break;
				}
				
				int line_x = JE_fontCenter(text[i + current_line].text, SMALL_FONT_SHAPES);
				int line_y = i * LINE_HEIGHT - y;
				
				if (text[i + current_line].effect & 0x20)
				{
					JE_outTextAdjust(line_x + 1, line_y, text[i + current_line].text, text[i + current_line].effect & 0x0f, -10, SMALL_FONT_SHAPES, false);
					JE_outTextAdjust(line_x - 1, line_y, text[i + current_line].text, text[i + current_line].effect & 0x0f, -10, SMALL_FONT_SHAPES, false);
				}
				
				JE_outTextAdjust(line_x, line_y, text[i + current_line].text, text[i + current_line].effect & 0x0f, -4, SMALL_FONT_SHAPES, false);
				
				if (text[i + current_line].effect & 0x10)
				{
					for (int j = 0; j < LINE_HEIGHT; j++)
					{
						if (line_y + j >= 10 && line_y + j <= surface_height - 10)
						{
							int foo = sin((((line_y + j) / 2) % 10) / 5.0f * M_PI) * 3;
							memmove(&((Uint8 *)VGAScreenSeg->pixels)[VGAScreenSeg->pitch * (line_y + j) + foo],
									&((Uint8 *)VGAScreenSeg->pixels)[VGAScreenSeg->pitch * (line_y + j)],
									VGAScreenSeg->pitch);
						}
					}
				}
			}
		}

		y++;
		y %= LINE_HEIGHT;
		if (y == 0)
		{
			if (current_line < 0 || text[current_line].text != NULL)
			{
				current_line++;
			} else {
				current_line = -visible_lines;
			}
		}

		if (!ale)
		{
			for (int i = MAX_COINS/2; i < MAX_COINS; i++)
			{
				struct coin_type *coin = &coins[i];
				JE_drawShape2(coin->x, coin->y, coin_defs[coin->type].shape_num + coin->cur_frame, eShapes5);
			}
		}

		JE_bar(0, 0, surface_width - 1, 14, 0);
		JE_bar(0, surface_height - 14, surface_width - 1, surface_height - 1, 0);
		
		if (!ale)
		{
			for (int i = 0; i < MAX_COINS; i++)
			{
				struct coin_type *coin = &coins[i];
				
				if (coin->backwards)
				{
					coin->cur_frame--;
				} else {
					coin->cur_frame++;
				}
				if (coin->cur_frame == coin_defs[coin->type].frame_count)
				{
					if (coin_defs[coin->type].reverse_anim)
					{
						coin->backwards = true;
						coin->cur_frame -= 2;
					} else {
						coin->cur_frame = 0;
					}
				}
				if (coin->cur_frame == -1)
				{
					coin->cur_frame = 1;
					coin->backwards = false;
				}
				
				coin->y += coin->vel;
				if (coin->y > surface_height - 14)
				{
					coin->x = rand() % (surface_width - 12);
					coin->y = 0;
					
					coin->vel = (rand() % 4) + 1;
					coin->type = rand() % COUNTOF(coin_defs);
					coin->cur_frame = rand() % coin_defs[coin->type].frame_count;
				}
			}
		} else {
			for (int i = 0; i < COUNTOF(beer); i++)
			{
				while (beer[i].vx == 0)
				{
					beer[i].x = rand() % (surface_width - 24);
					beer[i].y = rand() % (surface_height - 28 - 50);
					
					beer[i].vx = (rand() % 5) - 2;
				}
				
				beer[i].vy++;
				
				if (beer[i].x + beer[i].vx > surface_width - 24 || beer[i].x + beer[i].vx < 0) // check if the beer hit the sides
				{
					beer[i].vx = -beer[i].vx;
				}
				beer[i].x += beer[i].vx;
				
				if (beer[i].y + beer[i].vy > surface_height - 28) // check if the beer hit the bottom
				{
					if ((beer[i].vy) < 8) // make sure the beer bounces!
					{
						beer[i].vy += rand() % 2;
					} else if (beer[i].vy > 16) { // make sure the beer doesn't bounce too high
						beer[i].vy = 16;
					}
					beer[i].vy = -beer[i].vy + (rand() % 3 - 1);
					
					beer[i].x += (beer[i].vx > 0 ? 1 : -1) * (i % 2 ? 1 : -1);
				}
				beer[i].y += beer[i].vy;
				
				JE_drawShape2x2(beer[i].x, beer[i].y, BEER_SHAPE, eShapes5);
			}
		}
		
		JE_showVGA();
		
		if (fade_in)
		{
			fade_in = false;
			JE_fadeColor(10);
			JE_setPalette(254, 63, 63, 63);
		}
		
		int delaycount_temp = target - SDL_GetTicks();
		if (delaycount_temp > 0)
		{
			SDL_Delay(delaycount_temp);
		}
	}
	
	JE_fadeBlack(10);
}
