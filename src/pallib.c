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
#include "nortvars.h"
#include "error.h"
#include "starfade.h"

#define NO_EXTERNS
#include "pallib.h"
#undef NO_EXTERNS

JE_PalType palettes;
JE_word palNum;

void JE_loadPals( void )
{
	FILE *f;
	int i;

	palNum = 0;

	JE_resetFile(&f, "PALETTE.DAT");
	while (palNum < MAX_PAL && !feof(f))
	{
		for (i = 0; i < 256; i++)
		{
			palettes[palNum][i].r = getc(f);
			palettes[palNum][i].g = getc(f);
			palettes[palNum][i].b = getc(f);
		}
		palNum++;
	}
	fclose(f);
}

void JE_zPal( JE_byte palette )
{
	JE_updateColorsFast(&palettes[palette]);
}
