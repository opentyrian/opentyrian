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

#define NO_EXTERNS
#include "backgrnd.h"
#undef NO_EXTERNS

/*Special Background 2 and Background 3*/

/*Back Pos 3*/
JE_word backPos, backPos2, backPos3;
JE_word backMove, backMove2, backMove3;

/*Main Maps*/
JE_word megaDataSeg, megaDataOfs, megaData2Seg, megaData2Ofs, megaData3Seg, megaData3Ofs;
JE_word mapx, mapy, mapx2, mapx3, mapy2, mapy3;
JE_byte **mapyPos, **mapy2Pos, **mapy3Pos;
JE_word mapxPos, oldmapxOfs, mapxOfs, mapx2Ofs, mapx2Pos, mapx3Pos, oldMapx3Ofs, mapx3Ofs, tempMapxOfs,
        mapxbpPos, mapx2bpPos, mapx3bpPos;
JE_byte map1yDelay, map1yDelayMax, map2yDelay, map2yDelayMax;


void      *smoothiesScreen;
JE_word    smoothiesSeg;
JE_boolean anySmoothies;
JE_word    TSS;       /*Temp smoothies screen*/
JE_byte    SDAT[9]; /* [1..9] */

JE_byte temp, temp2;
