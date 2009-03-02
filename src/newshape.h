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
#ifndef NEWSHAPE_H
#define NEWSHAPE_H

#include "opentyr.h"

#include "SDL.h"


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

typedef JE_byte *JE_ShapeArrayType[MAX_TABLE][MAXIMUM_SHAPE]; /* [1..maxtable, 1..maximumshape] */

extern SDL_Surface *tempScreenSeg;
extern JE_ShapeArrayType shapeArray;
extern JE_word shapeX[MAX_TABLE][MAXIMUM_SHAPE],
               shapeY[MAX_TABLE][MAXIMUM_SHAPE];
extern JE_word shapeSize[MAX_TABLE][MAXIMUM_SHAPE];
extern JE_boolean shapeExist[MAX_TABLE][MAXIMUM_SHAPE];
extern JE_byte maxShape[MAX_TABLE];
extern JE_byte mouseGrabShape[24 * 28];

void JE_newLoadShapes( JE_byte table, char *shapefile );
void JE_newLoadShapesB( JE_byte table, FILE *f );
void JE_newCompressBlock( JE_byte **shape, JE_word xsize, JE_word ysize, JE_word *shapesize );
void JE_newPurgeShapes( JE_byte table );

void blit_shape( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index ); // JE_newDrawCShapeNum
void blit_shape_blend( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index ); // JE_newDrawCShapeTrick
void blit_shape_hv_unsafe( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value ); // JE_newDrawCShapeBright
void blit_shape_hv( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value ); // JE_newDrawCShapeAdjust
void blit_shape_hv_blend( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value ); // JE_newDrawCShapeModify
void blit_shape_dark( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, bool black ); // JE_newDrawCShapeDarken, JE_newDrawCShapeShadow

void JE_drawShapeTypeOne( JE_word x, JE_word y, JE_byte *shape );
void JE_grabShapeTypeOne( JE_word x, JE_word y, JE_byte *shape );

void JE_mouseStart( void );
void JE_mouseReplace( void );

#endif /* NEWSHAPE_H */

// kate: tab-width 4; vim: set noet:
