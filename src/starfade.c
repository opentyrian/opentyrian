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
#include "starfade.h"

#include "nortsong.h"
#include "nortvars.h"
#include "starfade.h"
#include "vga256d.h"


JE_word nocolorsx3;
JE_ColorType dummysub;
JE_ColorType dummypalette;
JE_ColorType black = {{0,0,0}}; /* Rest is filled with 0's too */
JE_ColorType colors, colors2;

void JE_updateColorsFast( JE_ColorType *colorBuffer )
{
	SDL_Color p[256];
	int i;

	for (i = 0; i < 256; i++)
	{
		p[i].r = (*colorBuffer)[i].r << 2;
		p[i].g = (*colorBuffer)[i].g << 2;
		p[i].b = (*colorBuffer)[i].b << 2;
	}

	SDL_SetColors(display_surface, p, 0, 256);
}

void JE_fadeColors( JE_ColorType *fromColors, JE_ColorType *toColors, JE_byte startCol, JE_byte noColors, JE_byte noSteps )
{
	SDL_Color p[256];
	int s, i;

	for (s = 0; s <= noSteps; s++)
	{
		setdelay(1);
		for (i = 0; i <= noColors; i++)
		{
			p[i].r = ((*fromColors)[i].r + ((((*toColors)[i].r - (*fromColors)[i].r) * s) / noSteps)) << 2;
			p[i].g = ((*fromColors)[i].g + ((((*toColors)[i].g - (*fromColors)[i].g) * s) / noSteps)) << 2;
			p[i].b = ((*fromColors)[i].b + ((((*toColors)[i].b - (*fromColors)[i].b) * s) / noSteps)) << 2;
		}
		SDL_SetColors(display_surface, p, startCol, noColors + 1);
		wait_delay();
	}
}

void JE_fadeBlack( JE_byte steps )
{
	JE_fadeColors(&colors, &black, 0, 255, steps);
}

void JE_fadeColor( JE_byte steps )
{
	JE_fadeColors(&black, &colors, 0, 255, steps);
}

void JE_fadeWhite( JE_byte steps )
{
	memset(black, 63, sizeof(black));
	JE_fadeColors(&colors, &black, 0, 255, steps);
	memcpy(colors, black, sizeof(colors));
	memset(black, 0, sizeof(black));
}
