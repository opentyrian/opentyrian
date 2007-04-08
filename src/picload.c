/*
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
#include "picload.h"
#include "error.h"
#include "pcxmast.h"
#include "starfade.h"
#include "nortvars.h"
#include "pallib.h"
#include "vga256d.h"

#include <string.h>

JE_boolean notYetLoadedPCX = TRUE;

void JE_LoadPIC( JE_byte PCXnumber, JE_boolean storepal )
{
    typedef JE_byte JE_buftype[63000]; /* [1..63000] */

    JE_word x;
    JE_buftype buf;
    FILE *PCXfile;

    int i;
    JE_byte *p;
    unsigned char *s;   /* screen pointer, 8-bit specific */

    s = (unsigned char *)VGAScreen->pixels;

    PCXnumber--;

    JE_resetFileExt(&PCXfile, "TYRIAN.PIC", FALSE);

    /*Same as old AnalyzePic*/
    if (notYetLoadedPCX)
    {
        notYetLoadedPCX = FALSE;
        fread(&x, 2, 1, PCXfile);
        for (x = 0; x < PCXnum; x++)
        {
            fread(&pcxpos[x], sizeof(pcxpos[x]), 1, PCXfile);
        }
        fseek(PCXfile, 0, SEEK_END);
        pcxpos[PCXnum] = ftell(PCXfile);
    }

    fseek(PCXfile, pcxpos[PCXnumber], SEEK_SET);
    fread(buf, 1, pcxpos[PCXnumber + 1] - pcxpos[PCXnumber], PCXfile);
    fclose(PCXfile);

    p = (JE_byte *)buf;
    for (i = 0; i < 320 * 200; )
    {
        if ((*p & 0xc0) == 0xc0)
        {
            i += (*p & 0x3f);
            memset(s, *(p + 1), (*p & 0x3f));
            s += (*p & 0x3f); p += 2;
        } else {
            i++;
            *s = *p;
            s++; p++;
        }
        if (i && (i % 320 == 0))
        {
            s += VGAScreen->w - 320;
        }
    }

    memcpy(colors, palettes[pcxpal[PCXnumber]], sizeof(colors));
    if (storepal)
    {
        JE_UpdateColorsFast(&colors);
    }
}
