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
#include "loudness.h"
#include "mainint.h"
#include "musmast.h"
#include "nortsong.h"
#include "nortvars.h"
#include "params.h"

#include "SDL.h"

#include <stdio.h>

const JE_byte shapereorderlist[7] = {1, 2, 5, 0, 3, 4, 6};

int main( int argc, char *argv[] )
{
	SDL_Init( 0 );

	JE_detectCFG();

	printf("\nWelcome to... >> OpenTyrian v0.1 <<\n\n");

	JE_scanForEpisodes();

	JE_initvga256();
	init_keyboard();

	recordFileNum = 1;
	playDemoNum = 0;
	playDemo = FALSE;

	/* TODO: LoadConfiguration(); */

	/* TODO: Tyrian originally checked availible memory here. */

	JE_paramCheck(argc, argv);

	if (scanForJoystick)
	{
		JE_joystickInit();
		if (joystick_installed)
		{
			printf("Joystick detected. %d %d\n", jCenterX, jCenterY);
		} else {
			printf("No joystick found.\n");
		}
	} else {
		printf("Joystick override.\n");
		joystick_installed = FALSE;
	}

	if (mouse_installed)
	{
		printf("Mouse Detected.   ");
		if (mouse_threeButton)
		{
			printf("Mouse driver reports three buttons.");
		}
		printf("\n");
	} else {
		printf("No mouse found.\n");
	}

	if (tyrianXmas)
	{
		if (JE_getFileSize("TYRIANC.SHP") == 0)
		{
			tyrianXmas = FALSE;
		}
		/*if (JE_getFileSize("VOICESC.SHP") == 0) tyrianXmas = FALSE;*/
		if (tyrianXmas)
		{
			printf("*****************************\n");
			printf("Christmas has been detected.\n");
			printf("  Activate Christmas? (Y/N)\n");
			printf("*****************************\n");
			wait_input(TRUE,FALSE,FALSE);
			if (lastkey_sym != SDLK_y)
			{
				tyrianXmas = FALSE;
			}
		} else {
			printf("Christmas is missing.\n");
		}
	}

	/* Default Options */
	youAreCheating = FALSE;
	smoothScroll = TRUE;
	showMemLeft = FALSE;
	playerPasswordInput = TRUE;

	printf("Initializing SDL audio...\n");		
	JE_loadSong(1);
	
	if ( TRUE ) /* TODO: Check if sound is enabled, handle either appropriately */
	{
		/* SYN: This code block doesn't really resemble the original, because the
			underlying sound code is very different. I've left out some stuff that
		    checked hardware values and stuff here. */
		
		JE_initialize(0, 0, 0, 0, 0); /* TODO: Fix arguments */
		
		if (soundEffects)
		{
			JE_multiSampleInit(0, 0, 0, 0); /* TODO: Fix arguments */
			
			/* I don't think these messages matter, but I'll replace them with more useful stuff if I can. */
			/*if (soundEffects == 2) printf("SoundBlaster active");
			printf ("DSP Version ***\n");
			printf ("SB port ***\n");
			printf ("Interrupt ***\n");*/
			
			JE_loadSndFile();
		}
		
	} else {
		JE_initialize(0, 0, 0, 0, 0); /* TODO: Fix arguments here, too */
	}

	if (recordDemo)
	{
		printf("Game will be recorded.\n");
	}

	megaData1 = malloc(sizeof(*megaData1));
	megaData2 = malloc(sizeof(*megaData2));
	megaData3 = malloc(sizeof(*megaData3));

	newshape_init();
	JE_loadMainShapeTables();
	/* TODO JE_loadExtraShapes;*/  /*Editship*/

	JE_loadHelpText();
	/*debuginfo("Help text complete");*/

	JE_loadPals();

	SDL_LockSurface(VGAScreen);
	JE_main();
	SDL_UnlockSurface(VGAScreen);

	JE_showVGA();

	JE_closevga256();

	return 0;
}
