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
#include "keyboard.h"
#include "joystick.h"
#include "episodes.h"
#include "varz.h"

#include "SDL.h"

#include <stdio.h>

const JE_byte shapereorderlist[7] = {1, 2, 5, 0, 3, 4, 6};

int main( int argc, char *argv[] )
{
    FILE *f;
    JE_integer shpnumb;
    int i;

	JE_detectCFG();
	JE_scanForEpisodes();

	printf("\nWelcome to... >> OpenTyrian v0.1 <<\n\n");

	/* TODO: InitKeyboard */

	/* TODO: Not sure what these are about?
	 * recordfilenum = 1;
	 * playdemonum = 0; 
	 * playdemo = false; */

	/* TODO: LoadConfiguration */

	/* TODO: Tyrian originally checked availible memory here. */

    enemyDat = malloc(sizeof(*enemyDat));
    weaponPort = malloc(sizeof(*weaponPort));
    weapons = malloc(sizeof(*weapons));
    ships = malloc(sizeof(*ships));
    options = malloc(sizeof(*options));
    powerSys = malloc(sizeof(*powerSys));
    shields = malloc(sizeof(*shields));
    special = malloc(sizeof(*special));

    megaData1 = malloc(sizeof(*megaData1));
    megaData2 = malloc(sizeof(*megaData2));
    megaData3 = malloc(sizeof(*megaData3));


	/* here ends line 92771 of TYRIAN2.PAS
	 * TODO: Finish it and stuff. */

    SDL_Init( 0 );

    JE_initvga256();

    init_keyboard();
    JE_joystickInit();

    newshape_init();
    JE_loadHelpText();

    JE_loadpals();

    /* [UGH] */
    JE_resetFileExt(&f, "TYRIAN.SHP", FALSE);
    if(!f)
    {
        return(1);
    }

    fread(&shpnumb, 2, 1, f);

    for(i = 0; i < shpnumb; i++)
    {
        fread(&shppos[i], 4, 1, f);
    }
    /*shppos[shpnumb+1]=filesize(f);*/

    for(i = 0; i < 7; i++)
    {  /*Load EST shapes*/
        fseek(f, shppos[i], SEEK_SET);
        JE_NewLoadShapesB(shapereorderlist[i], f);
    }
    /* [/UGH] */

    SDL_LockSurface(VGAScreen);
    OpeningAnim();
    TitleScreen(TRUE);
    SDL_UnlockSurface(VGAScreen);

    JE_ShowVGA();

    JE_closevga256();

	return 0;
}
