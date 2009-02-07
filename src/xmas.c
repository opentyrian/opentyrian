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
#include "xmas.h"
#include "fonthand.h"
#include "keyboard.h"
#include "newshape.h"
#include "palette.h"
#include "setup.h"
#include "video.h"

#include <stdio.h>
#include <time.h>

bool xmas = false;

bool xmas_time( void )
{
	time_t now = time(NULL);
	return localtime(&now)->tm_mon == 11;
}

bool xmas_prompt( void )
{
	const char *xmas_text[] = {
		"Christmas has been detected.",
		"Activate Christmas?",
		"Yes",
		"No"
	};
	
	JE_updateColorsFast(palettes[0]);
	
	JE_outTextAdjust(JE_fontCenter(xmas_text[0], SMALL_FONT_SHAPES), 85, xmas_text[0], 4, -2, SMALL_FONT_SHAPES, true);
	JE_outTextAdjust(JE_fontCenter(xmas_text[1], SMALL_FONT_SHAPES), 100, xmas_text[1], 2, -2, SMALL_FONT_SHAPES, false);
	
	int selection = 0;
	
	bool decided = false;
	while (!decided)
	{
		JE_outTextAdjust(JE_fontCenter(xmas_text[2], SMALL_FONT_SHAPES) - 20, 120, xmas_text[2], 15, (selection == 0) ? -2 : -4, SMALL_FONT_SHAPES, true);
		JE_outTextAdjust(JE_fontCenter(xmas_text[3], SMALL_FONT_SHAPES) + 20, 120, xmas_text[3], 15, (selection == 1) ? -2 : -4, SMALL_FONT_SHAPES, true);
		
		JE_showVGA();
		
		JE_word temp = 0;
		JE_textMenuWait(&temp, false);
		
		if (newkey)
		{
			switch (lastkey_sym)
			{
				case SDLK_LEFT:
					if (selection == 0)
						selection = 2;
					selection--;
					break;
				case SDLK_RIGHT:
					selection++;
					selection %= 2;
					break;
				case SDLK_RETURN:
					decided = true;
					break;
				case SDLK_ESCAPE:
					return false;
				default:
					break;
			}
		}
	}
	
	return (selection == 0);
}
