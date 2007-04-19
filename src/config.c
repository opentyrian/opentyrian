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
#include "episodes.h"
#include "nortsong.h"

#define NO_EXTERNS
#include "config.h"
#undef NO_EXTERNS

#include <stdio.h>
#include <string.h>

/* Configuration Load/Save handler */

const JE_byte cryptKey[10] = /* [1..10] */
{
	15, 50, 89, 240, 147, 34, 86, 9, 32, 208
};

const JE_KeySettingType defaultKeySettings =
{
	72, 80, 75, 77, 57, 28, 29, 56
};

const char defaultHighScoreNames[34][23] = /* [1..34] of string [22] */
{/*1P*/
/*TYR*/   "The Prime Chair", /*13*/
          "Transon Lohk",
          "Javi Onukala",
          "Mantori",
          "Nortaneous",
          "Dougan",
          "Reid",
          "General Zinglon",
          "Late Gyges Phildren",
          "Vykromod",
          "Beppo",
          "Borogar",
          "ShipMaster Carlos",

/*OTHER*/ "Jill", /*5*/
          "Darcy",
          "Jake Stone",
          "Malvineous Havershim",
          "Marta Louise Velasquez",

/*JAZZ*/  "Jazz Jackrabbit", /*3*/
          "Eva Earlong",
          "Devan Shell",

/*OMF*/   "Crystal Devroe", /*11*/
          "Steffan Tommas",
          "Milano Angston",
          "Christian",
          "Shirro",
          "Jean-Paul",
          "Ibrahim Hothe",
          "Angel",
          "Cossette Akira",
          "Raven",
          "Hans Kreissack",

/*DARE*/  "Tyler", /*2*/
          "Rennis the Rat Guard"
};

const char defaultTeamNames[22][25] = /* [1..22] of string [24] */
{
	"Jackrabbits",
	"Team Tyrian",
	"The Elam Brothers",
	"Dare to Dream Team",
	"Pinball Freaks",
	"Extreme Pinball Freaks",
	"Team Vykromod",
	"Epic All-Stars",
	"Hans Keissack's WARriors",
	"Team Overkill",
	"Pied Pipers",
	"Gencore Growlers",
	"Microsol Masters",
	"Beta Warriors",
	"Team Loco",
	"The Shellians",
	"Jungle Jills",
	"Murderous Malvineous",
	"The Traffic Department",
	"Clan Mikal",
	"Clan Patrok",
	"Carlos' Crawlers"
};


const JE_EditorItemAvailType initialItemAvail =
{
	1,1,1,0,0,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0, /* Front/Rear Weapons 1-38  */
	0,0,0,0,0,0,0,0,0,0,1,                                                           /* Fill                     */
	1,0,0,0,0,1,0,0,0,1,1,0,1,0,0,0,0,0,                                             /* Sidekicks          51-68 */
	0,0,0,0,0,0,0,0,0,0,0,                                                           /* Fill                     */
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                                   /* Special Weapons    81-93 */
	0,0,0,0,0                                                                        /* Fill                     */
};

/* Last 2 bytes = Word
 *
 * Max Value = 1680
 * X div  60 = Armor  (1-28)
 * X div 168 = Shield (1-12)
 * X div 280 = Engine (1-06)
 */


JE_boolean smoothies[9]; /* [1..9] */
JE_byte starShowVGASpecialCode;

/* Stars */
struct
{
	JE_byte sC;
	JE_word sLoc;
	JE_word sMov;
} starDat[MAX_STARS]; /* [1..Maxstars] */
JE_word starY;


/* CubeData */
JE_word lastCubeMax, cubeMax;
JE_word cubeList[4]; /* [1..4] */

/* High-Score Stuff */
JE_boolean gameHasRepeated;

/* Difficulty */
JE_shortint difficultyLevel, oldDifficultyLevel, initialDifficulty;

/* Player Stuff */
JE_longint score, score2;

JE_integer    power, lastPower, powerAdd;
JE_PItemsType pItems, pItemsPlayer2, pItemsBack, pItemsBack2;
JE_shortint   shield, shieldMax, shieldSet;
JE_shortint   shield2, shieldMax2;
JE_integer    armorLevel, armorLevel2;
JE_byte       shieldWait, shieldT;

JE_byte          shotRepeat[11], shotMultiPos[11]; /* [1..11] */  /* 7,8 = Superbomb */
JE_byte          portConfig[10]; /* [1..10] */
JE_boolean       portConfigDone;
JE_PortPowerType portPower, lastPortPower;

JE_boolean resetVersion;

/* Level Data */
char    lastLevelName[11], levelName[11]; /* string [10] */
JE_byte mainLevel, nextLevel, saveLevel;   /*Current Level #*/

/* Keyboard Junk */
JE_KeySettingType keySettings;

/* Configuration */
JE_shortint levelFilter, levelFilterNew, levelBrightness, levelBrightnessChg;
JE_boolean  filtrationAvail, filterActive, filterFade, filterFadeStart;

JE_boolean gameJustLoaded;

JE_boolean galagaMode;

JE_boolean extraGame;

JE_boolean twoPlayerMode, twoPlayerLinked, onePlayerAction, superTyrian, trentWin;
JE_byte    superArcadeMode;

JE_byte    superArcadePowerup;

JE_real linkGunDirec;
JE_byte playerDevice1, playerDevice2;
JE_byte inputDevice1, inputDevice2;

JE_byte secretHint;
JE_byte background3over;
JE_byte background2over;
JE_byte gammaCorrection;
JE_boolean superPause,
           explosionTransparent,
           youAreCheating,
           displayScore,
           soundHasChanged,
           background2, smoothScroll, wild, superWild, starActive,
           topEnemyOver,
           skyEnemyOverAll,
           background2notTransparent,
           tempb;
JE_byte temp;
JE_word tempw;

JE_byte versionNum;   /* SW 1.0 and SW/Reg 1.1 = 0 or 1
                       * EA 1.2 = 2 */

JE_byte    fastPlay;
JE_boolean pentiumMode;

/* Savegame files */
JE_boolean playerPasswordInput;
JE_byte    inputDevice;  /* 0=Mouse   1=Joystick   2=Gravis GamePad */
JE_byte    gameSpeed;
JE_byte    processorType;  /* 1=386 2=486 3=Pentium Hyper */

JE_SaveFilesType saveFiles; /*array[1..saveLevelnum] of savefiletype;*/
JE_byte *saveFilePointer;
JE_SaveGameTemp saveTemp;
JE_byte *saveTempPointer;

JE_word editorLevel;   /*Initial value 800*/

JE_word x;


const JE_byte StringCryptKey[10] = {99, 204, 129, 63, 255, 71, 19, 25, 62, 1};

void JE_decryptString( JE_string s, JE_byte len )
{
	int i;

	for (i = len-1; i >= 0; i--)
	{
		s[i] ^= StringCryptKey[((i+1) % 10)];
		if (i > 0)
		{
			s[i] ^= s[i-1];
		}
	}
}

void JE_readCryptLn( FILE* f, JE_string s )
{
	int size;

	size = getc(f);
	fread(s, size, 1, f);
	s[size] = '\0';
	JE_decryptString(s, size);
}

void JE_skipCryptLn( FILE* f )
{
	char size;

	size = getc(f);
	fseek(f, size, SEEK_CUR);
}


void JE_setupStars( void )
{
	int z;
	
	for (z = 0; z < MAX_STARS; z++)
	{
		starDat[z].sLoc = (rand() % 200) * 320;
		starDat[z].sMov = (rand() % 3 + 2) * 320;
		starDat[z].sC = (rand() % 16) + (9 * 16);
	}
}

void JE_saveGame( JE_byte slot, JE_string name )
{
	int i;
	
	saveFiles[slot].initialDifficulty = initialDifficulty;
	saveFiles[slot].gameHasRepeated = gameHasRepeated;
	saveFiles[slot].level = saveLevel;
	
	pItems[3 - 1] = superArcadeMode;
	if (superArcadeMode == 0 && onePlayerAction)
	{
		pItems[3 - 1] = 255;
	}
	if (superTyrian) 
	{
		pItems[3 - 1] = 254;
	}
	
	memcpy(saveFiles[slot].items, pItems, sizeof(pItems));
		
	if (superArcadeMode > 253) 
	{
		pItems[3 - 1] = 0;
	}
	if (twoPlayerMode)
	{
		memcpy(saveFiles[slot].lastItems, pItemsPlayer2, sizeof(pItemsPlayer2));
	} else {
		memcpy(saveFiles[slot].lastItems, pItemsBack2, sizeof(pItemsBack2));
	}
	
	saveFiles[slot].score = score;
	saveFiles[slot].score2 = score2;
	memcpy(saveFiles[slot].levelName, lastLevelName, sizeof(lastLevelName));
	saveFiles[slot].cubes = lastCubeMax;

	if (strcmp(lastLevelName, "Completed") == 0)
	{
		temp = episodeNum - 1;
		if (temp < 1)
		{
			temp = 4; /* JE: {Episodemax is 4 for completion purposes} */
		}
		saveFiles[slot].episode = temp;
	} else {
		saveFiles[slot].episode = episodeNum;
	}
		
	saveFiles[slot].difficulty = difficultyLevel;
	saveFiles[slot].secretHint = secretHint;
	saveFiles[slot].input1 = inputDevice1;
	saveFiles[slot].input2 = inputDevice2;

	memcpy(saveFiles[slot].name, name, sizeof(name));

	saveFiles[slot].power[0] = portPower[0];
	saveFiles[slot].power[1] = portPower[1];

	JE_saveConfiguration();
}
  
void JE_initProcessorType( void )
{
	/* SYN: Originally this proc looked at your hardware specs and chose appropriate options. We don't care, so I'll just set
	   decent defaults here. */
	
	wild = FALSE;
	superWild = FALSE;
	smoothScroll = TRUE;
	explosionTransparent = TRUE;
	filtrationAvail = TRUE;
	background2 = TRUE;
	displayScore = TRUE;	

}

void JE_setNewGameSpeed( void )
{
	pentiumMode = TRUE;
	
/*  CASE FastPlay OF
    0 : BEGIN
          speed := $4300;
          smoothscroll := TRUE;
          framecountmax := 2;
        END;
    1 : BEGIN
          speed := $3000;
          smoothscroll := TRUE;
          framecountmax := 2;
        END;
    2 : BEGIN
          speed := $2000;
          smoothscroll := FALSE;
          framecountmax := 2;
        END;
    3 : BEGIN
          speed := $5300;
          smoothscroll := TRUE;
          framecountmax := 4;
        END;
    4 : BEGIN
          speed := $4300;
          smoothscroll := TRUE;
          framecountmax := 3;
        END;
    5 : BEGIN
          speed := $4300;
          smoothscroll := TRUE;
          framecountmax := 2;
          PentiumMode := TRUE;
        END;
  END;*/
  
  frameCount = frameCountMax;
  JE_resetTimerInt();
  JE_setTimerInt();	
}

void JE_loadConfiguration( void );

void JE_saveConfiguration( void )
{
	
}

void JE_loadGame( JE_byte slot );
