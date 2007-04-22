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
#include "vga256d.h"
#include "error.h"
#include "loudness.h"
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


JE_boolean smoothies[9] = /* [1..9] */
{ 0, 0, 0, 0, 0, 0, 0, 0, 0 };


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

JE_boolean twoPlayerMode, twoPlayerLinked, onePlayerAction, superTyrian;
JE_boolean trentWin = FALSE;
JE_byte    superArcadeMode;

JE_byte    superArcadePowerup;

JE_real linkGunDirec;
JE_byte playerDevice1, playerDevice2;
JE_byte inputDevice1, inputDevice2;

JE_byte secretHint;
JE_byte background3over;
JE_byte background2over;
JE_byte gammaCorrection;
JE_boolean superPause = FALSE;
JE_boolean explosionTransparent,
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
JE_SaveFilesType *saveFilePointer = &saveFiles;
JE_SaveGameTemp saveTemp;
JE_SaveGameTemp *saveTempPointer = &saveTemp;

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

void JE_loadGame( JE_byte slot )
{ 
	JE_byte temp5;

	superTyrian = FALSE;
	onePlayerAction = FALSE;
	twoPlayerMode = FALSE;
	extraGame = FALSE;
	galagaMode = FALSE;
  
	initialDifficulty = saveFiles[slot].initialDifficulty;
	gameHasRepeated = saveFiles[slot].gameHasRepeated;
	twoPlayerMode = slot > 11; /* TODO: Verify this value, may need reindexing! */
	difficultyLevel = saveFiles[slot].difficulty;
	memcpy(pItems, saveFiles[slot].items, sizeof(pItems));
	superArcadeMode = pItems[3 - 1];
	
	if (superArcadeMode == 255)
	{
		onePlayerAction = TRUE;
		superArcadeMode = 0;
		pItems [3 - 1] = 0;
	} 
	else if (superArcadeMode == 254)
	{
		onePlayerAction = TRUE;
		superArcadeMode = 0;
		pItems [3 - 1] = 0;
		superTyrian = TRUE;
	} 
	else if (superArcadeMode > 0)
	{
		onePlayerAction = TRUE;
	}
  
	if (twoPlayerMode)
	{
		memcpy(pItemsPlayer2, saveFiles[slot].lastItems, sizeof(pItemsPlayer2));
		onePlayerAction = FALSE;
	} else {
		memcpy(pItemsBack2, saveFiles[slot].lastItems, sizeof(pItemsBack2));
	}

	/* {Compatibility with old version} */
	/* SYN: TODO: See if any of these need reindexing */
	if (pItemsPlayer2[7] < 101)
	{
		pItemsPlayer2[7] = 101;
		pItemsPlayer2[8] = pItemsPlayer2[4];
	}
	
	score = saveFiles[slot].score;
	score2 = saveFiles[slot].score2;
	mainLevel = saveFiles[slot].level;
	cubeMax = saveFiles[slot].cubes;
	lastCubeMax = cubeMax;

	secretHint = saveFiles[slot].secretHint;
	inputDevice1 = saveFiles[slot].input1;
	inputDevice2 = saveFiles[slot].input2;

	portPower[temp] = saveFiles[slot].power[0];
	portPower[temp] = saveFiles[slot].power[1];
  
	temp5 = saveFiles[slot].episode;
	
	memcpy(levelName, saveFiles[slot].levelName, sizeof(levelName));
  
	if (strcmp(lastLevelName, "Completed") == 0)
	{
	  if (temp5 == 4)
	  {
		temp5 = 1;
	  }
	  else if (temp5 < 4)
	  {
		temp5++;
	  }
	  /* {Increment 1-3 to 2-4.  Episode 4 goes to 1.  Episode 5 stands still.} */
	}
  
	JE_initEpisode(temp5);
	saveLevel = mainLevel;
	memcpy(lastLevelName, levelName, sizeof(levelName));
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

  
	switch (gameSpeed)
	{
	case 1:  /* Slug Mode */
		fastPlay = 3;
		break;
	case 2:  /* Slower */
		fastPlay = 4;
		break;
	case 3: /* Slow */
		fastPlay = 5;
		break;
	case 4: /* Normal */
		fastPlay = 0;
		break;
	case 5: /* Pentium Hyper */
		fastPlay = 1;
		break;
	}
	
}

void JE_setNewGameSpeed( void )
{
	pentiumMode = TRUE;
	
	switch (fastPlay)
	{
	case 0:
		speed = 0x4300;
		smoothScroll = TRUE;
		frameCountMax = 2;
		break;
	case 1:
		speed = 0x3000;
		smoothScroll = TRUE;
		frameCountMax = 2;
		break;
	case 2:
		speed = 0x2000;
		smoothScroll = FALSE;
		frameCountMax = 2;
		break;
	case 3:
		speed = 0x5300;
		smoothScroll = TRUE;
		frameCountMax = 4;
		break;
	case 4:
		speed = 0x4300;
		smoothScroll = TRUE;
		frameCountMax = 3;
		break;
	case 5:
		speed = 0x4300;
		smoothScroll = TRUE;
		frameCountMax = 2;
		pentiumMode = TRUE;
		break;
	}
  
  frameCount = frameCountMax;
  JE_resetTimerInt();
  JE_setTimerInt();	
}

void JE_encryptSaveTemp( void )
{
	JE_SaveGameTemp s2, s3;
	char c;
	JE_word x;
	JE_byte y, z;
	
	memcpy(s3, saveTemp, sizeof(s3));
	
	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y += s3[x];
	}
	saveTemp[SAVE_FILE_SIZE + 1 - 1] = y;

	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y -= s3[x];
	}
	saveTemp[SAVE_FILE_SIZE + 2 - 1] = y;

	y = 1;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y = (y * s3[x]) + 1;
	}
	saveTemp[SAVE_FILE_SIZE + 3 - 1] = y;
	
	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y = y ^ s3[x];
	}
	saveTemp[SAVE_FILE_SIZE + 4 - 1] = y;
	
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		saveTemp[x] = saveTemp[x] ^ cryptKey[x % 10];
		if (x > 1)
		{
			saveTemp[x] = saveTemp[x] ^ saveTemp[x - 1];
		}
	}
}

void JE_decryptSaveTemp( void )
{
	JE_boolean correct = TRUE;
	JE_SaveGameTemp s2;
	/*JE_word x;*/
	int x;
	JE_byte y, z;
	
	/* Decrypt save game file */
	for (x = (SAVE_FILE_SIZE - 1); x >= 0; x--)
	{
		s2[x] = saveTemp[x] ^ (cryptKey[x % 10]);
		if (x > 0)
		{
			s2[x] = s2[x] ^ saveTemp[x - 1];
		}
	}
	
	/* Check save file for correctitude */
	y = 0;
	for (x = 0; x < (int) SAVE_FILE_SIZE; x++)
	{
		y += s2[x];
	}
	if (saveTemp[SAVE_FILE_SIZE] != y)
	{
		correct = FALSE;
	}

	y = 0;
	for (x = 0; x < (int) SAVE_FILE_SIZE; x++)
	{
		y -= s2[x];
	}
	if (saveTemp[SAVE_FILE_SIZE + 2 - 1] != y)
	{
		correct = FALSE;
	}

	y = 1;
	for (x = 0; x < (int) SAVE_FILE_SIZE; x++)
	{
		y = (y * s2[x]) + 1;
	}
	if (saveTemp[SAVE_FILE_SIZE + 3 - 1] != y)
	{
		correct = FALSE;
	}	

	y = 0;
	for (x = 0; x < (int) SAVE_FILE_SIZE; x++)
	{
		y = y ^ s2[x];
	}
	if (saveTemp[SAVE_FILE_SIZE + 4 - 1] != y)
	{
		correct = FALSE;
	}		
	
	/* Barf and die if save file doesn't validate */
	if (!correct)
	{
		printf("Error reading save file!\n");
		exit(255);
	}
	
	/* Keep decrypted version */
	memcpy(saveTemp, s2, sizeof(s2));
}

void JE_loadConfiguration( void )
{
	FILE *fi;
	
	ErrorActive = TRUE;
	
	if (JE_isCFGThere())
	{
		/* ASSIGN (f, 'TYRIAN.CFG');
		RESET (f, 1);
		BLOCKREAD (f, Background2, 1);
		BLOCKREAD (f, GameSpeed  , 1);

		BLOCKREAD (f, InputDevice, 1);
		InputDevice := 0;

		BLOCKREAD (f, Jconfigure , 1);
		IF Jconfigure = 0 THEN
		Jconfigure := 1;
		BLOCKREAD (f, VersionNum , 1);
		IF ResetVersion THEN
		VersionNum := 2;    {Shareware 1.0 and Registered 1.1 = 1}
		BLOCKREAD (f, Processortype, 1);
		BLOCKREAD (f, midiport   , 1);
		BLOCKREAD (f, soundeffects, 1);
		BLOCKREAD (f, Gammacorrection, 1);
		BLOCKREAD (f, difficultylevel, 1);
		BLOCKREAD (f, joybuttonassign, 4);

		BLOCKREAD (f, Tyr_musicvolume, 2);
		BLOCKREAD (f, FXvolume   , 2);

		BLOCKWRITE (f, InputDevice1, 1);
		BLOCKWRITE (f, InputDevice2, 1);

		BLOCKREAD (f, keysettings, SIZEOF (keysettings) );
		CLOSE (f);
		*/
	} else {
		/*
		joybuttonassign := defaultjoybuttonassign;
		midiport := 1;
		soundeffects := 1;
		Jconfigure := 0;
		keysettings := defaultkeysettings;
		Background2 := TRUE;
		InputDevice := 0;
		Tyr_musicvolume := 255;
		FXvolume := 128;
		Gammacorrection := 0;
		Processortype := 3;
		GameSpeed := 4;
		InputDevice1 := 0;
		InputDevice2 := 0;
		*/
    }
  
	/*
  IF Tyr_musicvolume > 255 THEN
    Tyr_musicvolume := 255;
  IF FXvolume > 254 THEN
    FXvolume := 254;
  IF FXvolume < 14 THEN
    FXvolume := 14;*/
  
  soundActive = TRUE;
  musicActive = TRUE;
  
  JE_setVol(tyr_musicVolume, fxVolume);
  
	/*
  IF getfilesize ('Tyrian.SAV') <> 0 THEN
    BEGIN
      ASSIGN (f, 'TYRIAN.SAV');
      RESET (f, 1);
      BLOCKREAD (f, savetemp, SIZEOF (savetemp) );
      DecryptSaveTemp;
      
      editorlevel := btow (savetemp [SIZEOF (savetemp) - 5], savetemp [SIZEOF (savetemp) - 4]);
      
      ASM
        mov dx, ds
        
        mov cx, savefilessize
        
        mov si, savetempofs
        mov di, savefilesofs
        
        mov ax, savefilesseg
        mov bx, savetempseg
        mov ds, bx
        mov es, ax
        
        cld
        rep movsb
        
        mov ds, dx
      END;
      
      CLOSE (f);
    END 
  ELSE
    BEGIN
      editorlevel := 800;
      FOR Z := 1 TO 100 DO
        BEGIN
          savetemp [SIZEOF (savefiles) + Z] := initialitemavail [Z];
        END;
      FOR Z := 1 TO savefilesnum DO
        BEGIN
          savefiles [Z] .level := 0;
          FOR Y := 1 TO 14 DO
            savefiles [Z] .name [Y] := ' ';
          savefiles [Z] .highscore1 := (RANDOM (20) + 1) * 1000;
          IF (Z - 1) MOD 6 > 2 THEN
            BEGIN
              savefiles [Z] .highscore2 := (RANDOM (20) + 1) * 1000;
              savefiles [Z] .highscorename := defaultteamnames [RANDOM (22) + 1];
            END
          ELSE
            savefiles [Z] .highscorename := defaulthighscorenames [RANDOM (34) + 1];
        END;
    END;
  ErrorActive := FALSE;
  
  calcFXvol;
  InitProcessorType;
  */
}

void JE_saveConfiguration( void )
{
	STUB(saveConfiguration);
}

/*
FUNCTION NextEpisode : BOOLEAN;
VAR found : BOOLEAN;
BEGIN
  found := FindNextEpisode;
  
  IF found THEN
    mainlevel := 0;
END;
*/
