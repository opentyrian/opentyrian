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
#include "vga256d.h"

#include "config.h" // For fullscreen stuff
#include "keyboard.h"
#include "newshape.h" // For tempScreenSeg
#include "palette.h"
#include "video.h"

#include "SDL.h"
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

JE_boolean mouseInstalled = true;
JE_char k;

/* JE: From Nortsong */
JE_word speed; /* JE: holds timer speed for 70Hz */

JE_byte scancode;
JE_byte outcol;

bool fullscreen_enabled = false;

palette_t vga_palette = {
	{0, 0, 0}, {0, 0, 168}, {0, 168, 0}, {0, 168, 168}, {168, 0, 0}, {168, 0, 168}, {168, 84, 0}, {168, 168, 168}, {84, 84, 84}, {84, 84, 252}, {84, 252, 84}, {84, 252, 252}, {252, 84, 84}, {252, 84, 252}, {252, 252, 84}, {252, 252, 252},
	{0, 0, 0}, {20, 20, 20}, {32, 32, 32}, {44, 44, 44}, {56, 56, 56}, {68, 68, 68}, {80, 80, 80}, {96, 96, 96}, {112, 112, 112}, {128, 128, 128}, {144, 144, 144}, {160, 160, 160}, {180, 180, 180}, {200, 200, 200}, {224, 224, 224}, {252, 252, 252},
	{0, 0, 252}, {64, 0, 252}, {124, 0, 252}, {188, 0, 252}, {252, 0, 252}, {252, 0, 188}, {252, 0, 124}, {252, 0, 64}, {252, 0, 0}, {252, 64, 0}, {252, 124, 0}, {252, 188, 0}, {252, 252, 0}, {188, 252, 0}, {124, 252, 0}, {64, 252, 0},
	{0, 252, 0}, {0, 252, 64}, {0, 252, 124}, {0, 252, 188}, {0, 252, 252}, {0, 188, 252}, {0, 124, 252}, {0, 64, 252}, {124, 124, 252}, {156, 124, 252}, {188, 124, 252}, {220, 124, 252}, {252, 124, 252}, {252, 124, 220}, {252, 124, 188}, {252, 124, 156},
	{252, 124, 124}, {252, 156, 124}, {252, 188, 124}, {252, 220, 124}, {252, 252, 124}, {220, 252, 124}, {188, 252, 124}, {156, 252, 124}, {124, 252, 124}, {124, 252, 156}, {124, 252, 188}, {124, 252, 220}, {124, 252, 252}, {124, 220, 252}, {124, 188, 252}, {124, 156, 252},
	{180, 180, 252}, {196, 180, 252}, {216, 180, 252}, {232, 180, 252}, {252, 180, 252}, {252, 180, 232}, {252, 180, 216}, {252, 180, 196}, {252, 180, 180}, {252, 196, 180}, {252, 216, 180}, {252, 232, 180}, {252, 252, 180}, {232, 252, 180}, {216, 252, 180}, {196, 252, 180},
	{180, 252, 180}, {180, 252, 196}, {180, 252, 216}, {180, 252, 232}, {180, 252, 252}, {180, 232, 252}, {180, 216, 252}, {180, 196, 252}, {0, 0, 112}, {28, 0, 112}, {56, 0, 112}, {84, 0, 112}, {112, 0, 112}, {112, 0, 84}, {112, 0, 56}, {112, 0, 28},
	{112, 0, 0}, {112, 28, 0}, {112, 56, 0}, {112, 84, 0}, {112, 112, 0}, {84, 112, 0}, {56, 112, 0}, {28, 112, 0}, {0, 112, 0}, {0, 112, 28}, {0, 112, 56}, {0, 112, 84}, {0, 112, 112}, {0, 84, 112}, {0, 56, 112}, {0, 28, 112},
	{56, 56, 112}, {68, 56, 112}, {84, 56, 112}, {96, 56, 112}, {112, 56, 112}, {112, 56, 96}, {112, 56, 84}, {112, 56, 68}, {112, 56, 56}, {112, 68, 56}, {112, 84, 56}, {112, 96, 56}, {112, 112, 56}, {96, 112, 56}, {84, 112, 56}, {68, 112, 56},
	{56, 112, 56}, {56, 112, 68}, {56, 112, 84}, {56, 112, 96}, {56, 112, 112}, {56, 96, 112}, {56, 84, 112}, {56, 68, 112}, {80, 80, 112}, {88, 80, 112}, {96, 80, 112}, {104, 80, 112}, {112, 80, 112}, {112, 80, 104}, {112, 80, 96}, {112, 80, 88},
	{112, 80, 80}, {112, 88, 80}, {112, 96, 80}, {112, 104, 80}, {112, 112, 80}, {104, 112, 80}, {96, 112, 80}, {88, 112, 80}, {80, 112, 80}, {80, 112, 88}, {80, 112, 96}, {80, 112, 104}, {80, 112, 112}, {80, 104, 112}, {80, 96, 112}, {80, 88, 112},
	{0, 0, 64}, {16, 0, 64}, {32, 0, 64}, {48, 0, 64}, {64, 0, 64}, {64, 0, 48}, {64, 0, 32}, {64, 0, 16}, {64, 0, 0}, {64, 16, 0}, {64, 32, 0}, {64, 48, 0}, {64, 64, 0}, {48, 64, 0}, {32, 64, 0}, {16, 64, 0},
	{0, 64, 0}, {0, 64, 16}, {0, 64, 32}, {0, 64, 48}, {0, 64, 64}, {0, 48, 64}, {0, 32, 64}, {0, 16, 64}, {32, 32, 64}, {40, 32, 64}, {48, 32, 64}, {56, 32, 64}, {64, 32, 64}, {64, 32, 56}, {64, 32, 48}, {64, 32, 40},
	{64, 32, 32}, {64, 40, 32}, {64, 48, 32}, {64, 56, 32}, {64, 64, 32}, {56, 64, 32}, {48, 64, 32}, {40, 64, 32}, {32, 64, 32}, {32, 64, 40}, {32, 64, 48}, {32, 64, 56}, {32, 64, 64}, {32, 56, 64}, {32, 48, 64}, {32, 40, 64},
	{44, 44, 64}, {48, 44, 64}, {52, 44, 64}, {60, 44, 64}, {64, 44, 64}, {64, 44, 60}, {64, 44, 52}, {64, 44, 48}, {64, 44, 44}, {64, 48, 44}, {64, 52, 44}, {64, 60, 44}, {64, 64, 44}, {60, 64, 44}, {52, 64, 44}, {48, 64, 44},
	{44, 64, 44}, {44, 64, 48}, {44, 64, 52}, {44, 64, 60}, {44, 64, 64}, {44, 60, 64}, {44, 52, 64}, {44, 48, 64}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
};

void JE_pix( JE_word x, JE_word y, JE_byte c )
{
	JE_pix2(x,y,c);
}

void JE_pix2( JE_word x, JE_word y, JE_byte c )
{
	/* Bad things happen if we don't clip */
	if (x <  VGAScreen->pitch && y <  VGAScreen->h)
	{
		Uint8 *vga = VGAScreen->pixels;
		vga[y * VGAScreen->pitch + x] = c;
	}
}

void JE_pixCool( JE_word x, JE_word y, JE_byte c )
{
	JE_pix3(x, y, c);
}

void JE_pix3( JE_word x, JE_word y, JE_byte c )
{
	/* Originally impemented as several direct accesses */
	JE_pix2(x, y, c);
	JE_pix2(x - 1, y, c);
	JE_pix2(x + 1, y, c);
	JE_pix2(x, y - 1, c);
	JE_pix2(x, y + 1, c);
}

void JE_pixAbs( JE_word x, JE_byte c )
{
	if (x < VGAScreen->pitch * VGAScreen->h)
	{
		Uint8 *vga = VGAScreen->pixels;
		vga[x] = c;
	}
}

void JE_getPix( JE_word x, JE_word y, JE_byte *c )
{
	/* Bad things happen if we don't clip */
	if (x <  VGAScreen->pitch && y <  VGAScreen->h)
	{
		Uint8 *vga = VGAScreen->pixels;
		*c = vga[y * VGAScreen->pitch + x];
	}
}

JE_byte JE_getPixel( JE_word x, JE_word y )
{
	/* Bad things happen if we don't clip */
	if (x <  VGAScreen->pitch && y <  VGAScreen->h)
	{
		Uint8 *vga = VGAScreen->pixels;
		return vga[y * VGAScreen->pitch + x];
	}

	return 0;
}

void JE_rectangle( JE_word a, JE_word b, JE_word c, JE_word d, JE_word e ) /* x1, y1, x2, y2, color */
{
	if (a < VGAScreen->pitch && b < VGAScreen->h &&
	    c < VGAScreen->pitch && d < VGAScreen->h)
	{
		Uint8 *vga = VGAScreen->pixels;
		int i;

		/* Top line */
		memset(&vga[b * VGAScreen->pitch + a], e, c - a + 1);

		/* Bottom line */
		memset(&vga[d * VGAScreen->pitch + a], e, c - a + 1);

		/* Left line */
		for (i = (b + 1) * VGAScreen->pitch + a; i < (d * VGAScreen->pitch + a); i += VGAScreen->pitch)
		{
			vga[i] = e;
		}

		/* Right line */
		for (i = (b + 1) * VGAScreen->pitch + c; i < (d * VGAScreen->pitch + c); i += VGAScreen->pitch)
		{
			vga[i] = e;
		}
	} else {
		printf("!!! WARNING: Rectangle clipped: %d %d %d %d %d\n", a, b, c, d, e);
	}
}

void JE_bar( JE_word a, JE_word b, JE_word c, JE_word d, JE_byte e ) /* x1, y1, x2, y2, color */
{
	if (a < VGAScreen->pitch && b < VGAScreen->h &&
	    c < VGAScreen->pitch && d < VGAScreen->h)
	{
		Uint8 *vga = VGAScreen->pixels;
		int i, width;

		width = c - a + 1;

		for (i = b * VGAScreen->pitch + a; i <= d * VGAScreen->pitch + a; i += VGAScreen->pitch)
		{
			memset(&vga[i], e, width);
		}
	} else {
		printf("!!! WARNING: Filled Rectangle clipped: %d %d %d %d %d\n", a, b, c, d, e);
	}
}

void JE_c_bar( JE_word a, JE_word b, JE_word c, JE_word d, JE_byte e )
{
	if (a < VGAScreen->pitch && b < VGAScreen->h &&
	    c < VGAScreen->pitch && d < VGAScreen->h)
	{
		Uint8 *vga = VGAScreenSeg->pixels;
		int i, width;

		width = c - a + 1;

		for (i = b * VGAScreen->pitch + a; i <= d * VGAScreen->pitch + a; i += VGAScreen->pitch)
		{
			memset(&vga[i], e, width);
		}
	} else {
		printf("!!! WARNING: Filled Rectangle clipped: %d %d %d %d %d\n", a,b,c,d,e);
	}
}

void JE_barShade( JE_word a, JE_word b, JE_word c, JE_word d ) /* x1, y1, x2, y2 */
{
	if (a < VGAScreen->pitch && b < VGAScreen->h &&
	    c < VGAScreen->pitch && d < VGAScreen->h)
	{
		Uint8 *vga = VGAScreen->pixels;
		int i, j, width;

		width = c - a + 1;

		for (i = b * VGAScreen->pitch + a; i <= d * VGAScreen->pitch + a; i += VGAScreen->pitch)
		{
			for (j = 0; j < width; j++)
			{
				vga[i + j] = ((vga[i + j] & 0x0F) >> 1) | (vga[i + j] & 0xF0);
			}
		}
	} else {
		printf("!!! WARNING: Darker Rectangle clipped: %d %d %d %d\n", a,b,c,d);
	}
}

void JE_barShade2( JE_word a, JE_word b, JE_word c, JE_word d )
{
	JE_barShade(a + 3, b + 2, c - 3, d - 2);
}

void JE_barBright( JE_word a, JE_word b, JE_word c, JE_word d ) /* x1, y1, x2, y2 */
{
	if (a < VGAScreen->pitch && b < VGAScreen->h &&
	    c < VGAScreen->pitch && d < VGAScreen->h)
	{
		Uint8 *vga = VGAScreen->pixels;
		int i, j, width;

		width = c-a+1;

		for (i = b * VGAScreen->pitch + a; i <= d * VGAScreen->pitch + a; i += VGAScreen->pitch)
		{
			for (j = 0; j < width; j++)
			{
				JE_byte al, ah;
				al = ah = vga[i + j];

				ah &= 0xF0;
				al = (al & 0x0F) + 2;

				if (al > 0x0F)
				{
					al = 0x0F;
				}

				vga[i + j] = al + ah;
			}
		}
	} else {
		printf("!!! WARNING: Brighter Rectangle clipped: %d %d %d %d\n", a,b,c,d);
	}
}

void JE_circle( JE_word x, JE_byte y, JE_word z, JE_byte c ) /* z == radius */
{
	JE_real a = 0, rx, ry, rz, b;
	Uint8 *vga;

	while (a < 6.29)
	{
		a += (160-z)/16000.0; /* Magic numbers everywhere! */

		rx = x; ry = y; rz = z;

		b = x + floor(sin(a)*z+(y+floor(cos(a)*z))*320);

		vga = VGAScreen->pixels;
		vga[(int)b] = c;
	}
}

void JE_line( JE_word a, JE_byte b, JE_longint c, JE_byte d, JE_byte e )
{
	JE_real g, h, x, y;
	JE_integer z, v;
	Uint8 *vga;

	v = round(sqrt(abs((a*a)-(c*c))+abs((b*b)-(d*d)) / 4));
	g = (c-a)/(double)v; h = (d-b)/(double)v;
	x = a; y = b;

	vga = VGAScreen->pixels;

	for (z = 0; z <= v; z++)
	{
		vga[(int)(round(x) + round(y)) * VGAScreen->pitch] = e;
		x += g; y += h;
	}
}

// kate: tab-width 4; vim: set noet:
