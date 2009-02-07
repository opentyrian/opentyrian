/* 
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
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
#include "helptext.h"

#include "config.h"
#include "episodes.h"
#include "error.h"
#include "fonthand.h"
#include "menus.h"
#include "newshape.h"

#include <string.h>


const JE_byte menuHelp[MAX_MENU][11] = /* [1..maxmenu, 1..11] */
{
	{  1, 34,  2,  3,  4,  5,                  0, 0, 0, 0, 0 },
	{  6,  7,  8,  9, 10, 11, 11, 12,                0, 0, 0 },
	{ 13, 14, 15, 15, 16, 17, 12,                 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{  4, 30, 30,  3,  5,                   0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 16, 17, 15, 15, 12,                   0, 0, 0, 0, 0, 0 },
	{ 31, 31, 31, 31, 32, 12,                  0, 0, 0, 0, 0 },
	{  4, 34,  3,  5,                    0, 0, 0, 0, 0, 0, 0 }
};

JE_byte verticalHeight = 7;
JE_byte helpBoxColor = 12;
JE_byte helpBoxBrightness = 1;
JE_byte helpBoxShadeType = FULL_SHADE;

char helpTxt[MAX_HELP_MESSAGE][231]; /* [1..maxhelpmessage] of string [230]; */
char pName[21][16];                /* [1..21] of string [15] */
char miscText[68][42];             /* [1..68] of string [41] */
char miscTextB[5][11];             /* [1..5] of string [10] */
char keyName[8][18];               /* [1..8] of string [17] */
char menuText[7][21];              /* [1..7] of string [20] */
char outputs[9][31];               /* [1..9] of string [30] */
char topicName[6][21];             /* [1..6] of string [20] */
char mainMenuHelp[34][66];
char inGameText[6][21];            /* [1..6] of string [20] */
char detailLevel[6][13];           /* [1..6] of string [12] */
char gameSpeedText[5][13];         /* [1..5] of string [12] */
char inputDevices[3][13];          /* [1..3] of string [12] */
char networkText[4][21];           /* [1..4] of string [20] */
char difficultyNameB[11][21];      /* [0..9] of string [20] */
char joyButtonNames[5][21];        /* [1..5] of string [20] */
char superShips[11][26];           /* [0..10] of string [25] */
char specialName[9][10];           /* [1..9] of string [9] */
char destructHelp[25][22];
char weaponNames[17][17];          /* [1..17] of string [16] */
char destructModeName[DESTRUCT_MODES][13]; /* [1..destructmodes] of string [12] */
char shipInfo[13][2][256];
char menuInt[MAX_MENU + 1][11][18];   /* [0..maxmenu, 1..11] of string [17] */


JE_byte temp, temp2;

void JE_helpBox( JE_word x, JE_word y, char *message, JE_byte boxwidth )
{
	JE_byte startpos, endpos, pos;
	JE_boolean endstring;

	char substring[256];

	if (strlen(message) == 0)
	{
		return;
	}

	pos = 1;
	endpos = 0;
	endstring = false;

	do
	{
		startpos = endpos + 1;

		do
		{
			endpos = pos;
			do
			{
				pos++;
				if (pos == strlen(message))
				{
					endstring = true;
					if (pos - startpos < boxwidth)
					{
						endpos = pos + 1;
					}
				}

			} while (!(message[pos-1] == ' ' || endstring));

		} while (!(pos - startpos > boxwidth || endstring));

		JE_textShade(x, y, strnztcpy(substring, message + startpos - 1, endpos - startpos), helpBoxColor, helpBoxBrightness, helpBoxShadeType);

		y += verticalHeight;

	} while (!endstring);

	if (endpos != pos + 1)
	{
		JE_textShade(x, y, message + endpos, helpBoxColor, helpBoxBrightness, helpBoxShadeType);
	}

	helpBoxColor = 12;
	helpBoxShadeType = FULL_SHADE;
}

void JE_HBox( JE_word x, JE_word y, JE_byte messagenum, JE_byte boxwidth )
{
	JE_helpBox(x, y, helpTxt[messagenum-1], boxwidth);
}

void JE_loadHelpText( void )
{
	FILE *f; int i;

	JE_resetFile(&f, "tyrian.hdt");
	efread(&episode1DataLoc, sizeof(JE_longint), 1, f);
	JE_skipCryptLn(f);

	for (i = 0; i < MAX_HELP_MESSAGE; i++)
	{
		JE_readCryptLn(f, helpTxt[i]);       /*Online Help*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 21; i++)
	{
		JE_readCryptLn(f, pName[i]);         /*Planet names*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 68; i++)
	{
		JE_readCryptLn(f, miscText[i]);      /*Miscellaneous text*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 5; i++)
	{
		JE_readCryptLn(f, miscTextB[i]);     /*Little Miscellaneous text*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 11; i++)
	{
		JE_readCryptLn(f, menuInt[6][i]);    /*Key names*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 7; i++)
	{
		JE_readCryptLn(f, menuText[i]);      /*Main Menu*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 9; i++)
	{
		JE_readCryptLn(f, outputs[i]);       /*Event text*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 6; i++)
	{
		JE_readCryptLn(f, topicName[i]);     /*Help topics*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 34; i++)
	{
		JE_readCryptLn(f,mainMenuHelp[i]);  /*Main Menu Help*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 7; i++)
	{
		JE_readCryptLn(f, menuInt[1][i]);    /*Menu 1 - Main*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 9; i++)
	{
		JE_readCryptLn(f, menuInt[2][i]);    /*Menu 2 - Items*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 8; i++)
	{
		JE_readCryptLn(f, menuInt[3][i]);    /*Menu 3 - Options*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 6; i++)
	{
		JE_readCryptLn(f, inGameText[i]);    /*InGame Menu*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 6; i++)
	{
		JE_readCryptLn(f, detailLevel[i]);   /*Detail Level*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 5; i++)
	{
		JE_readCryptLn(f, gameSpeedText[i]); /*Game speed text*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i <= 5; i++)
	{
		JE_readCryptLn(f, episode_name[i]);
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i <= 6; i++)
	{
		JE_readCryptLn(f, difficulty_name[i]);
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i <= 4; i++)
	{
		JE_readCryptLn(f, gameplay_name[i]);
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 6; i++)
	{
		JE_readCryptLn(f, menuInt[10][i]);       /*Menu 10 - 2Player Main*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 3; i++)
	{
		JE_readCryptLn(f, inputDevices[i]);      /*Input Devices*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 4; i++)
	{
		JE_readCryptLn(f, networkText[i]);       /*Network text*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 4; i++)
	{
		JE_readCryptLn(f, menuInt[11][i]);       /*Menu 11 - 2Player Network*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i <= 10; i++)
	{
		JE_readCryptLn(f, difficultyNameB[i]);   /*HighScore Difficulty Names*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 6; i++)
	{
		JE_readCryptLn(f, menuInt[12][i]);       /*Menu 12 - Network Options*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 7; i++)
	{
		JE_readCryptLn(f, menuInt[13][i]);       /*Menu 13 - Joystick*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 5; i++)
	{
		JE_readCryptLn(f, joyButtonNames[i]);    /*Joystick Button Assignments*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i <= 10; i++)
	{
		JE_readCryptLn(f, superShips[i]);        /*SuperShips - For Super Arcade Mode*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 9; i++)
	{
		JE_readCryptLn(f, specialName[i]);       /*SuperShips - For Super Arcade Mode*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 25; i++)
	{
		JE_readCryptLn(f, destructHelp[i]);      /*Secret DESTRUCT game*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 17; i++)
	{
		JE_readCryptLn(f, weaponNames[i]);       /*Secret DESTRUCT weapons*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < DESTRUCT_MODES; i++)
	{
		JE_readCryptLn(f, destructModeName[i]);  /*Secret DESTRUCT modes*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 13; i++)
	{
		JE_readCryptLn(f, shipInfo[i][0]);       /*NEW: Ship Info*/
		JE_readCryptLn(f, shipInfo[i][1]);       /*NEW: Ship Info*/
	}
	JE_skipCryptLn(f); JE_skipCryptLn(f);

	for (i = 0; i < 5; i++)
	{
		JE_readCryptLn(f, menuInt[14][i]);       /*Menu 12 - Network Options*/
	}

	fclose(f);
}

// kate: tab-width 4; vim: set noet:
