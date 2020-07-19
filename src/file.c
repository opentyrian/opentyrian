/*
 * OpenTyrian: A modern cross-platform port of Tyrian
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

#include "opentyr.h"
#include "varz.h"

#include "SDL.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *custom_data_dir = NULL;

// finds the Tyrian data directory
const char *data_dir( void )
{
	const char *dirs[] =
	{
		custom_data_dir,
		TYRIAN_DIR,
		"data",
		".",
	};

	static const char *dir = NULL;

	if (dir != NULL)
		return dir;

	for (uint i = 0; i < COUNTOF(dirs); ++i)
	{
		if (dirs[i] == NULL)
			continue;

		FILE *f = dir_fopen(dirs[i], "tyrian1.lvl", "rb");
		if (f)
		{
			fclose(f);

			dir = dirs[i];
			break;
		}
	}

	if (dir == NULL) // data not found
		dir = "";

	return dir;
}

// prepend directory and fopen
FILE *dir_fopen( const char *dir, const char *file, const char *mode )
{
	char *path = malloc(strlen(dir) + 1 + strlen(file) + 1);
	sprintf(path, "%s/%s", dir, file);

	FILE *f = fopen(path, mode);

	free(path);

	return f;
}

// warn when dir_fopen fails
FILE *dir_fopen_warn(  const char *dir, const char *file, const char *mode )
{
	FILE *f = dir_fopen(dir, file, mode);

	if (f == NULL)
		fprintf(stderr, "warning: failed to open '%s': %s\n", file, strerror(errno));

	return f;
}

// die when dir_fopen fails
FILE *dir_fopen_die( const char *dir, const char *file, const char *mode )
{
	FILE *f = dir_fopen(dir, file, mode);

	if (f == NULL)
	{
		fprintf(stderr, "error: failed to open '%s': %s\n", file, strerror(errno));
		fprintf(stderr, "error: One or more of the required Tyrian " TYRIAN_VERSION " data files could not be found.\n"
		                "       Please read the README file.\n");
		JE_tyrianHalt(1);
	}

	return f;
}

// check if file can be opened for reading
bool dir_file_exists( const char *dir, const char *file )
{
	FILE *f = dir_fopen(dir, file, "rb");
	if (f != NULL)
		fclose(f);
	return (f != NULL);
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

void fread_die(void *buffer, size_t size, size_t count, FILE *stream)
{
	size_t result = fread(buffer, size, count, stream);
	if (result != count)
	{
		fprintf(stderr, "error: An unexpected problem occurred while reading from a file.\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
}

void fwrite_die(const void *buffer, size_t size, size_t count, FILE *stream)
{
	size_t result = fwrite(buffer, size, count, stream);
	if (result != count)
	{
		fprintf(stderr, "error: An unexpected problem occurred while writing to a file.\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
}
