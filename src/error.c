/* vim: set noet:
 *
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Team
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
#include "vga256d.h"

#define NO_EXTERNS
#include "error.h"
#undef NO_EXTERNS

#include <stdio.h>

JE_char dir[12];

JE_boolean ErrorActive = TRUE;
JE_boolean ErrorOccurred = FALSE;

static const char *tyrian_searchpaths[] = { "data", "tyrian", "tyrian2k" };

JE_longint JE_getFileSize( const char *filename )
{
	FILE *f;
	JE_longint size = 0;

	ErrorActive = FALSE;
	f = fopen(JE_locateFile(filename), "rb");
	ErrorActive = TRUE;
	if (ErrorOccurred)
	{
		ErrorOccurred = FALSE;
		return 0;
	}

	fseek(f, 0, SEEK_END);
	size = ftell(f);

	fclose(f);

	return size;
}

void JE_errorHand( const char *s )
{
	if (ErrorActive)
	{
		JE_closeVGA256();
		printf("WARNING: Unable to find Tyrian data files.\n"
		       "Stopped on file %s.\n"
		       "OpenTyrian needs the Tyrian data files to run. Please read the README file.\n\n", s);
		exit(1);
	} else {
		ErrorOccurred = 1;
	}
}

JE_boolean JE_find( const char *s )
{
	FILE *f;

	if ((f = fopen(s, "r")))
	{
		fclose(f);
		return TRUE;
	} else {
		return FALSE;
	}
}

void JE_findTyrian( const char *filename )
{
	char *strbuf;

	if (JE_find(filename))
	{
		dir[0] = '\0';
	} else {
		unsigned int i;
		/* If you ever add a longer dir, increase the magic number. */
		size_t tmpsize = (strlen(filename)+10) * sizeof (*strbuf);

		/* Let's find it! */
		printf("Searching for Tyrian files...\n\n");

		strbuf = malloc(tmpsize);
		for (i = 0; i < COUNTOF(tyrian_searchpaths); i++)
		{
			sprintf(strbuf, "%s/%s", tyrian_searchpaths[i], filename);
			if (JE_find(strbuf))
			{
				free(strbuf);

				sprintf(dir, "%s/", tyrian_searchpaths[i]);
				printf("Tyrian data files found at %s\n\n", dir);
				return;
			}
		}
		JE_closeVGA256();

		printf("WARNING: Unable to find Tyrian data files.\n"
		       "Stopped on file %s.\n"
		       "OpenTyrian needs the Tyrian data files to run. Please read the README file.\n\n", filename);
		exit(1);
	}
}

char *JE_locateFile( const char *filename ) /* !!! WARNING: Non-reentrant !!! */
{
	static JE_char buf[1024];

	if (!JE_find(filename))
	{
		if (strcmp(dir, "") == 0 && ErrorActive)
		{
			JE_findTyrian(filename);
		}

		strcpy(buf, dir);
		strcat(buf, filename);
		if (!JE_find(buf))
		{
			JE_errorHand(filename);
		}
	} else {
		strcpy(buf, filename);
	}

	return buf;
}

void JE_resetFile( FILE **f, const char *filename )
{
	printf("!!! WARNING: JE_resetfile is deprecated!\n");
	JE_resetFileExt(f, filename, TRUE);
}

void JE_resetFileExt( FILE **f, const char *filename, JE_boolean write ) /* Newly added. */
{
	*f = fopen(JE_locateFile(filename), (write ? "r+b" : "rb"));
}

void JE_resetText( FILE **f, const char *filename )
{
	printf("!!! WARNING: JE_resettext is deprecated!\n");
	JE_resetTextExt(f, filename, TRUE);
}

void JE_resetTextExt( FILE **f, const char *filename, JE_boolean write ) /* Newly added */
{
	*f = fopen(JE_locateFile(filename), (write ? "r+" : "r"));
}

JE_boolean JE_isCFGThere( void ) /* Warning: It actually returns false when the config file exists */
{
	JE_boolean tempb;
	JE_byte x;

	tempb = !JE_find("TYRIAN.CFG");

	if (!tempb)
	{
		x = JE_getFileSize("TYRIAN.CFG");

		if (x != 28)
		{
			tempb = TRUE;
		}
	}

	return tempb;
}

void JE_detectCFG( void )
{
	if (JE_isCFGThere())
	{
		printf("\nYou must run SETUP before playing Tyrian.\n");
		exit(1);
	}
}
