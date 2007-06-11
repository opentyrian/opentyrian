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
#include "network.h"
#include "nortsong.h"
#include "nortvars.h"
#include "params.h"
#include "fonthand.h"
#include "picload.h"
#include "starfade.h"
#include "jukebox.h"
#include "setup.h"
#include "scroller.h"

#include "SDL.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

const JE_byte shapereorderlist[7] = {1, 2, 5, 0, 3, 4, 6};

const char *opentyrian_str = "OpenTyrian";
const char *opentyrian_menu_items[] = 
{
	"About OpenTyrian",
	"Setup",
	/* "Play Destruct", */
	"Jukebox",
	"Return to Main Menu"
};

/* zero-terminated strncpy */
char *strnztcpy( char *to, char *from, size_t count )
{
	to[count] = '\0';
	return strncpy(to, from, count);
}

/* endian-swapping fread */
int efread( void *buffer, size_t size, size_t num, FILE *stream )
{
	int i, f = fread(buffer, size, num, stream);

	switch (size)
	{
		case 2:
			for (i = 0; i < num; i++)
			{
				((Uint16 *)buffer)[i] = SDL_SwapLE16(((Uint16 *)buffer)[i]);
			}
			break;
		case 4:
			for (i = 0; i < num; i++)
			{
				((Uint32 *)buffer)[i] = SDL_SwapLE32(((Uint32 *)buffer)[i]);
			}
			break;
		case 8:
			for (i = 0; i < num; i++)
			{
				((Uint64 *)buffer)[i] = SDL_SwapLE64(((Uint64 *)buffer)[i]);
			}
			break;
		default:
			break;
	}

	return f;
}

/* endian-swapping fwrite */
int efwrite( void *buffer, size_t size, size_t num, FILE *stream )
{
	void *swap_buffer;
	int i, f;

	switch (size)
	{
		case 2:
			swap_buffer = malloc(size * num);
			for (i = 0; i < num; i++)
			{
				((Uint16 *)swap_buffer)[i] = SDL_SwapLE16(((Uint16 *)buffer)[i]);
			}
			break;
		case 4:
			swap_buffer = malloc(size * num);
			for (i = 0; i < num; i++)
			{
				((Uint32 *)swap_buffer)[i] = SDL_SwapLE32(((Uint32 *)buffer)[i]);
			}
			break;
		case 8:
			swap_buffer = malloc(size * num);
			for (i = 0; i < num; i++)
			{
				((Uint64 *)swap_buffer)[i] = SDL_SwapLE64(((Uint64 *)buffer)[i]);
			}
			break;
		default:
			swap_buffer = buffer;
			break;
	}

	f = fwrite(swap_buffer, size, num, stream);

	if (swap_buffer != buffer)
	{
		free(swap_buffer);
	}

	return f;
}

void opentyrian_menu( void )
{
	
	int sel = 0;
	int maxSel = COUNTOF(opentyrian_menu_items) - 1;
	int i;
	JE_boolean quit;
	
	JE_fadeBlack(10);
	JE_loadPic(13, FALSE); /* 2, 5, or 13? */

	JE_outTextAdjust(JE_fontCenter(opentyrian_str, FONT_SHAPES), 5, opentyrian_str, 15, -3, FONT_SHAPES, FALSE);
	
	for (i = 0; i <= maxSel; i++)
	{
		JE_outTextAdjust(JE_fontCenter(opentyrian_menu_items[i], SMALL_FONT_SHAPES),
		                 (i != maxSel) ? (i * 16 + 32) : 118, opentyrian_menu_items[i],
		                 15, -4, SMALL_FONT_SHAPES, TRUE);
	}

	memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
	JE_showVGA();
	JE_fadeColor(20);
	wait_noinput(TRUE, FALSE, FALSE);
	quit = FALSE;
	
	if (currentJukeboxSong == 0) currentJukeboxSong = 37; /* A Field for Mag */
	JE_playSong(currentJukeboxSong);

	
	do
	{
		memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));
		JE_outTextAdjust(JE_fontCenter(opentyrian_menu_items[sel], SMALL_FONT_SHAPES),
		                 (sel != maxSel) ? (sel * 16 + 32) : 118, opentyrian_menu_items[sel],
		                 15, -2, SMALL_FONT_SHAPES, TRUE);
		
		JE_showVGA();

		tempW = 0;
		JE_textMenuWait(&tempW, FALSE);
		
		if (newkey) {
			switch (lastkey_sym)
			{
				case SDLK_UP:
					sel--;
					if (sel < 0)
					{
						sel = maxSel;
					}
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_DOWN:
					sel++;
					if (sel > maxSel)
					{
						sel = 0;
					}
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_RETURN:
					switch (sel)
					{
						case 0: /* About */
							JE_playSampleNum(SELECT);
							scroller3d(about_text);
							JE_loadPic(13, FALSE);
							JE_fadeColor(20);
							break;
						case 1: /* Setup */
							/* TODO: Implement this */
							JE_playSampleNum(WRONG);
							break;
						case 2: /* Jukebox */
							JE_playSampleNum(SELECT);
							JE_jukeboxGo();
							JE_loadPic(13, FALSE);
							JE_fadeColor(20);
							break;
						default: /* Return to main menu */
							quit = TRUE;
							JE_playSampleNum(ESC);
							break;
					}
					break;
				case SDLK_ESCAPE:
					quit = TRUE;
					JE_playSampleNum(ESC);
					return;
					break;
				default:
					break;
			}
		}
	} while (!quit);
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
