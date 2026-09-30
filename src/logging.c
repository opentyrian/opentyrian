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

#include <stdarg.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <debugapi.h>
#endif

static void logMessageV(const char *priority, const char *fmt, va_list ap)
{
#ifdef _WIN32
	va_list apCopy;
	va_copy(apCopy, ap);
#endif

	fprintf(stderr, "%s: ", priority);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);

#ifdef _WIN32
	if (IsDebuggerPresent())
	{
		char buffer[4096];
		int len = snprintf(buffer, sizeof buffer, "%s: ", priority);
		vsnprintf(buffer + len, sizeof buffer - len, fmt, apCopy);
		OutputDebugStringA(buffer);
		OutputDebugStringA("\n");
	}

	va_end(apCopy);
#endif
}

PRINTF_VARARG_FUNC(2)
void logMessage(const char *priority, PRINTF_FORMAT_STRING const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	logMessageV(priority, fmt, ap);

	va_end(ap);
}

PRINTF_VARARG_FUNC(1)
void logFatal(PRINTF_FORMAT_STRING const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	logMessageV("CRITICAL", fmt, ap);

	va_end(ap);
}
