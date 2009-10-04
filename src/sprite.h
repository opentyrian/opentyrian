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

#define FONT_SHAPES       0
#define SMALL_FONT_SHAPES 1
#define TINY_FONT         2
#define PLANET_SHAPES     3
#define FACE_SHAPES       4
#define OPTION_SHAPES     5 /*Also contains help shapes*/
#define WEAPON_SHAPES     6
#define EXTRA_SHAPES      7 /*Used for Ending pics*/

#define SPRITE_TABLES_MAX        8
#define SPRITES_PER_TABLE_MAX  151

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
	Sprite sprite[SPRITES_PER_TABLE_MAX];
}
Sprite_array;

Sprite_array sprite_table[SPRITE_TABLES_MAX];

static inline Sprite *sprite( unsigned int table, unsigned int index )
{
	assert(table < COUNTOF(sprite_table));
	assert(index < COUNTOF(sprite_table->sprite));
	return &sprite_table[table].sprite[index];
}

static inline bool sprite_exists( unsigned int table, unsigned int index )
{
	return (sprite(table, index)->data != NULL);
}

void load_sprites_file( unsigned table, const char *filename );
void load_sprites( unsigned int table, FILE *f );
void free_sprites( unsigned int table );

void blit_shape( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index ); // JE_newDrawCShapeNum
void blit_shape_blend( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index ); // JE_newDrawCShapeTrick
void blit_shape_hv_unsafe( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value ); // JE_newDrawCShapeBright
void blit_shape_hv( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value ); // JE_newDrawCShapeAdjust
void blit_shape_hv_blend( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value ); // JE_newDrawCShapeModify
void blit_shape_dark( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, bool black ); // JE_newDrawCShapeDarken, JE_newDrawCShapeShadow


extern JE_byte *eShapes1, *eShapes2, *eShapes3, *eShapes4, *eShapes5, *eShapes6;
extern JE_byte *shapesC1, *shapes6, *shapes9, *shapesW2;
extern JE_word eShapes1Size, eShapes2Size, eShapes3Size, eShapes4Size, eShapes5Size, eShapes6Size, shapesC1Size, shapes6Size, shapes9Size, shapesW2Size;

void JE_loadCompShapesB( JE_byte **shapes, FILE *f, JE_word shapeSize );

void JE_loadMainShapeTables( const char *shpfile );
void free_main_shape_tables( void );

void JE_loadCompShapes( JE_byte **shapes, JE_word *shapeSize, JE_char s );

void JE_drawShape2( int x, int y, int s, JE_byte *shape );
void JE_superDrawShape2( int x, int y, int s, JE_byte *shape );
void JE_drawShape2Shadow( int x, int y, int s, JE_byte *shape );

void JE_drawShape2x2( int x, int y, int s, JE_byte *shape );
void JE_superDrawShape2x2( int x, int y, int s, JE_byte *shape );
void JE_drawShape2x2Shadow( int x, int y, int s, JE_byte *shape );

#endif // SPRITE_H

// kate: tab-width 4; vim: set noet:
