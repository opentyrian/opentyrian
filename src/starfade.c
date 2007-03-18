/*
 * OpenTyrian Classic: A moden cross-platform port of Tyrian
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

JE_colortype black, colors, colors2;

void JE_UpdateColorsFast( JE_colortype *ColorBuffer )
{
    SDL_Color p[256];
    int i;

    for(i = 0; i < 256; i++)
    {
        p[i].r = (*ColorBuffer)[i].r << 2;
        p[i].g = (*ColorBuffer)[i].g << 2;
        p[i].b = (*ColorBuffer)[i].b << 2;
    }

    SDL_SetColors(VGAScreenSeg, p, 0, 256);
}

void JE_FadeColors( JE_colortype *FromColors, JE_colortype *ToColors, JE_byte StartCol, JE_byte NoColors, JE_byte NoSteps )
{
    SDL_Color p[256];
    int s, i;

    for(s = 0; s <= NoSteps; s++) {
        for(i = 0; i <= NoColors; i++) {
            p[i].r = ((*FromColors)[i].r + ((((*ToColors)[i].r - (*FromColors)[i].r) * s) / NoSteps)) << 2;
            p[i].g = ((*FromColors)[i].g + ((((*ToColors)[i].g - (*FromColors)[i].g) * s) / NoSteps)) << 2;
            p[i].b = ((*FromColors)[i].b + ((((*ToColors)[i].b - (*FromColors)[i].b) * s) / NoSteps)) << 2;
        }
        SDL_SetColors(VGAScreenSeg, p, StartCol, NoColors + 1);
        SDL_Flip(VGAScreenSeg);
        SDL_Delay(16); /* TODO */
    }
}

void JE_FadeBlack( JE_byte steps )
{
    JE_FadeColors(&colors, &black, 0, 255, steps);
}

void JE_FadeColor( JE_byte steps )
{
    JE_FadeColors(&black, &colors, 0, 255, steps);
}

void JE_FadeWhite( JE_byte steps )
{
    memset(black, 63, sizeof(black));
    JE_FadeColors(&colors, &black, 0, 255, steps);
    memcpy(colors, black, sizeof(colors));
    memset(black, 0, sizeof(black));
}
