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

#include "file.h"

#include <errno.h>

// check if file can be opened for reading
bool file_exists( const char *file )
{
	FILE *f = fopen(file, "r");
	if (f != NULL)
		fclose(f);
	return (f != NULL);
}

// warn (but do not die) when fopen fails
FILE *fopen_warn( const char *file, const char *mode )
{
	errno = 0;
	
	FILE *f = fopen(file, mode);
	
	if (!f)
	{
		char buf[100];
		snprintf(buf, sizeof(buf), "warning: failed to open '%s'", file);
		perror(buf);
	}
	
	return f;
}

// returns end-of-file position
long ftell_eof( FILE *f )
{
	long pos = ftell(f);
	
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	
	fseek(f, pos, SEEK_SET);
	
	return size;
}

// kate: tab-width 4; vim: set noet:
