/*
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
#ifndef ERROR_H
#define ERROR_H

#include "opentyr.h"

#ifndef NO_EXTERNS
extern JE_boolean ErrorActive;
extern JE_boolean ErrorOccurred;
#endif

JE_boolean JE_find( const JE_string s );
void JE_resetfile( FILE **f, const JE_string filename );
void JE_resetfileext( FILE **f, const JE_string filename, JE_boolean write );
void JE_resettext( FILE **f, const JE_string filename );
void JE_resettextext( FILE **f, const JE_string filename, JE_boolean write );
JE_string JE_locatefile( const JE_string filename );
void JE_findtyrian( const JE_string filename );
JE_boolean JE_IsCFGThere( void );
/* void JE_OutputString( JE_string s ); Obscure DOS trick (?) */
void JE_DetectCFG( void );
JE_longint JE_getfilesize( const JE_string filename );

#endif /* ERROR_H */
