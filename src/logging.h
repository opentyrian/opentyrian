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
#ifndef LOGGING_H
#define LOGGING_H

#if defined(_MSC_VER) && (_MSC_VER >= 1600) /* VS 2010 and above */
#include <sal.h>

#define PRINTF_FORMAT_STRING _Printf_format_string_
#else
#define PRINTF_FORMAT_STRING
#endif

#ifdef __GNUC__
#define PRINTF_VARARG_FUNC(fmtargnumber) __attribute__((format(printf, fmtargnumber, fmtargnumber + 1)))
#else
#define PRINTF_VARARG_FUNC(fmtargnumber)
#endif

#ifdef NDEBUG
#define logDebug(...) ((void)0)
#else
#define logDebug(...) logMessage("DEBUG", __VA_ARGS__)
#endif
#define logInfo(...) logMessage("INFO", __VA_ARGS__)
#define logWarn(...) logMessage("WARN", __VA_ARGS__)
#define logError(...) logMessage("ERROR", __VA_ARGS__)

void logMessage(const char *priority, PRINTF_FORMAT_STRING const char *fmt, ...) PRINTF_VARARG_FUNC(2);
void logFatal(PRINTF_FORMAT_STRING const char *fmt, ...) PRINTF_VARARG_FUNC(1);

#endif /* LOGGING_H */
