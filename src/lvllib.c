/*
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

#define NO_EXTERNS
#include "lvllib.h"
#undef NO_EXTERNS

JE_LvlPosType lvlPos;

char levelFile[13]; /* string [12] */
JE_word lvlNum;

void JE_analyzeLevel( void )
{
    JE_word x;
    FILE *f;

    JE_resetFileExt(&f, levelFile, FALSE);
    fread(&lvlNum, 2, 1, f);
    for(x = 0; x < lvlNum; x++)
        fread(&lvlPos[x], sizeof(lvlPos[x]), 1, f);
    fseek(f, 0, SEEK_END);
    lvlPos[lvlNum] = ftell(f);
    fclose(f);
}
