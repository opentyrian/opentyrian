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

#include "backgrnd.h"
#include "fonthand.h"
#include "joystick.h"
#include "jukebox.h"
#include "keyboard.h"
#include "newshape.h"
#include "nortsong.h"
#include "nortvars.h"
#include "picload.h"
#include "starfade.h"
#include "varz.h"
#include "vga256d.h"

#include "SDL.h"


#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

const struct about_text_type about_text[] =
{
	{0x10, "----- ~OpenTyrian~ -----"},
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
	{0x10, "----- ~Developers~ -----"}, /* in alphabetical order */
	{0x00, ""},
	{0x03, "MindlessXD"},
	{0x00, ""},
	{0x04, "syntaxglitch"},
	{0x00, ""},
	{0x07, "yuriks"},
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
	{0x12, "And a big thanks to ~Jason~"},
	{0x12, "for making this possible."},
	{0x00, ""},
	{0x00, ""},
	{0x00, ""},
	{0x00, ""},
	{0x00, "Press a key to leave."},
	{0x00, NULL}
};

#define BEER_SHAPE 241

/* Text is an array of strings terminated by a NULL */
void scroller_sine( const struct about_text_type text[] )
{
	int visible_lines = surface_height / 15 + 1;
	int current_line = -visible_lines;
	int y = 0;
	bool fade_in = true;
	
	struct { int x, y, ay, vx, vy; } beer[5];
	
	memset(beer, 0, sizeof(beer));
	
	JE_fadeBlack(10);
	
	wait_noinput(true, true, joystick_installed);
	
	currentJukeboxSong = 41; /* BEER! =D */
	JE_playSong(currentJukeboxSong);
	
	while (!JE_anyButton())
	{
		setdelay(3);
		
		JE_clr256();
		
		for (int i = 0; i < visible_lines; i++)
		{
			if (current_line + i >= 0)
			{
				if (text[current_line + i].text == NULL)
					goto no_more_text;
				
				int line_x = JE_fontCenter(text[i + current_line].text, SMALL_FONT_SHAPES);
				int line_y = i * 15 - y;
				
				JE_outTextAdjust(line_x, line_y, text[i + current_line].text, text[i + current_line].effect & 0x0f, -4, SMALL_FONT_SHAPES, false);
				
				if (text[i + current_line].effect & 0x10)
					for (int j = 0; j < 15; j++)
						if (line_y + j >= 10 && line_y + j <= surface_height - 10)
						{
							int foo = sin((((line_y + j) / 2) % 13) / 6.5f * M_PI) * 3;
							memmove(&((Uint8 *)VGAScreenSeg->pixels)[VGAScreenSeg->pitch * (line_y + j) + foo],
									&((Uint8 *)VGAScreenSeg->pixels)[VGAScreenSeg->pitch * (line_y + j)],
									VGAScreenSeg->pitch);
						}
			}
		}
no_more_text:
		
		y++;
		y %= 15;
		if (y == 0 && (current_line < 0 || text[current_line].text != NULL)) // stop if we've hit the end of text
			current_line++;
		
		JE_bar(0, 0, surface_width - 1, 9, 0);
		JE_bar(0, surface_height - 10, surface_width - 1, surface_height - 1, 0);
		
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
				beer[i].vx = -beer[i].vx;
			beer[i].x += beer[i].vx;
			
			if (beer[i].y + beer[i].vy > surface_height - 28) // check if the beer hit the bottom
			{
				beer[i].vy = -(beer[i].vy * 15 / 16) + (rand() % 3 - 1);
				if ((beer[i].vy) > -10) // make sure the beer bounces!
					beer[i].vy--;
			}
			beer[i].y += beer[i].vy;
			
			JE_drawShape2x2(beer[i].x, beer[i].y, BEER_SHAPE, eShapes5);
		}
		
		JE_showVGA();
		
		if (fade_in)
		{
			fade_in = false;
			JE_fadeColor(10);
			JE_setPalette(254, 63, 63, 63);
		}
		
		int delaycount_temp;
		if ((delaycount_temp = target - SDL_GetTicks()) > 0)
			SDL_Delay(delaycount_temp);
	}
	
	JE_fadeBlack(10);
}
