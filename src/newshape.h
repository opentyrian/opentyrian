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
#ifndef NEWSHAPE_H
#define NEWSHAPE_H

#include "opentyr.h"

#include "SDL.h"

#define OLD_BLACK         0
#define NEW_BLACK         253

/* Font faces */
#define PLANET_SHAPES     0
#define FONT_SHAPES       1
#define SMALL_FONT_SHAPES 2
#define FACE_SHAPES       3
#define OPTION_SHAPES     4 /*Also contains help shapes*/
#define TINY_FONT         5
#define WEAPON_SHAPES     6
#define EXTRA_SHAPES      7 /*Used for Ending pics*/

#define MAXIMUM_SHAPE     151
#define MAX_TABLE         8

typedef JE_byte *JE_ShapeArrayType[MAX_TABLE][MAXIMUM_SHAPE]; /* [1..maxtable, 1..maximumshape] */

#ifndef NO_EXTERNS
extern SDL_Surface *tempScreenSeg;
extern JE_ShapeArrayType *shapeArray;
extern JE_word shapeX[MAX_TABLE][MAXIMUM_SHAPE],
               shapeY[MAX_TABLE][MAXIMUM_SHAPE];
extern JE_word shapeSize[MAX_TABLE][MAXIMUM_SHAPE];
extern JE_boolean shapeExist[MAX_TABLE][MAXIMUM_SHAPE];
extern JE_byte maxShape[MAX_TABLE];
extern JE_byte mouseGrabShape[24*28];
extern JE_boolean loadOverride;
/*extern JE_word min, max;*/
#endif

void JE_newLoadShapes( JE_byte table, char *shapefile );
void JE_newLoadShapesB( JE_byte table, FILE *f );
void JE_newCompressBlock( JE_byte **shape, JE_word xsize, JE_word ysize, JE_word *shapesize );
void JE_newDrawShape( JE_byte *shape, JE_word xsize, JE_word ysize );
void JE_newDrawCShape( JE_byte *shape, JE_word xsize, JE_word ysize );
void JE_newDrawCShapeNum( JE_byte table, JE_byte shape, JE_word x, JE_word y );
void JE_newPurgeShapes( JE_byte table );
/*void JE_OverrideLoadShapes( JE_byte table, char *shapefile, JE_word minimum, JE_word maximum );*/

void JE_drawShapeTypeOne( JE_word x, JE_word y, JE_byte *shape );
void JE_grabShapeTypeOne( JE_word x, JE_word y, JE_byte *shape );

JE_boolean JE_waitAction( JE_byte time, JE_boolean checkjoystick );
void JE_mouseStart( void );
void JE_mouseReplace( void );

void newshape_init( void );

void JE_drawNext( JE_byte draw );
void JE_drawNShape (void *shape, JE_word xsize, JE_word ysize);

#endif /* NEWSHAPE_H */
