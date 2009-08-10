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
#ifndef BACKGRND_H
#define BACKGRND_H

#include "opentyr.h"

#include <inttypes.h>


extern JE_word backPos, backPos2, backPos3;
extern JE_word backMove, backMove2, backMove3;
extern JE_word mapX, mapY, mapX2, mapX3, mapY2, mapY3;
extern JE_byte **mapYPos, **mapY2Pos, **mapY3Pos;
extern JE_word mapXPos, oldMapXOfs, mapXOfs, mapX2Ofs, mapX2Pos, mapX3Pos, oldMapX3Ofs, mapX3Ofs, tempMapXOfs;
extern intptr_t mapXbpPos, mapX2bpPos, mapX3bpPos;
extern JE_byte map1YDelay, map1YDelayMax, map2YDelay, map2YDelayMax;
extern SDL_Surface *smoothiesScreen;
extern JE_boolean anySmoothies;
extern JE_byte SDAT[9];

void JE_darkenBackground( JE_word neat );

void blit_background_row( SDL_Surface *surface, int x, int y, Uint8 **map );
void blit_background_row_blend( SDL_Surface *surface, int x, int y, Uint8 **map );

void draw_background_1( SDL_Surface *surface );
void draw_background_2( SDL_Surface *surface );
void draw_background_2_blend( SDL_Surface *surface );
void draw_background_3( SDL_Surface *surface );

void JE_filterScreen( JE_shortint col, JE_shortint generic_int );

void JE_checkSmoothies( void );
void JE_initSmoothies( void );
void JE_smoothies1( void );
void JE_smoothies2( void );
void JE_smoothies3( void );
void JE_smoothies4( void );
/*smoothies #5 is used for 3*/
/*smoothies #9 is a vertical flip*/

#endif /* BACKGRND_H */

// kate: tab-width 4; vim: set noet:
