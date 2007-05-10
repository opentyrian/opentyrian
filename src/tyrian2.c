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
#include "newshape.h"
#include "fonthand.h"
#include "vga256d.h"
#include "picload.h"
#include "starfade.h"
#include "helptext.h"
#include "nortsong.h"
#include "pallib.h"
#include "pcxmast.h"
#include "keyboard.h"
#include "varz.h"
#include "joystick.h"
#include "setup.h"
#include "mainint.h"
#include "sndmast.h"
#include "params.h"
#include "network.h"
#include "loudness.h"
#include "backgrnd.h"
#include "error.h"
#include "episodes.h"
#include "lvllib.h"

#include "tyrian2.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

JE_byte planetAni, planetAniWait;
JE_byte currentDotNum, currentDotWait;
JE_real navX, navY, newNavX, newNavY;
JE_integer tempNavX, tempNavY;
JE_byte planetDots[5]; /* [1..5] */
JE_integer planetDotX[5][10], planetDotY[5][10]; /* [1..5, 1..10] */

/* Level Event Data */
JE_boolean quit, first, loadLevelOk, refade;
JE_byte newPal, curPal, oldPal;

struct JE_EventRecType eventRec[EVENT_MAXIMUM]; /* [1..eventMaximum] */
JE_word levelEnemyMax;
JE_word levelEnemyFrequency;
JE_word levelEnemy[40]; /* [1..40] */

char tempStr[31];

/* Data used for ItemScreen procedure to indicate items available */
JE_byte itemAvail[9][10]; /* [1..9, 1..10] */
JE_byte itemAvailMax[9]; /* [1..9] */

const JE_word generatorX[5] = { 61, 63, 66, 65, 62 };
const JE_word generatorY[5] = { 83, 84, 85, 83, 96 };

const JE_byte rearWeaponList[40] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3,
	4, 0, 5, 6, 0, 0, 7, 0, 0, 2, 1,
	0, 7, 0, 6, 0, 1, 1, 4, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1
};
const JE_word rearWeaponX[7] = { 41, 27,  49,  43, 51, 39, 41 };
const JE_word rearWeaponY[7] = { 92, 92, 113, 102, 97, 96, 76 };

const JE_byte frontWeaponList[42] = {
	 5, 10, 4, 9, 3, 6, 11, 2, 0, 0,
	 0,  0, 8, 9, 0, 0,  1, 0, 5, 1,
	 0,  0, 4, 0, 5, 0,  5, 0, 0, 0,
	10,  1, 1, 1, 1, 1,  1, 1, 1, 1,
	 4, 10
};
const JE_word frontWeaponX[12] = { 58, 65, 65, 53, 60, 50, 57, 50, 60, 51, 52, 57 };
const JE_word frontWeaponY[12] = { 38, 53, 41, 36, 48, 35, 41, 35, 53, 41, 39, 31 };

const JE_word planetX[21] = { 200, 150, 240, 300, 270, 280, 320, 260, 220, 150, 160, 210, 80, 240, 220, 180, 310, 330, 150, 240, 200 };
const JE_word planetY[21] = {  40,  90,  90,  80, 170,  30,  50, 130, 120, 150, 220, 200, 80,  50, 160,  10,  55,  55,  90,  90,  40 };

const JE_byte tyrian2_weapons[42] = {
	 1,  2,  3,  4,  5,  6,  7,  8, 9, 10,
	11, 12, 22,  6, 14,  0, 15, 16, 1, 15,
	10,  9,  3, 16,  1, 14,  1,  9, 9, 12,
	 2,  1,  1,  1,  1,  1,  1,  1, 1,  1,
	 3,  2
};

const JE_MenuChoiceType menuChoicesDefault = { 7, 9, 8, 0, 0, 11, (SAVE_FILES_NUM / 2) + 2, 0, 0, 6, 4, 6, 7, 5 };

const JE_byte menuEsc[MAX_MENU] = { 0, 1, 1, 1, 2, 3, 3, 1, 8, 0, 0, 11, 3, 0 };

const JE_byte itemAvailMap[7] = { 1, 2, 3, 9, 4, 6, 7 };

const JE_byte weaponReset[7] = { 0, 1, 2, 0, 0, 3, 4 };

const JE_byte mouseSelectionY[MAX_MENU] = { 16, 16, 16, 16, 26, 12, 11, 28, 0, 16, 16, 16, 24, 16 };

void JE_main( void )
{
	int i, j, k;
	JE_byte **bp, *src;
	unsigned char *s; /* screen pointer, 8-bit specific */

	loadTitleScreen = TRUE;

	/* Setup Player Items/General Data */
	fixedExplosions = FALSE;
	explosionMoveUp = 0;
	for (z = 0; z < 12; z++)
	{
		pItems[z] = 0;
	}
	shieldSet = 5;

	/* Setup Graphics */
	/* JE_initVGA256(); This shouldn't be done this because we're already doing it in main() */
	JE_updateColorsFast(&black);

	/*debuginfo('Initiating Configuration');*/


	/* Setup Input Device */
	/*JConfigure:=false;*/

	debug = FALSE;

	/* NOTE: BEGIN MAIN PROGRAM HERE AFTER LOADING A GAME OR STARTING A NEW ONE */

	/* ----------- GAME ROUTINES ------------------------------------- */
	/* We need to jump to the beginning to make space for the routines */
	/* --------------------------------------------------------------- */
	goto start_level_first;

	/* TODO */

start_level:

start_level_first:

	/*stopsequence;*/
	/*debuginfo('Setting Master Sound Volume');*/
	JE_setVol(tyrMusicVolume, fxVolume);

	JE_loadCompShapes(&shapes6, &shapes6Size, '1');  /* Items */

	endLevel = FALSE;
	reallyEndLevel = FALSE;
	playerEndLevel = FALSE;
	extraGame = FALSE;

	/*debuginfo('Loading LEVELS.DAT');*/

	doNotSaveBackup = FALSE;
	JE_loadMap();
	JE_selectSong(0xC001); /*Fade song out*/

	playerAlive = TRUE;
	playerAliveB = TRUE;
	oldDifficultyLevel = difficultyLevel;
	if (episodeNum == 4)
	{
		difficultyLevel--;
	}
	if (difficultyLevel < 1)
	{
		difficultyLevel = 1;
	}

	if (loadDestruct)
	{
		if (eShapes1 != NULL)
		{
			free(eShapes1);
		}
		/* TODO JE_destructGame();*/
		loadDestruct = FALSE;
		loadTitleScreen = TRUE;
		goto start_level_first;
	}

	PX = 100;
	PY = 180;

	PXB = 190;
	PYB = 180;

	playerHNotReady = TRUE;

	lastPXShotMove = PX;
	lastPYShotMove = PY;

	if (twoPlayerMode)
	{
		JE_loadPic(6, FALSE);
	} else {
		JE_loadPic(3, FALSE);
	}

	tempScreenSeg = VGAScreen;
	JE_drawOptions();

	if (twoPlayerMode)
	{
		temp = 76;
	} else {
		temp = 118;
	}
	JE_outText(268, temp, levelName, 12, 4);

	JE_showVGA();
	/* TODO JE_gammaCorrect(&colors, gammaCorrection);*/
	JE_fadeColor(50);

	JE_loadCompShapes(&shapes6, &shapes6Size, '6'); /* Explosions */

	/* MAPX will already be set correctly */
	mapy = 300 - 8;
	mapy2 = 600 - 8;
	mapy3 = 600 - 8;
	mapyPos = &megaData1->mainmap[mapy][0] - 1;
	mapy2Pos = &megaData2->mainmap[mapy2][0] - 1;
	mapy3Pos = &megaData3->mainmap[mapy3][0] - 1;
	mapxPos = 0;
	mapxOfs = 0;
	mapx2Pos = 0;
	mapx3Pos = 0;
	mapx3Ofs = 0;
	mapxbpPos = 0;
	mapx2bpPos = 0;
	mapx3bpPos = 0;

	map1yDelay = 1;
	map1yDelayMax = 1;
	map2yDelay = 1;
	map2yDelayMax = 1;

	musicFade = FALSE;

	backPos = 0;
	backPos2 = 0;
	backPos3 = 0;
	power = 0;
	starY = 320;

	/* Setup maximum player speed */
	/* ==== Mouse Input ==== */
/*	if (timMode) then begin
		basespeed = 80;
		basespeedkeyh = (basespeed / 3) + 1;
		basespeedkeyv = (basespeed / 4) + 1;
	} else {*/
	baseSpeed = 6;
	baseSpeedKeyH = (baseSpeed / 4) + 1;
	baseSpeedKeyV = (baseSpeed / 4) + 1;
/*	}*/

	baseSpeedOld = baseSpeed;
	baseSpeedOld2 = (baseSpeed * 0.7f) + 1;
	baseSpeed2  = 100 - (((baseSpeed + 1) / 4) + 1);
	baseSpeed2B = 100 + 100 - baseSpeed2;
	baseSpeed   = 100 - (((baseSpeed + 1) / 4) + 1);
	baseSpeedB  = 100 + 100 - baseSpeed;
	shadowyDist = 10;

	/* Setup player ship graphics */
	JE_getShipInfo();
	tempI     = (((PX - mouseX) / (100 - baseSpeed)) * 2) * 168;
	lastTurn  = 0;
	lastTurnB = 0;
	lastTurn2 = 0;
	lastTurnB = 0;

	playerInvulnerable1 = 100;
	playerInvulnerable2 = 100;

	newkey = FALSE;

	/* Initialize Level Data and Debug Mode */
	levelEnd = 255;
	levelEndWarp = -4;
	levelEndFxWait = 0;
	warningCol = 120;
	warningColChange = 1;
	warningSoundDelay = 0;
	armorShipDelay = 50;

	bonusLevel = FALSE;
	readyToEndLevel = FALSE;
	firstGameOver = TRUE;
	eventLoc = 1;
	curLoc = 0;
	backMove = 1;
	backMove2 = 2;
	backMove3 = 3;
	explodeMove = 640;
	enemiesActive = TRUE;
	for(temp = 0; temp < 3; temp++)
	{
		button[temp] = FALSE;
	}
	stopBackgrounds = FALSE;
	stopBackgroundNum = 0;
	background3x1   = FALSE;
	background3x1b  = FALSE;
	background3over = 0;
	background2over = 1;
	topEnemyOver = FALSE;
	skyEnemyOverAll = FALSE;
	smallEnemyAdjust = FALSE;
	starActive = TRUE;
	enemyContinualDamage = FALSE;
	levelEnemyFrequency = 96;
	quitRequested = FALSE;

	memset(statBar, 0, sizeof(statBar));

	forceEvents = FALSE;  /*Force events to continue if background movement = 0*/

	uniqueEnemy = FALSE;  /*Look in MakeEnemy under shape bank*/

	superEnemy254Jump = 0;   /*When Enemy with PL 254 dies*/

	/* Filter Status */
	filterActive = TRUE;
	filterFade = TRUE;
	filterFadeStart = FALSE;
	levelFilter = -99;
	levelBrightness = -14;
	levelBrightnessChg = 1;

	background2notTransparent = FALSE;

	/* Initially erase power bars */
	lastPower = power / 10;

	/* Initial Text */
	JE_drawTextWindow(miscText[20]);

	/* Setup Armor/Shield Data */
	shieldWait = 1;
	shield     = shields[pItems[9]-1].mpwr;
	shieldT    = shields[pItems[9]-1].tpwr * 20;
	shieldMax  = shield * 2;
	shield2    = shields[pItemsPlayer2[9]-1].mpwr;
	shieldMax2 = shield * 2;
	JE_drawShield();
	JE_drawArmor();

	superBomb[0] = 0;
	superBomb[2] = 0;

	/* Set cubes to 0 */
	cubeMax = 0;

	lastPortPower[0] = 0;
	lastPortPower[1] = 0;
	lastPortPower[2] = 0;
	lastPortPower[3] = 0;

	/* Secret Level Display */
	flash = 0;
	flashChange = 1;
	displayTime = 0;

	JE_playSong(levelSong);

	playerFollow = FALSE;

	/*if not JConfigure and (InputDevice=1) then CalibrateJoy;*/

	JE_drawPortConfigButtons();

	/* --- MAIN LOOP --- */

	newkey = FALSE;
	
	if (isNetworkGame)
	{
		/* TODO 
		gameQuitDelay = streamLagFrames + 1;
		JE_clearSpecialRequests();
		RANDOMIZE;
		randSeed = 32402394;
		randomCount = 0;
		*/
	}

	JE_setupStars();

	JE_setNewGameSpeed();

	JE_setVol(tyrMusicVolume, fxPlayVol >> 2);

	/*Save backup game*/
	if (!playDemo && !doNotSaveBackup)
	{
		if (twoPlayerMode)
		{
			temp = 22;
		} else {
			temp = 11;
		}
		JE_saveGame(temp, "LAST LEVEL    ");
	}

	/*Set keyboard to NO BIOS*/
	/* NOTE callBIOShandler = FALSE; */

	memset(lastKey, 0, sizeof(lastKey));
	if (recordDemo && !playDemo)
	{
		do
		{
			sprintf(tempStr, "DEMOREC.%d", recordFileNum);
			tempb = JE_find(tempStr);
			if (tempb)
			{
				recordFileNum++;
			}
		} while (tempb);

		JE_resetFile(&recordFile, tempStr);
		if (!recordFile)
		{
			exit(1);
		}

		fwrite(&episodeNum, 1, 1, recordFile);
		fwrite(levelName, 1, 10, recordFile);
		fwrite(&lvlFileNum, 1, 1, recordFile);
		fwrite(pItems, 1, 12, recordFile);
		fwrite(portPower, 1, 5, recordFile);
		fwrite(&levelSong, 1, 1, recordFile);

		lastMoveWait = 0;
	}

	twoPlayerLinked = FALSE;
	linkGunDirec = M_PI;

	JE_calcPurpleBall(1);
	JE_calcPurpleBall(2);

	damageRate = 2;  /*Normal Rate for Collision Damage*/

	chargeWait   = 5;
	chargeLevel  = 0;
	chargeMax    = 5;
	chargeGr     = 0;
	chargeGrWait = 3;

	portConfigChange = FALSE;

	makeMouseDelay = FALSE;

	exchangeCount = 0;

	/*Destruction Ratio*/
	totalEnemy = 0;
	enemyKilled = 0;

	/*InputDevices*/
	if (twoPlayerMode && !isNetworkGame)
	{
		playerDevice1 = inputDevice1;
		playerDevice2 = inputDevice2;
	} else {
		playerDevice1 = 0;
		playerDevice2 = 0;
	}

	astralDuration = 0;

	superArcadePowerUp = 1;

	yourInGameMenuRequest = FALSE;

	constantLastX = -1;

	playerStillExploding = 0;
	playerStillExploding2 = 0;

	if (isNetworkGame)
	{
		/* TODO */
	}

	if (isNetworkGame)
	{
		JE_loadItemDat();
	}

	memset(enemyAvail,       1, sizeof(enemyAvail));
	memset(explodeAvail,     0, sizeof(explodeAvail));
	memset(enemyShotAvail,   1, sizeof(enemyShotAvail));

	/*Initialize Shots*/
	memset(playerShotData,   0, sizeof(playerShotData));
	memset(shotAvail,        0, sizeof(shotAvail));
	memset(shotMultiPos,     0, sizeof(shotMultiPos));
	memset(shotRepeat,       1, sizeof(shotRepeat));

	memset(button,           0, sizeof(button));

	memset(REXavail,         0, sizeof(REXavail));
	memset(REXdat,           0, sizeof(REXdat));
	memset(globalFlags,      0, sizeof(globalFlags));

	/* --- Clear Sound Queue --- */
	memset(soundQueue,       0, sizeof(soundQueue));
	soundQueue[3] = V_GOOD_LUCK;

	memset(enemyShapeTables, 0, sizeof(enemyShapeTables));
	memset(enemy,            0, sizeof(enemy));

	memset(SFCurrentcode,    0, sizeof(SFCurrentcode));
	memset(SFExecuted,       0, sizeof(SFExecuted));

	zinglonDuration = 0;
	specialWait = 0;
	nextSpecialWait = 0;
	optionAttachmentMove  = 0;    /*Launch the Attachments!*/
	optionAttachmentLinked = TRUE;

	editShip1 = FALSE;
	editShip2 = FALSE;

	memset(smoothies, 0, sizeof(smoothies));

	levelTimer = FALSE;
	randomExplosions = FALSE;

	lastSP = 0;
	memset(SPZ, 0, sizeof(SPZ));

	returnActive = FALSE;

	galagaShotFreq = 0;

	if (galagaMode)
	{
		difficultyLevel = 2;
	}
	galagaLife = 10000;

	JE_drawOptionLevel();

	BKwrap2 = &megaData1->mainmap[0][0];
	BKwrap2to = &megaData1->mainmap[0][0];

levelloop :

	if (isNetworkGame)
	{
		smoothies[8] = FALSE;
		smoothies[5] = FALSE;
	} else {
		starShowVGASpecialCode = smoothies[8] + (smoothies [5] << 1);
	}

	/*Background Wrapping*/
	if (mapy2Pos <= BKwrap2)
	{
		mapy2Pos = BKwrap2to;
	}


	allPlayersGone = !playerAlive &&
	                 (!playerAliveB || !twoPlayerMode) &&
	                 ((portPower[0] == 1 && playerStillExploding == 0) || (!onePlayerAction && !twoPlayerMode)) &&
	                 ((portPower[1] == 1 && playerStillExploding2 == 0) || !twoPlayerMode);


	/*-----MUSIC FADE------*/
	if (musicFade)
	{
		if (tempVolume > 10)
		{
			tempVolume--;
			JE_setVol(tempVolume, fxVolume);
		} else {
			musicFade = FALSE;
		}
	}

	if (!allPlayersGone && levelEnd > 0 && endLevel)
	{
		JE_playSong(10);
		musicFade = FALSE;
	} else {
		if (!playing && musicActive && firstGameOver)
		{
			JE_playSong(levelSong);
			playing = TRUE;
		}
	}



	if (!endLevel)
	{    /*MAIN DRAWING IS STOPPED STARTING HERE*/
		/* TODO */
	}    /*MAIN DRAWING IS STOPPED ENDING   HERE*/

	/*---------------------------EVENTS-------------------------*/
	while (eventRec[eventLoc].eventTime <= curLoc && eventLoc <= maxEvent)
	{
		JE_eventSystem();
	}

	if (isNetworkGame && reallyEndLevel)
	{
		/* TODO goto startlevel;*/
	}


	/* SMOOTHIES! */
	JE_checkSmoothies();
	if (anySmoothies)
	{
		memcpy(VGAScreen->pixels, smoothiesScreen, sizeof(smoothiesSeg));
	}

	/* --- BACKGROUNDS --- */
	/* --- BACKGROUND 1 --- */

	if (forceEvents && !backMove)
	{
		curLoc++;
	}

	if (map1yDelayMax > 1 && backMove < 2)
	{
		if (map1yDelay == 1)
		{
			backMove = 1;
		} else {
			backMove = 0;
		}
	}

	/*Draw background*/
	if (astralDuration == 0)
	{
		/* BP is used by all backgrounds */

		s = (unsigned char *)tempScreenSeg->pixels;

		/* Offset for top */
		s += 11 * 24;
		s += mapxPos;

		/* Map location number in BP */
		bp = mapyPos;
		bp += mapxbpPos;

		if (backPos)
		{
			/* --- BACKGROUND 1 TOP --- */
			for (i = 12; i; i--)
			{
				
				/* move to previous map X location */
				bp--;
				
				src = *bp;
				src += (28 - backPos) * 24;
				
				for (j = backPos; j; j--)
				{
					memcpy(s, src, 24);

					s += tempScreenSeg->w;
					src += 24;
				}
				
				s -= backPos * tempScreenSeg->w + 24;
			}
			
			s += 24 * 11;
			s += backPos * tempScreenSeg->w + 24;
			
			/* Increment Map Location for next line */
			bp += 14 - 2;   /* (Map Width) */
			
		}
		bp += 14;   /* (Map Width) */

		/* --- BACKGROUND 1 CENTER --- */

		/* Outer loop - Screen 6 lines high */
		for (i = 6; i; i--)
		{
			for (j = 12; j; j--)
			{
				/* move to previous map X location */
				bp--;
				src = *bp;
				
				for (k = 0; k < 28; k++)
				{
					memcpy(s, src, 24);

					s += tempScreenSeg->w;
					src += 24;
				}

				s -= tempScreenSeg->w * 28 + 24;
			}
			
			/* Increment Map Location for next line */
			bp += 14 + 14 - 2;  /* (Map Width) */
			
			s += tempScreenSeg->w * 28 + 24 * 12;
		}

		if (backPos <= 15)
		{
			/* --- BACKGROUND 1 BOTTOM --- */
			for (i = 12; i; i--)
			{
				/* move to previous map X location */
				bp--;
				src = *bp;

				for (j = 15 - backPos + 1; j; j--)
				{
					memcpy(s, src, 24);

					s += tempScreenSeg->w;
					src += 24;
				}
				
				s -= (15 - backPos + 1) * tempScreenSeg->w + 24;
			}
		}
	} else {
		JE_clr256();
	}

	/*Set Movement of background 1*/
	if (--map1yDelay == 0)
	{
		map1yDelay = map1yDelayMax;

		curLoc += backMove;

		backPos += backMove;

		if (backPos > 27)
		{
			backPos -= 28;
			mapy--;
			mapyPos -= 14;  /*Map Width*/
		}
	}

	/*---------------------------STARS--------------------------*/
	/* DRAWSTARS */
	if (starActive || astralDuration > 0)
	{
		/* TODO */
	}

	if (processorType > 1 && smoothies[4])
	{
		JE_smoothies3();
	}

	/*-----------------------BACKGROUNDS------------------------*/
	/*-----------------------BACKGROUND 2------------------------*/
	if (background2over == 3)
	{
		JE_drawBackground2();
		background2 = TRUE;
	}

	if (background2over == 0)
	{
		if (!(smoothies[1] && processorType < 4) && !(smoothies[0] && processorType == 3))
		{
			if (wild && !background2notTransparent)
			{
				JE_superBackground2();
			} else {
				JE_drawBackground2();
			}
		}
	}

	if (smoothies[0] && processorType > 2 && SDAT[0] == 0)
	{
		JE_smoothies1();
	}
	if (smoothies[1] && processorType > 2)
	{
		JE_smoothies2();
	}

	/*-----------------------Ground Enemy------------------------*/
	/* TODO */

	if (smoothies[0] && processorType > 2 && SDAT[0] > 0)
	{
		JE_smoothies1();
	}

	if (superWild)
	{
		neat += 3;
		JE_darkenBackground(neat);
	}

	/*-----------------------BACKGROUNDS------------------------*/
	/*-----------------------BACKGROUND 2------------------------*/
	if (!(smoothies[1] && processorType < 4) &&
	    !(smoothies[0] && processorType == 3))
	{
		if (background2over == 1)
		{
			if (wild && !background2notTransparent)
			{
				JE_superBackground2();
			} else {
				JE_drawBackground2();
			}
		}
	}

	if (superWild)
	{
		neat++;
		JE_darkenBackground(neat);
	}

	if (background3over == 2)
	{
		JE_drawBackground3();
	}

	/* New Enemy */
	if (enemiesActive && rand() % 100 > levelEnemyFrequency)
	{
		/* TODO */
	}

	if (processorType > 1 && smoothies[2])
	{
		JE_smoothies3();
	}
	if (processorType > 1 && smoothies[3])
	{
		JE_smoothies4();
	}

	/* Draw Sky Enemy */
	if (!skyEnemyOverAll)
	{
		/* TODO */
	}

	if (background3over == 0)
	{
		JE_drawBackground3();
	}

	/* Draw Top Enemy */
	if (!topEnemyOver)
	{
		/* TODO */
	}

	/* Player Shot Images */
	for (z = 0; z < MAX_PWEAPON; z++)
	{
		if (shotAvail[z] != 0)
		{
			shotAvail[z]--;
			if (z != MAX_PWEAPON - 1)
			{

				playerShotData[z].shotXM += playerShotData[z].shotXC;
				playerShotData[z].shotX += playerShotData[z].shotXM;
				tempI4 = playerShotData[z].shotXM;
	
				if (playerShotData[z].shotXM > 100)
				{
					if (playerShotData[z].shotXM == 101)
					{
						playerShotData[z].shotX -= 101;
						playerShotData[z].shotX += PXChange;
						playerShotData[z].shotY += PYChange;
					} else {
						playerShotData[z].shotX -= 120;
						playerShotData[z].shotX += PXChange;
					}
				}
	
				playerShotData[z].shotYM += playerShotData[z].shotYC;
				playerShotData[z].shotY += playerShotData[z].shotYM;
	
				if (playerShotData[z].shotXM > 100)
				{
					playerShotData[z].shotY -= 120;
					playerShotData[z].shotY += PYChange;
				}
	
				if (playerShotData[z].shotComplicated != 0)
				{
					playerShotData[z].shotDevX += playerShotData[z].shotDirX;
					playerShotData[z].shotX += playerShotData[z].shotDevX;
					
					if (abs(playerShotData[z].shotDevX) == playerShotData[z].shotCirSizeX)
					{
						playerShotData[z].shotDirX = -playerShotData[z].shotDirX;
					}
					
					playerShotData[z].shotDevY += playerShotData[z].shotDirY;
					playerShotData[z].shotY += playerShotData[z].shotDevY;
					
					if (abs(playerShotData[z].shotDevY) == playerShotData[z].shotCirSizeY)
					{
						playerShotData[z].shotDirY = -playerShotData[z].shotDirY;
					}
					/*Double Speed Circle Shots - add a second copy of above loop*/
				}
				
				tempShotX = playerShotData[z].shotX;
				tempShotY = playerShotData[z].shotY;
				
				if (playerShotData[z].shotX < -34 || playerShotData[z].shotX > 290 || 
				    playerShotData[z].shotY < -15 || playerShotData[z].shotY > 190)
				{
					shotAvail[z] = 0;
					goto drawplayershotloopend;
				}
					
				if (playerShotData[z].shotTrail != 255)
				{
					if (playerShotData[z].shotTrail == 98)
					{
						/* TODO JE_setupExplosion(playerShotData[z].shotX - playerShotData[z].shotXM, playerShotData[z].shotY - playerShotData[z].shotYM, playerShotData[z].shotTrail);*/
					} else {
						/* TODO JE_setupExplosion(playerShotData[z].shotX, playerShotData[z].shotY, playerShotData[z].shotTrail);*/
					}
				}

				if (playerShotData[z].aimAtEnemy != 0)
				{
					if (--playerShotData[z].aimDelay == 0) {
						playerShotData[z].aimDelay = playerShotData[z].aimDelayMax;

						if (enemyAvail[playerShotData[z].aimAtEnemy] != 1)
						{
							if (playerShotData[z].shotX < enemy[playerShotData[z].aimAtEnemy].ex)
							{
								playerShotData[z].shotXM++;
							} else {
								playerShotData[z].shotXM--;
							}
							if (playerShotData[z].shotY < enemy[playerShotData[z].aimAtEnemy].ey)
							{
								playerShotData[z].shotYM++;
							} else {
								playerShotData[z].shotYM--;
							}
						} else {
							if (playerShotData[z].shotXM > 0)
							{
								playerShotData[z].shotXM++;
							} else {
								playerShotData[z].shotXM--;
							}
						}
					}
				}
				
				tempw = playerShotData[z].shotGR + playerShotData[z].shotAni;
				if (++playerShotData[z].shotAni == playerShotData[z].shotAniMax)
				{
					playerShotData[z].shotAni = 0;
				}
				
				tempI2 = playerShotData[z].shotDmg;
				temp2 = playerShotData[z].shotBlastFilter;
				chain = playerShotData[z].chainReaction;
				playerNum = playerShotData[z].playerNumber;
				
				tempSpecial = tempw > 60000;

				if (tempSpecial)
				{
					JE_newDrawCShapeTrickNum(OPTION_SHAPES, tempw - 60000 - 1, tempShotX+1, tempShotY);
					tempX2 = shapeX[OPTION_SHAPES][tempw - 60000 - 1] >> 1;
					tempY2 = shapeY[OPTION_SHAPES][tempw - 60000 - 1] >> 1;
				} else {
					if (tempw > 1000)
					{
						/* TODO JE_doSP(tempShotX+1 + 6, tempShotY + 6, 5, 3, (tempw / 1000) << 4);*/
						tempw = tempw % 1000;
					}
					if (tempw > 500)
					{
						if (background2 && tempShotY + shadowyDist < 190 && tempI4 < 100)
						{
							JE_drawShape2Shadow(tempShotX+1, tempShotY + shadowyDist, tempw - 500, shapesW2);
						}
						JE_drawShape2(tempShotX+1, tempShotY, tempw - 500, shapesW2);
					} else {
						if (background2 && tempShotY + shadowyDist < 190 && tempI4 < 100)
						{
							JE_drawShape2Shadow (tempShotX+1, tempShotY + shadowyDist, tempw, shapesC1);
						}
						JE_drawShape2(tempShotX+1, tempShotY, tempw, shapesC1);
					}
				}

			}
				
			/* TODO */
				
drawplayershotloopend:
			;
		}
	}

	/* Player movement indicators for shots that track your ship */
	lastPXShotMove = PX;
	lastPYShotMove = PY;

	/*=================================*/
	/*=======Collisions Detection======*/
	/*=================================*/

	if (playerAlive && !endLevel)
	{
		JE_playerCollide(&PX, &PY, &lastTurn, &lastTurn2, &score, &armorLevel, &shield, &playerAlive,
		                 &playerStillExploding, 1, playerInvulnerable1);
    }

	if (twoPlayerMode && playerAliveB && !endLevel)
		JE_playerCollide(&PXB, &PYB, &lastTurnB, &lastTurn2B, &score2, &armorLevel2, &shield2, &playerAliveB,
		                 &playerStillExploding2, 2, playerInvulnerable2);

	if (firstGameOver)
	{
		JE_mainGamePlayerFunctions();      /*--------PLAYER DRAW+MOVEMENT---------*/
	}

	/*Network Check #1*/
	netSuccess = FALSE;

	if (!endLevel)
	{    /*MAIN DRAWING IS STOPPED STARTING HERE*/

		/* Draw Enemy Shots */
		/* TODO */
	}

	if (background3over == 1)
	{
		JE_drawBackground3();
	}

	/* Draw Top Enemy */
	if (topEnemyOver)
	{
		/* TODO */
	}

	/* Draw Sky Enemy */
	if (skyEnemyOverAll)
	{
		/* TODO */
	}

	/*-------------------------- Sequenced Explosions -------------------------*/
	enemyStillExploding = FALSE;
	for (tempREX = 0; tempREX < 20; tempREX++)
	{
		if (REXavail[tempREX] != 0)
		{
			enemyStillExploding = TRUE;
			if (REXdat[tempREX].delay > 0)
			{
				REXdat[tempREX].delay--;
			} else {
				REXdat[tempREX].ey += backMove2 + 1;
				tempX = REXdat[tempREX].ex + (rand() % 24) - 12;
				tempY = REXdat[tempREX].ey + (rand() % 27) - 24;
				if (REXdat[tempREX].big)
				{
					JE_setupExplosionLarge(FALSE, 2, tempX, tempY);
					if (REXavail[tempREX] == 1 || rand() % 5 == 1)
					{
						soundQueue[7] = 11;
					} else {
						soundQueue[6] = 9;
					}
					REXdat[tempREX].delay = 4 + (rand() % 3);
				} else {
					JE_setupExplosion(tempX, tempY, 1);
					soundQueue[5] = 4;
					REXdat[tempREX].delay = 3;
				}
				REXavail[tempREX]--;
			}
		}
	}

	/*---------------------------- Draw Explosions ----------------------------*/
	/* TODO	*/

	if (!portConfigChange)
	{
		portConfigDone = TRUE;
	}


	/*-----------------------BACKGROUNDS------------------------*/
	/*-----------------------BACKGROUND 2------------------------*/
	if (!(smoothies[1] && processorType < 4) &&
	    !(smoothies[0] && processorType == 3))
	{
		if (background2over == 2)
		{
			if (wild && !background2notTransparent)
			{
				JE_superBackground2();
			} else {
				JE_drawBackground2();
			}
		}
	}

	/*-------------------------Warning---------------------------*/
	if ((playerAlive && armorLevel < 6) ||
	    (twoPlayerMode && !galagaMode && playerAliveB && armorLevel2 < 6))
	{
		/* TODO */
	}

	/*------- Random Explosions --------*/
	if (randomExplosions)
	{
		if (rand() % 10 == 1)
		{
			JE_setupExplosionLarge(FALSE, 20, rand() % 280, rand() % 180);
		}
	}


	/*=================================*/
	/*=======The Sound Routine=========*/
	/*=================================*/
	/* TODO */

}

/* --- Load Level/Map Data --- */
void JE_loadMap( void )
{

	FILE *lvlFile, *shpFile;
/*	FILE *tempFile;*/ /*Extract map file from LVL file*/


	JE_char char_mapFile, char_shapeFile;

	JE_DanCShape shape;
	JE_boolean shapeBlank;

	
	FILE *f;
/*	FILE *f2;*/
	JE_char k2, k3;
	JE_word x, y;
	JE_integer yy, z, a, b;
	JE_word mapSh[3][128]; /* [1..3, 0..127] */
	JE_byte *ref[3][128]; /* [1..3, 0..127] */
	char s[256];
	JE_byte col, planets, shade;


	JE_word yLoc;
	JE_shortint yChg;

	JE_byte mapBuf[15 * 600]; /* [1..15 * 600] */
	JE_word bufLoc;

	char buffer[256];
	int i;
	unsigned char pic_buffer[320*200]; /* screen buffer, 8-bit specific */
	unsigned char *vga, *pic, *vga2; /* screen pointer, 8-bit specific */

	lastCubeMax = cubeMax;

	/*Defaults*/
	songBuy = DEFAULT_SONG_BUY;  /*Item Screen default song*/
	
	if (loadTitleScreen || playDemo)
	{
		JE_openingAnim();
		JE_titleScreen(TRUE);
		loadTitleScreen = FALSE;
	}
	
	/* Load LEVELS.DAT - Section = MAINLEVEL */
	saveLevel = mainLevel;
	
new_game:
	galagaMode  = FALSE;
	useLastBank = FALSE;
	extraGame   = FALSE;
	haltGame = FALSE;
	if (loadTitleScreen)
	{
		JE_openingAnim();
		JE_titleScreen(TRUE);
		loadTitleScreen = FALSE;
	}
	
	gameLoaded = FALSE;
	
	
	first = TRUE;

	if (!playDemo && !loadDestruct)
	{
		do
		{
			JE_resetFile(&lvlFile, macroFile);

			x = 0;
			jumpSection = FALSE;
			loadLevelOk = FALSE;

			/* Seek Section # Mainlevel */
			while (x < mainLevel)
			{
				JE_readCryptLn(lvlFile, s);
				if (s[0] == '*')
				{
					x++;
					s[0] = ' ';
				}
			}
			
			ESCPressed = FALSE;

			do
			{

				if (gameLoaded)
				{
					if (mainLevel == 0)
					{
						loadTitleScreen = TRUE;
					}
					fclose(lvlFile);
					goto new_game;
				}

				sprintf(s, " ");
				JE_readCryptLn(lvlFile, s);

				switch (s[0])
				{
					case ']':
						switch (s[1])
						{
							case 'A':
								/* TODO JE_playAnim("TYREND.ANM", 1, TRUE, 7);*/
								break;

							case 'G':
								mapOrigin = atoi(strnztcpy(buffer, s + 4, 2));
								mapPNum   = atoi(strnztcpy(buffer, s + 7, 1));
								for (i = 0; i < mapPNum; i++)
								{
									mapPlanet[i] = atoi(strnztcpy(buffer, s + 1 + (i + 1) * 8, 2));
									mapSection[i] = atoi(strnztcpy(buffer, s + 4 + (i + 1) * 8, 3));
								}
								break;

							case '?':
								temp = atoi(strnztcpy(buffer, s + 4, 2));
								for (i = 0; i < temp; i++)
								{
									cubeList[i] = atoi(strnztcpy(buffer, s + 3 + (i + 1) * 4, 3));
								}
								if (cubeMax > temp)
								{
									cubeMax = temp;
								}
								break;

							case '!':
								cubeMax = atoi(strnztcpy(buffer, s + 4, 2));    /*Auto set CubeMax*/
								break;
							case '+':
								temp = atoi(strnztcpy(buffer, s + 4, 2));
								cubeMax += temp;
								if (cubeMax > 4)
								{
									cubeMax = 4;
								}
								break;

							case 'g':
								galagaMode = TRUE;   /*GALAGA mode*/
								memcpy(&pItemsPlayer2, &pItems, sizeof(pItemsPlayer2));
								pItemsPlayer2[1] = 15; /*Player 2 starts with 15 - MultiCannon and 2 single shot options*/
								pItemsPlayer2[3] = 0;
								pItemsPlayer2[4] = 0;
								break;

							case 'x':
								extraGame = TRUE;
								break;

							case 'e': /*ENGAGE mode*/
								doNotSaveBackup = TRUE;
								constantDie = FALSE;
								onePlayerAction = TRUE;
								superTyrian = TRUE;
								twoPlayerMode = FALSE;

								score = 0;

								portPower[0] = 3;
								portPower[1] = 0;
								pItems[11] = 13;
								pItems[ 0] = 39;
								pItems[ 2] = 255;

								pItems[1] = 0; /*Normally 0 - Rear Weapon*/
								pItems[3] = 0;
								pItems[4] = 0;
								pItems[5] = 2;
								pItems[6] = 2;
								pItems[7] = 1;
								pItems[9] = 4;
								pItems[10] = 0; /*Secret Weapons*/
								break;

							case 'J':
								temp = atoi(strnztcpy(buffer, s + 3, 3));
								mainLevel = temp;
								jumpSection = TRUE;
								break;
							case '2':
								temp = atoi(strnztcpy(buffer, s + 3, 3));
								if (twoPlayerMode || onePlayerAction)
								{
									mainLevel = temp;
									jumpSection = TRUE;
								}
								break;
							case 'w':
								temp = atoi(strnztcpy(buffer, s + 3, 3));   /*Allowed to go to Time War?*/
								if (pItems[11] == 13)
								{
									mainLevel = temp;
									jumpSection = TRUE;
								}
								break;
							case 't':
								temp = atoi(strnztcpy(buffer, s + 3, 3));
								if (levelTimer && levelTimerCountdown == 0)
								{
									mainLevel = temp;
									jumpSection = TRUE;
								}
								break;
							case 'l':
								temp = atoi(strnztcpy(buffer, s + 3, 3));
								if (!playerAlive || (twoPlayerMode && !playerAliveB))
								{
									mainLevel = temp;
									jumpSection = TRUE;
								}
								break;
							case 's':
								saveLevel = mainLevel;
								break; /*store savepoint*/
							case 'b':
								if (twoPlayerMode)
								{
									temp = 22;
								} else {
									temp = 11;
								}
								/* TODO JE_saveGame(11, "LAST LEVEL    ");*/
								break;

							case 'i':
								temp = atoi(strnztcpy(buffer, s + 3, 3));
								songBuy = temp;
								break;
							case 'I': /*Load Items Available Information*/

								memset(&itemAvail, 0, sizeof(itemAvail));

								for (temp = 0; temp < 9; temp++)
								{
									JE_readCryptLn(lvlFile, s);

									strcat(strcpy(s, s + 8), " ");
									temp2 = 0;
									/* TODO while (JE_getNumber(s, itemAvail[temp][temp2])
									{
										temp2++;
									}*/

									itemAvailMax[temp] = temp2;
								}

								JE_itemScreen();
								break;

							case 'L':
								nextLevel = atoi(strnztcpy(buffer, s + 9, 3));
								strnztcpy(levelName, s + 13, 9);
								levelSong = atoi(strnztcpy(buffer, s + 22, 2));
								if (nextLevel == 0)
								{
									nextLevel = mainLevel + 1;
								}
								lvlFileNum = atoi(strnztcpy(buffer, s + 25, 2));
								loadLevelOk = TRUE;
								bonusLevelCurrent = (strlen(s) > 28) & (s[28] == '$');
								normalBonusLevelCurrent = (strlen(s) > 27) & (s[27] == '$');
								gameJustLoaded = FALSE;
								break;

							case '@':
								useLastBank = !useLastBank;
								break;

							case 'Q':
								ESCPressed = FALSE;
								temp = secretHint + (rand() % 3) * 3;

								if (twoPlayerMode)
								{
									sprintf(levelWarningText[0], "%s %d", miscText[40], score);
									sprintf(levelWarningText[1], "%s %d", miscText[41], score2);
									strcpy(levelWarningText[2], "");
									levelWarningLines = 3;
								} else {
									sprintf(levelWarningText[0], "%s %d", miscText[37], JE_totalScore(score, pItems));
									strcpy(levelWarningText[1], "");
									levelWarningLines = 2;
								}

								for (x = 0; x < temp - 1; x++)
								{
									do
									{
										JE_readCryptLn(lvlFile, s);
									} while (s[0] != '#');
								}

								do
								{
									JE_readCryptLn(lvlFile, s);
									strcpy(levelWarningText[levelWarningLines], s);
									levelWarningLines++;
								} while (s[0] != '#');
								levelWarningLines--;

								JE_wipeKey();
								frameCountMax = 4;
								if (!constantPlay)
								{
									JE_displayText();
								}

								JE_fadeBlack(15);

								tempb = JE_nextEpisode();

								if (jumpBackToEpisode1 && !twoPlayerMode)
								{

									JE_loadPic(1, FALSE);
									JE_clr256();

									if (superTyrian)
									{
										if (initialDifficulty == 8)
										{
											superArcadeMode = SA + 1;
										} else {
											superArcadeMode = 1;
										}

										jumpSection = TRUE;
										loadTitleScreen = TRUE;
									}

									if (superArcadeMode < SA + 2)
									{
										if (SANextShip[superArcadeMode] == 9)
										{
											sprintf(buffer, "Or play... %s", specialName[7]);
											JE_dString(80, 180, buffer, SMALL_FONT_SHAPES);
										}

										if (SANextShip[superArcadeMode] != 9)
										{
											JE_dString(JE_fontCenter(superShips[0], FONT_SHAPES), 30, superShips[0], FONT_SHAPES);
											JE_dString(JE_fontCenter(superShips[SANextShip[superArcadeMode]], SMALL_FONT_SHAPES), 100, superShips[SANextShip[superArcadeMode]], SMALL_FONT_SHAPES);
										} else {
											sprintf(buffer, "%s %s", miscTextB[4], pName[0]);
											JE_dString(JE_fontCenter(buffer, FONT_SHAPES), 100, buffer, FONT_SHAPES);
										}

										if (SANextShip[superArcadeMode] < 7)
										{
											JE_drawShape2x2(148, 70, ships[SAShip[SANextShip[superArcadeMode]]].shipgraphic, shapes9);
										} else {
											if (SANextShip[superArcadeMode] == 7)
											{
												trentWin = TRUE;
											}
										}

										sprintf(buffer, "Type %s at Title", specialName[SANextShip[superArcadeMode]-1]);
										JE_dString(JE_fontCenter(buffer, SMALL_FONT_SHAPES), 160, buffer, SMALL_FONT_SHAPES);
										JE_showVGA();

										JE_fadeColor(50);
										if (!constantPlay)
										{
											while (!JE_anyButton());
										}
									}

									jumpSection = TRUE;
									if (isNetworkGame)
									{
										JE_readTextSync();
									}
									if (superTyrian)
									{
										JE_fadeBlack(10);
									}
								}
								break;

							case 'P':
								if (!constantPlay)
								{
									tempX = atoi(strnztcpy(buffer, s + 3, 3));
									if (tempX > 900)
									{
										memcpy(&colors, &palettes[pcxpal[tempX-1 - 900]], sizeof(colors));
										JE_clr256();
										JE_showVGA();
										JE_fadeColor(1);
									} else {
										if (tempX == 0)
										{
											/* TODO JE_loadPcx("TSHP2.PCX", FALSE);*/
										} else {
											JE_loadPic(tempX, FALSE);
										}
										JE_showVGA();
										JE_fadeColor(10);
									}
								}
								break;

							case 'U':
								if (!constantPlay)
								{
									/* TODO JE_setNetByte(0);*/
									memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
									
									tempX = atoi(strnztcpy(buffer, s + 3, 3));
									JE_loadPic(tempX, FALSE);
									memcpy(pic_buffer, VGAScreen->pixels, sizeof(pic_buffer));
									
									service_SDL_events(TRUE);
									for (z = 0; z <= 199; z++)
									{
										service_SDL_events(FALSE);
										if (!newkey && !ESCPressed)
										{
											vga = VGAScreen->pixels;
											vga2 = VGAScreen2Seg;
											pic = pic_buffer + (199 - z) * 320;

											setdelay(1); /* attempting to emulate JE_waitRetrace();*/
											for (y = 0; y < 199; y++)
											{
												if (y <= z)
												{
													memcpy(vga, pic, 320);
													pic += 320;
												} else {
													memcpy(vga, vga2, 320);
													vga2 += 320;
												}
												vga += VGAScreen->w;
											}
											JE_showVGA();
											wait_delay();

											if (isNetworkGame)
											{
												/* TODO JE_updateStream();*/
												if (netQuit)
												{
													JE_tyrianHalt(5);
												}
											}
										}
									}
									memcpy(VGAScreen->pixels, pic_buffer, sizeof(pic_buffer));
								}
								break;

							case 'V':
								if (!constantPlay)
								{
									/* TODO JE_setNetByte(0);*/
									memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
									
									tempX = atoi(strnztcpy(buffer, s + 3, 3));
									JE_loadPic(tempX, FALSE);
									memcpy(pic_buffer, VGAScreen->pixels, sizeof(pic_buffer));
									
									service_SDL_events(TRUE);
									for (z = 0; z <= 199; z++)
									{
										service_SDL_events(FALSE);
										if (!newkey && !ESCPressed)
										{
											vga = VGAScreen->pixels;
											vga2 = VGAScreen2Seg;
											pic = pic_buffer;

											setdelay(1); /* attempting to emulate JE_waitRetrace();*/
											for (y = 0; y < 199; y++)
											{
												if (y <= 199 - z)
												{
													memcpy(vga, vga2, 320);
													vga2 += 320;
												} else {
													memcpy(vga, pic, 320);
													pic += 320;
												}
												vga += VGAScreen->w;
											}
											JE_showVGA();
											wait_delay();

											if (isNetworkGame)
											{
												/* TODO JE_updateStream();*/
												if (netQuit)
												{
													JE_tyrianHalt(5);
												}
											}
										}
									}
									memcpy(VGAScreen->pixels, pic_buffer, sizeof(pic_buffer));
								}
								break;

							case 'R':
								if (!constantPlay)
								{
									/* TODO JE_setNetByte(0);*/
									memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));

									tempX = atoi(strnztcpy(buffer, s + 3, 3));
									JE_loadPic(tempX, FALSE);
									memcpy(pic_buffer, VGAScreen->pixels, sizeof(pic_buffer));

									service_SDL_events(TRUE);
									for (z = 0; z <= 318; z++)
									{
										service_SDL_events(FALSE);
										if (!newkey && !ESCPressed)
										{
											vga = VGAScreen->pixels;
											vga2 = VGAScreen2Seg;
											pic = pic_buffer;

											setdelay(1); /* attempting to emulate JE_waitRetrace();*/
											for(y = 0; y < 200; y++)
											{
												memcpy(vga, vga2 + z, 319 - z);
												vga += 320 - z;
												vga2 += 320;
												memcpy(vga, pic, z + 1);
												vga += z;
												pic += 320;
											}
											JE_showVGA();
											wait_delay();

											if (isNetworkGame)
											{
												/* TODO JE_UpdateStream();*/
												if (netQuit)
												{
													JE_tyrianHalt(5);
												}
											}
										}
									}
									memcpy(VGAScreen->pixels, pic_buffer, sizeof(pic_buffer));
								}
								break;

							case 'C':
								if (!isNetworkGame)
								{
									JE_fadeBlack(10);
								}
								JE_clr256();
								JE_showVGA();
								memcpy(colors, palettes[7], sizeof(colors));
								JE_updateColorsFast(&colors);
								break;

							case 'B':
								if (!isNetworkGame)
								{
									JE_fadeBlack(10);
								}
								break;
							case 'F':
								if (!isNetworkGame)
								{
									JE_fadeWhite(100);
									JE_fadeBlack(30);
								}
								JE_clr256();
								JE_showVGA();
								break;

							case 'W':
								if (!constantPlay)
								{
									if (!ESCPressed)
									{
										JE_wipeKey();
										warningCol = 14 * 16 + 5;
										warningColChange = 1;
										warningSoundDelay = 0;
										levelWarningDisplay = (s[2] == 'y');
										levelWarningLines = 0;
										frameCountMax = atoi(strnztcpy(buffer, s + 4, 2));
										setjasondelay2(6);
										warningRed = frameCountMax / 10;
										frameCountMax = frameCountMax % 10;

										do
										{
											JE_readCryptLn(lvlFile, s);

											if (s[0] != '#')
											{
												strcpy(levelWarningText[levelWarningLines], s);
												levelWarningLines++;
											}
										} while (!(s[0] == '#'));

										JE_displayText();
										newkey = FALSE;
									}
								}
								break;

							case 'H':
								if (initialDifficulty < 3)
								{
									mainLevel = atoi(strnztcpy(buffer, s + 4, 3));
									jumpSection = TRUE;
								}
								break;

							case 'h':
								if (initialDifficulty > 2)
								{
									JE_readCryptLn(lvlFile, s);
								}
								break;

							case 'S':
								if (isNetworkGame)
								{
									JE_readTextSync();
								}
								break;

							case 'n':
								ESCPressed = FALSE;
								break;

							/* TODO */

							case 'M':
								temp = atoi(strnztcpy(buffer, s + 3, 3));
								JE_playSong(temp);
								break;

							/* TODO */
						}
					break;
				}


			} while (!(loadLevelOk || jumpSection));


			fclose(lvlFile);

		} while (!loadLevelOk);
	}

	if (!loadDestruct)
	{

		if (playDemo)
		{

			difficultyLevel = 2;
			sprintf(tempStr, "DEMO.%d", playDemoNum);
			JE_resetFile(&recordFile, tempStr);

			bonusLevelCurrent = FALSE;

			temp = fgetc(recordFile);
			JE_initEpisode(temp);
			fread(levelName, 1, 10, recordFile); levelName[10] = '\0';
			lvlFileNum = fgetc(recordFile);
			fread(pItems, 1, 12, recordFile);
			fread(portPower, 1, 5, recordFile);
			levelSong = fgetc(recordFile);

			temp = fgetc(recordFile);
			temp2 = fgetc(recordFile);
			lastMoveWait = (temp << 8) | temp2;
			nextDemoOperation = fgetc(recordFile);

			firstEvent = TRUE;

			/*debuginfo('Demo loaded.');*/
		} else {
			JE_fadeColors(&colors, &black, 0, 255, 50);
		}


		JE_resetFile(&lvlFile, levelFile);
		fseek(lvlFile, lvlPos[(lvlFileNum-1) * 2], SEEK_SET);

		char_mapFile = fgetc(lvlFile);
		char_shapeFile = fgetc(lvlFile);
		fread(&mapx,  2, 1, lvlFile);
		fread(&mapx2, 2, 1, lvlFile);
		fread(&mapx3, 2, 1, lvlFile);

		fread(&levelEnemyMax, 2, 1, lvlFile);
		for (x = 0; x < levelEnemyMax; x++)
		{
			fread(&levelEnemy[x], 2, 1, lvlFile);
		}

		fread(&maxEvent, 2, 1, lvlFile);
		for (x = 0; x < maxEvent; x++)
		{
			fread(&eventRec[x].eventTime, sizeof(JE_word), 1, lvlFile);
			fread(&eventRec[x].eventType, sizeof(JE_byte), 1, lvlFile);
			fread(&eventRec[x].eventDat,  sizeof(JE_integer), 1, lvlFile);
			fread(&eventRec[x].eventDat2, sizeof(JE_integer), 1, lvlFile);
			fread(&eventRec[x].eventDat3, sizeof(JE_shortint), 1, lvlFile);
			fread(&eventRec[x].eventDat5, sizeof(JE_shortint), 1, lvlFile);
			fread(&eventRec[x].eventDat6, sizeof(JE_shortint), 1, lvlFile);
			fread(&eventRec[x].eventDat4, sizeof(JE_byte), 1, lvlFile);
		}
		eventRec[x].eventTime = 65500;  /*Not needed but just in case*/

		/*debuginfo('Level loaded.');*/

		/*debuginfo('Loading Map');*/

		/* MAP SHAPE LOOKUP TABLE - Each map is directly after level */
		fread(mapSh, sizeof(mapSh), 1, lvlFile);
		for (temp = 0; temp < 3; temp++)
		{
			for (temp2 = 0; temp2 < 128; temp2++)
			{
				mapSh[temp][temp2] = SDL_SwapBE16(mapSh[temp][temp2]);
			}
		}

		/* Read Shapes.DAT */
		sprintf(tempStr, "SHAPES%c.DAT", char_shapeFile);
		JE_resetFile(&shpFile, tempStr);

		for (z = 0; z < 600; z++)
		{
			shapeBlank = fgetc(shpFile);

			if (shapeBlank)
			{
				memset(shape, 0, sizeof(shape));
			} else {
				fread(shape, sizeof(shape), 1, shpFile);
			}

			/* Match 1 */
			for (x = 0; x <= 71; x++)
			{
				if (mapSh[0][x] == z+1)
				{
					memcpy(megaData1->shapes[x].sh, shape, sizeof(JE_DanCShape));

					ref[0][x] = (JE_byte *)megaData1->shapes[x].sh;
				}
			}

			/* Match 2 */
			for (x = 0; x <= 71; x++)
			{
				if (mapSh[1][x] == z+1)
				{
					if (x != 71 && !shapeBlank)
					{
						memcpy(megaData2->shapes[x].sh, shape, sizeof(JE_DanCShape));

						y = 1;
						for (yy = 0; yy < (24 * 28) >> 1; yy++)
						{
							if (shape[yy] == 0)
							{
								y = 0;
							}
						}

						megaData2->shapes[x].fill = y;
						ref[1][x] = (JE_byte *)megaData2->shapes[x].sh;
					} else {
						ref[1][x] = NULL;
					}
				}
			}

			/*Match 3*/
			for (x = 0; x <= 71; x++)
			{
				if (mapSh[2][x] == z+1)
				{
					if (x < 70 && !shapeBlank)
					{
						memcpy(megaData3->shapes[x].sh, shape, sizeof(JE_DanCShape));

						y = 1;
						for (yy = 0; yy < (24 * 28) >> 1; yy++)
						{
							if (shape[yy] == 0)
							{
								y = 0;
							}
						}

						megaData3->shapes[x].fill = y;
						ref[2][x] = (JE_byte *)megaData3->shapes[x].sh;
					} else {
						ref[2][x] = NULL;
					}
				}
			}
		}

		fclose(shpFile);

		fread(mapBuf, 1, 14 * 300, lvlFile);
		bufLoc = 0;              /* MAP NUMBER 1 */
		for (y = 0; y < 300; y++)
		{
			for (x = 0; x < 14; x++)
			{
				megaData1->mainmap[y][x] = ref[0][mapBuf[bufLoc]];
				bufLoc++;
			}
		}

		fread(mapBuf, 1, 14 * 600, lvlFile);
		bufLoc = 0;              /* MAP NUMBER 2 */
		for (y = 0; y < 600; y++)
		{
			for (x = 0; x < 14; x++)
			{
				megaData2->mainmap[y][x] = ref[1][mapBuf[bufLoc]];
				bufLoc++;
			}
		}

		fread(mapBuf, 1, 15 * 600, lvlFile);
		bufLoc = 0;              /* MAP NUMBER 3 */
		for (y = 0; y < 600; y++)
		{
			for (x = 0; x < 15; x++)
			{
				megaData3->mainmap[y][x] = ref[2][mapBuf[bufLoc]];
				bufLoc++;
			}
		}

		fclose(lvlFile);

		/* Note: The map data is automatically calculated with the correct mapsh
		value and then the pointer is calculated using the formula (MAPSH-1)*168.
		Then, we'll automatically add S2Ofs to get the exact offset location into
		the shape table! This makes it VERY FAST! */

		/*debuginfo('Map file done.');*/
		/* End of find loop for LEVEL??.DAT */
	} /*LoadDestruct?*/

}

void JE_titleScreen( JE_boolean animate )
{
	JE_boolean quit = 0;

	const int menunum = 7;
	JE_byte namego[SA + 2] = {0}; /* [1..SA+2] */
	JE_word waitForDemo;
	JE_byte menu = 0;
	JE_boolean redraw = TRUE,
	           fadeIn = FALSE,
	           first = TRUE;
	JE_char flash;
	JE_word z;

	JE_word temp; /* JE_byte temp; from varz.h will overflow in for loop */

	JE_initPlayerData();

	/*PlayCredits;*/

	JE_sortHighScores();

	if (haltGame)
	{
		JE_tyrianHalt(0);
	}

	tempScreenSeg = VGAScreen;

	joystickWaitMax = 80;
	joystickWait = 0;

	gameLoaded = FALSE;
	jumpSection = FALSE;

	/* If IsNetworkActive { TODO } else { */

	do
	{
		defaultBrightness = -3;

		/* Animate instead of quickly fading in */
		if (redraw)
		{
			if (currentSong != SONG_TITLE) JE_playSong(SONG_TITLE);
				
			menu = 0;
			redraw = FALSE;
			if (animate)
			{
				if (fadeIn)
				{
					JE_fadeBlack(10);
				}
				JE_loadPic(4, FALSE);

				memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));

				if (moveTyrianLogoUp)
				{
					temp = 62;
				} else {
					temp = 4;
				}

				JE_newDrawCShapeNum(PLANET_SHAPES, 146, 11, temp);


				memcpy(colors2, colors, sizeof(colors));
				for (temp = 256-16; temp < 256; temp++)
				{
					colors[temp].r = 0;
					colors[temp].g = 0;
					colors[temp].b = 0;
				}
				colors2[temp].r = 0;

				JE_showVGA();
				JE_fadeColor(10);

				fadeIn = FALSE;

				if (moveTyrianLogoUp)
				{
					for (temp = 61; temp >= 4; temp -= 2)
					{
						int i;

						setjasondelay(2);
						memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));

						JE_newDrawCShapeNum(PLANET_SHAPES, 146, 11, temp);

						JE_showVGA();
						wait_delay();
					}
				}
				moveTyrianLogoUp = FALSE;

				/* Draw Menu Text on Screen */
				for (temp = 0; temp < menunum; temp++)
				{
					tempX = 104+(temp)*13;
					if (temp == 4) /* OpenTyrian override */
					{
						tempY = JE_fontCenter(opentyrian_str, SMALL_FONT_SHAPES);

						JE_outTextAdjust(tempY-1,tempX-1,opentyrian_str,15,-10,SMALL_FONT_SHAPES,FALSE);
						JE_outTextAdjust(tempY+1,tempX+1,opentyrian_str,15,-10,SMALL_FONT_SHAPES,FALSE);
						JE_outTextAdjust(tempY+1,tempX-1,opentyrian_str,15,-10,SMALL_FONT_SHAPES,FALSE);
						JE_outTextAdjust(tempY-1,tempX+1,opentyrian_str,15,-10,SMALL_FONT_SHAPES,FALSE);
						JE_outTextAdjust(tempY,tempX,opentyrian_str,15,-3,SMALL_FONT_SHAPES,FALSE);
					} else {
						tempY = JE_fontCenter(menuText[temp],SMALL_FONT_SHAPES);

						JE_outTextAdjust(tempY-1,tempX-1,menuText[temp],15,-10,SMALL_FONT_SHAPES,FALSE);
						JE_outTextAdjust(tempY+1,tempX+1,menuText[temp],15,-10,SMALL_FONT_SHAPES,FALSE);
						JE_outTextAdjust(tempY+1,tempX-1,menuText[temp],15,-10,SMALL_FONT_SHAPES,FALSE);
						JE_outTextAdjust(tempY-1,tempX+1,menuText[temp],15,-10,SMALL_FONT_SHAPES,FALSE);
						JE_outTextAdjust(tempY,tempX,menuText[temp],15,-3,SMALL_FONT_SHAPES,FALSE);
					}
				}
				JE_showVGA();

				JE_fadeColors(&colors, &colors2, 0, 255, 20);

				memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
			}
		}

		memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));

		for (temp = 0; temp < menunum; temp++)
		{
			if (temp == 4) /* OpenTyrian override */
			{
				JE_outTextAdjust(JE_fontCenter(opentyrian_str, SMALL_FONT_SHAPES), 104+temp*13,
				                 opentyrian_str, 15, -3+((temp == menu) * 2), SMALL_FONT_SHAPES, FALSE);
			} else {
				JE_outTextAdjust(JE_fontCenter(menuText[temp], SMALL_FONT_SHAPES), 104+temp*13,
				                 menuText[temp], 15, -3+((temp == menu) * 2), SMALL_FONT_SHAPES, FALSE);
			}
		}

		JE_showVGA();

		first = FALSE;

		if (trentWin)
		{
			quit = TRUE;
			goto trentWinsGame;
		}

		waitForDemo = 2000;
		JE_textMenuWait(&waitForDemo, FALSE);

		/* TODO: Crapload of stuff */

		if (newkey)
		{
			switch (lastkey_sym)
			{
				case SDLK_UP:
					if (menu == 0)
					{
						menu = menunum-1;
					} else {
						menu--;
					}
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_DOWN:
					if (menu == menunum-1)
					{
						menu = 0;
					} else {
						menu++;
					}
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_RETURN:
					JE_playSampleNum(SELECT);
					switch (menu)
					{
						case 0: /* New game */
							JE_fadeBlack(10);
							if (JE_playerSelect())
							{
								if (netQuit)
								{
									JE_tyrianHalt(9);
								}
	
								if (JE_episodeSelect() && JE_difficultySelect())
								{
									gameLoaded = TRUE;
								} else {
									redraw = TRUE;
									fadeIn = TRUE;
								}
	
								initialDifficulty = difficultyLevel;
	
								if (onePlayerAction)
								{
									score = 0;
									pItems[11] = 8;
								} else {
									if (twoPlayerMode)
									{
										score = 0;
										score2 = 0;
										pItems[11] = 11;
										difficultyLevel++;
										inputDevice1 = 1;
										inputDevice2 = 2;
									} else {
										if (richMode)
										{
											score = 1000000;
										} else {
											switch (episodeNum)
											{
												case 1:
													score = 10000;
													break;
												case 2:
													score = 15000;
													break;
												case 3:
													score = 20000;
													break;
												case 4:
													score = 30000;
													break;
											}
										}
									}
								}
							}
							fadeIn = TRUE;
							break;
						case 1: /* Load game */
							JE_loadScreen();
							if (!gameLoaded)
							{
								redraw = TRUE;
							}
							fadeIn = TRUE;
							break;
						case 2: /* High scores */
							JE_highScoreScreen();
							fadeIn = TRUE;
							break;
						case 3: /* Instructions */
							JE_helpSystem(1);
							redraw = TRUE;
							fadeIn = TRUE;
							break;
						case 4: /* Ordering info, now OpenTyrian menu*/
							opentyrian_menu();
							redraw = TRUE;
							fadeIn = TRUE;
							break;
						case 5: /* Demo */
							JE_initPlayerData();
							playDemo = TRUE;
							if (playDemoNum++ > 4)
							{
								playDemoNum = 0;
							}
							break;
						case 6: /* Quit */
							quit = TRUE;
							break;
					}
					redraw = TRUE;
					break;
				case SDLK_ESCAPE:
					quit = TRUE;
					break;
				default:
					break;
			}
		}
	} while (!(quit || gameLoaded || jumpSection || playDemo || loadDestruct));

trentWinsGame:
	JE_fadeBlack(15);
	if (quit)
	{
		JE_tyrianHalt(0);
	}

	/* } (IsNetworkActive) */
}

void JE_openingAnim( void )
{
	/*JE_clr256();*/

	moveTyrianLogoUp = TRUE;

	if (!isNetworkGame && !stoppedDemo)
	{
		memcpy(colors, black, sizeof(colors));
		memset(black, 63, sizeof(black));
		JE_fadeColors(&colors, &black, 0, 255, 50);

		JE_loadPic(10, FALSE);
		JE_showVGA();

		JE_fadeColors(&black, &colors, 0, 255, 50);
		memset(black, 0, sizeof(black));

		setjasondelay(200);
		while(!(delaycount() == 0 || JE_anyButton()));

		JE_fadeBlack(15);

		JE_loadPic(12, FALSE);
		JE_showVGA();

		memcpy(colors, palettes[pcxpal[11]], sizeof(colors));
		JE_fadeColor(10);

		setjasondelay(200);
		while(!(delaycount() == 0 || JE_anyButton()));

		JE_fadeBlack(10);
	}
}

void JE_readTextSync( void )
{
	JE_clr256();
	JE_showVGA();
	JE_loadPic(1, TRUE);

	JE_barShade(3, 3, 316, 196);
	JE_barShade(1, 1, 318, 198);
	JE_dString(10, 160, "Waiting for other player.", SMALL_FONT_SHAPES);
	JE_showVGA();

	/* TODO JE_setNetByte(251);*/

	do
	{
		setjasondelay(2);

		/* TODO JE_updateStream();*/
		if (netQuit)
		{
			JE_tyrianHalt(5);
		}

		while (delaycount());

	} while (0 /* TODO */);
}


void JE_displayText( void )
{
	/* Display Warning Text */
	tempY = 55;
	if (warningRed)
	{
		tempY = 2;
	}
	for (temp = 0; temp < levelWarningLines; temp++)
	{
		if (!ESCPressed)
		{
			JE_outCharGlow(10, tempY, levelWarningText[temp]);

			if (haltGame)
			{
				JE_tyrianHalt(5);
			}

			tempY += 10;
		}
	}
	if (frameCountMax != 0)
	{
		frameCountMax = 6;
		temp = 1;
	} else {
		temp = 0;
	}
	textGlowFont = TINY_FONT;
	tempw = 184;
	if (warningRed)
	{
		tempw = 7 * 16 + 6;
	}

	JE_outCharGlow(JE_fontCenter(miscText[4], TINY_FONT), tempw, miscText[4]);

	do
	{
		if (levelWarningDisplay)
		{
			JE_updateWarning();
		}

		setjasondelay(1);

		/* TODO JE_setNetByte(0);*/
		/* TODO JE_updateStream();*/
		if (netQuit)
		{
			JE_tyrianHalt(5);
		}

		while (delaycount());

    } while (!(JE_anyButton() || (frameCountMax == 0 && temp == 1) || ESCPressed));
    levelWarningDisplay = FALSE;
}


JE_boolean JE_searchFor/*enemy*/( JE_byte PLType )
{
	JE_boolean tempb = FALSE;
	JE_byte temp;

	for (temp = 0; temp < 100; temp++)
	{
		if (enemyAvail[temp] == 0 && enemy[temp].linknum == PLType)
		{
			temp5 = temp + 1;
			if (galagaMode)
			{
				enemy[temp].evalue += enemy[temp].evalue;
			}
			tempb = TRUE;
		}
	}
	return tempb;
}

void JE_eventSystem( void )
{
	JE_boolean tempb;

	/* TODO */

	eventLoc++;
}


JE_boolean quikSave;
JE_byte oldMenu;
JE_boolean backFromHelp;
JE_byte lastSelect;
JE_integer lastDirection;
JE_byte skipMove;
JE_byte tempPowerLevel[7];
JE_boolean firstMenu9, paletteChanged;
JE_MenuChoiceType menuChoices;
JE_longint shipValue;
JE_word curX, curY, curWindow, selectX, selectY, tempX, tempY, tempAvail, x, y, textYPos;
JE_byte flashDelay;
JE_integer col, colC;
JE_byte curAvail, curSelectDat;
JE_byte lastCurSel;
JE_word mouseSetY;
JE_boolean firstRun;
JE_integer curMenu;
JE_byte curSel[MAX_MENU];
JE_byte curItemType, curItem, cursor;
JE_boolean buyActive, sellActive, sellViewActive, buyViewActive, /*flash,*/ purchase, cannotAfford, slotFull;
JE_boolean leftPower, rightPower, rightPowerAfford;

char cubeHdr[4][81];
char cubeText[4][90][CUBE_WIDTH];
char cubeHdr2[4][13];
JE_byte faceNum[4];
JE_word cubeMaxY[4];
JE_byte currentCube;
JE_boolean keyboardUsed;
JE_word faceX, faceY;

JE_byte currentFaceNum;

JE_longint JE_cashLeft( void )
{
	JE_longint templ;
	JE_byte x;
	JE_word itemNum;

	templ = score;
	itemNum = pItems[pItemButtonMap[curSel[1] - 2]];

	templ -= JE_getCost(curSel[1], itemNum);

	tempw = 0;

	switch (curSel[1])
	{
		case 3:
		case 4:
			tempW2 = weaponPort[itemNum].cost;
			for (x = 1; x < portPower[curSel[1] - 3]; x++)
			{
				tempw += tempW2 * x;
				templ -= tempw;
			}
			break;
	}

	return templ;
}

void JE_itemScreen( void )
{
	JE_loadCubes();

	JE_wipeKey();

	tempScreenSeg = VGAScreen;

	memcpy(menuChoices, menuChoicesDefault, sizeof(menuChoices));

	first = TRUE;
	refade = FALSE;

	joystickWaitMax = 10;
	joystickWait = 0;

	JE_playSong(songBuy);

	JE_loadPic(1, FALSE);

	newPal = 0;
	JE_showVGA();

	JE_updateColorsFast(&colors);

	col = 1;
	gameLoaded = FALSE;
	curItemType = 1;
	cursor = 1;
	curItem = 0;

	for (x = 0; x < MAX_MENU; x++)
	{
		curSel[x] = 1;
	}

	curMenu = 0;
	curX = 1;
	curY = 1;
	curWindow = 1;

	/* TODO */
	for (x = 0; x < 7; x++)
	{
		temp = pItemsBack2[pItemButtonMap[x]];
		temp2 = 0;

		for (y = 0; y < itemAvailMax[itemAvailMap[x]]; y++)
		{
			if (itemAvail[itemAvailMap[x]][y] == temp)
			{
				temp2 = 1;
			}
		}

		if (temp2 == 0)
		{
			itemAvailMax[itemAvailMap[x]]++;
			itemAvail[itemAvailMap[x]][itemAvailMax[itemAvailMap[x]]] = temp;
		}
	}

	memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));

	keyboardUsed = FALSE;
	firstMenu9 = FALSE;
	backFromHelp = FALSE;

item_screen_start:

	do
	{
		JE_getShipInfo();

		quit = FALSE;

		/* JE: If curMenu==1 and twoPlayerMode is on, then force move to menu 10 */
		if (curMenu == 0)
		{
			if (twoPlayerMode)
			{
				curMenu = 9;
			}
			if (isNetworkGame || onePlayerAction)
			{
				curMenu = 10;
			}
			if (superTyrian)
			{
				curMenu = 13;
			}
		}

		paletteChanged = FALSE;

		leftPower = FALSE;
		rightPower = FALSE;

		/* JE: Sort items in merchant inventory */
		for (x = 0; x < 9; x++)
		{
			if (itemAvailMax[x] > 1)
			{
				for (temp = 0; temp < itemAvailMax[x] - 1; temp++)
				{
					for (temp2 = temp+1; temp2 < itemAvailMax[x]; temp2++)
					{
						if (itemAvail[x][temp] == 0 || (itemAvail[x][temp] > itemAvail[x][temp2] && itemAvail[x][temp2] != 0))
						{
							temp3 = itemAvail[x][temp];
							itemAvail[x][temp] = itemAvail[x][temp2];
							itemAvail[x][temp2] = temp3;
						}
					}
				}
			}
		}

		if (curMenu != 8 || firstMenu9)
		{
			memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));
		}

		defaultBrightness = -3;

		/* JE: --- STEP I - Draw the menu --- */
		if (curMenu == 3)
		{
			planetAni = 0;
			keyboardUsed = FALSE;
			currentDotNum = 0;
			currentDotWait = 8;
			planetAniWait = 3;
			JE_updateNavScreen();
		}

		if (curMenu != 4)
		{
			JE_drawMenuHeader();
		}

		if ((curMenu >= 0 && curMenu <= 3) || (curMenu >= 9 || curMenu <= 13))
		{
			JE_drawMenuChoices();
		}

		if (skipMove > 0)
		{
			skipMove--;
		}

		if (curMenu == 0)
		{
			for (x = 1; x <= cubeMax; x++)
			{
				JE_newDrawCShapeDarkenNum(OPTION_SHAPES, 34, 190 + x*18 + 2, 37+1);
				JE_newDrawCShapeNum(OPTION_SHAPES, 34, 190 + x*18, 37);
			}
		}

		if (curMenu == 12)
		{
			for (temp = 1; temp <= 4; temp++)
			{
				JE_textShade(214, 34 + temp*24 - 8, joyButton[joyButtonAssign[temp]], 15, 2, DARKEN);
			}
		}

		if (curMenu == 6)
		{
			if (twoPlayerMode)
			{
				min = 13;
				max = 24;
			} else {
				min = 2;
				max = 13;
			}

			for (x = min; x <= max; x++)
			{
				if (x - min + 2 == curSel[curMenu])
				{
					temp2 = 15;
				} else {
					temp2 = 28;
				}

				if (x == max)
				{
					strcpy(tempStr, miscText[6]);
				} else {
					if (saveFiles[x-1].level == 0)
					{
						strcpy(tempStr, miscText[3]);
					} else {
						strcpy(tempStr, saveFiles[x-1].name);
					}
				}

				tempY = 38 + (x - min)*11;

				JE_textShade(163, tempY, tempStr, temp2 / 16, temp2 % 16 - 8, DARKEN);

				if (x - min + 2 == curSel[curMenu])
				{
					if (keyboardUsed)
					{
						JE_setMousePosition(610, 38 + (x - min) * 11);
					}
				}

				if (x < max)
				{
					if (x - min + 2 == curSel[curMenu])
					{
						temp2 = 252;
					} else {
						temp2 = 250;
					}

					if (saveFiles[x-1].level == 0)
					{
						strcpy(tempStr, "-----");
					} else {
						char buf[20];

						strcpy(tempStr, saveFiles[x-1].levelName);

						snprintf(buf, sizeof buf, "%s%d", miscTextB[1], saveFiles[x-1].episode);
						JE_textShade(297, tempY, buf, temp2 / 16, temp2 % 16 - 8, DARKEN);
					}

					JE_textShade(245, tempY, tempStr, temp2 / 16, temp2 % 16 - 8, DARKEN);
				}

				JE_drawMenuHeader();
			}
		}

		if (curMenu == 5)
		{
			for (x = 1; x < 11; x++)
			{
				if (x == curSel[curMenu])
				{
					temp2 = 15;
					if (keyboardUsed)
					{
						JE_setMousePosition(610, 38 + (x - 1)*12);
					}
				} else {
					temp2 = 28;
				}

				JE_textShade(166, 38 + (x - 1)*12, menuInt[curMenu][x], temp2 / 16, temp2 % 16 - 8, DARKEN);

				if (x < 9)
				{
					if (x == curSel[curMenu])
					{
						temp2 = 252;
					} else {
						temp2 = 250;
					}
					JE_textShade(236, 38 + (x - 1)*12, keyNames[keySettings[x-1]], temp2 / 16, temp2 % 16 - 8, DARKEN);
				}
			}

			menuChoices[5] = 10;
		}

		if (curMenu == 4)
		{
			while (curSel[4] < menuChoices[4] && JE_getCost(curSel[1], itemAvail[itemAvailMap[curSel[2]-1]][curSel[5]]) > score)
			{
				curSel[4] += lastDirection;
				if (curSel[4] < 1)
				{
					curSel[4] = menuChoices[4];
				}
				if (curSel[4] > menuChoices[4])
				{
					curSel[4] = 1;
				}
			}

			if (curSel[4] == menuChoices[4])
			{
				pItems[pItemButtonMap[curSel[1]-1]] = pItemsBack[pItemButtonMap[curSel[2]-1]];
			} else {
				pItems[pItemButtonMap[curSel[1]-1]] = itemAvail[itemAvailMap[curSel[1]-1]][curSel[4]-1];
			}

			if ((curSel[1] == 3 && curSel[4] < menuChoices[4]) || (curSel[1] == 4 && curSel[4] < menuChoices[4]-1))
			{
				if (curSel[1] == 3)
				{
					temp = portPower[0];
				} else {
					temp = portPower[1];
				}

				/* JE: Only needed if change */
				tempW3 = JE_getCost(curSel[1], itemAvail[itemAvailMap[curSel[1]-1]][curSel[5]-1]);

				leftPower  = portPower[curSel[1] - 2] > 1;
				rightPower = portPower[curSel[1] - 2] < 11;

				if (rightPower)
				{
					rightPowerAfford = JE_cashLeft() >= upgradeCost;
				}
			} else {
				leftPower = FALSE;
				rightPower = FALSE;
			}

			JE_dString(74 + JE_fontCenter(menuInt[1][curSel[1]], FONT_SHAPES), 10, menuInt[1][curSel[1]], FONT_SHAPES);
			temp2 = pItems[pItemButtonMap[curSel[1]]];

			for (tempW = 0; tempW < menuChoices[curMenu]; tempW++)
			{
				tempY = 40 + (tempW - 1) * 26;

				if (tempW < menuChoices[4])
				{
					tempW3 = JE_getCost(curSel[1], itemAvail[itemAvailMap[curSel[1]-1]][tempW]);
				} else {
					tempW3 = 0;
				}

				if (tempW > score)
				{
					temp4 = 4;
				} else {
					temp4 = 0;
				}

				temp = itemAvail[itemAvailMap[curSel[1]-1]][tempW];
				switch (curSel[1])
				{
					case 1:
						if (temp > 90)
						{
							snprintf(tempStr, sizeof tempStr, "Custom Ship %d", temp - 90);
							/*strcpy(tempStr, "Custom Ship " + JE_st(temp - 90));*/
						} else {
							strcpy(tempStr, ships[temp].name);
						}
						break;
					case 2:
					case 3:
						strcpy(tempStr, weaponPort[temp].name);
						break;
					case 4:
						strcpy(tempStr, shields[temp].name);
						break;
					case 5:
						strcpy(tempStr, powerSys[temp].name);
						break;
					case 6:
					case 7:
						strcpy(tempStr, options[temp].name);
						break;
				}
				if (tempW == curSel[curMenu]-1)
				{
					if (keyboardUsed)
					{
						JE_setMousePosition(610, tempY+10);
					}
					temp2 = 15;
				} else {
					temp2 = 28;
				}

				JE_getShipInfo();

				if (temp == pItemsBack[pItemButtonMap[curSel[1]]] && temp != 0 && tempW != menuChoices[curMenu]-1)
				{
					JE_bar(160, tempY+7, 300, tempY+11, 227);
					JE_drawShape2(298, tempY+2, 247, shapes9);
				}

				if (tempW == menuChoices[curMenu]-1)
				{
					strcpy(tempStr, miscText[14]);
				}
				JE_textShade(185, tempY, tempStr, temp2 / 16, temp2 % 16 -8-temp4, DARKEN);

				if (tempW < menuChoices[curMenu]-1)
				{
					JE_drawItem(curSel[1]-1, temp, 160, tempY-4);
				}

				if (tempW == curSel[curMenu]-1)
				{
					temp2 = 15;
				} else {
					temp2 = 28;
				}

				if (tempW != menuChoices[curMenu]-1)
				{
					char buf[20];

					snprintf(buf, sizeof buf, "Cost: %d", tempW3);
					JE_textShade(187, tempY-10, buf, temp2 / 16, temp2 % 16 -8-temp4, DARKEN);
				}
			}
		}

		if (curMenu == 4 && (curSel[1] >= 2 && curSel[1] <= 7 && curSel[1] != 4))
		{
			joystickWaitMax = 20;
		} else {
			joystickWaitMax = 10;
		}

		/* YKS: Ouch */
		if ((curMenu == 0 || curMenu == 1 || curMenu == 6) || ((curMenu == 10 || curMenu == 11) && onePlayerAction) || ((curMenu == 3 || curMenu == 6) && !twoPlayerMode) || (curMenu == 5 && (curSel[1] >= 1 && curSel[1] <= 6)))
		{
			if (curMenu != 4)
			{
				char buf[20];

				snprintf(buf, sizeof buf, "%d", score);
				JE_textShade(65, 173, buf, 1, 6, DARKEN);
			}
			JE_barDrawShadow(42, 152, 3, 14, armorLevel, 2, 13);
			JE_barDrawShadow(104, 152, 2, 14, shields[pItems[9]].mpwr * 2, 2, 13);
		}

		/* TODO */
		service_SDL_events(FALSE);
		JE_showVGA();
	} while (!(quit || gameLoaded || jumpSection));
}

void JE_loadCubes( void )
{
	char s[256], s2[256], s3[256];
	JE_byte cube;
	JE_word x, y;
	JE_byte startPos, endPos, pos;
	JE_boolean nextLine;
	JE_boolean endString;
	FILE *f;
	JE_byte lastWidth, curWidth;
	JE_boolean pastStringLen, pastStringWidth;
	JE_byte temp;

	char buffer[256];

	memset(cubeText, 0, sizeof(cubeText));

	for (cube = 0; cube < cubeMax; cube++)
	{

		JE_resetFile(&f, cubeFile);
	
		tempw = cubeList[cube];
	
		do
		{
			do
			{
				JE_readCryptLn(f, s);
			} while (s[0] != '*');
			tempw--;
		} while (tempw != 0);
	
		faceNum[cube] = atoi(strnztcpy(buffer, s + 4, 2));
	
		JE_readCryptLn(f, cubeHdr[cube]);
		JE_readCryptLn(f, cubeHdr2[cube]);
	
		curWidth = 0;
		x = 5;
		y = 0;
		strcpy(s3, "");
	
		s2[0] = ' ';
	
		do
		{
		JE_readCryptLn(f, s2);

			if (s2[0] != '*')
			{

				sprintf(s, "%s %s", s3, s2);

				pos = 1;
				endPos = 0;
				endString = FALSE;

				do
				{
					startPos = endPos + 1;

					do
					{
						endPos = pos;
						lastWidth = curWidth;
						do
						{
							temp = s[pos - 1];

							if (temp > 32 && temp < 169 && fontMap[temp] != 255 && (*shapeArray)[5][fontMap[temp]] != NULL)
							{
								curWidth =+ shapeX[5][fontMap[temp]] + 1;
							} else {
								if (temp == 32)
								{
									curWidth += 6;
								}
							}

							pos++;
							if (pos == strlen(s))
							{
								endString = TRUE;
								if ((pos - startPos < CUBE_WIDTH) /*check for string length*/ && (curWidth <= LINE_WIDTH))
								{
									endPos = pos + 1;
									lastWidth = curWidth;
								}
							}

						} while (!(s[pos - 1] == ' ' || endString));

						pastStringLen = (pos - startPos > CUBE_WIDTH);
						pastStringWidth = (curWidth > LINE_WIDTH);

					} while (!(pastStringLen || pastStringWidth || endString));

					if (!endString || pastStringLen || pastStringWidth)
					{    /*Start new line*/
						strnztcpy(cubeText[cube][y], s + startPos - 1, endPos - startPos);
						y++;
						strnztcpy(s3, s + endPos, 255);
						curWidth = curWidth - lastWidth;
						if (s[endPos] == ' ')
						{
							curWidth -= 6;
						}
					} else {
						strnztcpy(s3, s + startPos - 1, 255);
						curWidth = 0;
					}

				} while (!endString);

				if (strlen(s2) == 0)
				{
					if (strlen(s3) != 0)
					{
						strcpy(cubeText[cube][y], s3);
					}
					y++;
					strcpy(s3, "");
				}

			}

		} while (s2[0] != '*');

		strcpy(cubeText[cube][y], s3);
		while (!strcmp(cubeText[cube][y], ""))
		{
			y--;
		}
		cubeMaxY[cube] = y + 1;


		fclose(f);
	}
}

void JE_drawItem( JE_byte itemType, JE_word itemNum, JE_word x, JE_word y )
{
	JE_word tempw;

	if (itemNum > 0)
	{
		switch (itemType)
		{
			case 2:
			case 3:
				tempw = weaponPort[itemNum].itemgraphic;
				break;
			case 5:
				tempw = powerSys[itemNum].itemgraphic;
				break;
			case 6:
			case 7:
				tempw = options[itemNum].itemgraphic;
				break;
			case 4:
				tempw = shields[itemNum].itemgraphic;
				break;
		}

		if (itemType == 1)
		{
			if (itemNum > 90)
			{
				shipGRptr = shapes9;
				shipGR = JE_SGR(itemNum - 90, &shipGRptr);
				JE_drawShape2x2(x, y, shipGR, shipGRptr);
			} else {
				JE_drawShape2x2(x, y, ships[itemNum].shipgraphic, shapes9);
			}
		} else {
			if (tempw > 0)
			{
				JE_drawShape2x2(x, y, tempw, shapes6);
			}
		}
	}
}

void JE_drawMenuHeader( void )
{
	switch (curMenu)
	{
		case 8:
			strcpy(tempStr, cubeHdr2[curSel[7]-2]);
			break;
		case 7:
			strcpy(tempStr, menuInt[0][2]);
			break;
		case 6:
			strcpy(tempStr, menuInt[2][performSave + 2]);
			break;
		default:
			strcpy(tempStr, menuInt[curMenu][0]);
			break;
	}
	JE_dString(74 + JE_fontCenter(tempStr, FONT_SHAPES), 10, tempStr, FONT_SHAPES);
}

void JE_drawMenuChoices( void )
{
	JE_byte x;
	char *str;

	for (x = 1; x < menuChoices[curMenu]; x++)
	{
		if (curMenu == 12)
		{
			tempY = 38 + x * 24 - 8;
		} else {
			tempY = 38 + x * 16;
		}
		
		if (curMenu == 0)
		{
			if (x == 7)
			{
				tempY += 16;
			}
		}

		if (curMenu == 9)
		{
			if (x > 3)
			{
				tempY += 16;
			}
			if (x > 4)
			{
				tempY += 16;
			}
		}

		if (!(curMenu == 3 && x == menuChoices[curMenu]))
		{
			tempY -= 16;
		}

		str = malloc(strlen(menuInt[curMenu][x])+1);
		if (curSel[curMenu] == x)
		{
			str[0] = '~';
			strcpy(str+1, menuInt[curMenu][x]);
		} else {
			strcpy(str, menuInt[curMenu][x]);
		}
		JE_dString(166, tempY, str, SMALL_FONT_SHAPES);
		free(str);

		if (keyboardUsed && curSel[curMenu] == x)
		{
			JE_setMousePosition(610, tempY + 6);
		}
	}
}

void JE_updateNavScreen( void )
{
	JE_byte x;

	tempNavX = ROUND(navX);
	tempNavY = ROUND(navY);
	JE_bar(19, 16, 135, 169, 2);
	JE_drawLines(TRUE);
	JE_drawLines(FALSE);
	JE_drawDots();

	for (x = 1; x <= 11; x++)
	{
		JE_drawPlanet(x);
	}

	for (x = 1; x < menuChoices[4]; x++)
	{
		if (mapPlanet[x-1] > 11)
		{
			JE_drawPlanet(mapPlanet[x-1]);
		}
	}

	if (mapOrigin > 11)
	{
		JE_drawPlanet(mapOrigin);
	}

	JE_newDrawCShapeNum(OPTION_SHAPES, 29, 0, 0);

	if (curSel[3] < menuChoices[3])
	{
		newNavX = (planetX[mapOrigin] - shapeX[PLANET_SHAPES][PGR[mapOrigin]] / 2 + planetX[mapPlanet[curSel[3]-2]] -  shapeX[PLANET_SHAPES][PGR[mapPlanet[curSel[3]-2]]] / 2) / 2.0;
		newNavY = (planetY[mapOrigin] - shapeY[PLANET_SHAPES][PGR[mapOrigin]] / 2 + planetY[mapPlanet[curSel[3]-2]] -  shapeY[PLANET_SHAPES][PGR[mapPlanet[curSel[3]-2]]] / 2) / 2.0;
	}

	navX = navX + (newNavX - navX) / 2.0;
	navY = navY + (newNavY - navY) / 2.0;

	if (abs(newNavX - navX) < 1)
	{
		navX = newNavX;
	}
	if (abs(newNavY - navY) < 1)
	{
		navY = newNavY;
	}

	JE_bar(314, 0, 319, 199, 230);

	if (planetAniWait > 0)
	{
		planetAniWait--;
	} else {
		planetAni++;
		if (planetAni > 14)
		{
			planetAni = 0;
		}
		planetAniWait = 3;
	}

	if (currentDotWait > 0)
	{
		currentDotWait--;
	} else {
		if (currentDotNum < planetDots[curSel[3]-2])
		{
			currentDotNum++;
		}
		currentDotWait = 5;
	}
}

void JE_drawLines( JE_boolean dark )
{
	JE_byte x, y;
	JE_integer tempX, tempY;
	JE_integer tempX2, tempY2;
	JE_word tempW, tempW2;

	tempX2 = -10;
	tempY2 = 0;

	tempW = 0;
	for (x = 0; x < 20; x++)
	{
		tempW += 15;
		tempX = tempW - tempX2;

		if (tempX > 18 && tempX < 135)
		{
			if (dark)
			{
				JE_rectangle(tempX + 1, 0, tempX + 1, 199, 32+3);
			} else {
				JE_rectangle(tempX, 0, tempX, 199, 32+5);
			}
		}
	}

	tempW = 0;
	for (y = 0; y < 20; y++)
	{
		tempW += 15;
		tempY = tempW - tempY2;

		if (tempY > 15 && tempY < 169)
		{
			if (dark)
			{
				JE_rectangle(0, tempY + 1, 319, tempY + 1, 32+3);
			} else {
				JE_rectangle(0, tempY, 319, tempY, 32+5);
			}

			tempW2 = 0;

			for (x = 0; x < 20; x++)
			{
				tempW2 += 15;
				tempX = tempW2 - tempX2;
				if (tempX > 18 && tempX < 135)
				{
					JE_pix3(tempX, tempY, 32+6);
				}
			}
		}
	}
}

void JE_drawDots( void )
{
	JE_byte x, y;
	JE_integer tempX, tempY;

	for (x = 1; x <= mapPNum; x++)
	{
		for (y = 1; y <= planetDots[x]; y++)
		{
			tempX = planetDotX[x][y] - tempNavX + 66 - 2;
			tempY = planetDotY[x][y] - tempNavY + 85 - 2;
			if (tempX > 0 && tempX < 140 && tempY > 0 && tempY < 168)
			{
				if (x == curSel[3]-1 && y <= currentDotNum)
				{
					JE_newDrawCShapeNum(OPTION_SHAPES, 31, tempX, tempY);
				} else {
					JE_newDrawCShapeNum(OPTION_SHAPES, 30, tempX, tempY);
				}
			}
		}
	}
}

void JE_drawPlanet( JE_byte planetNum )
{
	JE_integer tempX, tempY, tempZ;

	tempZ = PGR[planetNum];
	tempX = planetX[planetNum] + 66 - tempNavX - shapeX[PLANET_SHAPES][tempZ] / 2;
	tempY = planetY[planetNum] + 85 - tempNavY - shapeY[PLANET_SHAPES][tempZ] / 2;

	if (tempX > -7 && tempX + shapeX[PLANET_SHAPES][tempZ] < 170 && tempY > 0 && tempY < 160)
	{
		if (PAni[planetNum])
		{
			tempZ += planetAni;
		}
		JE_newDrawCShapeDarken((*shapeArray)[PLANET_SHAPES][tempZ], shapeX[PLANET_SHAPES][tempZ], shapeY[PLANET_SHAPES][tempZ], tempX + 3, tempY + 3);
		JE_newDrawCShapeNum(PLANET_SHAPES, tempZ, tempX, tempY);
	}
}
