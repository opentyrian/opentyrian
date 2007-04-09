/* vim: set noet:
 *
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Team
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
#include "error.h"
#include "vga256d.h"
#include "keyboard.h"
#include "joystick.h"

#define NO_EXTERNS
#include "nortvars.h"
#undef NO_EXTERNS

/* File constants for Saving ShapeFile */
const JE_byte NV_shapeactive   = 0x01;
const JE_byte NV_shapeinactive = 0x00;

JE_boolean scanForJoystick;
JE_boolean inputDetected;
JE_word lastMouseX, lastMouseY;

/*Mouse Data*/
/*Mouse_Installed is in VGA256d*/
JE_byte mouseCursor;
JE_boolean mouse_threeButton;
JE_word mouseX, mouseY, mouseButton;


void JE_loadShapeFile( JE_shapetype *Shapes, JE_char s )
{
	FILE *f;
	JE_word x;
	JE_boolean active;

	char buffer[12];
	sprintf(buffer, "SHAPES%c.DAT", s);

	JE_resetFileExt(&f, buffer, FALSE);

	for (x = 0; x < 304; x++)
	{
		active = getc(f);

		if (active)
		{
			fread((*Shapes)[x], 1, sizeof(*(*Shapes)[x]), f);
		} else {
			memset((*Shapes)[x], 0, sizeof(*(*Shapes)[x]));
		}
	}

	fclose(f);

	/*fprintf(stderr, "Shapes%c completed.\n", s);*/
}

void JE_loadNewShapeFile( JE_newshapetype *Shapes, JE_char s )
{
	FILE *f;
	JE_word x, y, z;
	JE_boolean active;
	JE_shapetypeone tempshape;
	JE_byte black, color;

	char buffer[12];
	sprintf(buffer, "SHAPES%c.DAT", s);

	JE_resetFileExt(&f, buffer, FALSE);

	for (z = 0; z < 304; z++)
	{
		active = getc(f);

		if (active)
		{
			fread(tempshape, 1, sizeof(tempshape), f);

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
					(*Shapes)[z][y * 13] = 0;
				} else {
					if (color == 12)
					{  /* Compression Value 1 - All color */
						(*Shapes)[z][y * 13] = 1;
						for (x = 0; x <= 11; x++)
						{
							(*Shapes)[z][x + 1 + y * 13] = tempshape[x + y * 12];
						}
					} else {
						(*Shapes)[z][y * 13] = 2;
						for (x = 0; x <= 11; x++)
						{
							(*Shapes)[z][x + 1 + y * 13] = tempshape[x + y * 12];
						}
					}
				}
			}
		} else {
			memset((*Shapes)[z], 0, sizeof((*Shapes)[z]));
		}
	}

	fclose(f);

	/*fprintf(stderr, "Shapes%c completed.\n", s);*/
}

void JE_loadCompShapes( JE_byte **Shapes, JE_word *ShapeSize, JE_char s )
{
	FILE *f;

	char buffer[11];
	sprintf(buffer, "NEWSH%c.SHP", s);

	if (*Shapes != NULL)
	{
		free(*Shapes);
	}

	JE_resetFileExt(&f, buffer, FALSE);

	fseek(f, 0, SEEK_END);
	*ShapeSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	*Shapes = malloc(*ShapeSize);

	fread(*Shapes, 1, *ShapeSize, f);

	fclose(f);
}

void JE_drawShape2( JE_word x, JE_word y, JE_word s_, JE_byte *Shape )
{
	JE_byte *p; /* shape pointer */
	unsigned char *s; /* screen pointer, 8-bit specific */

	int i;

	s = (unsigned char *)VGAScreen->pixels;
	s += y * VGAScreen->w + x;

	p = Shape + ((s_ - 1) * 2);
	p = Shape + *(short *)p;

	while (*p != 0x0f)
	{
		s += *p & 0x0f;
		i = (*p & 0xf0) >> 4;
		if (i)
		{
			while (i--)
			{
				p++;
				*s = *p;
				s++;
			}
		} else {
			s -= 12;
			s += VGAScreen->w;
		}
		p++;
	}
}

void JE_superDrawShape2( JE_word x, JE_word y, JE_word s_, JE_byte *Shape )
{
	JE_byte *p; /* shape pointer */
	unsigned char *s; /* screen pointer, 8-bit specific */

	int i;

	s = (unsigned char *)VGAScreen->pixels;
	s += y * VGAScreen->w + x;

	p = Shape + ((s_ - 1) * 2);
	p = Shape + *(short *)p;

	while (*p != 0x0f)
	{
		s += *p & 0x0f;
		i = (*p & 0xf0) >> 4;
		if (i)
		{
			while (i--)
			{
				p++;
				*s = (((*p & 0x0f) + (*s & 0x0f)) >> 1) | (*p & 0xf0);
				s++;
			}
		} else {
			s -= 12;
			s += VGAScreen->w;
		}
		p++;
	}
}

void JE_drawShape2Shadow( JE_word x, JE_word y, JE_word s_, JE_byte *Shape )
{
	JE_byte *p; /* shape pointer */
	unsigned char *s; /* screen pointer, 8-bit specific */

	int i;

	s = (unsigned char *)VGAScreen->pixels;
	s += y * VGAScreen->w + x;

	p = Shape + ((s_ - 1) * 2);
	p = Shape + *(short *)p;

	while (*p != 0x0f)
	{
		s += *p & 0x0f;
		i = (*p & 0xf0) >> 4;
		if (i)
		{
			while (i--)
			{
				p++;
				*s = ((*s & 0x0f) >> 1) + (*s & 0xf0);
				s++;
			}
		} else {
			s -= 12;
			s += VGAScreen->w;
		}
		p++;
	}
}

void JE_drawShape2x2( JE_word x, JE_word y, JE_word s, JE_byte *Shape )
{
	JE_drawShape2(x,    y,    s,    Shape);
	JE_drawShape2(x+12, y,    s+1,  Shape);
	JE_drawShape2(x,    y+14, s+19, Shape);
	JE_drawShape2(x+12, y+14, s+20, Shape);
}

void JE_superDrawShape2x2( JE_word x, JE_word y, JE_word s, JE_byte *Shape )
{
	JE_superDrawShape2(x,    y,    s,    Shape);
	JE_superDrawShape2(x+12, y,    s+1,  Shape);
	JE_superDrawShape2(x,    y+14, s+19, Shape);
	JE_superDrawShape2(x+12, y+14, s+20, Shape);
}

void JE_drawShape2x2Shadow( JE_word x, JE_word y, JE_word s, JE_byte *Shape )
{
	JE_drawShape2Shadow(x,    y,    s,    Shape);
	JE_drawShape2Shadow(x+12, y,    s+1,  Shape);
	JE_drawShape2Shadow(x,    y+14, s+19, Shape);
	JE_drawShape2Shadow(x+12, y+14, s+20, Shape);
}

JE_boolean JE_anyButton( void )
{
	button[0] = FALSE;
	service_SDL_events(TRUE);
	JE_joystick2();
	return newkey || mousedown || button[0];
}

/* TODO */
