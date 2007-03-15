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
#include "pallib.h"
#include "nortvars.h"
#include "error.h"
#include "starfade.h"

JE_paltype palettes;
JE_word palnum;

void JE_loadpals ( void )
{
    FILE *f;
    int i;

    palnum = 0;

    f = fopen(JE_locatefile("PALETTE.DAT"), "rb");
    while (palnum < maxpal && !feof(f))
    {
        for (i = 0; i < 256; i++)
        {
            palettes[palnum][i].r = getc(f);
            palettes[palnum][i].g = getc(f);
            palettes[palnum][i].b = getc(f);
        }
        palnum++;
    }
    fclose(f);
}

void JE_ZPal( JE_byte palette )
{
    JE_UpdateColorsFast(&palettes[palette]);
}

void pallib_init( void )
{
    JE_loadpals();
}
