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
#include "vga256d.h"
#include "error.h"
#include "pallib.h"
#include "newshape.h"
#include "shpmast.h"
#include "tyrian2.h"
#include "helptext.h"

#include "SDL.h"

#include <stdio.h>

const JE_byte shapereorderlist[7] = {1, 2, 5, 0, 3, 4, 6};

int main( int argc, char *argv[] )
{
    JE_char a = '!';
    FILE *f;
    JE_integer shpnumb;

	/* TODO: DetectCFG */
	/* TODO: scanforepisodes */

	printf("\nWelcome to... >> OpenTyrian v0.1 <<\n\n");

	/* TODO: InitKeyboard */

	/* TODO: Not sure what these are about?
	 * recordfilenum = 1;
	 * playdemonum = 0; 
	 * playdemo = false; */

	/* TODO: LoadConfiguration */

	/* TODO: Tyrian originally checked availible memory here. */

	/* here ends line 92771 of TYRIAN2.PAS
	 * TODO: Finish it and stuff. */

    SDL_Init( 0 );

    JE_initvga256();

    newshape_init();
    JE_loadhelptext();

    JE_loadpals();

    /* [UGH] */
    JE_resetfileext(&f, "TYRIAN.SHP", FALSE);
    if(!f)
    {
        return(1);
    }

    fread(&shpnumb, 2, 1, f);

    for(x = 0; x < shpnumb; x++)
    {
        fread(&shppos[x], 4, 1, f);
    }
    /*shppos[shpnumb+1]=filesize(f);*/

    for(x = 0; x < 7; x++)
    {  /*Load EST shapes*/
        fseek(f, shppos[x], SEEK_SET);
        JE_NewLoadShapesB(shapereorderlist[x], f);
    }
    /* [/UGH] */

    SDL_LockSurface(VGAScreen);
    TitleScreen(TRUE);
    SDL_UnlockSurface(VGAScreen);

    JE_ShowVGA();

    JE_getk(&a);
    printf("Key pressed: %d\n", a);

    JE_closevga256();

	return 0;
}
