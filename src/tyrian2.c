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

#include "tyrian2.h"

#include <string.h>

/* Level Event Data */
struct JE_EventRecType eventRec[EVENT_MAXIMUM]; /* [1..eventMaximum] */
JE_word levelEnemyMax;
JE_word levelEnemyFrequency;
JE_word levelEnemy[40]; /* [1..40] */

char tempstr[21]; /* string [20] */


void JE_main( void )
{
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

	/*============================GAME ROUTINES=================================*/
	/* We need to jump to the beginning to make space for the routines          */
	/*==========================================================================*/
	goto start_level_first;

	/* TODO */

start_level:

start_level_first:

	/*stopsequence;*/
	/*debuginfo('Setting Master Sound Volume');*/
	JE_setVol(0 /*Tyr_musicvolume*/, 0 /*fxvolume*/); /* TODO */

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
	mapyPos = (mapy * 28) + megaDataOfs - 2; /* TODO */
	mapy2Pos = (mapy2 * 28) + megaData2Ofs - 2; /* TODO */
	mapy3Pos = (mapy3 * 30) + megaData3Ofs - 2; /* TODO */
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
	/* TODO JE_GetShipInfo();*/
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
	/* TODO JE_drawShield();*/
	/* TODO JE_drawArmor();*/

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

	/* TODO */
}

/*========Load Level/Map Data========*/
void JE_loadMap( void )
{

	FILE *lvlFile, *shpFile;
/*	FILE *tempFile;*/ /*Extract map file from LVL file*/

	JE_byte planetAni, planetAniWait;

	JE_char char_mapFile, char_shapeFile;

	JE_DanCShape shape;
	JE_boolean shapeBlank;

	JE_real navx, navY, newNavX, newNavY;
	JE_integer tempNavX, tempNavY;
	
	FILE *f;
/*	FILE *f2;*/
	JE_char k2, k3;
	JE_word x, y;
	JE_integer yy, z, a, b;
	JE_word megaSh1Ofs, megaSh2Ofs, megaSh3Ofs;
	JE_word mapSh[3][128]; /* [1..3, 0..127] */
	JE_word ref[3][128]; /* [1..3, 0..127] */
	JE_string s;
	JE_boolean quit, first, loadLevelOk, refade;
	JE_byte col, planets, shade;

	/* Data used for ItemScreen procedure to indicate items available */
	JE_byte itemAvail[9][10]; /* [1..9, 1..10] */
	JE_byte itemAvailMax[9]; /* [1..9] */
	JE_integer planetDotX[5][10], planetDotY[5][10]; /* [1..5, 1..10] */
	JE_byte planetDots[5]; /* [1..5] */
	JE_byte currentDotNum, currentDotWait;

	JE_byte newPal, curPal, oldPal;
	JE_word yLoc;
	JE_shortint yChg;

	JE_byte mapBuf[15 * 600]; /* [1..15 * 600] */
	JE_word bufLoc;

	lastCubeMax = cubeMax;

	/*Defaults*/
	song_buy = DEFAULT_SONG_BUY;  /*Item Screen default song*/
	
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
		/*JE_openingAnim();*/
		JE_titleScreen(TRUE);
		loadTitleScreen = FALSE;
	}
	
	gameLoaded = FALSE;
	
	
	first = TRUE;

	/* TODO */
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

	/* TODO JE_initPlayerData(); */

	/*PlayCredits;*/

	/* TODO JE_sortHighScores;*/

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

						setdelay(2);
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
					tempY = JE_fontCenter(menuText[temp],SMALL_FONT_SHAPES);

					JE_outTextAdjust(tempY-1,tempX-1,menuText[temp],15,-10,SMALL_FONT_SHAPES,FALSE);
					JE_outTextAdjust(tempY+1,tempX+1,menuText[temp],15,-10,SMALL_FONT_SHAPES,FALSE);
					JE_outTextAdjust(tempY+1,tempX-1,menuText[temp],15,-10,SMALL_FONT_SHAPES,FALSE);
					JE_outTextAdjust(tempY-1,tempX+1,menuText[temp],15,-10,SMALL_FONT_SHAPES,FALSE);
					JE_outTextAdjust(tempY,tempX,menuText[temp],15,-3,SMALL_FONT_SHAPES,FALSE);
				}
				JE_showVGA();

				JE_fadeColors(&colors, &colors2, 0, 255, 20);

				memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
			}
		}

		memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));

		for (temp = 0; temp < menunum; temp++)
		{
			JE_outTextAdjust(JE_fontCenter(menuText[temp], SMALL_FONT_SHAPES),
			104+temp*13,
			menuText[temp], 15, -3+((temp == menu) * 2), SMALL_FONT_SHAPES, FALSE);
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
							/* JE_loadScreen(); */
							if (!gameLoaded)
							{
								redraw = TRUE;
							}
							fadeIn = TRUE;
							break;
						case 2: /* High scores */
							/* JE_highScoreScreen(); */
							fadeIn = TRUE;
							break;
						case 3: /* Instructions */
							JE_helpSystem(1);
							redraw = TRUE;
							fadeIn = TRUE;
							break;
						case 4: /* Ordering info */
							break;
						case 5: /* Demo */
							/* JE_initPlayerData(); */
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
					if (menu != 4) /* Tweak added to prevent fadeIn when selecting Ordering Info. */
					{
						redraw = TRUE;
					}
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

		setdelay(200);
		while(!(delaycount() == 0 || JE_anyButton()));

		JE_fadeBlack(15);

		JE_loadPic(12, FALSE);
		JE_showVGA();

		memcpy(colors, palettes[pcxpal[11]], sizeof(colors));
		JE_fadeColor(10);

		setdelay(200);
		while(!(delaycount() == 0 || JE_anyButton()));

		JE_fadeBlack(10);
	}
}
