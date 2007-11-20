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
#include "config.h"

#include "episodes.h"
#include "error.h"
#include "joystick.h"
#include "loudness.h"
#include "nortsong.h"
#include "nortvars.h"
#include "varz.h"
#include "vga256d.h"

#include <stdio.h>
#include <string.h>


/******** MAJOR TODO:
  SYN: High score data is stored one per save file slot. That makes 2 * 11 = 22 high scores.
  Each episode has six high scores. 6 * 4 = 24 OH SHI--

  I have no idea what is up with this, but I'm going to have to change substantial amounts and
  possibly partially break compatibility with the original. This will also get sorted out
  if/when we add support for Tyrian2000 data files, as I'll have to figure out what its save
  file format is (besides a couple kilobytes larger...).

  As it stands high scores are going to be broked for episode 4 (nevermind 5) and there's not
  much I can do about it. *emo tear* :'(

  I hope there aren't any other surprises like this waiting. We are using the code for v2.0,
  right? Right? :|
*/


/* Configuration Load/Save handler */

const JE_byte cryptKey[10] = /* [1..10] */
{
	15, 50, 89, 240, 147, 34, 86, 9, 32, 208
};

const JE_KeySettingType defaultKeySettings =
{
	SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_SPACE, SDLK_RETURN, SDLK_LCTRL, SDLK_LALT
/*	72, 80, 75, 77, 57, 28, 29, 56*/
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
StarDatType starDat[MAX_STARS]; /* [1..Maxstars] */
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
JE_boolean trentWin = false;
JE_byte    superArcadeMode;

JE_byte    superArcadePowerUp;

JE_real linkGunDirec;
JE_byte playerDevice1, playerDevice2;
JE_byte inputDevice1, inputDevice2;

JE_byte secretHint;
JE_byte background3over;
JE_byte background2over;
JE_byte gammaCorrection;
JE_boolean superPause = false;
JE_boolean explosionTransparent,
           youAreCheating,
           displayScore,
           soundHasChanged,
           background2, smoothScroll, wild, superWild, starActive,
           topEnemyOver,
           skyEnemyOverAll,
           background2notTransparent,
           tempb;

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
JE_SaveGameTemp saveTemp;

JE_word editorLevel;   /*Initial value 800*/

JE_word x;

bool fullscreen_set = false, fullscreen_enabled;

const JE_byte StringCryptKey[10] = {99, 204, 129, 63, 255, 71, 19, 25, 62, 1};

void JE_decryptString( char *s, JE_byte len )
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

void JE_readCryptLn( FILE* f, char *s )
{
	int size;

	size = getc(f);
	if (size < 0)
		size = 0;
	efread(s, 1, size, f);
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

	for (z = MAX_STARS; z--; )
	{
		starDat[z].sLoc = (rand() % 320) + (rand() % 200) * 320;
		starDat[z].sMov = ((rand() % 3) + 2) * 320;
		starDat[z].sC = (rand() % 16) + (9 * 16);
	}
}

void JE_saveGame( JE_byte slot, char *name )
{
	int i;

	saveFiles[slot-1].initialDifficulty = initialDifficulty;
	saveFiles[slot-1].gameHasRepeated = gameHasRepeated;
	saveFiles[slot-1].level = saveLevel;

	pItems[3-1] = superArcadeMode;
	if (superArcadeMode == 0 && onePlayerAction)
	{
		pItems[3-1] = 255;
	}
	if (superTyrian)
	{
		pItems[3-1] = 254;
	}

	memcpy(saveFiles[slot-1].items, pItems, sizeof(pItems));

	if (superArcadeMode > 253)
	{
		pItems[3-1] = 0;
	}
	if (twoPlayerMode)
	{
		memcpy(saveFiles[slot-1].lastItems, pItemsPlayer2, sizeof(pItemsPlayer2));
	} else {
		memcpy(saveFiles[slot-1].lastItems, pItemsBack2, sizeof(pItemsBack2));
	}

	saveFiles[slot-1].score  = score;
	saveFiles[slot-1].score2 = score2;
	memcpy(saveFiles[slot-1].levelName, lastLevelName, sizeof(lastLevelName));
	saveFiles[slot-1].cubes  = lastCubeMax;

	if (strcmp(lastLevelName, "Completed") == 0)
	{
		temp = episodeNum - 1;
		if (temp < 1)
		{
			temp = 4; /* JE: {Episodemax is 4 for completion purposes} */
		}
		saveFiles[slot-1].episode = temp;
	} else {
		saveFiles[slot-1].episode = episodeNum;
	}

	saveFiles[slot-1].difficulty = difficultyLevel;
	saveFiles[slot-1].secretHint = secretHint;
	saveFiles[slot-1].input1 = inputDevice1;
	saveFiles[slot-1].input2 = inputDevice2;

	strcpy(saveFiles[slot-1].name, name);

	for (x = 0; x < 2; x++)
	{
		saveFiles[slot-1].power[x] = portPower[x];
	}

	JE_saveConfiguration();
}

void JE_loadGame( JE_byte slot )
{
	JE_byte temp5;

	superTyrian = false;
	onePlayerAction = false;
	twoPlayerMode = false;
	extraGame = false;
	galagaMode = false;

	initialDifficulty = saveFiles[slot-1].initialDifficulty;
	gameHasRepeated   = saveFiles[slot-1].gameHasRepeated;
	twoPlayerMode     = (slot-1) > 10;
	difficultyLevel   = saveFiles[slot-1].difficulty;
	memcpy(pItems, saveFiles[slot-1].items, sizeof(pItems));
	superArcadeMode   = pItems[3-1];

	if (superArcadeMode == 255)
	{
		onePlayerAction = true;
		superArcadeMode = 0;
		pItems[3-1] = 0;
	} else if (superArcadeMode == 254) {
		onePlayerAction = true;
		superArcadeMode = 0;
		pItems[3-1] = 0;
		superTyrian = true;
	} else if (superArcadeMode > 0) {
		onePlayerAction = true;
	}

	if (twoPlayerMode)
	{
		memcpy(pItemsPlayer2, saveFiles[slot-1].lastItems, sizeof(pItemsPlayer2));
		onePlayerAction = false;
	} else {
		memcpy(pItemsBack2, saveFiles[slot-1].lastItems, sizeof(pItemsBack2));
	}

	/* Compatibility with old version */
	if (pItemsPlayer2[7-1] < 101)
	{
		pItemsPlayer2[7-1] = 101;
		pItemsPlayer2[8-1] = pItemsPlayer2[4-1];
	}

	score       = saveFiles[slot-1].score;
	score2      = saveFiles[slot-1].score2;
	mainLevel   = saveFiles[slot-1].level;
	cubeMax     = saveFiles[slot-1].cubes;
	lastCubeMax = cubeMax;

	secretHint   = saveFiles[slot-1].secretHint;
	inputDevice1 = saveFiles[slot-1].input1;
	inputDevice2 = saveFiles[slot-1].input2;

	for (temp = 0; temp < 2; temp++)
	{
		portPower[temp] = saveFiles[slot-1].power[temp];
	}

	temp5 = saveFiles[slot-1].episode;

	memcpy(levelName, saveFiles[slot-1].levelName, sizeof(levelName));

	if (strcmp(levelName, "Completed") == 0)
	{
		if (temp5 == 4)
		{
			temp5 = 1;
		} else if (temp5 < 4) {
			temp5++;
		}
		/* Increment 1-3 to 2-4.  Episode 4 goes to 1.  Episode 5 stands still. */
	}

	JE_initEpisode(temp5);
	saveLevel = mainLevel;
	memcpy(lastLevelName, levelName, sizeof(levelName));
}

void JE_initProcessorType( void )
{
	/* SYN: Originally this proc looked at your hardware specs and chose appropriate options. We don't care, so I'll just set
	   decent defaults here. */

	wild = false;
	superWild = false;
	smoothScroll = true;
	explosionTransparent = true;
	filtrationAvail = false;
	background2 = true;
	displayScore = true;

	switch (processorType)
	{
		case 1: /* 386 */
			background2 = false;
			displayScore = false;
			explosionTransparent = false;
			break;
		case 2: /* 486 - Default */
			break;
		case 3: /* High Detail */
			smoothScroll = false;
			break;
		case 4: /* Pentium */
			wild = true;
			filtrationAvail = true;
			break;
		case 5: /* Nonstandard VGA */
			smoothScroll = false;
			break;
		case 6: /* SuperWild */
			wild = true;
			superWild = true;
			filtrationAvail = true;
			break;
	}

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
	pentiumMode = false;

	switch (fastPlay)
	{
	case 0:
		speed = 0x4300;
		smoothScroll = true;
		frameCountMax = 2;
		break;
	case 1:
		speed = 0x3000;
		smoothScroll = true;
		frameCountMax = 2;
		break;
	case 2:
		speed = 0x2000;
		smoothScroll = false;
		frameCountMax = 2;
		break;
	case 3:
		speed = 0x5300;
		smoothScroll = true;
		frameCountMax = 4;
		break;
	case 4:
		speed = 0x4300;
		smoothScroll = true;
		frameCountMax = 3;
		break;
	case 5:
		speed = 0x4300;
		smoothScroll = true;
		frameCountMax = 2;
		pentiumMode = true;
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
	saveTemp[SAVE_FILE_SIZE] = y;

	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y -= s3[x];
	}
	saveTemp[SAVE_FILE_SIZE+1] = y;

	y = 1;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y = (y * s3[x]) + 1;
	}
	saveTemp[SAVE_FILE_SIZE+2] = y;

	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y = y ^ s3[x];
	}
	saveTemp[SAVE_FILE_SIZE+3] = y;

	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		saveTemp[x] = saveTemp[x] ^ cryptKey[(x+1) % 10];
		if (x > 0)
		{
			saveTemp[x] = saveTemp[x] ^ saveTemp[x - 1];
		}
	}
}

void JE_decryptSaveTemp( void )
{
	JE_boolean correct = true;
	JE_SaveGameTemp s2;
	/*JE_word x;*/
	int x;
	JE_byte y, z;

	/* Decrypt save game file */
	for (x = (SAVE_FILE_SIZE - 1); x >= 0; x--)
	{
		s2[x] = (JE_byte)saveTemp[x] ^ (JE_byte)(cryptKey[(x+1) % 10]);
		if (x > 0)
		{
			s2[x] ^= (JE_byte)saveTemp[x - 1];
		}

	}

	/* for (x = 0; x < SAVE_FILE_SIZE; x++) printf("%c", s2[x]); */

	/* Check save file for correctitude */
	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y += s2[x];
	}
	if (saveTemp[SAVE_FILE_SIZE] != y)
	{
		correct = false;
		printf("Failed additive checksum: %d vs %d\n", saveTemp[SAVE_FILE_SIZE], y);
	}

	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y -= s2[x];
	}
	if (saveTemp[SAVE_FILE_SIZE+1] != y)
	{
		correct = false;
		printf("Failed subtractive checksum: %d vs %d\n", saveTemp[SAVE_FILE_SIZE+1], y);
	}

	y = 1;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y = (y * s2[x]) + 1;
	}
	if (saveTemp[SAVE_FILE_SIZE+2] != y)
	{
		correct = false;
		printf("Failed multiplicative checksum: %d vs %d\n", saveTemp[SAVE_FILE_SIZE+2], y);
	}

	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y = y ^ s2[x];
	}
	if (saveTemp[SAVE_FILE_SIZE+3] != y)
	{
		correct = false;
		printf("Failed XOR'd checksum: %d vs %d\n", saveTemp[SAVE_FILE_SIZE+3], y);
	}

	/* Barf and die if save file doesn't validate */
	if (!correct)
	{
		printf("Error reading save file!\n");
		exit(255);
	}

	/* Keep decrypted version plz */
	memcpy(saveTemp, s2, sizeof(s2));
}

void JE_loadConfiguration( void )
{
	FILE *fi;
	int z;
	JE_byte *p;
	JE_byte junk;
	int y;

	errorActive = true;

	if (!JE_isCFGThere())
	{
		JE_resetFile(&fi, "tyrian.cfg");

		/* SYN: I've hardcoded the sizes here because the .CFG file format is fixed
		   anyways, so it's not like they'll change. */
		background2 = 0;
		efread(&background2, 1, 1, fi);
		efread(&gameSpeed, 1, 1, fi);

		/* Wait what? */
		efread(&inputDevice, 1, 1, fi);
		inputDevice = 0;

		efread(&jConfigure, 1, 1, fi);
		if (jConfigure == 0) /* Dunno what this is about. */
		{
			jConfigure = 1;
		}

		efread(&versionNum, 1, 1, fi);
		if (resetVersion)
		{
			versionNum = 2; /* JE: {Shareware 1.0 and Registered 1.1 = 1} */
		}

		efread(&processorType, 1, 1, fi);
		efread(&junk, 1, 1, fi); /* Unused variable -- was "BLOCKREAD (f, midiport   , 1);" */
		efread(&soundEffects, 1, 1, fi);
		efread(&gammaCorrection, 1, 1, fi);
		efread(&difficultyLevel, 1, 1, fi);
		efread(joyButtonAssign, 4, 1, fi); /* 4 bytes */

		efread(&tyrMusicVolume, 2, 1, fi);
		efread(&fxVolume, 2, 1, fi);

		efread(&inputDevice1, 1, 1, fi);
		efread(&inputDevice2, 1, 1, fi);

		/* efread(keySettings, 8, 1, fi); Fixed because we're using SDLKey now */
		efread(keySettings, sizeof(keySettings), 1, fi);

		if (!feof(fi))
		{
			/* Fullscreen settings */

			Uint8 tmp;
			efread(&tmp, sizeof(tmp), 1, fi);

			fullscreen_set = (tmp == 1);
		}

		fclose(fi);
	} else {
		memcpy(joyButtonAssign, defaultJoyButtonAssign, sizeof(joyButtonAssign));
		/*midiPort = 1;*/ /* We don't care about this. */
		soundEffects = 1;
		jConfigure = 0; /* TODO: Figure out what this is */
		memcpy(keySettings, defaultKeySettings, sizeof(keySettings));
		background2 = true;
		inputDevice = 0;
		tyrMusicVolume = 255;
		fxVolume = 128;
		gammaCorrection = 0;
		processorType = 3;
		gameSpeed = 4;
		inputDevice1 = 0;
		inputDevice2 = 0;

		fullscreen_set = false;
	}

	tyrMusicVolume = (tyrMusicVolume > 255) ? 255 : tyrMusicVolume;
	fxVolume = (fxVolume > 254) ? 254 : ((fxVolume < 14) ? 14 : fxVolume);

	soundActive = true;
	musicActive = true;

	JE_setVol(tyrMusicVolume, fxVolume);

	dont_die = true;
	JE_resetFile(&fi, "tyrian.sav");
	dont_die = false;

	if (fi)
	{

		fseek(fi, 0, SEEK_SET);
		efread(saveTemp, 1, sizeof(saveTemp), fi);
		JE_decryptSaveTemp();

		/* SYN: The original mostly blasted the save file into raw memory. However, our lives are not so
		   easy, because the C struct is necessarily a different size. So instead we have to loop
		   through each record and load fields manually. *emo tear* :'( */

		p = saveTemp;
		for (z = 0; z < SAVE_FILES_NUM; z++)
		{
			saveFiles[z].encode = *((JE_word*)p);
			p += sizeof(JE_word);

			saveFiles[z].level = *((JE_word*)p);
			p += sizeof(JE_word);

			memcpy(saveFiles[z].items, ((JE_PItemsType*)p), sizeof(JE_PItemsType));
			p += sizeof(JE_PItemsType);

			saveFiles[z].score = *((JE_longint*)p);
			p += sizeof(JE_longint);

			saveFiles[z].score2 = *((JE_longint*)p);
			p += sizeof(JE_longint);

			/* SYN: Pascal strings are prefixed by a byte holding the length! */
			p += 1; /* Skip length byte */
			memcpy(saveFiles[z].levelName, ((char*)p), 9);
			saveFiles[z].levelName[9] = 0;
			p += 9;

			/* This was a BYTE array, not a STRING, in the original. Go fig. */
			memcpy(saveFiles[z].name, ((char*)p), 14);
			saveFiles[z].name[14] = 0;
			p += 14;

			saveFiles[z].cubes = *((JE_byte*)p);
			p += sizeof(JE_byte);

			memcpy(saveFiles[z].power, ((JE_byte*)p), sizeof(JE_byte) * 2);
			p += (sizeof(JE_byte) * 2);

			saveFiles[z].episode = *((JE_byte*)p);
			p += sizeof(JE_byte);

			memcpy(saveFiles[z].lastItems, ((JE_PItemsType*)p), sizeof(JE_PItemsType));
			p += sizeof(JE_PItemsType);

			saveFiles[z].difficulty = *((JE_byte*)p);
			p += sizeof(JE_byte);

			saveFiles[z].secretHint = *((JE_byte*)p);
			p += sizeof(JE_byte);

			saveFiles[z].input1 = *((JE_byte*)p);
			p += sizeof(JE_byte);

			saveFiles[z].input2 = *((JE_byte*)p);
			p += sizeof(JE_byte);

			saveFiles[z].gameHasRepeated = *((JE_boolean*)p);
			p += 1; /* TODO: should be sizeof(JE_boolean) but that is 4 for some reason :( */

			saveFiles[z].initialDifficulty = *((JE_byte*)p);
			p += sizeof(JE_byte);

			saveFiles[z].highScore1 = *((JE_longint*)p);
			p += sizeof(JE_longint);

			saveFiles[z].highScore2 = *((JE_longint*)p);
			p += sizeof(JE_longint);

			p += 1; /* Skip length byte wheeee */
			memcpy(saveFiles[z].highScoreName, ((char*)p), 29);
			saveFiles[z].highScoreName[29] = 0;
			p += 29;

			/* TODO DEBUG printf("%s, %ld / %ld\n", saveFiles[z].highScoreName, saveFiles[z].highScore1, saveFiles[z].highScore2); */

			saveFiles[z].highScoreDiff = *((JE_byte*)p);
			p += sizeof(JE_byte);
		}

		/* SYN: This is truncating to bytes. I have no idea what this is doing or why. */
		/* TODO: Figure out what this is about and make sure it isn't broked. */
		editorLevel = JE_btow((JE_byte) (SIZEOF_SAVEGAMETEMP - 6), (JE_byte) (SIZEOF_SAVEGAMETEMP - 5));

		fclose(fi);
	} else {
		/* We didn't have a save file! Let's make up random stuff! */
		editorLevel = 800;

		for (z = 0; z < 100; z++)
		{
			saveTemp[SAVE_FILES_SIZE + z] = initialItemAvail[z];
		}

		for (z = 0; z < SAVE_FILES_NUM; z++)
		{
			saveFiles[z].level = 0;

			for (y = 0; y < 14; y++)
			{
				saveFiles[z].name[y] = ' ';
			}
			saveFiles[z].name[14] = 0;

			saveFiles[z].highScore1 = ((rand() % 20) + 1) * 1000;

			if (z % 6 > 2)
			{
				saveFiles[z].highScore2 = ((rand() % 20) + 1) * 1000;
				strcpy(saveFiles[z].highScoreName, defaultHighScoreNames[rand() % 22]);
			} else {
				strcpy(saveFiles[z].highScoreName, defaultHighScoreNames[rand() % 34]);
			}
		}
	}

	errorActive = false;

	JE_calcFXVol();
	JE_initProcessorType();
}

void JE_saveConfiguration( void )
{
	FILE *f;
	JE_byte *p, junk = 0;
	int z;

	p = saveTemp;
	for (z = 0; z < SAVE_FILES_NUM; z++)
	{
		*((JE_word*)p) = saveFiles[z].encode;
		p += sizeof(JE_word);

		*((JE_word*)p) = saveFiles[z].level;
		p += sizeof(JE_word);

		memcpy(((JE_PItemsType*)p), saveFiles[z].items, sizeof(JE_PItemsType));
		p += sizeof(JE_PItemsType);

		*((JE_longint*)p) = saveFiles[z].score ;
		p += sizeof(JE_longint);

		*((JE_longint*)p) = saveFiles[z].score2;
		p += sizeof(JE_longint);

		/* SYN: Pascal strings are prefixed by a byte holding the length! */
		*((JE_byte*)p) = strlen(saveFiles[z].levelName);
		p++;
		memcpy(((char*)p), saveFiles[z].levelName, 9);
		p += 9;

		/* This was a BYTE array, not a STRING, in the original. Go fig. */
		memcpy(((char*)p), saveFiles[z].name, 14);
		p += 14;

		*((JE_byte*)p) = saveFiles[z].cubes;
		p += sizeof(JE_byte);

		memcpy(((JE_byte*)p), saveFiles[z].power, sizeof(JE_byte) * 2);
		p += (sizeof(JE_byte) * 2);

		*((JE_byte*)p) = saveFiles[z].episode ;
		p += sizeof(JE_byte);

		memcpy(((JE_PItemsType*)p), saveFiles[z].lastItems, sizeof(JE_PItemsType));
		p += sizeof(JE_PItemsType);

		*((JE_byte*)p) = saveFiles[z].difficulty;
		p += sizeof(JE_byte);

		*((JE_byte*)p) = saveFiles[z].secretHint ;
		p += sizeof(JE_byte);

		*((JE_byte*)p) = saveFiles[z].input1;
		p += sizeof(JE_byte);

		*((JE_byte*)p) = saveFiles[z].input2;
		p += sizeof(JE_byte);

		*((JE_boolean*)p) = saveFiles[z].gameHasRepeated ;
		p++; /* TODO: should be sizeof(JE_boolean) but that is 4 for some reason :( */

		*((JE_byte*)p) = saveFiles[z].initialDifficulty;
		p += sizeof(JE_byte);

		*((JE_longint*)p) = saveFiles[z].highScore1;
		p += sizeof(JE_longint);

		*((JE_longint*)p) = saveFiles[z].highScore2 ;
		p += sizeof(JE_longint);

		p++; /* Skip length byte wheeee */
		memcpy(((char*)p), saveFiles[z].highScoreName, 29);
		p += 29;

		*((JE_byte*)p) = saveFiles[z].highScoreDiff ;
		p += sizeof(JE_byte);
	}

	saveTemp[SIZEOF_SAVEGAMETEMP - 6] = editorLevel >> 8;
	saveTemp[SIZEOF_SAVEGAMETEMP - 5] = editorLevel;

	JE_encryptSaveTemp();
	f = fopen_check("tyrian.sav", "wb");
	if (f)
	{
		efwrite(saveTemp, 1, sizeof(saveTemp), f);
		fclose(f);
	}
	JE_decryptSaveTemp();

	f = fopen_check("tyrian.cfg", "wb");
	if (f)
	{
		efwrite(&background2, 1, 1, f);
		efwrite(&gameSpeed, 1, 1, f);
		efwrite(&inputDevice, 1, 1, f);
		efwrite(&jConfigure, 1, 1, f);
		efwrite(&versionNum, 1, 1, f);
		efwrite(&processorType, 1, 1, f);
		efwrite(&junk, 1, 1, f); /* This isn't needed. Was: fwrite(midiPort, 1, sizeof(midiPort), f);*/
		efwrite(&soundEffects, 1, 1, f);
		efwrite(&gammaCorrection, 1, 1, f);
		efwrite(&difficultyLevel, 1, 1, f);
		efwrite(joyButtonAssign, 1, 4, f);

		efwrite(&tyrMusicVolume, 1, 2, f);
		efwrite(&fxVolume, 1, 2, f);

		efwrite(&inputDevice1, 1, 1, f);
		efwrite(&inputDevice2, 1, 1, f);

		/* efwrite(keySettings, 1, 8, f); */
		efwrite(keySettings, sizeof(keySettings), 1, f);

		/* New fullscreen stuff */
		Uint8 tmp = (fullscreen_set ? 1 : 0);
		efwrite(&tmp, sizeof(tmp), 1, f);
	
		fclose(f);
	}
}
