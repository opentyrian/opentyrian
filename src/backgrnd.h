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
#ifndef BACKGRND_H
#define BACKGRND_H

#include "opentyr.h"

#ifndef NO_EXTERNS
extern JE_word backPos, backPos2, backPos3;
extern JE_word backMove, backMove2, backMove3;
extern JE_word megaDataSeg, megaDataOfs, megaData2Seg, megaData2Ofs, megaData3Seg, megaData3Ofs;
extern JE_word mapx, mapy, mapx2, mapx3, mapy2, mapy3, mapyPos, mapy2Pos, mapy3Pos, mapxPos, oldmapxOfs, mapxOfs, mapx2Ofs, mapx2Pos, mapx3Pos, oldMapx3Ofs, mapx3Ofs, tempMapxOfs, mapxbpPos, mapx2bpPos, mapx3bpPos;
extern JE_byte map1yDelay, map1yDelayMax, map2yDelay, map2yDelayMax;
extern void *smoothiesScreen;
extern JE_word smoothiesSeg;
extern JE_boolean anySmoothies;
extern JE_word TSS;
extern JE_byte SDAT[9];
#endif

void JE_darkenBackground( JE_word neat );
void JE_drawBackground2( void );
void JE_superBackground2( void );
void JE_filterScreen( JE_shortint col, JE_shortint generic_int );

void JE_checkSmoothies( void );
void JE_smoothies1( void );
void JE_smoothies2( void );
void JE_smoothies3( void );
void JE_smoothies4( void );
/*smoothies #5 is used for 3*/
void JE_smoothies6( void );
/*smoothies #9 is a vertical flip*/

#endif /* BACKGRND_H */
