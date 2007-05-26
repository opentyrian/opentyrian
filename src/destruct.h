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


#ifndef DESTRUCT_H
#define DESTRUCT_H

#include "opentyr.h"

 #ifndef NO_EXTERNS
extern JE_word tempscreenseg;
extern JE_byte enddelay;
extern JE_boolean died;
extern JE_boolean firsttime;
#endif

void JE_introScreen( void );

void JE_makeExplosion( JE_word tempx, JE_word tempy, JE_byte shottype );

void JE_tempScreenChecking( void );

void JE_aliasDirt( void );

void JE_generateTerrain( void );

void JE_modeSelect( void );

void JE_destructGame( void );

void JE_destructMain( void );

JE_byte JE_placementPosition( JE_word x, JE_byte width );

JE_boolean JE_stabilityCheck( JE_integer x, JE_integer y );

void JE_helpScreen( void );

void JE_eSound( JE_byte sound );

void JE_pauseScreen( void );

void JE_superPixel( JE_word loc );

#endif
