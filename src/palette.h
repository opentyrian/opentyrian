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
#ifndef PALETTE_H
#define PALETTE_H

#include "opentyr.h"

#define MAX_PAL 23

typedef SDL_Color palette_t[256];
typedef palette_t JE_PalType[MAX_PAL]; /* [1..maxpal] */

extern JE_PalType palettes;
extern JE_word palNum;

extern palette_t palette;
extern Uint32 rgb_palette[256], yuv_palette[256];

extern palette_t black, colors, colors2;

void JE_loadPals( void );

void JE_updateColorsFast( palette_t ColorBuffer );

void init_step_fade_palette( int diff[256][3], palette_t colors, unsigned int first_color, unsigned int last_color );
void init_step_fade_solid( int diff[256][3], SDL_Color *color, unsigned int first_color, unsigned int last_color );
void step_fade_palette( int diff[256][3], int steps, unsigned int first_color, unsigned int last_color );

void fade_palette( palette_t colors, int steps, unsigned int first_color, unsigned int last_color );
void fade_solid( SDL_Color *color, int steps, unsigned int first_color, unsigned int last_color );

void fade_black( int steps );

void JE_fadeColors( palette_t fromColors, palette_t toColors, JE_byte startCol, JE_byte noColors, JE_byte noSteps );
void JE_fadeBlack( JE_byte steps );
void JE_fadeColor( JE_byte steps );
void JE_fadeWhite( JE_byte steps );

void JE_setPalette( JE_byte col, JE_byte red, JE_byte green, JE_byte blue );

Uint32 rgb_to_yuv( int r, int g, int b );

#endif /* PALETTE_H */

// kate: tab-width 4; vim: set noet:
