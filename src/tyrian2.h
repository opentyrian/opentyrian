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
#ifndef TYRIAN2_H
#define TYRIAN2_H

#include "opentyr.h"

#include "helptext.h"

#define CUBE_WIDTH 35
#define LINE_WIDTH 150

typedef JE_byte JE_MenuChoiceType[MAX_MENU];

void JE_main( void );
void JE_loadMap( void );
void JE_titleScreen( JE_boolean animate );
void JE_openingAnim( void );
void JE_readTextSync( void );
void JE_displayText( void );

JE_longint JE_cashLeft( void );
void JE_loadCubes( void );
void JE_itemScreen( void );

#endif /* TYRIAN2_H */
