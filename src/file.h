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
#ifndef FILE_H
#define FILE_H

#include "SDL_endian.h"
#include <stdbool.h>
#include <stdio.h>

bool file_exists( const char *file );
FILE *fopen_warn( const char *file, const char *mode );
long ftell_eof( FILE *f );

// endian-swapping fread/fwrite
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
size_t efread( void *buffer, size_t size, size_t num, FILE *stream );
size_t efwrite( void *buffer, size_t size, size_t num, FILE *stream );
#else
#define efread fread
#define efwrite fwrite
#endif

#endif // FILE_H

// kate: tab-width 4; vim: set noet:
