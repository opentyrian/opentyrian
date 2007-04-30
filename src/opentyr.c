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
#include "fonthand.h"

#include "SDL.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const JE_byte shapereorderlist[7] = {1, 2, 5, 0, 3, 4, 6};

const char *opentyrian_str = "OpenTyrian";

char *strnztcpy( char *to, char *from, size_t count )
{
	to[count] = '\0';
	return strncpy(to, from, count);
}

void opentyrian_menu( void )
{
	int i;

	wait_noinput(TRUE,TRUE,TRUE);

	memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));

	for (i = 8; i < 100-16; i+=12)
	{
		setjasondelay(2);
		memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));

		JE_bar(16,100-i, 304,100+i, 0);

		JE_showVGA();
		wait_delay();
	}

	memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));
	JE_bar(16,16, 304,184, 0);
	JE_outTextAdjust(JE_fontCenter(opentyrian_str, FONT_SHAPES), 32, opentyrian_str, 15, -3, FONT_SHAPES, FALSE);

	JE_showVGA();
	wait_input(TRUE,TRUE,TRUE);

	for (i = 100-16; i >= 16; i-=12)
	{
		setjasondelay(2);
		memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));

		JE_bar(16,100-i, 304,100+i, 0);

		JE_showVGA();
		wait_delay();
	}

	memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));
	JE_showVGA();
}

int main( int argc, char *argv[] )
{
	srand(time(NULL));

	SDL_Init( 0 );

	/* JE_detectCFG(); YKS: Removed */

	printf("\nWelcome to... >> OpenTyrian v0.1 <<\n\n");

	JE_scanForEpisodes();

	JE_initVGA256();
	init_keyboard();

	recordFileNum = 1;
	playDemoNum = 0;
	playDemo = FALSE;

	JE_loadConfiguration();

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

	if (mouseInstalled)
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
			printf("*****************************\n"
			       "Christmas has been detected.\n"
			       "  Activate Christmas? (Y/N)\n"
			       "*****************************\n");
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
	
	if ( noSound ) /* TODO: Check if sound is enabled, handle either appropriately */
	{
		/* TODO: Do we actually need to do anything here? */
		/* JE_initialize(0, 0, 0, 0, 0); */
	} else {
		/* SYN: This code block doesn't really resemble the original, because the
			underlying sound code is very different. I've left out some stuff that
		    checked hardware values and stuff here. */
		
		JE_initialize(0, 0, 0, 0, 0); /* TODO: Fix arguments */
		
		soundEffects = TRUE; /* TODO: find a real way to give this a value */
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

	JE_closeVGA256();

	return 0;
}
