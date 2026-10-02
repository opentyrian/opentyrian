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
#include "logging.h"

#include "opentyr.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

SDL_PRINTF_VARARG_FUNC(1)
void logFatal(SDL_PRINTF_FORMAT_STRING const char *fmt, ...)
{
	char buffer[256];
	char *bufferPtr = buffer;
	size_t bufferSize = sizeof buffer;

again:;
	va_list ap;
	va_start(ap, fmt);
	int temp = vsnprintf(bufferPtr, bufferSize, fmt, ap);
	va_end(ap);
	size_t neededLen = (size_t)MAX(0, temp);
	size_t len = MIN(neededLen, bufferSize);

	size_t neededSize = neededLen + sizeof "\n";
	if (neededSize > bufferSize)
	{
		if (bufferPtr != buffer)
			free(bufferPtr);

		bufferSize = neededSize;
		bufferPtr = malloc(bufferSize);
		if (bufferPtr != NULL)
			goto again;

		bufferPtr = buffer;
		bufferSize = sizeof buffer;
	}

	len = MIN(len, bufferSize - sizeof "\n");
	strcpy(bufferPtr + len, "\n");

	SDL_LogOutputFunction logOutput;
	void *userdata;
	SDL_LogGetOutputFunction(&logOutput, &userdata);
	if (logOutput != NULL)
		logOutput(userdata, SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, bufferPtr);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", bufferPtr, NULL);

	if (bufferPtr != buffer)
		free(bufferPtr);
}
