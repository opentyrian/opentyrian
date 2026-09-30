/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) The OpenTyrian Development Team
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
#include "helptext.h"

#include "file.h"
#include "fonthand.h"
#include "logging.h"
#include "menus.h"
#include "opentyr.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

const JE_byte menuHelp[MENU_MAX][11] = /* [1..maxmenu, 1..11] */
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

char helpTxt[39][231];                                                   /* [1..39] of string [230] */
char pName[21][16];                                                      /* [1..21] of string [15] */
char miscText[HELPTEXT_MISCTEXT_COUNT][42];                              /* [1..68] of string [41] */
char miscTextB[HELPTEXT_MISCTEXTB_COUNT][HELPTEXT_MISCTEXTB_SIZE];       /* [1..5] of string [10] */
char keyName[8][18];                                                     /* [1..8] of string [17] */
char menuText[7][HELPTEXT_MENUTEXT_SIZE];                                /* [1..7] of string [20] */
char outputs[9][31];                                                     /* [1..9] of string [30] */
char topicName[6][21];                                                   /* [1..6] of string [20] */
char mainMenuHelp[HELPTEXT_MAINMENUHELP_COUNT][66];                      /* [1..34] of string [65] */
char inGameText[6][21];                                                  /* [1..6] of string [20] */
char detailLevel[6][13];                                                 /* [1..6] of string [12] */
char gameSpeedText[5][13];                                               /* [1..5] of string [12] */
char inputDevices[3][13];                                                /* [1..3] of string [12] */
char networkText[HELPTEXT_NETWORKTEXT_COUNT][HELPTEXT_NETWORKTEXT_SIZE]; /* [1..4] of string [20] */
char difficultyNameB[11][21];                                            /* [0..9] of string [20] */
char joyButtonNames[5][21];                                              /* [1..5] of string [20] */
char superShips[HELPTEXT_SUPERSHIPS_COUNT][26];                          /* [0..10] of string [25] */
char specialName[HELPTEXT_SPECIALNAME_COUNT][10];                        /* [1..9] of string [9] */
char destructHelp[25][22];                                               /* [1..25] of string [21] */
char weaponNames[17][17];                                                /* [1..17] of string [16] */
char destructModeName[DESTRUCT_MODES][13];                               /* [1..destructmodes] of string [12] */
char shipInfo[HELPTEXT_SHIPINFO_COUNT][2][256];                          /* [1..13, 1..2] of string */
char menuInt[MENU_MAX+1][11][18];                                        /* [0..14, 1..11] of string [17] */

static void decrypt_string(char *s, size_t len)
{
	static const unsigned char crypt_key[] = { 204, 129, 63, 255, 71, 19, 25, 62, 1, 99 };

	if (len == 0)
		return;

	for (size_t i = len - 1; ; --i)
	{
		s[i] ^= crypt_key[i % sizeof(crypt_key)];
		if (i == 0)
			break;
		s[i] ^= s[i - 1];
	}
}

void readEncryptedString(File *file, char *dst, size_t size)
{
	Uint8 buffer[255];

	Uint8 len = fileReadU8(file);
	fileReadExactly(file, buffer, len);

	if (size == 0)
		return;

	decrypt_string((char *)buffer, len);

	assert(len < size);
	len = MIN(len, size - 1);

	memcpy(dst, buffer, len);
	dst[len] = '\0';
}

void JE_helpBox(SDL_Surface *screen,  int x, int y, const char *message, JE_byte boxWidth, JE_byte verticalHeight, JE_byte color, JE_byte brightness, JE_byte shadeType)
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
					if ((unsigned)(pos - startpos) < boxWidth)
					{
						endpos = pos + 1;
					}
				}

			} while (!(message[pos-1] == ' ' || endstring));

		} while (!((unsigned)(pos - startpos) > boxWidth || endstring));

		SDL_strlcpy(substring, message + startpos - 1, MIN((size_t)(endpos - startpos + 1), sizeof(substring)));
		JE_textShade(screen, x, y, substring, color, brightness, shadeType);

		y += verticalHeight;

	} while (!endstring);

	if (endpos != pos + 1)
	{
		JE_textShade(screen, x, y, message + endpos, color, brightness, shadeType);
	}
}

void JE_HBox(SDL_Surface *screen, int x, int y, JE_byte messageNum, JE_byte boxWidth, JE_byte verticalHeight, JE_byte color, JE_byte brightness)
{
	JE_helpBox(screen, x, y, helpTxt[messageNum-1], boxWidth, verticalHeight, color, brightness, FULL_SHADE);
}

void JE_loadHelpText(void)
{
	static const unsigned int menuInt_entries[MENU_MAX + 1] =
	{
		-1, 7, 9, 8, -1, -1, 11, -1, -1, -1, 6, 4, 6, 7, 5
	};
	
	const char *filename = "tyrian.hdt";

	File file = dataFileOpen(filename, "rb");
	if (file.error)
	{
		logFatal("Failed to open file '%s': %s", filename, fileGetError(&file));
		exit(EXIT_FAILURE);
	}

	(void)fileReadU32(&file);  // Episode 1-3 item data position

	/*Online Help*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(helpTxt); ++i)
		readEncryptedString(&file, helpTxt[i], sizeof helpTxt[i]);
	readEncryptedString(&file, NULL, 0);

	/*Planet names*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(pName); ++i)
		readEncryptedString(&file, pName[i], sizeof pName[i]);
	readEncryptedString(&file, NULL, 0);

	/*Miscellaneous text*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(miscText); ++i)
		readEncryptedString(&file, miscText[i], sizeof miscText[i]);
	readEncryptedString(&file, NULL, 0);

	/*Little Miscellaneous text*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(miscTextB); ++i)
		readEncryptedString(&file, miscTextB[i], sizeof miscTextB[i]);
	readEncryptedString(&file, NULL, 0);

	/*Key names*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < menuInt_entries[6]; ++i)
		readEncryptedString(&file, menuInt[6][i], sizeof menuInt[6][i]);
	readEncryptedString(&file, NULL, 0);

	/*Main Menu*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(menuText); ++i)
		readEncryptedString(&file, menuText[i], sizeof menuText[i]);
	readEncryptedString(&file, NULL, 0);

	/*Event text*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(outputs); ++i)
		readEncryptedString(&file, outputs[i], sizeof outputs[i]);
	readEncryptedString(&file, NULL, 0);

	/*Help topics*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(topicName); ++i)
		readEncryptedString(&file, topicName[i], sizeof topicName[i]);
	readEncryptedString(&file, NULL, 0);

	/*Main Menu Help*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(mainMenuHelp); ++i)
		readEncryptedString(&file, mainMenuHelp[i], sizeof mainMenuHelp[i]);
	readEncryptedString(&file, NULL, 0);

	/*Menu 1 - Main*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < menuInt_entries[1]; ++i)
		readEncryptedString(&file, menuInt[1][i], sizeof menuInt[1][i]);
	readEncryptedString(&file, NULL, 0);

	/*Menu 2 - Items*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < menuInt_entries[2]; ++i)
		readEncryptedString(&file, menuInt[2][i], sizeof menuInt[2][i]);
	readEncryptedString(&file, NULL, 0);

	/*Menu 3 - Options*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < menuInt_entries[3]; ++i)
		readEncryptedString(&file, menuInt[3][i], sizeof menuInt[3][i]);
	readEncryptedString(&file, NULL, 0);

	/*InGame Menu*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(inGameText); ++i)
		readEncryptedString(&file, inGameText[i], sizeof inGameText[i]);
	readEncryptedString(&file, NULL, 0);

	/*Detail Level*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(detailLevel); ++i)
		readEncryptedString(&file, detailLevel[i], sizeof detailLevel[i]);
	readEncryptedString(&file, NULL, 0);

	/*Game speed text*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(gameSpeedText); ++i)
		readEncryptedString(&file, gameSpeedText[i], sizeof gameSpeedText[i]);
	readEncryptedString(&file, NULL, 0);

	// episode names
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(episode_name); ++i)
		readEncryptedString(&file, episode_name[i], sizeof episode_name[i]);
	readEncryptedString(&file, NULL, 0);

	// difficulty names
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(difficulty_name); ++i)
		readEncryptedString(&file, difficulty_name[i], sizeof difficulty_name[i]);
	readEncryptedString(&file, NULL, 0);

	// gameplay mode names
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(gameplay_name); ++i)
		readEncryptedString(&file, gameplay_name[i], sizeof gameplay_name[i]);
	readEncryptedString(&file, NULL, 0);

	/*Menu 10 - 2Player Main*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < menuInt_entries[10]; ++i)
		readEncryptedString(&file, menuInt[10][i], sizeof menuInt[10][i]);
	readEncryptedString(&file, NULL, 0);

	/*Input Devices*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(inputDevices); ++i)
		readEncryptedString(&file, inputDevices[i], sizeof inputDevices[i]);
	readEncryptedString(&file, NULL, 0);

	/*Network text*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(networkText); ++i)
		readEncryptedString(&file, networkText[i], sizeof networkText[i]);
	readEncryptedString(&file, NULL, 0);

	/*Menu 11 - 2Player Network*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < menuInt_entries[11]; ++i)
		readEncryptedString(&file, menuInt[11][i], sizeof menuInt[11][i]);
	readEncryptedString(&file, NULL, 0);

	/*HighScore Difficulty Names*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(difficultyNameB); ++i)
		readEncryptedString(&file, difficultyNameB[i], sizeof difficultyNameB[i]);
	readEncryptedString(&file, NULL, 0);

	/*Menu 12 - Network Options*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < menuInt_entries[12]; ++i)
		readEncryptedString(&file, menuInt[12][i], sizeof menuInt[12][i]);
	readEncryptedString(&file, NULL, 0);

	/*Menu 13 - Joystick*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < menuInt_entries[13]; ++i)
		readEncryptedString(&file, menuInt[13][i], sizeof menuInt[13][i]);
	readEncryptedString(&file, NULL, 0);

	/*Joystick Button Assignments*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(joyButtonNames); ++i)
		readEncryptedString(&file, joyButtonNames[i], sizeof joyButtonNames[i]);
	readEncryptedString(&file, NULL, 0);

	/*SuperShips - For Super Arcade Mode*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(superShips); ++i)
		readEncryptedString(&file, superShips[i], sizeof superShips[i]);
	readEncryptedString(&file, NULL, 0);

	/*SuperShips - For Super Arcade Mode*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(specialName); ++i)
		readEncryptedString(&file, specialName[i], sizeof specialName[i]);
	readEncryptedString(&file, NULL, 0);

	/*Secret DESTRUCT game*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(destructHelp); ++i)
		readEncryptedString(&file, destructHelp[i], sizeof destructHelp[i]);
	readEncryptedString(&file, NULL, 0);

	/*Secret DESTRUCT weapons*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(weaponNames); ++i)
		readEncryptedString(&file, weaponNames[i], sizeof weaponNames[i]);
	readEncryptedString(&file, NULL, 0);

	/*Secret DESTRUCT modes*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(destructModeName); ++i)
		readEncryptedString(&file, destructModeName[i], sizeof destructModeName[i]);
	readEncryptedString(&file, NULL, 0);

	/*NEW: Ship Info*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < COUNTOF(shipInfo); ++i)
	{
		readEncryptedString(&file, shipInfo[i][0], sizeof shipInfo[i][0]);
		readEncryptedString(&file, shipInfo[i][1], sizeof shipInfo[i][1]);
	}
	readEncryptedString(&file, NULL, 0);

	/*Menu 12 - Network Options*/
	readEncryptedString(&file, NULL, 0);
	for (size_t i = 0; i < menuInt_entries[14]; ++i)
		readEncryptedString(&file, menuInt[14][i], sizeof menuInt[14][i]);

	if (file.error)
	{
		logFatal("Failed to read from file '%s': %s", filename, fileGetError(&file));
		exit(EXIT_FAILURE);
	}

	fileClose(&file);
}
