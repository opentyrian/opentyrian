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

/*************/
VGA256D_EXTERNS
/*************/

JE_colortype black, colors, colors2, DummyPalette, DummySub;
JE_word NoColorsX3;

void JE_UpdateColorsFast( JE_colortype *ColorBuffer )
{
    SDL_Color p[256];
    int i;

    for(i = 0; i < 256; i++)
    {
        p[i].r = (*ColorBuffer)[i].r;
        p[i].g = (*ColorBuffer)[i].g;
        p[i].b = (*ColorBuffer)[i].b;
    }

    SDL_SetColors(VGAScreenSeg, p, 0, 256);
}

/* TODO */
