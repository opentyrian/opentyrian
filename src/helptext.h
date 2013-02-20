/*
 * OpenTyrian: A modern cross-platform port of Tyrian
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
#ifndef HELPTEXT_H
#define HELPTEXT_H

#include "opentyr.h"

#define MENU_MAX 14

#define DESTRUCT_MODES 5

extern const JE_byte menuHelp[MENU_MAX][11];   /* [1..14, 1..11] */

extern JE_byte verticalHeight;
extern JE_byte helpBoxColor, helpBoxBrightness, helpBoxShadeType;

extern char helpTxt[39][231];           /* [1..39] of string [230] */
extern char pName[21][16];              /* [1..21] of string [15] */
extern char miscText[68][42];           /* [1..68] of string [41] */
extern char miscTextB[5][11];           /* [1..5] of string [10] */
extern char keyName[8][18];             /* [1..8] of string [17] */
extern char menuText[7][21];            /* [1..7] of string [20] */
extern char outputs[9][31];             /* [1..9] of string [30] */
extern char topicName[6][21];           /* [1..6] of string [20] */
extern char mainMenuHelp[34][66];       /* [1..34] of string [65] */
extern char inGameText[6][21];          /* [1..6] of string [20] */
extern char detailLevel[6][13];         /* [1..6] of string [12] */
extern char gameSpeedText[5][13];       /* [1..5] of string [12] */
extern char inputDevices[3][13];        /* [1..3] of string [12] */
extern char networkText[4][22];         /* [1..4] of string [20] */
extern char difficultyNameB[11][21];    /* [0..9] of string [20] */
extern char joyButtonNames[5][21];      /* [1..5] of string [20] */
extern char superShips[11][26];         /* [0..10] of string [25] */
extern char specialName[9][10];         /* [1..9] of string [9] */
extern char destructHelp[25][22];       /* [1..25] of string [21] */
extern char weaponNames[17][17];        /* [1..17] of string [16] */
extern char destructModeName[DESTRUCT_MODES][13]; /* [1..destructmodes] of string [12] */
extern char shipInfo[13][2][256];       /* [1..13, 1..2] of string */
extern char menuInt[MENU_MAX+1][11][18];       /* [0..14, 1..11] of string [17] */

void read_encrypted_pascal_string( char *s, int size, FILE *f );
void skip_pascal_string( FILE *f );

void JE_helpBox( SDL_Surface *screen, int x, int y, const char *message, unsigned int boxwidth );
void JE_HBox( SDL_Surface *screen, int x, int y, unsigned int  messagenum, unsigned int boxwidth );
void JE_loadHelpText( void );

#endif /* HELPTEXT_H */

