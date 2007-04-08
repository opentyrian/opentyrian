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
#include "starfade.h"
#include "nortvars.h"
#include "starfade.h"
#include "vga256d.h"
#include "nortsong.h"

JE_colortype black = {{0,0,0}}; /* Rest is filled with 0's too */
JE_colortype colors, colors2;

void JE_updateColorsFast( JE_colortype *ColorBuffer )
{
    SDL_Color p[256];
    int i;

    for (i = 0; i < 256; i++)
    {
        p[i].r = (*ColorBuffer)[i].r << 2;
        p[i].g = (*ColorBuffer)[i].g << 2;
        p[i].b = (*ColorBuffer)[i].b << 2;
    }

    SDL_SetColors(VGAScreen, p, 0, 256);
}

void JE_fadeColors( JE_colortype *FromColors, JE_colortype *ToColors, JE_byte StartCol, JE_byte NoColors, JE_byte NoSteps )
{
    SDL_Color p[256];
    int s, i;

    for (s = 0; s <= NoSteps; s++) {
        setdelay(1);
        for (i = 0; i <= NoColors; i++) {
            p[i].r = ((*FromColors)[i].r + ((((*ToColors)[i].r - (*FromColors)[i].r) * s) / NoSteps)) << 2;
            p[i].g = ((*FromColors)[i].g + ((((*ToColors)[i].g - (*FromColors)[i].g) * s) / NoSteps)) << 2;
            p[i].b = ((*FromColors)[i].b + ((((*ToColors)[i].b - (*FromColors)[i].b) * s) / NoSteps)) << 2;
        }
        SDL_SetColors(VGAScreen, p, StartCol, NoColors + 1);
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
