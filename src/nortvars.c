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
#include "nortvars.h"

#include "error.h"
#include "joystick.h"
#include "keyboard.h"
#include "vga256d.h"
#include "video.h"

#include <ctype.h>

/*JE_word z;*/
JE_word y;
/* File constants for Saving ShapeFile */
const JE_byte NV_shapeactive   = 0x01;
const JE_byte NV_shapeinactive = 0x00;

JE_boolean inputDetected;
JE_word lastMouseX, lastMouseY;

/*Mouse Data*/
/*Mouse_Installed is in VGA256d*/
JE_byte mouseCursor;
JE_boolean mouse_threeButton;
JE_word mouseX, mouseY, mouseButton;

JE_word z, y;

JE_word JE_btow(JE_byte a, JE_byte b)
{
	return (JE_word) (((short) b) * 256 + a);
}

void JE_loadShapeFile( JE_ShapeType *shapes, JE_char s )
{
	FILE *f;
	JE_word x;
	JE_boolean active;

	char buffer[12];
	sprintf(buffer, "shapes%c.dat", tolower(s));

	JE_resetFile(&f, buffer);

	for (x = 0; x < 304; x++)
	{
		active = getc(f);

		if (active)
		{
			efread((*shapes)[x], sizeof(JE_byte), sizeof(*(*shapes)[x]), f);
		} else {
			memset((*shapes)[x], 0, sizeof(*(*shapes)[x]));
		}
	}

	fclose(f);

	/*fprintf(stderr, "Shapes%c completed.\n", s);*/
}

void JE_loadNewShapeFile( JE_NewShapeType *shapes, JE_char s )
{
	FILE *f;
	JE_word x, y, z;
	JE_boolean active;
	JE_ShapeTypeOne tempshape;
	JE_byte black, color;

	char buffer[12];
	sprintf(buffer, "shapes%c.dat", tolower(s));

	JE_resetFile(&f, buffer);

	for (z = 0; z < 304; z++)
	{
		active = getc(f);

		if (active)
		{
			efread(tempshape, sizeof(JE_byte), sizeof(tempshape), f);

			for (y = 0; y <= 13; y++)
			{

				black = 0;
				color = 0;
				for (x = 0; x <= 11; x++)
				{
					if (tempshape[x + y * 12] == 0)
					{
						black++;
					} else {
						color++;
					}
				}

				if (black == 12)
				{  /* Compression Value 0 - All black */
					(*shapes)[z][y * 13] = 0;
				} else {
					if (color == 12)
					{  /* Compression Value 1 - All color */
						(*shapes)[z][y * 13] = 1;
						for (x = 0; x <= 11; x++)
						{
							(*shapes)[z][x + 1 + y * 13] = tempshape[x + y * 12];
						}
					} else {
						(*shapes)[z][y * 13] = 2;
						for (x = 0; x <= 11; x++)
						{
							(*shapes)[z][x + 1 + y * 13] = tempshape[x + y * 12];
						}
					}
				}
			}
		} else {
			memset((*shapes)[z], 0, sizeof((*shapes)[z]));
		}
	}

	fclose(f);

	/*fprintf(stderr, "Shapes%c completed.\n", s);*/
}

void JE_loadCompShapes( JE_byte **shapes, JE_word *shapeSize, JE_char s )
{
	FILE *f;

	char buffer[11];
	sprintf(buffer, "newsh%c.shp", tolower(s));

	if (*shapes != NULL)
	{
		free(*shapes);
	}

	JE_resetFile(&f, buffer);

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

void JE_barDraw( JE_word x, JE_word y, JE_word res, JE_word col, JE_word amt, JE_word xsize, JE_word ysize )
{
	xsize--;
	ysize--;
	for (z = 1; z <= amt / res; z++)
	{
		JE_bar(x, y,         x + xsize, y + ysize, col + 12);
		JE_bar(x, y,         x + xsize, y,         col + 13);
		JE_bar(x, y + ysize, x + xsize, y + ysize, col + 11);
		x += xsize + 2;
	}
	
	amt %= res;
	if (amt > 0)
	{
		JE_bar(x, y, x + xsize, y + ysize, col + ((12 / res) * amt));
	}
}

void JE_barDrawShadow( JE_word x, JE_word y, JE_word res, JE_word col, JE_word amt, JE_word xsize, JE_word ysize )
{
	xsize--;
	ysize--;

	for (z = 1; z <= amt / res; z++)
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
	for (z = 1; z <= amt / res; z++)
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
