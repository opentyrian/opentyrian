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
#include "opentyr.h"
#include "error.h"

#include "config.h"
#include "joystick.h"
#include "video.h"

#include <errno.h>
#include <stdio.h>

JE_word randomcount;
JE_char dir[500];

JE_boolean errorActive = true;
JE_boolean errorOccurred = false;

JE_boolean dont_die = false;

char err_msg[1000] = "No error!?";

#ifndef TARGET_MACOSX
static const char *tyrian_searchpaths[] = { "data", "tyrian", "tyrian2k" };
#endif

long get_stream_size( FILE *f )
{
	long size = 0;
	long pos;

	pos = ftell(f);

	fseek(f, 0, SEEK_END);
	size = ftell(f);

	fseek(f, pos, SEEK_SET);

	return size;
}

FILE *fopen_check( const char *file, const char *mode )
{
	char buf[50];
	FILE *f;

	errno = 0;
	f = fopen(file, mode);
	if (!f)
	{
		switch (errno)
		{
			case EACCES:
				strcpy(buf, "access denied");
				break;
			case ENOENT:
				strcpy(buf, "no such file");
				break;
			default:
				strcpy(buf, "unknown error");
				break;
		}
		snprintf(err_msg, sizeof(err_msg), "warning: failed to open '%s' (mode '%s'): %s\n", file, mode, buf);
		fprintf(stderr, "%s", err_msg);
		
		return NULL;
	}
	
	return f;
}


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

JE_boolean JE_find( const char *s )
{
	FILE *f;

	if ((f = fopen(s, "r")))
	{
		fclose(f);
		return true;
	} else {
		return false;
	}
}

void JE_findTyrian( const char *filename )
{
	char *strbuf;

	if (JE_find(filename))
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
			if (JE_find(strbuf))
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

	if (JE_find(filename))
	{
		strcpy(buf, filename);
	} else {
		if (strcmp(dir, "") == 0 && errorActive)
		{
			JE_findTyrian(filename);
		}

		snprintf(buf, sizeof buf, "%s%s", dir, filename);
		if (!JE_find(buf))
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
	*f = tmp ? fopen_check(tmp, "rb") : NULL;
}

void JE_resetText( FILE **f, const char *filename )
{
	char *tmp;

	tmp = JE_locateFile(filename);
	*f = tmp ? fopen_check(tmp, "r") : NULL;
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
