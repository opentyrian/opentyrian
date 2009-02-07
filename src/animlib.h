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
#ifndef ANIMLIB_H
#define ANIMLIB_H

#include "opentyr.h"


extern JE_word currentpageofs;
extern JE_word currentpageseg;

void JE_playRunSkipDump( JE_word bufferoffset );

void JE_closeAnim( void );

void JE_loadAnim( JE_char *filename );

void JE_playAnim( JE_char *animfile, JE_byte startingframe, JE_boolean keyhalt, JE_byte speed );

void JE_renderFrame( JE_word framenumber );

JE_word JE_findPage ( JE_word framenumber );

void JE_drawFrame( JE_word framenumber );

void JE_loadPage( JE_word pagenumber );

#endif /* ANIMLIB_H */

// kate: tab-width 4; vim: set noet:
