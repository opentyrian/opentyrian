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
#include "opentyr.h"
#include "picload.h"
#include "error.h"
#include "pcxmast.h"
#include "starfade.h"
#include "nortvars.h"
#include "pallib.h"

/*************/
PCXMAST_EXTERNS
STARFADE_EXTERNS
PALLIB_EXTERNS
/*************/

JE_boolean NotYetLoadedPCX = TRUE;

void JE_LoadPIC( JE_byte PCXnumber, JE_boolean storepal )
{
    typedef JE_byte JE_buftype[63000]; /* [1..63000] */

    JE_word x;
    JE_buftype *buf;
    FILE *PCXfile;

    PCXnumber--;

    JE_resetfile(&PCXfile, "TYRIAN.PIC");

    if(NotYetLoadedPCX)
    {     /*Same as old AnalyzePic*/
        NotYetLoadedPCX = FALSE;
        fread(&x, 2, 1, PCXfile);
        for(x = 0; x < PCXnum; x++)
            fread(&pcxpos[x], sizeof(pcxpos[x]), 1, PCXfile);
        fseek(PCXfile, 0, SEEK_END);
        pcxpos[PCXnum] = ftell(PCXfile);
    }

    fseek(PCXfile, pcxpos[PCXnumber], SEEK_SET);
    buf = malloc(sizeof(*buf));
    fread(buf, 1, pcxpos[PCXnumber + 1] - pcxpos[PCXnumber], PCXfile);
    fclose(PCXfile);

    /* TODO: Unpack_PCX;*/
    free(buf);

    memcpy(colors, (*palettes)[pcxpal[PCXnumber]], sizeof(colors));
    if(storepal)
        /* TODO: JE_updatecolorsfast(&colors)*/;
}
