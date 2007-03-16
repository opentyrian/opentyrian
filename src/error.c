/*
 * OpenTyrian Classic: A moden cross-platform port of Tyrian
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
#include <stdio.h>

#include "opentyr.h"
#include "error.h"
#include "vga256d.h"

JE_char dir[12];

JE_boolean ErrorActive = TRUE;
JE_boolean ErrorOccurred = FALSE;

static const char *tyrian_searchpaths[] = { "data", "tyrian", "tyrian2k" };

JE_longint JE_getfilesize( const JE_string filename )
{
    FILE *f;
    JE_longint size = 0;

    if (!(f = fopen(filename, "rb")))
    {
        return 0;
    }

    while (fgetc(f) != EOF)
    {
        size++;
    }
    fclose(f);

    return size;
}

void JE_errorhand( const JE_string s )
{
    if (ErrorActive)
    {
        JE_closevga256();
        printf("Warning: File not found --> %s <--\n", s);
        getchar();
        exit(1);
    } else {
        ErrorOccurred = 1;
    }
}

JE_boolean JE_find( const JE_string s )
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

void JE_findtyrian( const JE_string filename )
{
    JE_byte x;
    JE_string strbuf;

    if (JE_find(filename))
    {
        dir[0] = '\0';
    } else {
        int i;

        /* Let's find it! */
        printf("Searching for Tyrian files...\n\n");

        /* If you ever add a longer dir, increase the magic number. */
        strbuf = malloc((strlen(filename)+10) * sizeof (*strbuf));
        for (i = 0; i < COUNTOF(tyrian_searchpaths); i++)
        {
            strcpy(strbuf, tyrian_searchpaths[i]);
            strcat(strcat(strbuf, "/"), filename);
            if (JE_find(strbuf))
            {
                free(strbuf);

                strcpy(dir, tyrian_searchpaths[i]);
                strcat(dir, "/");
                printf("Tyrian data files found at %s\n\n", dir);
                return;
            }
        }
        JE_closevga256();

        printf("WARNING: Unable to find Tyrian data files.\n"
               "Stopped on file %s\n."
               "OpenTyrian needs the Tyrian data files to run. Please read the README file.\n\n", filename);
        exit(1);
    }
}

JE_string JE_locatefile( const JE_string filename ) /* !!! WARNING: Non-reentrant !!! */
{
    static JE_char buf[1024];

    if (!JE_find(filename))
    {
        if (strcmp(dir, "") == 0 && ErrorActive)
        {
            JE_findtyrian(filename);
        }

        strcpy(buf, dir);
        strcat(buf, filename);

        if (!JE_find(buf))
        {
            JE_errorhand(filename);
        }
    } else {
        strcpy(buf, filename);
    }

    return buf;
}

void JE_resetfile( FILE **f, const JE_string filename )
{
    printf("!!! WARNING: JE_resetfile is deprecated!\n");
    JE_resetfileext(f, filename, TRUE);
}

void JE_resetfileext( FILE **f, const JE_string filename, JE_boolean write ) /* Newly added. */
{
    *f = fopen(JE_locatefile(filename), (write ? "r+b" : "rb"));
}

void JE_resettext( FILE **f, const JE_string filename )
{
    printf("!!! WARNING: JE_resettext is deprecated!\n");
    JE_resettextext(f, filename, TRUE);
}

void JE_resettextext( FILE **f, const JE_string filename, JE_boolean write ) /* Newly added */
{
    *f = fopen(JE_locatefile(filename), (write ? "r+" : "r"));
}

JE_boolean JE_IsCFGThere( void ) /* Warning: It actually returns false when the config file exists */
{
    JE_boolean tempb;
    JE_byte x;

    tempb = !JE_find("TYRIAN.CFG");

    if (!tempb)
    {
        x = JE_getfilesize("TYRIAN.CFG");

        if (x != 28)
        {
            tempb = TRUE;
        }
    }

    return tempb;
}

void JE_DetectCFG( void )
{
    if (JE_IsCFGThere())
    {
        printf("\nYou must run SETUP before playing Tyrian.\n");
        exit(1);
    }
}
