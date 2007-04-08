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
#ifndef CONFIG_H
#define CONFIG_H

#include "opentyr.h"

#include <stdio.h>


#define SaveFilesNum (11 * 2)

#define MaxStars 100

#define SaveFileSize  (sizeof(SaveGameTemp) - 4)
#define SaveFilesSize (sizeof(JE_SaveFilesType))

typedef JE_byte JE_KeySettingType[8]; /* [1..8] */
typedef JE_byte JE_PItemsType[12]; /* [1..12] */

typedef JE_byte JE_EditorItemAvailType[100]; /* [1..100] */

typedef struct
{
	JE_word       encode;
	JE_word       level;
	JE_PItemsType items;
	JE_longint    score;
	JE_longint    score2;
	char          levelName[10]; /* string [9]; */
	JE_char       name[14]; /* [1..14] */
	JE_byte       cubes;
	JE_byte       power[2]; /* [1..2] */
	JE_byte       episode;
	JE_PItemsType lastItems;
	JE_byte       difficulty;
	JE_byte       secretHint;
	JE_byte       input1;
	JE_byte       input2;
	JE_boolean    gameHasRepeated; /*See if you went from one episode to another*/
	JE_byte       initialDifficulty;

	/* High Scores - Each episode has both sets of 1&2 player selections - with 3 in each */
	JE_longint    highScore1,
	              highScore2;
	char          highScoreName[30]; /* string [29] */
	JE_byte       highScoreDiff;
} JE_SaveFileType;

typedef JE_SaveFileType JE_SaveFilesType[SaveFilesNum]; /* [1..savefilesnum] */
typedef JE_byte JE_SaveGameTemp[sizeof(JE_SaveFilesType) + 4 + 100]; /* [1..sizeof(savefilestype) + 4 + 100] */

typedef JE_byte JE_PortPowerType[7]; /* [1..7] */

#ifndef NO_EXTERNS
extern const JE_byte cryptKey[10];
extern const JE_KeySettingType defaultKeySettings;
extern const char defaultHighScoreNames[34][23];
extern const char defaultTeamNames[22][25];
extern const JE_EditorItemAvailType initialItemAvail;
extern JE_boolean smoothies[9];
extern JE_byte starShowVGASpecialCode;
extern struct
{
	JE_byte sC;
	JE_word sLoc;
	JE_word sMov;
} starDat[MaxStars];
extern JE_word starY;
extern JE_word lastCubeMax, cubeMax;
extern JE_word cubeList[4];
extern JE_boolean gameHasRepeated;
extern JE_shortint difficultyLevel, oldDifficultyLevel, initialDifficulty;
extern JE_longint score, score2;
extern JE_integer power, lastPower, powerAdd;
extern JE_PItemsType pItems, pItemsPlayer2, pItemsBack, pItemsBack2;
extern JE_shortint shield, shieldMax, shieldSet;
extern JE_shortint shield2, shieldMax2;
extern JE_integer armorLevel, armorLevel2;
extern JE_byte shieldWait, shieldT;
extern JE_byte shotRepeat[11], shotMultiPos[11];
extern JE_byte portConfig[10];
extern JE_boolean portConfigDone;
extern JE_PortPowerType portPower, lastPortPower;
extern JE_boolean resetVersion;
extern char lastLevelName[11], levelName[11];
extern JE_byte mainLevel, nextLevel, saveLevel;
extern JE_KeySettingType keySettings;
extern JE_shortint levelFilter, levelFilterNew, levelBrightness, levelBrightnessChg;
extern JE_boolean filtrationAvail, filterActive, filterFade, filterFadeStart;
extern JE_boolean gameJustLoaded;
extern JE_boolean galagaMode;
extern JE_boolean extraGame;
extern JE_boolean twoPlayerMode, twoPlayerLinked, onePlayerAction, superTyrian, trentWin;
extern JE_byte superArcadeMode;
extern JE_byte superArcadePowerup;
extern JE_real linkGunDirec;
extern JE_byte playerDevice1, playerDevice2;
extern JE_byte inputDevice1, inputDevice2;
extern JE_byte secretHint;
extern JE_byte background3OVer;
extern JE_byte background2OVer;
extern JE_byte gammaCorrection;
extern JE_boolean superPause, explosionTransparent, youAreCheating, displayScore, soundHasChanged, background2, smoothScroll, wild, superWild, starActive, topEnemyOver, skyEnemyOverAll, background2NotTransparent, tempb;
extern JE_byte temp;
extern JE_word tempw;
extern JE_byte versionNum;
extern JE_byte fastPlay;
extern JE_boolean pentiumMode;
extern JE_boolean playerPasswordInput;
extern JE_byte inputDevice;
extern JE_byte gameSpeed;
extern JE_byte processorType;
extern JE_SaveFilesType saveFiles;
extern JE_byte *saveFilePointer;
extern JE_SaveGameTemp saveTemp;
extern JE_byte *saveTempPointer;
extern JE_word editorLevel;
#endif

void JE_initProcessorType( void );
void JE_setNewGameSpeed( void );
void JE_loadConfiguration( void );
void JE_saveConfiguration( void );
void JE_readCryptLn( FILE* f, JE_string s );
void JE_skipCryptLn( FILE* f );

void JE_setupStars( void );

void JE_saveGame( JE_byte slot, JE_string name );
void JE_loadGame( JE_byte slot );

void JE_decryptString( JE_string s, JE_byte len );

#endif /* CONFIG_H */
