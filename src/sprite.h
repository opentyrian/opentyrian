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
#ifndef SPRITE_H
#define SPRITE_H

#include "opentyr.h"

#include "SDL.h"
#include <assert.h>

#define OLD_BLACK         0
#define NEW_BLACK         253

/* Font faces */
#define FONT_SHAPES       0
#define SMALL_FONT_SHAPES 1
#define TINY_FONT         2
#define PLANET_SHAPES     3
#define FACE_SHAPES       4
#define OPTION_SHAPES     5 /*Also contains help shapes*/
#define WEAPON_SHAPES     6
#define EXTRA_SHAPES      7 /*Used for Ending pics*/

#define MAXIMUM_SHAPE     151
#define MAX_TABLE         8

typedef struct
{
	Uint16 width, height;
	Uint16 size;
	Uint8 *data;
}
Sprite;

typedef struct
{
	unsigned int count;
	Sprite sprite[MAXIMUM_SHAPE];
}
Sprite_array;

Sprite_array sprite_table[MAX_TABLE];

static inline Sprite *sprite( unsigned int table, unsigned int index )
{
	assert(table < MAX_TABLE);
	assert(index < MAXIMUM_SHAPE);
	return &sprite_table[table].sprite[index];
}

static inline bool sprite_exists( unsigned int table, unsigned int index )
{
	return (sprite(table, index)->data != NULL);
}

void JE_newLoadShapes( unsigned table, const char *shapefile );
void JE_newLoadShapesB( unsigned int table, FILE *f );
void free_sprites( unsigned int table );

void blit_shape( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index ); // JE_newDrawCShapeNum
void blit_shape_blend( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index ); // JE_newDrawCShapeTrick
void blit_shape_hv_unsafe( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value ); // JE_newDrawCShapeBright
void blit_shape_hv( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value ); // JE_newDrawCShapeAdjust
void blit_shape_hv_blend( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value ); // JE_newDrawCShapeModify
void blit_shape_dark( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, bool black ); // JE_newDrawCShapeDarken, JE_newDrawCShapeShadow

#endif // SPRITE_H

// kate: tab-width 4; vim: set noet:
