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


#ifndef EDITSHIP_H
#define EDITSHIP_H

#include "opentyr.h"

typedef int JE_ShipsType; /* TODO: FIX THIS! just a placeholder! */

#ifndef NO_EXTERNS
extern JE_boolean extraavail;
extern JE_word extrashapeofs;
extern JE_ShipsType ships;
extern void * extrashapes;
extern JE_word extrashapeseg;
extern JE_word extrashapesize;
#endif

void JE_startNewShape( void );

void JE_buildRec( void );

void JE_encryptShips( void );

void JE_add( JE_byte nextbyte );

void JE_loadExtraShapes( void );

void JE_compressShapeFile( void );

void JE_decryptShips( void );

void JE_endShape( void );

#endif
