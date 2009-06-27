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
#include "config.h"
#include "error.h"
#include "file.h"
#include "joystick.h"
#include "opentyr.h"
#include "video.h"

#include <stdio.h>

JE_char dir[500];

JE_boolean errorActive = true;
JE_boolean errorOccurred = false;

JE_boolean dont_die = false;

#ifndef TARGET_MACOSX
static const char *tyrian_searchpaths[] = { "data", "tyrian", "tyrian21" };
#endif

JE_longint JE_getFileSize( const char *filename )
{
	FILE *f;
	JE_longint size = 0;

	errorActive = false;
	f = fopen(JE_locateFile(filename), "rb");
	errorActive = true;
	if (errorOccurred)
	{
		errorOccurred = false;
		return 0;
	}

	fseek(f, 0, SEEK_END);
	size = ftell(f);

	fclose(f);

	return size;
}

void JE_errorHand( const char *s )
{
	if (errorActive)
	{
		deinit_video();
		printf("WARNING: Unable to find Tyrian data files.\n"
		       "Stopped on file %s.\n"
		       "OpenTyrian needs the Tyrian data files to run. Please read the README file.\n\n", s);
		exit(1);
	} else {
		errorOccurred = 1;
	}
}

void JE_findTyrian( const char *filename )
{
	char *strbuf;

	if (file_exists(filename))
	{
		dir[0] = '\0';
	} else {
		/* Let's find it! */
		printf("Searching for Tyrian files...\n\n");

#ifdef TARGET_MACOSX
		snprintf(dir, sizeof(dir), "%s", tyrian_game_folder());
		printf("Tyrian data files found at %s\n\n", dir);
#else /* TARGET_MACOSX */
		for (int i = 0; i < COUNTOF(tyrian_searchpaths); i++)
		{
			strbuf = malloc(strlen(tyrian_searchpaths[i]) + strlen(filename) + 2);
			
			sprintf(strbuf, "%s/%s", tyrian_searchpaths[i], filename);
			if (file_exists(strbuf))
			{
				free(strbuf);
				
				sprintf(dir, "%s/", tyrian_searchpaths[i]);
				printf("Tyrian data files found at %s\n\n", dir);
				return;
			}
			
			free(strbuf);
		}
#endif /* TARGET_MACOSX */
	}
}

char *JE_locateFile( const char *filename ) /* !!! WARNING: Non-reentrant !!! */
{
	static JE_char buf[1024];

	if (file_exists(filename))
	{
		strcpy(buf, filename);
	} else {
		if (strcmp(dir, "") == 0 && errorActive)
		{
			JE_findTyrian(filename);
		}

		snprintf(buf, sizeof buf, "%s%s", dir, filename);
		if (!file_exists(buf))
		{
			if (dont_die)
			{
				return NULL;
			}
			errorActive = true;
			JE_errorHand(filename);
		}

	}

	return buf;
}

void JE_resetFile( FILE **f, const char *filename )
{
	char *tmp;

	tmp = JE_locateFile(filename);
	*f = tmp ? fopen_warn(tmp, "rb") : NULL;
}

void JE_resetText( FILE **f, const char *filename )
{
	char *tmp;

	tmp = JE_locateFile(filename);
	*f = tmp ? fopen_warn(tmp, "r") : NULL;
}

void JE_DetectCFG( void )
{
	STUB();
}

void JE_outputString( JE_char* s )
{
	STUB();
}

// kate: tab-width 4; vim: set noet:
