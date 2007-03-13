/*
 * Jumpers Editor++: A cross-platform and extendable editor and player for the Jumper series of games;
 * Copyright (C) 2007  Yuri K. Schlesner
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

#include "SDL.h"

#include <stdio.h>

/*************/
VGA256D_EXTERNS
/*************/

int main( int argc, char *argv[] )
{
    JE_char a = '!';

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

    JE_SetPalette(1, 0xFF, 0xFF,  0x0);
    JE_SetPalette(2, 0x00, 0x00, 0xAA);
    JE_SetPalette(3,  0x0, 0xFF,  0x0);

    SDL_LockSurface(VGAScreenSeg);

    JE_rectangle(1, 1, 318, 198, 1);
    JE_bar(150,90, 170, 110, 2);
    JE_circle(160, 100, 32, 3);
    JE_line(1,1, 318, 198, 1);

    SDL_UnlockSurface(VGAScreenSeg);

    JE_ShowVGARetrace();
    SDL_SaveBMP(VGAScreenSeg, "sshot.bmp");

    JE_getk(&a);
    printf("Key pressed: %d\n", a);

    JE_closevga256();

	return 0;
}
