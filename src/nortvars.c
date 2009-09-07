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
#include "file.h"
#include "joystick.h"
#include "keyboard.h"
#include "nortvars.h"
#include "opentyr.h"
#include "vga256d.h"
#include "video.h"

#include <ctype.h>

JE_boolean inputDetected;
JE_word lastMouseX, lastMouseY;

JE_byte mouseCursor;
JE_boolean mouse_threeButton = true;
JE_word mouseX, mouseY, mouseButton;

JE_word JE_btow(JE_byte a, JE_byte b)
{
	return (JE_word) (((short) b) * 256 + a);
}

void JE_loadCompShapes( JE_byte **shapes, JE_word *shapeSize, JE_char s )
{
	char buffer[11];
	sprintf(buffer, "newsh%c.shp", tolower(s));

	if (*shapes != NULL)
	{
		free(*shapes);
	}

	FILE *f = dir_fopen_die(data_dir(), buffer, "rb");

	fseek(f, 0, SEEK_END);
	*shapeSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	*shapes = malloc(*shapeSize);

	efread(*shapes, sizeof(JE_byte), *shapeSize, f);

	fclose(f);
}

void JE_drawShape2( int x, int y, int s_, JE_byte *shape )
{
	JE_byte *p; /* shape pointer */
	Uint8 *s; /* screen pointer, 8-bit specific */
	Uint8 *s_limit; /* buffer boundary */

	int i;

	s = (Uint8 *)VGAScreen->pixels;
	s += y * VGAScreen->pitch + x;

	s_limit = (Uint8 *)VGAScreen->pixels;
	s_limit += VGAScreen->h * VGAScreen->pitch;

	p = shape;
	p += SDL_SwapLE16(((JE_word *)p)[s_ - 1]);

	while (*p != 0x0f)
	{
		s += *p & 0x0f;
		i = (*p & 0xf0) >> 4;
		if (i)
		{
			while (i--)
			{
				p++;
				if (s >= s_limit)
					return;
				if ((void *)s >= VGAScreen->pixels)
					*s = *p;
				s++;
			}
		} else {
			s -= 12;
			s += VGAScreen->pitch;
		}
		p++;
	}
}

void JE_superDrawShape2( int x, int y, int s_, JE_byte *shape )
{
	JE_byte *p; /* shape pointer */
	Uint8 *s; /* screen pointer, 8-bit specific */
	Uint8 *s_limit; /* buffer boundary */

	int i;

	s = (Uint8 *)VGAScreen->pixels;
	s += y * VGAScreen->pitch + x;

	s_limit = (Uint8 *)VGAScreen->pixels;
	s_limit += VGAScreen->h * VGAScreen->pitch;

	p = shape;
	p += SDL_SwapLE16(((JE_word *)p)[s_ - 1]);

	while (*p != 0x0f)
	{
		s += *p & 0x0f;
		i = (*p & 0xf0) >> 4;
		if (i)
		{
			while (i--)
			{
				p++;
				if (s >= s_limit)
					return;
				if ((void *)s >= VGAScreen->pixels)
					*s = (((*p & 0x0f) + (*s & 0x0f)) >> 1) | (*p & 0xf0);
				s++;
			}
		} else {
			s -= 12;
			s += VGAScreen->pitch;
		}
		p++;
	}
}

void JE_drawShape2Shadow( int x, int y, int s_, JE_byte *shape )
{
	JE_byte *p; /* shape pointer */
	Uint8 *s; /* screen pointer, 8-bit specific */
	Uint8 *s_limit; /* buffer boundary */

	int i;

	s = (Uint8 *)VGAScreen->pixels;
	s += y * VGAScreen->pitch + x;

	s_limit = (Uint8 *)VGAScreen->pixels;
	s_limit += VGAScreen->h * VGAScreen->pitch;

	p = shape;
	p += SDL_SwapLE16(((JE_word *)p)[s_ - 1]);

	while (*p != 0x0f)
	{
		s += *p & 0x0f;
		i = (*p & 0xf0) >> 4;
		if (i)
		{
			while (i--)
			{
				p++;
				if (s >= s_limit)
					return;
				if ((void *)s >= VGAScreen->pixels)
					*s = ((*s & 0x0f) >> 1) + (*s & 0xf0);
				s++;
			}
		} else {
			s -= 12;
			s += VGAScreen->pitch;
		}
		p++;
	}
}

void JE_drawShape2x2( int x, int y, int s, JE_byte *shape )
{
	JE_drawShape2(x,    y,    s,    shape);
	JE_drawShape2(x+12, y,    s+1,  shape);
	JE_drawShape2(x,    y+14, s+19, shape);
	JE_drawShape2(x+12, y+14, s+20, shape);
}

void JE_superDrawShape2x2( int x, int y, int s, JE_byte *shape )
{
	JE_superDrawShape2(x,    y,    s,    shape);
	JE_superDrawShape2(x+12, y,    s+1,  shape);
	JE_superDrawShape2(x,    y+14, s+19, shape);
	JE_superDrawShape2(x+12, y+14, s+20, shape);
}

void JE_drawShape2x2Shadow( int x, int y, int s, JE_byte *shape )
{
	JE_drawShape2Shadow(x,    y,    s,    shape);
	JE_drawShape2Shadow(x+12, y,    s+1,  shape);
	JE_drawShape2Shadow(x,    y+14, s+19, shape);
	JE_drawShape2Shadow(x+12, y+14, s+20, shape);
}

JE_boolean JE_anyButton( void )
{
	poll_joysticks();
	service_SDL_events(true);
	return newkey || mousedown || joydown;
}

void JE_dBar3( JE_integer x,  JE_integer y,  JE_integer num,  JE_integer col )
{
	JE_byte z;
	JE_byte zWait = 2;

	col += 2;

	for (z = 0; z <= num; z++)
	{
		JE_rectangle(x, y - 1, x + 8, y, col); /* <MXD> SEGa000 */
		if (zWait > 0)
		{
			zWait--;
		} else {
			col++;
			zWait = 1;
		}
		y -= 2;
	}
}

void JE_barDrawShadow( JE_word x, JE_word y, JE_word res, JE_word col, JE_word amt, JE_word xsize, JE_word ysize )
{
	xsize--;
	ysize--;

	for (int z = 1; z <= amt / res; z++)
	{
		JE_barShade(x+2, y+2, x+xsize+2, y+ysize+2);
		JE_bar(x, y, x+xsize, y+ysize, col+12);
		JE_bar(x, y, x+xsize, y, col+13);
		JE_pix(x, y, col+15);
		JE_bar(x, y+ysize, x+xsize, y+ysize, col+11);
		x += xsize + 2;
	}

	amt %= res;
	if (amt > 0)
	{
		JE_barShade(x+2, y+2, x+xsize+2, y+ysize+2);
		JE_bar(x,y, x+xsize, y+ysize, col+(12 / res * amt));
	}
}

void JE_barDrawDirect( JE_word x, JE_word y, JE_word res, JE_word col, JE_word amt, JE_word xsize, JE_word ysize )
{
	xsize--;
	ysize--;
	for (int z = 1; z <= amt / res; z++)
	{
		JE_c_bar(x, y, x + xsize, y + ysize, col + 12);
		x += xsize + 2;
	}
	
	amt %= res;
	if (amt > 0)
	{
		JE_c_bar(x, y, x + xsize, y + ysize, col + ((12 / res) * amt));
	}
}

void JE_wipeKey( void )
{
	// /!\ Doesn't seems to affect anything.
}

// kate: tab-width 4; vim: set noet:
