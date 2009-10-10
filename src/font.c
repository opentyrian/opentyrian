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
#include "font.h"
#include "fonthand.h"
#include "sprite.h"

#include <stdbool.h>

void font_draw_hv( SDL_Surface *surface, int x, int y, const char *text, Font font, FontAlignment alignment, Uint8 hue, Sint8 value )
{
	switch (alignment)
	{
	case left_aligned:
		break;
	case centered:
		x -= JE_textWidth(text, font) / 2;
		break;
	case right_aligned:
		x -= JE_textWidth(text, font);
		break;
	}
	
	bool highlight = false;
	
	for (; *text != '\0'; ++text)
	{
		int sprite_id = font_ascii[(unsigned char)*text];
		
		switch (*text)
		{
		case ' ':
			x += 6;
			break;
			
		case '~':
			highlight = !highlight;
			if (highlight)
				value += 4;
			else
				value -= 4;
			break;
			
		default:
			if (sprite_id != -1 && sprite_exists(font, sprite_id))
			{
				blit_sprite_hv(surface, x, y, font, sprite_id, hue, value);
				
				x += sprite(font, sprite_id)->width + 1;
			}
			break;
		}
	}
}
