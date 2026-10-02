/*
* OpenTyrian: A modern cross-platform port of Tyrian
* Copyright (C) The OpenTyrian Development Team
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
#include "pal.h"

#include "logging.h"

#include <stdbool.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32

char *getBasePath(void)
{
	DWORD size = 32;

again:;
	char *path = malloc(size);
	if (path == NULL)
	{
		logFatal("Out of memory.");
		exit(EXIT_FAILURE);
	}

	DWORD len = GetModuleFileNameA(NULL, path, size);
	if (len == size)
	{
		free(path);

		size *= 2;
		goto again;
	}
	else if (len == 0)
	{
		free(path);

		return NULL;
	}

	// Trim filename.
	for (; len > 0; --len)
	{
		if (path[len - 1] == '\\')
		{
			path[len] = '\0';
			break;
		}
	}

	if (len == 0)
	{
		free(path);

		return NULL;
	}

	return path;
}

#else

char *getBasePath(void)
{
	ssize_t size = 32;

again:;
	char *path = malloc(size);
	if (path == NULL)
	{
		logFatal("Out of memory.");
		exit(EXIT_FAILURE);
	}

	ssize_t len = readlink("/proc/self/exe", path, size);
	if (len == size)
	{
		free(path);

		size *= 2;
		goto again;
	}
	else if (len < 0)
	{
		free(path);

		return NULL;
	}

	// Trim filename.
	for (; len > 0; --len)
	{
		if (path[len - 1] == '/')
		{
			path[len] = '\0';
			break;
		}
	}

	if (len == 0)
	{
		free(path);

		return NULL;
	}

	return path;
}

#endif
