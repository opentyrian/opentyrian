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
#ifndef OPENTYR_H
#define OPENTYR_H

#ifndef SVN_REV
#define SVN_REV "unknown"
#endif

#include "SDL.h"
#include "SDL_endian.h"

#include <math.h> /* For the ROUND() macro */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define STUB() printf("TODO: %s:%d %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__)

/* Gets number of elements in an array.
 * !!! USE WITH ARRAYS ONLY !!! */
#define COUNTOF(x) (sizeof(x) / sizeof *(x))

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

typedef Sint32 JE_longint;
typedef Sint16 JE_integer;
typedef Sint8  JE_shortint;
typedef Uint16 JE_word;
typedef Uint8  JE_byte;
typedef bool   JE_boolean;
typedef char   JE_char;
typedef double JE_real;

extern const char *opentyrian_str, *opentyrian_version;

char *strnztcpy( char *to, const char *from, size_t count );

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
size_t efread( void *buffer, size_t size, size_t num, FILE *stream );
size_t efwrite( void *buffer, size_t size, size_t num, FILE *stream );
#else
#define efread fread
#define efwrite fwrite
#endif


void opentyrian_menu( void );

#endif /* OPENTYR_H */

// kate: tab-width 4; vim: set noet:
