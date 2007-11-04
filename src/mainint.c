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

#include "backgrnd.h"
#include "config.h"
#include "editship.h"
#include "episodes.h"
#include "error.h"
#include "fonthand.h"
#include "helptext.h"
#include "helptext.h"
#include "joystick.h"
#include "keyboard.h"
#include "loudness.h"
#include "network.h"
#include "newshape.h"
#include "nortsong.h"
#include "nortvars.h"
#include "pallib.h"
#include "params.h"
#include "pcxload.h"
#include "pcxmast.h"
#include "picload.h"
#include "setup.h"
#include "shpmast.h"
#include "sndmast.h"
#include "starfade.h"
#include "varz.h"
#include "vga256d.h"

#define NO_EXTERNS
#include "mainint.h"
#undef NO_EXTERNS

#include <ctype.h>


#define MAX_PAGE 8
#define TOPICS 6
const JE_byte topicStart[TOPICS] = { 0, 1, 2, 3, 7, 255 };

JE_shortint constantLastX;
JE_word textErase;
JE_word upgradeCost;
JE_word downgradeCost;
JE_boolean performSave;
JE_boolean jumpSection;
JE_boolean useLastBank; /* See if I want to use the last 16 colors for DisplayText */

/* Draws a message at the bottom text window on the playing screen */
void JE_drawTextWindow( char *text )
{
	tempScreenSeg = VGAScreenSeg; /*sega000*/
	if (textErase > 0)
	{
		JE_newDrawCShapeNum(OPTION_SHAPES, 37, 16, 189);
	}
	textErase = 100;
	tempScreenSeg = VGAScreenSeg; /*sega000*/
	JE_outText(20, 190, text, 0, 4);
}

void JE_outCharGlow( JE_word x, JE_word y, char *s )
{
	JE_integer maxloc, loc, z;
	JE_shortint glowcol[60]; /* [1..60] */
	JE_shortint glowcolc[60]; /* [1..60] */
	JE_word textloc[60]; /* [1..60] */
	JE_byte b = 0, bank;

	setjasondelay2(1);

	JE_setNetByte(0);

	if (useLastBank)
	{
		bank = 15;
	} else {
		bank = 14;
	}
	if (warningRed)
	{
		bank = 7;
	}

	if (strcmp(s, ""))
	{
		if (frameCountMax == 0)
		{
			JE_textShade(x, y, s, bank, 0, PART_SHADE);
			JE_showVGA();
		} else {

			maxloc = strlen(s);
			tempScreenSeg = VGAScreen;
			for (z = 0; z < 60; z++)
			{
				glowcol[z] = -8;
				glowcolc[z] = 1;
			}

			loc = x;
			for (z = 0; z < maxloc; z++)
			{
				textloc[z] = loc;
				if (s[z] == ' ')
				{
					loc += 6;
				} else {
					loc += shapeX[TINY_FONT][fontMap[(int)s[z]-33]] + 1;
				}
			}

			for (loc = 0; (unsigned)loc < strlen(s) + 28; loc++)
			{
				if (!ESCPressed)
				{
					setjasondelay(frameCountMax);

					JE_updateStream();
					if (netQuit)
					{
						return;
					}

					for (z = loc - 29; z < loc; z++)
					{
						if (z >= 0 && z < maxloc)
						{
							b = s[z];
							if (b > 32 && b < 126)
							{
								JE_newDrawCShapeAdjust((*shapeArray)[TINY_FONT][fontMap[b-33]], shapeX[TINY_FONT][fontMap[b-33]], shapeY[TINY_FONT][fontMap[b-33]], textloc[z], y, bank, glowcol[z]);
								glowcol[z] += glowcolc[z];
								if (glowcol[z] > 9)
								{
									glowcolc[z] = -1;
								}
							}
						}
					}
					if (b > 32 && b < 126 && z < maxloc)
					{
						JE_newDrawCShapeShadow((*shapeArray)[TINY_FONT][fontMap[b-33]], shapeX[TINY_FONT][fontMap[b-33]], shapeY[TINY_FONT][fontMap[b-33]], textloc[z-1] + 1, y + 1);
					}
					if (JE_anyButton())
					{
						frameCountMax = 0;
					}
					do {
						if (levelWarningDisplay)
						{
							JE_updateWarning();
						}
						JE_updateStream();
					} while (!(delaycount() == 0 || ESCPressed));
					JE_showVGA();
				}
			}
		}
	}
}

void JE_drawPortConfigButtons( void )
{
	if (!twoPlayerMode)
	{
		if (portConfig[1] == 1)
		{
			tempScreenSeg = VGAScreenSeg;
			JE_newDrawCShapeNum(OPTION_SHAPES, 19, 285, 44);
			tempScreenSeg = VGAScreenSeg;
			JE_newDrawCShapeNum(OPTION_SHAPES, 20, 302, 44);
		} else {
			tempScreenSeg = VGAScreenSeg;
			JE_newDrawCShapeNum(OPTION_SHAPES, 20, 285, 44);
			tempScreenSeg = VGAScreenSeg;
			JE_newDrawCShapeNum(OPTION_SHAPES, 19, 302, 44);
		}
	}
}

void JE_helpSystem( JE_byte startTopic )
{
	JE_integer page, lastPage = 0;
	JE_byte menu;
	JE_char flash;

	page = topicStart[startTopic-1];

	JE_fadeBlack(10);
	JE_loadPic(2, false);
	JE_playSong(SONG_MAPVIEW);
	JE_showVGA();
	JE_fadeColor(10);

	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

	joystickWaitMax = 120; joystickWait = 0;

	do
	{
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);

		temp2 = 0;

		for (temp = 0; temp < TOPICS; temp++)
		{
			if (topicStart[temp] <= page)
			{
				temp2 = temp;
			}
		}

		if (page > 0)
		{
			JE_char buf[128];

			sprintf(buf, "%s %d", miscText[24], page-topicStart[temp2]+1);
			JE_outText(10, 192, buf, 13, 5);

			sprintf(buf, "%s %d of %d", miscText[25], page, MAX_PAGE);
			JE_outText(220, 192, buf, 13, 5);

			JE_dString(JE_fontCenter(topicName[temp2], SMALL_FONT_SHAPES), 1, topicName[temp2], SMALL_FONT_SHAPES);
		}

		menu = 0;

		helpBoxBrightness = 3;
		verticalHeight = 8;

		switch (page)
		{
			case 0:
				menu = 2;
				if (lastPage == MAX_PAGE)
				{
					menu = TOPICS;
				}
				joystickWaitMax = 120; joystickWait = 0;
				JE_dString(JE_fontCenter(topicName[0], FONT_SHAPES), 30, topicName[0], FONT_SHAPES);

				do
				{
					for (temp = 1; temp <= TOPICS; temp++)
					{
						char buf[21+1];

						if (temp == menu-1)
						{
							strcpy(buf+1, topicName[temp]);
							buf[0] = '~';
						} else {
							strcpy(buf, topicName[temp]);
						}

						JE_dString(JE_fontCenter(topicName[temp], SMALL_FONT_SHAPES), temp * 20 + 40, buf, SMALL_FONT_SHAPES);
					}

					JE_waitRetrace();
					JE_showVGA();

					tempW = 0;
					JE_textMenuWait(&tempW, false);
					if (newkey)
					{
						switch (lastkey_sym)
						{
							case SDLK_UP:
								menu--;
								if (menu < 2)
								{
									menu = TOPICS;
								}
								JE_playSampleNum(CURSOR_MOVE);
								break;
							case SDLK_DOWN:
								menu++;
								if (menu > TOPICS)
								{
									menu = 2;
								}
								JE_playSampleNum(CURSOR_MOVE);
								break;
							default:
								break;
						}
					}
				} while (!(lastkey_sym == SDLK_ESCAPE || lastkey_sym == SDLK_RETURN));

				if (lastkey_sym == SDLK_RETURN)
				{
					page = topicStart[menu-1];
					JE_playSampleNum(CLICK);
				}

				joystickWaitMax = 120; joystickWait = 80;
				break;
			case 1: /* One-Player Menu */
				JE_HBox(10,  20,  2, 60);
				JE_HBox(10,  50,  5, 60);
				JE_HBox(10,  80, 21, 60);
				JE_HBox(10, 110,  1, 60);
				JE_HBox(10, 140, 28, 60);
				break;
			case 2: /* Two-Player Menu */
				JE_HBox(10,  20,  1, 60);
				JE_HBox(10,  60,  2, 60);
				JE_HBox(10, 100, 21, 60);
				JE_HBox(10, 140, 28, 60);
				break;
			case 3: /* Upgrade Ship */
				JE_HBox(10,  20,  5, 60);
				JE_HBox(10,  70,  6, 60);
				JE_HBox(10, 110,  7, 60);
				break;
			case 4:
				JE_HBox(10,  20,  8, 60);
				JE_HBox(10,  55,  9, 60);
				JE_HBox(10,  87, 10, 60);
				JE_HBox(10, 120, 11, 60);
				JE_HBox(10, 170, 13, 60);
				break;
			case 5:
				JE_HBox(10,  20, 14, 60);
				JE_HBox(10,  80, 15, 60);
				JE_HBox(10, 120, 16, 60);
				break;
			case 6:
				JE_HBox(10,  20, 17, 60);
				JE_HBox(10,  40, 18, 60);
				JE_HBox(10, 130, 20, 60);
				break;
			case 7: /* Options */
				JE_HBox(10,  20, 21, 60);
				JE_HBox(10,  70, 22, 60);
				JE_HBox(10, 110, 23, 60);
				JE_HBox(10, 140, 24, 60);
				break;
			case 8:
				JE_HBox(10,  20, 25, 60);
				JE_HBox(10,  60, 26, 60);
				JE_HBox(10, 100, 27, 60);
				JE_HBox(10, 140, 28, 60);
				JE_HBox(10, 170, 29, 60);
				break;
		}

		helpBoxBrightness = 1;
		verticalHeight = 7;

		lastPage = page;

		if (menu == 0)
		{
			JE_showVGA();
			do
			{
				service_SDL_events(false);
			} while (mousedown);
			while (!JE_waitAction(1, true));

			if (newmouse)
			{
				switch (lastmouse_but)
				{
					case SDL_BUTTON_LEFT:
						lastkey_sym = SDLK_RIGHT;
						break;
					case SDL_BUTTON_RIGHT:
						lastkey_sym = SDLK_LEFT;
						break;
					case SDL_BUTTON_MIDDLE:
						lastkey_sym = SDLK_ESCAPE;
						break;
				}
				do
				{
					service_SDL_events(false);
				} while (mousedown);
				newkey = true;
			}

			if (newkey)
			{
				switch (lastkey_sym)
				{
					case SDLK_LEFT:
					case SDLK_UP:
					case SDLK_PAGEUP:
						page--;
						JE_playSampleNum(CURSOR_MOVE);
						break;
					case SDLK_RIGHT:
					case SDLK_DOWN:
					case SDLK_PAGEDOWN:
					case SDLK_RETURN:
					case SDLK_SPACE:
						if (page == MAX_PAGE)
						{
							page = 0;
						} else {
							page++;
						}
						JE_playSampleNum(CURSOR_MOVE);
						break;
					case SDLK_F1:
						page = 0;
						JE_playSampleNum(CURSOR_MOVE);
						break;
					default:
						break;
				}
			}
		}

		if (page == 255)
		{
			lastkey_sym = SDLK_ESCAPE;
		}
	} while (lastkey_sym != SDLK_ESCAPE);
}

JE_boolean JE_playerSelect( void )
{
	JE_byte maxSel;
	JE_byte sel;
	JE_boolean quit;

	JE_loadPic(2, false);
	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);
	JE_showVGA();
	JE_fadeColor(20);
	quit = false;

	sel = 1;
	maxSel = 4;

	do
	{

		JE_dString(JE_fontCenter(playerName[0], FONT_SHAPES), 20, playerName[0], FONT_SHAPES);

		for (temp = 1; temp <= maxSel; temp++)
		{
			JE_outTextAdjust(JE_fontCenter(playerName[temp], SMALL_FONT_SHAPES), temp * 24 + 30, playerName[temp], 15, - 4 + ((sel == temp) << 1), SMALL_FONT_SHAPES, true);
		}

		/*BETA TEST VERSION*/
		/*  JE_Dstring(JE_FontCenter(misctext[34], FONT_SHAPES), 170, misctext[34], FONT_SHAPES);*/

		JE_showVGA();
		tempW = 0;
		JE_textMenuWait(&tempW, false);

		if (newkey)
		{
			switch (lastkey_sym)
			{
				case SDLK_UP:
					sel--;
					if (sel < 1)
					{
						sel = maxSel;
					}
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_DOWN:
					sel++;
					if (sel > maxSel)
					{
						sel = 1;
					}
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_RETURN:
					quit = true;
					twoPlayerMode = (sel == 3);
					onePlayerAction = (sel == 2);
					JE_playSampleNum(SELECT);
					if (sel == 4)
					{
						netQuit = true;
					}
					break;
				case SDLK_ESCAPE:
				quit = true;
				JE_playSampleNum(ESC);
				return false;
				break;
				default:
					break;
			}
		}

	} while (!quit);

	return true; /*MXD assumes this default return value here*/
}

JE_boolean JE_episodeSelect( void )
{
	JE_byte sel;
	JE_boolean quit;
	JE_byte max;

	max = EPISODE_MAX;

	if (!(episodeAvail[0] && episodeAvail[1] && episodeAvail[2]))
	{
		episodeAvail[3] = false;
	}

	if (episodeAvail[4] == false)
	{
		max = 4;
	}

	/*if (!episodeavail[3]) max = 3;*/

startepisodeselect:
	JE_loadPic(2, false);
	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);
	JE_showVGA();
	JE_fadeColor(10);
	quit = false;

	sel = 1;

	do
	{

		JE_dString(JE_fontCenter(episodeName[0], FONT_SHAPES), 20, episodeName[0], FONT_SHAPES);

		for (temp = 1; temp <= max; temp++)
		{
			JE_outTextAdjust(20, temp * 30 + 20, episodeName[temp], 15, - 4 - (!episodeAvail[temp-1] << 2) + ((sel == temp) << 1), SMALL_FONT_SHAPES, true);
		}

		/*JE_Dstring(JE_fontCenter(misctext[34], FONT_SHAPES), 170, misctext[34], FONT_SHAPES);*/

		JE_showVGA();
		tempW = 0;
		JE_textMenuWait(&tempW, false);

		if (newkey)
		{
			switch (lastkey_sym)
			{
				case SDLK_UP:
					sel--;
					if (sel < 1)
					{
						sel = max;
					}
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_DOWN:
					sel++;
					if (sel > max)
					{
						sel = 1;
					}
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_RETURN:
					if (episodeAvail[sel-1])
					{
						JE_playSampleNum(SELECT);

						quit = true;
						JE_initEpisode(sel);
						return true;
					} else {
						if (sel > 1)
						{
							char buf[] = "episode-.pcx";

							JE_playSampleNum(ESC);
							JE_fadeBlack (10);

							buf[7] = '0' + sel;
							JE_loadPCX(buf, false);
							verticalHeight = 9;
							helpBoxColor = 15;
							helpBoxBrightness = 4;
							helpBoxShadeType = FULL_SHADE;
							JE_helpBox(10, 10, helpTxt[29], 50);
							JE_showVGA();
							JE_fadeColor(10);
							wait_input(true,true,true);
							lastkey_sym = 0;
							JE_fadeBlack(10);
							goto startepisodeselect;
						}
					}
					break;
				case SDLK_ESCAPE:
					quit = true;
					JE_playSampleNum(ESC);
					return false;
					break;
				default:
					break;
			}
		}

	} while (!(quit || haltGame /*|| netQuit*/));
	pItems[8] = episodeNum;

	return false; /*MXD assumes this default return value here*/
}

JE_boolean JE_difficultySelect( void )
{
	JE_byte maxSel;
	JE_byte sel;
	JE_boolean quit;

	JE_loadPic(2, false);
	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);
	JE_showVGA();
	JE_fadeColor(20);
	quit = false;

	sel = 2;
	maxSel = 3;

	do
	{
		JE_dString(JE_fontCenter(difficultyName[0], FONT_SHAPES), 20, difficultyName[0], FONT_SHAPES);

		for (temp = 1; temp <= maxSel; temp++)
		{
			JE_outTextAdjust(JE_fontCenter(difficultyName[temp], SMALL_FONT_SHAPES), temp * 24 + 30, difficultyName[temp], 15, - 4 + ((sel == temp) << 1), SMALL_FONT_SHAPES, true);
		}

		/*BETA TEST VERSION*/
		/*  JE_Dstring(JE_FontCenter(misctext[34], FONT_SHAPES), 170, misctext[34], FONT_SHAPES);*/

		JE_showVGA();
		tempW = 0;
		JE_textMenuWait(&tempW, false);

		if (keysactive[SDLK_l] && keysactive[SDLK_o] && keysactive[SDLK_r] && keysactive[SDLK_d])
		{
			if (maxSel == 5)
			{
				maxSel = 6;
			}
		}

		if (newkey) {
			switch (lastkey_sym)
			{
				case SDLK_UP:
					sel--;
					if (sel < 1)
					{
						sel = maxSel;
					}
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_DOWN:
					sel++;
					if (sel > maxSel)
					{
						sel = 1;
					}
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_RETURN:
					quit = true;

					if (sel == 5)
					{
						sel++;
					}
					if (sel == 6)
					{
						sel = 8;
					}
					difficultyLevel = sel;
					JE_playSampleNum(SELECT);
					break;
				case SDLK_ESCAPE:
					quit = true;
					JE_playSampleNum(ESC);
					return false;
					break;
				case SDLK_g:
					if (SDL_GetModState() & KMOD_SHIFT)
					{
						if (maxSel < 4)
						{
							maxSel = 4;
						}
					}
					break;
				case SDLK_RIGHTBRACKET:
					if (SDL_GetModState() & KMOD_SHIFT)
					{
						if (maxSel == 4)
						{
							maxSel = 5;
						}
					}
					break;
				default:
					break;
			}
		}

	} while (!(quit || haltGame /*|| netQuit*/));

	return true; /*MXD assumes this default return value here*/
}

void JE_loadCompShapesB( JE_byte **shapes, FILE *f, JE_word shapeSize )
{
	*shapes = malloc(shapeSize);
	efread(*shapes, sizeof(JE_byte), shapeSize, f);
}

void JE_loadMainShapeTables( void )
{
	const JE_byte shapeReorderList[7] /* [1..7] */ = {1, 2, 5, 0, 3, 4, 6};

	FILE *f;

	typedef JE_longint JE_ShpPosType[SHP_NUM + 1]; /* [1..shpnum + 1] */

	JE_ShpPosType shpPos;
	JE_word shpNumb;

	if (tyrianXmas)
	{
		JE_resetFile(&f, "tyrianc.shp");
	} else {
		JE_resetFile(&f, "tyrian.shp");
	}

	efread(&shpNumb, sizeof(JE_word), 1, f);
	for (x = 0; x < shpNumb; x++)
	{
		efread(&shpPos[x], sizeof(JE_longint), 1, f);
	}
	fseek(f, 0, SEEK_END);
	shpPos[shpNumb] = ftell(f);

	/*fclose(f);*/

	/*Load EST shapes*/
	for (temp = 0; temp < 7; temp++)
	{
		fseek(f, shpPos[temp], SEEK_SET);
		JE_newLoadShapesB(shapeReorderList[temp], f);
	}

	shapesC1Size = shpPos[temp + 1] - shpPos[temp];
	JE_loadCompShapesB(&shapesC1, f, shapesC1Size);
	temp++;

	shapes9Size = shpPos[temp + 1] - shpPos[temp];
	JE_loadCompShapesB(&shapes9 , f, shapes9Size);
	temp++;

	eShapes6Size = shpPos[temp + 1] - shpPos[temp];
	JE_loadCompShapesB(&eShapes6, f, eShapes6Size);
	temp++;

	eShapes5Size = shpPos[temp + 1] - shpPos[temp];
	JE_loadCompShapesB(&eShapes5, f, eShapes5Size);
	temp++;

	shapesW2Size = shpPos[temp + 1] - shpPos[temp];
	JE_loadCompShapesB(&shapesW2, f, shapesW2Size);

	fclose(f);
}

JE_word JE_powerLevelCost( JE_word base, JE_byte level )
{
	JE_byte x;
	JE_word tempCost = 0;

	if (level > 0 && level < 12)
	{
		for (x = 1; x <= level; x++)
		{
			tempCost += base * x;
		}
	}

	return tempCost;
}

JE_longint JE_getCost( JE_byte itemType, JE_word itemNum )
{
	switch (itemType)
	{
		case 2:
			if (itemNum > 90)
			{
				tempW2 = 100;
			} else {
				tempW2 = ships[itemNum].cost;
			}
			break;
		case 3:
		case 4:
			tempW2 = weaponPort[itemNum].cost;
			downgradeCost = JE_powerLevelCost(tempW2, portPower[itemType-3]-1);
			upgradeCost = JE_powerLevelCost(tempW2, portPower[itemType-3]);
			break;
		case 5:
			tempW2 = shields[itemNum].cost;
			break;
		case 6:
			tempW2 = powerSys[itemNum].cost;
			break;
		case 7:
		case 8:
			tempW2 = options[itemNum].cost;
			break;
	}

	return tempW2;
}

void JE_loadScreen( void )
{
	JE_boolean quit;
	JE_byte sel, screen, min = 0, max = 0;
	char *tempstr;
	char *tempstr2;
	JE_boolean mal_str = false;
	int len;

	tempstr = NULL;

	/* TODO: Most of the actual save logic, string handling */

	JE_fadeBlack(10);
	JE_loadPic(2, false);
	JE_showVGA();
	JE_fadeColor(10);

	screen = 1;
	sel = 1;
	quit = false;

	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

	do
	{
		while (mousedown)
		{
			service_SDL_events(false);
			tempX = mouse_x;
			tempY = mouse_y;
		}

		memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);

		JE_dString(JE_fontCenter(miscText[38 + screen - 1], FONT_SHAPES), 5, miscText[38 + screen - 1], FONT_SHAPES);

		switch (screen)
		{
		case 1:
			min = 1;
			max = 12;
			/* {drawshape2x2(290,6 ,281,shapes6ofs,shapes6seg);} */
			break;
		case 2:
			min = 12;
			max = 23;
			/* {drawshape2x2( 10,6 ,279,shapes6ofs,shapes6seg);} */
		}

		/* {extshade(fontcenter(misctext[56],_TinyFont),192,misctext[56], 15,2,_FullShade);} */

		/* SYN: Go through text line by line */
		for (x = min; x <= max; x++)
		{
			tempY = 30 + (x - min) * 13;

			if (x == max)
			{
				/* Last line is return to main menu, not a save game */
				if (mal_str)
				{
					free(tempstr);
					mal_str = false;
				}
				tempstr = miscText[34 - 1];

				if (x == sel) /* Highlight if selected */
				{
					temp2 = 254;
				} else {
					temp2 = 250;
				}
			} else {
				if (x == sel) /* Highlight if selected */
				{
					temp2 = 254;
				} else {
					temp2 = 250 - ((saveFiles[x - 1].level == 0) << 1);
				}

				if (saveFiles[x - 1].level == 0) /* I think this means the save file is unused */
				{
					if (mal_str)
					{
						free(tempstr);
						mal_str = false;
					}
					tempstr = miscText[3 - 1];
				} else {
					if (mal_str)
					{
						free(tempstr);
						mal_str = false;
					}
					tempstr = saveFiles[x - 1].name;
				}
			}

			/* Write first column text */
			JE_textShade(10, tempY, tempstr, 13, (temp2 % 16) - 8, FULL_SHADE);

			if (x < max) /* Write additional columns for all but the last row */
			{
				if (saveFiles[x - 1].level == 0)
				{
					if (mal_str)
					{
						free(tempstr);
					}
					tempstr = malloc(7);
					mal_str = true;
					strcpy(tempstr, "-----"); /* Unused save slot */
				} else {
					tempstr = saveFiles[x - 1].levelName;
					tempstr2 = malloc(5 + strlen(miscTextB[2-1]));
					sprintf(tempstr2, "%s %d", miscTextB[2-1], saveFiles[x - 1].episode);
					JE_textShade(250, tempY, tempstr2, 5, (temp2 % 16) - 8, FULL_SHADE);
					free(tempstr2);
				}

				len = strlen(miscTextB[3-1]) + 2 + strlen(tempstr);
				tempstr2 = malloc(len);
				sprintf(tempstr2, "%s %s", miscTextB[3 - 1], tempstr);
				JE_textShade(120, tempY, tempstr2, 5, (temp2 % 16) - 8, FULL_SHADE);
				free(tempstr2);
			}

		}

		if (screen == 2)
		{
			JE_drawShape2x2(90, 180, 279, shapes6);
		}
		if (screen == 1)
		{
			JE_drawShape2x2(220, 180, 281, shapes6);
		}

		helpBoxColor = 15;
		JE_helpBox(110, 182, miscText[56-1], 25);

		JE_showVGA();

		tempW = 0;
		JE_textMenuWait(&tempW, false);


		if (newkey)
		{
			switch (lastkey_sym)
			{
			case SDLK_UP:
				sel--;
				if (sel < min)
				{
					sel = max;
				}
				JE_playSampleNum(CURSOR_MOVE);
				break;
			case SDLK_DOWN:
				sel++;
				if (sel > max)
				{
					sel = min;
				}
				JE_playSampleNum(CURSOR_MOVE);
				break;
			case SDLK_LEFT:
			case SDLK_RIGHT:
				if (screen == 1)
				{
					screen = 2;
					sel += 11;
				} else {
					screen = 1;
					sel -= 11;
				}
				break;
			case SDLK_RETURN:
				if (sel < max)
				{
					if (saveFiles[sel - 1].level > 0)
					{
						JE_playSampleNum (SELECT);
						performSave = false;
						JE_operation(sel);
						quit = true;
					} else {
						JE_playSampleNum (WRONG);
					}
				} else {
					quit = true;
				}


				break;
			case SDLK_ESCAPE:
				quit = true;
				break;
			default:
				break;
			}

		}
	} while (!quit);
}

JE_longint JE_totalScore( JE_longint score, JE_PItemsType pitems )
{
	JE_longint tempL = score;

	tempL += JE_getValue(2, pItems[11]);
	tempL += JE_getValue(3, pItems[0]);
	tempL += JE_getValue(4, pItems[1]);
	tempL += JE_getValue(5, pItems[9]);
	tempL += JE_getValue(6, pItems[5]);
	tempL += JE_getValue(7, pItems[3]);
	tempL += JE_getValue(8, pItems[4]);

	return tempL;
}

JE_longint JE_getValue( JE_byte itemType, JE_word itemNum )
{
	JE_longint tempW2 = 0;
	JE_byte z;

	switch (itemType)
	{
		case 2:
			tempW2 = ships[itemNum].cost;
			break;
		case 3:
		case 4:
			tempW2 = weaponPort[itemNum].cost;
			tempW3 = tempW2;
			for (z = 0; z < portPower[itemType-2]; z++)
			{
				tempW2 += JE_powerLevelCost(tempW3, z);
			}
			break;
		case 5:
			tempW2 = shields[itemNum].cost;
			break;
		case 6:
			tempW2 = powerSys[itemNum].cost;
			break;
		case 7:
		case 8:
			tempW2 = options[itemNum].cost;
			break;
	}

	return tempW2;
}

JE_boolean JE_nextEpisode( void )
{
	JE_boolean found;
	JE_integer x;
	JE_longint temp;
	
	strcpy(lastLevelName, "Completed");
	x = episodeNum;
	found = JE_findNextEpisode();
	
	if (!isNetworkGame && !gameHasRepeated
	    && (jumpBackToEpisode1 || x == 3)
	    && x != 4
	    && !constantPlay
	    && !(pItems[9-1] == 4))
	{
		JE_highScoreCheck();
		gameHasRepeated = true;
	}
	
	if (found)
	{
		gameLoaded = true;
		mainLevel = FIRST_LEVEL;
		saveLevel = FIRST_LEVEL;
		
		if (jumpBackToEpisode1 && !isNetworkGame && x == 1)
		{
			JE_loadOrderingInfo();
		}
		
		if (jumpBackToEpisode1 && x > 2 && !constantPlay)
		{
			JE_playCredits();
		}
		
		if (jumpBackToEpisode1 && (rand() % 6) == 0)
		{
			pItems[1-1] = 23;
			pItems[2-1] = 24;
			pItems[12-1] = 2;
			portPower[1-1] = 1;
			portPower[2-1] = 1;
			pItemsPlayer2[2-1] = 24;
			memcpy(pItemsBack2, pItems, sizeof(pItemsBack2));
		}
		
		JE_playSong(27);
		
		JE_clr256();
		memcpy(colors, palettes[6-1], sizeof(colors));
		
		tempScreenSeg = VGAScreen;
		
		JE_dString(JE_fontCenter(episodeName[episodeNum], SMALL_FONT_SHAPES),
		           130, episodeName[episodeNum], SMALL_FONT_SHAPES);
		
		JE_dString(JE_fontCenter(miscText[5-1], SMALL_FONT_SHAPES), 185,
		           miscText[5-1], SMALL_FONT_SHAPES);
		
		JE_showVGA();
		JE_fadeColor(15);
		
		JE_wipeKey();
		if (!constantPlay)
		{
			do
			{
				JE_updateStream();
			} while (!JE_anyButton());
		}
		JE_fadeBlack(15);
		
	} else {
		mainLevel = 0;
	}
	
	return found;
}

void JE_initPlayerData( void )
{
	/* JE: New Game Items/Data */
	pItems[0] = 1;  /* Normally 1 - Front Weapon */
	pItems[1] = 0;  /* Normally 0 - Rear Weapon */
	pItems[2] = 0;  /* SuperArcade mode */
	pItems[3] = 0;  /* Left Option */
	pItems[4] = 0;  /* Right Option */
	pItems[5] = 2;  /* Generator */
	pItems[6] = 2;
	pItems[7] = 1;
	pItems[8] = 0;  /* Starting Episode num */
	pItems[9] = 4;  /* Shields */
	pItems[10] = 0; /* Secret Weapons - Normally 0 */
	pItems[11] = 1; /* Normally 1 - Player Ship */
	memcpy(pItemsBack2, pItems, sizeof(pItems));
	memcpy(pItemsPlayer2, pItems, sizeof(pItems));
	pItemsPlayer2[1] = 15; /* Player 2 starts with 15 - MultiCannon and 2 single shot options */
	pItemsPlayer2[3] = 0;
	pItemsPlayer2[4] = 0;
	pItemsPlayer2[6] = 101; /* Player 2  Option Mode 101,102,103 */
	pItemsPlayer2[7] = 0; /* Player 2  Option Type */
	for (temp = 0; temp < 2; temp++)
	{
		portConfig[temp] = 1;
	}
	gameHasRepeated = false;
	onePlayerAction = false;
	superArcadeMode = 0;
	superTyrian = false;
	/* twoplayerarcade:=false; */
	twoPlayerMode = false;

	secretHint = (rand()%3) + 1;

	armorLevel = ships[pItems[11]].dmg;
	portPower[0] = 1;
	portPower[1] = 1;
	portConfig[1] = 1;

	mainLevel = FIRST_LEVEL;
	saveLevel = FIRST_LEVEL;

	strcpy(lastLevelName, miscText[20-1]);
}

void JE_sortHighScores( void )
{
	JE_byte x, temp = 0;

	for (x = 0; x < 6; x++)
	{
		JE_sort();
		temp += 3;
	}
}

void JE_highScoreScreen( void )
{
	int min = 1;
	int max = 1;

	int x, y, z;
	short int chg;
	int quit;
	char scoretemp[32];

	JE_fadeBlack(10);
	JE_loadPic(2, false);
	JE_showVGA();
	JE_fadeColor(10);
	tempScreenSeg = VGAScreen;

	while (max < EPISODE_MAX && episodeAvail[max+1])
	{
		max++;
	}

	max = 3; /* TODO: Ep. 4 high scores are broked! */

	quit = false;
	x = 1;
	chg = 1;

	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

	do
	{
		if (episodeAvail[x])
		{
			memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);

			JE_dString( JE_fontCenter( miscText[51 - 1], FONT_SHAPES), 03, miscText[51 - 1], FONT_SHAPES);
			JE_dString( JE_fontCenter( episodeName[x], SMALL_FONT_SHAPES), 30, episodeName[x], SMALL_FONT_SHAPES);

			/* Player 1 */
			temp = (x * 6) - 6;

			JE_dString( JE_fontCenter( miscText[47 - 1], SMALL_FONT_SHAPES), 55, miscText[47 - 1], SMALL_FONT_SHAPES);

			for (z = 0; z < 3; z++)
			{
				temp5 = saveFiles[temp + z].highScoreDiff;
				if (temp5 > 9)
				{
					saveFiles[temp + z].highScoreDiff = 0;
					temp5 = 0;
				}
				sprintf(scoretemp, "~#%d:~ %d", z + 1, saveFiles[temp+z].highScore1);
				JE_textShade(250, ((z+1) * 10) + 65 , difficultyNameB[temp5], 15, temp5 + ((JE_byte) (temp5 == 0)) - 1, FULL_SHADE);
				JE_textShade(20, ((z+1) * 10) + 65 , scoretemp, 15, 0, FULL_SHADE);
				JE_textShade(110, ((z+1) * 10) + 65 , saveFiles[temp + z].highScoreName, 15, 2, FULL_SHADE);
			}

			/* Player 2 */
			temp += 3;

			JE_dString( JE_fontCenter( miscText[48 - 1], SMALL_FONT_SHAPES), 120, miscText[48 - 1], SMALL_FONT_SHAPES);

			/*{        textshade(20,125,misctext[49],15,3,_FullShade);
			  textshade(80,125,misctext[50],15,3,_FullShade);}*/

			for (z = 0; z < 3; z++)
			{
				temp5 = saveFiles[temp + z].highScoreDiff;
				if (temp5 > 9)
				{
					saveFiles[temp + z].highScoreDiff = 0;
					temp5 = 0;
				}
				sprintf(scoretemp, "~#%d:~ %d", z + 1, saveFiles[temp+z].highScore1); /* Not .highScore2 for some reason */
				JE_textShade(250, ((z+1) * 10) + 125 , difficultyNameB[temp5], 15, temp5 + ((JE_byte) (temp5 == 0)) - 1, FULL_SHADE);
				JE_textShade(20, ((z+1) * 10) + 125 , scoretemp, 15, 0, FULL_SHADE);
				JE_textShade(110, ((z+1) * 10) + 125 , saveFiles[temp + z].highScoreName, 15, 2, FULL_SHADE);
			}

			if (x > 1)
			{
				JE_drawShape2x2( 90, 180, 279, shapes6);
			}

			if ( ( (x < 2) && episodeAvail[2] ) || ( (x < 3) && episodeAvail[3] ) )
			{
				JE_drawShape2x2( 220, 180, 281, shapes6);
			}

			helpBoxColor = 15;
			JE_helpBox(110, 182, miscText[57 - 1], 25);

			/* {Dstring(fontcenter(misctext[57],_SmallFontShapes),190,misctext[57],_SmallFontShapes);} */

			JE_showVGA();

			tempW = 0;
			JE_textMenuWait(&tempW, false);

			if (newkey)
			{
				switch (lastkey_sym)
				{
				case SDLK_LEFT:
					x--;
					chg = -1;
					break;
				case SDLK_RIGHT:
					x++;
					chg = 1;
					break;
				default:
					break;
				}
			}

		} else {
			x += chg;
		}

		x = ( x < min ) ? max : ( x > max ) ? min : x;

		if (newkey)
		{
			switch (lastkey_sym)
			{
			case SDLK_RETURN:
			case SDLK_ESCAPE:
				quit = true;
				break;
			default:
				break;
			}
		}

	} while (!quit);

}

void JE_gammaCorrect_func( JE_byte *col, JE_real r )
{
	*col = round(*col * r);
	if (*col > 63)
	{
		*col = 63;
	}
}

void JE_gammaCorrect( JE_ColorType *colorBuffer, JE_byte gamma )
{
	int x;
	JE_real r = 1 + (JE_real)gamma / 10;
	
	for (x = 0; x < 256; x++)
	{
		JE_gammaCorrect_func(&(*colorBuffer)[x].r, r);
		JE_gammaCorrect_func(&(*colorBuffer)[x].g, r);
		JE_gammaCorrect_func(&(*colorBuffer)[x].b, r);
	}
}

JE_boolean JE_gammaCheck( void )
{
	Uint8 temp = keysactive[SDLK_F11];
	if (temp)
	{
		keysactive[SDLK_F11] = false;
		newkey = false;
		gammaCorrection = (gammaCorrection + 1) % 4;
		memcpy(colors, palettes[pcxpal[3-1]], sizeof(colors));
		JE_gammaCorrect(&colors, gammaCorrection);
		JE_updateColorsFast(&colors);
	}
	return temp;
}

/* void JE_textMenuWait( JE_word *waitTime, JE_boolean doGamma ); /!\ In setup.h */
void JE_loadOrderingInfo( void )
{
	/* YKS: Unused on the port */
	STUB();
}

void JE_doInGameSetup( void )
{
	haltGame = false;
	/* TODO syncnet (0);*/
	if (yourInGameMenuRequest)
	{
		useButtonAssign = false; /*Joystick button remapping*/
		if (JE_inGameSetup())
		{
			reallyEndLevel = true;
			playerEndLevel = true;
		}
		if (!isNetworkGame)
		{
			/* TODO */
		}
		quitRequested = false;
		keysactive[SDLK_RETURN] = false;
	}
	
	if (isNetworkActive && !isNetworkGame)
	{
		/* TODO JE_arenaPoll();*/
	}
	
	if (isNetworkGame)
	{
		/* TODO */
	}
	
	useButtonAssign = true;  /*Joystick button remapping*/
	/* TODO gameQuitDelay = streamLagFrames + 1;*/
	yourInGameMenuRequest = false;
	skipStarShowVGA = true;
}

JE_boolean JE_inGameSetup( void )
{
	SDL_Surface *temp_surface = VGAScreen;
	VGAScreen = VGAScreenSeg; /* side-effect of game_screen */
	
	JE_boolean returnvalue = false;
	
	const JE_byte help[6] /* [1..6] */ = {15, 15, 28, 29, 26, 27};
	JE_byte  sel;
	JE_boolean quit;
	JE_word x, y, z;

	tempScreenSeg = VGAScreenSeg; /* <MXD> ? */
	/* TODO callBiosHandler = true;*/
	JE_clearKeyboard();
	JE_wipeKey();
	
	quit = false;
	sel = 1;
	
	JE_barShade(3, 13, 217, 137); /*Main Box*/
	JE_barShade(5, 15, 215, 135);
	
	JE_barShade(3, 143, 257, 157); /*Help Box*/
	JE_barShade(5, 145, 255, 155);
	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);
	
	do
	{
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
		
		for (x = 0; x < 6; x++)
		{
			JE_outTextAdjust(10, (x + 1) * 20, inGameText[x], 15, ((sel == x+1) << 1) - 4, SMALL_FONT_SHAPES, true);
		}
		
		JE_outTextAdjust(120, 3 * 20, detailLevel[processorType-1], 15, ((sel == 3) << 1) - 4, SMALL_FONT_SHAPES, true);
		JE_outTextAdjust(120, 4 * 20, gameSpeedText[gameSpeed-1],   15, ((sel == 4) << 1) - 4, SMALL_FONT_SHAPES, true);
		
		JE_outTextAdjust(10, 147, mainMenuHelp[help[sel-1]-1], 14, 6, TINY_FONT, true);
		
		JE_barDrawShadow(120, 20, 1, 16, tyrMusicVolume / 12, 3, 13);
		JE_barDrawShadow(120, 40, 1, 16, fxVolume / 12, 3, 13);
		
		JE_showVGA();
		
		tempW = 0;
		JE_textMenuWait(&tempW, true);
		
		if (inputDetected)
		{
			switch (lastkey_sym)
			{
				case SDLK_RETURN:
					JE_playSampleNum(SELECT);
					switch (sel)
					{
						case 1:
						case 2:
						case 3:
						case 4:
							sel = 5;
							break;
						case 5:
							quit = true;
							break;
						case 6:
							returnvalue = true;
							quit = true;
							if (constantPlay)
							{
								JE_tyrianHalt(0);
							}
							
							if (isNetworkGame)
							{ /*Tell other computer to exit*/
								netQuit = true;
								haltGame = true;
								playerEndLevel = true;
							}
							break;
					}
					break;
				case SDLK_ESCAPE:
					quit = true;
					JE_playSampleNum(ESC);
					break;
				case SDLK_UP:
					if (--sel < 1)
					{
						sel = 6;
					}
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_DOWN:
					if (++sel > 6)
					{
						sel = 1;
					}
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_LEFT:
					switch (sel)
					{
						case 1:
							JE_changeVolume(&tyrMusicVolume, -16, &fxVolume, 0);
							if (!musicActive)
							{
								musicActive = true;
								temp = currentSong;
								currentSong = 0;
								JE_playSong(temp);
							}
							break;
						case 2:
							JE_changeVolume(&tyrMusicVolume, 0, &fxVolume, -16);
							soundActive = true;
							break;
						case 3:
							if (--processorType < 1)
							{
								processorType = 4;
							}
							JE_initProcessorType();
							JE_setNewGameSpeed();
							break;
						case 4:
							if (--gameSpeed < 1)
							{
								gameSpeed = 5;
							}
							JE_initProcessorType();
							JE_setNewGameSpeed();
							break;
					}
					if (sel < 5)
					{
						JE_playSampleNum(CURSOR_MOVE);
					}
					break;
				case SDLK_RIGHT:
					switch (sel)
					{
						case 1:
							JE_changeVolume(&tyrMusicVolume, 16, &fxVolume, 0);
							if (!musicActive)
							{
								musicActive = true;
								temp = currentSong;
								currentSong = 0;
								JE_playSong(temp);
							}
							break;
						case 2:
							JE_changeVolume(&tyrMusicVolume, 0, &fxVolume, 16);
							soundActive = true;
							break;
						case 3:
							if (++processorType > 4)
							{
								processorType = 1;
							}
							JE_initProcessorType();
							JE_setNewGameSpeed();
							break;
						case 4:
							if (++gameSpeed > 5)
							{
								gameSpeed = 1;
							}
							JE_initProcessorType();
							JE_setNewGameSpeed();
							break;
					}
					if (sel < 5)
					{
						JE_playSampleNum(CURSOR_MOVE);
					}
					break;
				case SDLK_w:
					if (sel == 3)
					{
						processorType = 6;
						JE_initProcessorType();
					}
				default:
					break;
			}
		}
		
	} while (!(quit || haltGame || netQuit));
	/* TODO callBiosHandler = false;*/
	netQuit = false;
	
	VGAScreen = temp_surface; /* side-effect of game_screen */
	
	return returnvalue;
}

void JE_inGameHelp( void )
{
	STUB();
}

void JE_highScoreCheck( void )
{
	STUB();
}

void JE_setNewGameVol( void )
{
	if (!soundActive)
	{
		temp = 5;
	} else {
		temp = fxVolume;
	}
	if (!musicActive)
	{
		temp2 = 15;
	} else {
		temp2 = tyrMusicVolume;
	}

	JE_setVol(temp2, temp);
}

void JE_changeDifficulty( void )
{
	JE_byte newDifficultyLevel;
	JE_longint temp;

	if (twoPlayerMode)
	{
		temp = score + score2;
	} else {
		temp = JE_totalScore(score, pItems);
	}

	switch (initialDifficulty)
	{
		case 1:
			temp = round(temp * 0.4);
			break;
		case 2:
			temp = round(temp * 0.8);
			break;
		case 3:
			temp = round(temp * 1.3);
			break;
		case 4:
			temp = round(temp * 1.6);
			break;
		case 5:
		case 6:
			temp = round(temp * 2);
			break;
		case 7:
		case 8:
		case 9:
			temp = round(temp * 3);
			break;
	}

	if (twoPlayerMode)
	{
		if (temp < 10000)
		{
			newDifficultyLevel = 1; /* Easy */
		} else if (temp < 20000) {
			newDifficultyLevel = 2; /* Normal */
		} else if (temp < 50000) {
			newDifficultyLevel = 3; /* Hard */
		} else if (temp < 80000) {
			newDifficultyLevel = 4; /* Impossible */
		} else if (temp < 125000) {
			newDifficultyLevel = 5; /* Impossible B */
		} else if (temp < 200000) {
			newDifficultyLevel = 6; /* Suicide */
		} else if (temp < 400000) {
			newDifficultyLevel = 7; /* Maniacal */
		} else if (temp < 600000) {
			newDifficultyLevel = 8; /* Zinglon */
		} else {
			newDifficultyLevel = 9; /* Nortaneous */
		}
	} else {
		if (temp < 40000)
		{
			newDifficultyLevel = 1; /* Easy */
		} else if (temp < 70000) {
			newDifficultyLevel = 2; /* Normal */
		} else if (temp < 150000) {
			newDifficultyLevel = 3; /* Hard */
		} else if (temp < 300000) {
			newDifficultyLevel = 4; /* Impossible */
		} else if (temp < 600000) {
			newDifficultyLevel = 5; /* Impossible B */
		} else if (temp < 1000000) {
			newDifficultyLevel = 6; /* Suicide */
		} else if (temp < 2000000) {
			newDifficultyLevel = 7; /* Maniacal */
		} else if (temp < 3000000) {
			newDifficultyLevel = 8; /* Zinglon */
		} else {
			newDifficultyLevel = 9; /* Nortaneous */
		}
	}

	if (newDifficultyLevel > difficultyLevel)
	{
		difficultyLevel = newDifficultyLevel;
	}

}

void JE_doDemoKeys( void )
{
	if (lastKey[0] > 0)
	{
		PY -= CURRENT_KEY_SPEED;
	}
	if (lastKey[1] > 0)
	{
		PY += CURRENT_KEY_SPEED;
	}
	if (lastKey[2] > 0)
	{
		PX -= CURRENT_KEY_SPEED;
	}
	if (lastKey[3] > 0)
	{
		PX += CURRENT_KEY_SPEED;
	}
	if (lastKey[4] > 0)
	{
		button[1-1] = true;
	}
	if (lastKey[5] > 0)
	{
		button[4-1] = true;
	}
	if (lastKey[6] > 0)
	{
		button[2-1] = true;
	}
	if (lastKey[7] > 0)
	{
		button[3-1] = true;
	}
}

void JE_readDemoKeys( void )
{
	temp = nextDemoOperation;

	lastKey[0] = (temp & 0x01) > 0;
	lastKey[1] = (temp & 0x02) > 0;
	lastKey[2] = (temp & 0x04) > 0;
	lastKey[3] = (temp & 0x08) > 0;
	lastKey[4] = (temp & 0x10) > 0;
	lastKey[6] = (temp & 0x40) > 0;
	lastKey[7] = (temp & 0x80) > 0;

	temp = getc(recordFile);
	temp2 = getc(recordFile);
	lastMoveWait = temp << 8 | temp2;
	nextDemoOperation = getc(recordFile);
}

/*Street Fighter codes*/
void JE_SFCodes( JE_byte playerNum_, JE_integer PX_, JE_integer PY_, JE_integer mouseX_, JE_integer mouseY_, JE_PItemsType pItems_ )
{
	JE_byte temp, temp2, temp3, temp4, temp5;
	
	/*Get direction*/
	tempW = pItems_[11]; // Get player ship
	if (playerNum_ == 2 && tempW < 15)
	{
		tempW = 0;
	}
	
	if (tempW < 15)
	{
		
		temp2 = (mouseY_ > PY_) +    /*UP*/
		        (mouseY_ < PY_) +    /*DOWN*/
		        (PX < mouseX_) +    /*LEFT*/
		        (PX > mouseX_);     /*RIGHT*/
		temp = (mouseY_ > PY_) * 1 + /*UP*/
		       (mouseY_ < PY_) * 2 + /*DOWN*/
		       (PX_ < mouseX_) * 3 + /*LEFT*/
		       (PX_ > mouseX_) * 4;  /*RIGHT*/
		
		if (temp == 0) // no direction being pressed
		{
			if (!button[0]) // if fire button is released
			{
				temp = 9;
				temp2 = 1;
			} else {
				temp2 = 0;
				temp = 99;
			}
		}
		
		if (temp2 == 1) // if exactly one direction pressed or firebutton is released
		{
			temp += button[0] * 4;
			
			temp3 = superTyrian ? 21 : 3;
			for (temp2 = 0; temp2 < temp3; temp2++)
			{
				
				/*Use SuperTyrian ShipCombos or not?*/
				temp5 = superTyrian ? shipCombosB[temp2] : shipCombos[tempW][temp2];
				
				// temp5 == selected combo in ship
				if (temp5 == 0) /* combo doesn't exists */
				{
					// mark twiddles as cancelled/finished
					SFCurrentCode[playerNum_-1][temp2] = 0;
				} else {
					// get next combo key
					temp4 = keyboardCombos[temp5-1][SFCurrentCode[playerNum_-1][temp2]];
					
					// correct key
					if (temp4 == temp)
					{
						SFCurrentCode[playerNum_-1][temp2]++;
						
						temp4 = keyboardCombos[temp5-1][SFCurrentCode[playerNum_-1][temp2]];
						if (temp4 > 100 && temp4 <= 100 + SPECIAL_NUM)
						{
							SFCurrentCode[playerNum_-1][temp2] = 0;
							SFExecuted[playerNum_-1] = temp4 - 100;
						}
					} else {
						if ((temp != 9) &&
						    (temp4 - 1) % 4 != (temp - 1) % 4 &&
						    (SFCurrentCode[playerNum_-1][temp2] == 0 ||
						     keyboardCombos[temp5-1][SFCurrentCode[playerNum_-1][temp2]-1] != temp))
						{
							SFCurrentCode[playerNum_-1][temp2] = 0;
						}
					}
				}
			}
		}
		
	}
}

void JE_func( JE_byte col )
{
	STUB();
}

void JE_sort( void )
{
	JE_byte a, b;

	for (a = 0; a < 2; a++)
	{
		for (b = a+1; b < 3; b++)
		{
			if (saveFiles[temp+a].highScore1 < saveFiles[temp+b].highScore1)
			{
				JE_longint tempLI;
				char tempStr[30];
				JE_byte tempByte;

				tempLI = saveFiles[temp+a].highScore1;
				saveFiles[temp+a].highScore1 = saveFiles[temp+b].highScore1;
				saveFiles[temp+b].highScore1 = tempLI;

				strcpy(tempStr, saveFiles[temp+a].highScoreName);
				strcpy(saveFiles[temp+a].highScoreName, saveFiles[temp+b].highScoreName);
				strcpy(saveFiles[temp+b].highScoreName, tempStr);

				tempByte = saveFiles[temp+a].highScoreDiff;
				saveFiles[temp+a].highScoreDiff = saveFiles[temp+b].highScoreDiff;
				saveFiles[temp+b].highScoreDiff = tempByte;
			}
		}
	}
}

JE_boolean JE_getPassword( void )
{
	STUB();
	return false;
}

void JE_playCredits( void )
{
	const int maxlines = 132;
	typedef char JE_CreditStringType[maxlines][66];
	
	JE_CreditStringType credstr;
	JE_word x, max = 0, maxlen = 0;
	JE_integer curpos, newpos;
	JE_byte yloc;
	FILE *f;
	JE_byte currentpic = 1, fade = 0;
	JE_shortint fadechg = 1;
	JE_byte currentship = 0;
	JE_integer shipx = 0, shipxwait = 0;
	JE_shortint shipxc = 0, shipxca = 0;
	
	JE_newLoadShapes(EXTRA_SHAPES, "estsc.shp");
	
	setjasondelay(1000);
	
	if (currentSong != 9)
		JE_playSong(9);
	JE_resetFile(&f, "tyrian.cdt");
	while (!feof(f))
	{
		maxlen += 20 * 3;
		JE_readCryptLn(f, credstr[max]);
		max++;
	}
	
	memcpy(colors, palettes[6-1], sizeof(colors));
	JE_clr256();
	JE_showVGA();
	JE_fadeColor(2);
	
	tempScreenSeg = VGAScreenSeg;
	
	for (x = 0; x < maxlen; x++)
	{
		setjasondelay2(1);
		JE_clr256();
		
		JE_newDrawCShapeAdjust((*shapeArray)[EXTRA_SHAPES][currentpic-1], shapeX[EXTRA_SHAPES][currentpic-1], shapeY[EXTRA_SHAPES][currentpic-1], 319 - shapeX[EXTRA_SHAPES][currentpic-1], 100 - (shapeY[EXTRA_SHAPES][currentpic-1] / 2), 0, fade - 15);
		
		fade += fadechg;
		if (fade == 0 && fadechg == -1)
		{
			fadechg = 1;
			currentpic++;
			if (currentpic > maxShape[EXTRA_SHAPES])
				currentpic = 1;
		}
		if (fade == 15)
			fadechg = 0;
		
		if (delaycount() == 0)
		{
			fadechg = -1;
			setjasondelay(900);
		}
		
		curpos = (x / 3) / 20;
		yloc = 20 - ((x / 3) % 20);
		
		if (x % 200 == 0)
		{
			currentship = (rand() % 11) + 1;
			shipxwait = (rand() % 80) + 10;
			if ((rand() % 2) == 1)
			{
				shipx = 1;
				shipxc = 0;
				shipxca = 1;
			}
			else
			{
				shipx = 900;
				shipxc = 0;
				shipxca = -1;
			}
		}
		
		shipxwait--;
		if (shipxwait == 0)
		{
			if (shipx == 1 || shipx == 900)
				shipxc = 0;
			shipxca = -shipxca;
			shipxwait = (rand() % 40) + 15;
		}
		shipxc += shipxca;
		shipx += shipxc;
		if (shipx < 1)
		{
			shipx = 1;
			shipxwait = 1;
		}
		if (shipx > 900)
		{
			shipx = 900;
			shipxwait = 1;
		}
      	tempI = shipxc * shipxc;
		if (450 + tempI < 0 || 450 + tempI > 900)
		{
			if (shipxca < 0 && shipxc < 0)
				shipxwait = 1;
			if (shipxca > 0 && shipxc > 0)
				shipxwait = 1;
		}
		tempW = ships[currentship].shipgraphic;
		if (shipxc < -10)
			tempW -= 2;
		if (shipxc < -20)
			tempW -= 2;
		if (shipxc > 10)
			tempW += 2;
		if (shipxc > 20)
			tempW += 2;
		JE_drawShape2x2(shipx / 40, 184 - (x % 200), tempW, shapes9);
		
		for (newpos = curpos - 9; newpos <= curpos; newpos++)
		{
			if (newpos > 0 && newpos <= max)
			{
				if (strcmp(&credstr[newpos-1][0], ".") && strlen(credstr[newpos-1]))
				{
					JE_outTextAdjust(110 - JE_textWidth(&credstr[newpos-1][1], SMALL_FONT_SHAPES) / 2 + abs((yloc / 18) % 4 - 2) - 1, yloc - 1, &credstr[newpos-1][1], credstr[newpos-1][0] - 65, -8, SMALL_FONT_SHAPES, false);
					JE_outTextAdjust(110 - JE_textWidth(&credstr[newpos-1][1], SMALL_FONT_SHAPES) / 2, yloc, &credstr[newpos-1][1], credstr[newpos-1][0] - 65, -2, SMALL_FONT_SHAPES, false);
				}
			}
			
			yloc += 20;
		}
		
		JE_bar(0,  0, 319, 10, 0);
		JE_bar(0, 190, 319, 199, 0);
		
		if (currentpic == maxShape[EXTRA_SHAPES])
			JE_outTextAdjust(5, 180, miscText[55-1], 2, -2, SMALL_FONT_SHAPES, false);
		
		JE_updateStream();
		int delaycount_temp;
		if ((delaycount_temp = target2 - SDL_GetTicks()) > 0)
			SDL_Delay(delaycount_temp);
		
		JE_showVGA();
		if (JE_anyButton())
		{
			x = maxlen - 1;
		} else {
			if (newpos == maxlines - 8)
				JE_selectSong(0xC001);
			if (x == maxlen - 1)
			{
				x--;
				if (currentSong != 10)
					JE_playSong(10);
			}
		}
	}
	
	JE_fadeBlack(10);
	
	JE_newPurgeShapes(EXTRA_SHAPES);
}

void JE_endLevelAni( void )
{
	JE_word x, y;
	JE_byte temp;
	char tempStr[256];
	
	Sint8 i;
	
	if (!constantPlay)
	{
		/*Grant Bonus Items*/
		/*Front/Rear*/
		saveTemp[SAVE_FILES_SIZE + pItems[1-1]] = 1;
		saveTemp[SAVE_FILES_SIZE + pItems[2-1]] = 1;
		saveTemp[SAVE_FILES_SIZE + pItemsPlayer2[1-1]] = 1;
		saveTemp[SAVE_FILES_SIZE + pItemsPlayer2[2-1]] = 1;
		
		/*Special*/
		if (pItems[11-1] < 21)
		{
			saveTemp[SAVE_FILES_SIZE + 81 + pItems[11-1]] = 1;
		}
		
		/*Options*/
		saveTemp[SAVE_FILES_SIZE + 51 + pItems[4-1]] = 1;
		saveTemp[SAVE_FILES_SIZE + 51 + pItems[5-1]] = 1;
		saveTemp[SAVE_FILES_SIZE + 51 + pItemsPlayer2[4-1]] = 1;
		saveTemp[SAVE_FILES_SIZE + 51 + pItemsPlayer2[5-1]] = 1;
	}
	
	JE_changeDifficulty();
	
	memcpy(pItemsBack2, pItems, sizeof(pItemsBack2));
	strcpy(lastLevelName, levelName);
	
	JE_setNetByte(0);
	
	JE_updateStream();
	JE_updateStream();
	JE_updateStream();
	JE_updateStream();
	
	JE_wipeKey();
	frameCountMax = 4;
	textGlowFont = SMALL_FONT_SHAPES;
	
	JE_setPalette(254, 63, 63, 63);
	
	if (!levelTimer || levelTimerCountdown > 0 || !(episodeNum == 4))
	{
		JE_playSampleNum(V_LEVEL_END);
	} else {
		JE_playSong(22);
	}
  
	if (bonusLevel)
	{
		JE_outTextGlow(20, 20, miscText[17-1]);
	} else if (playerAlive && (!twoPlayerMode || playerAliveB)) {
		sprintf(tempStr, "%s %s", miscText[27-1], levelName);
		JE_outTextGlow(20, 20, tempStr);
	} else {
		sprintf(tempStr, "%s %s", miscText[62-1], levelName);
		JE_outTextGlow(20, 20, tempStr);
	}
	
	JE_updateStream();
	if (netQuit)
		exit(0);
	
	if (twoPlayerMode)
	{
		sprintf(tempStr, "%s %d", miscText[41-1], score);
		JE_outTextGlow(30, 50, tempStr);
		
		sprintf(tempStr, "%s %d", miscText[42-1], score2);
		JE_outTextGlow(30, 70, tempStr);
	} else {
		sprintf(tempStr, "%s %d", miscText[28-1], score);
		JE_outTextGlow(30, 50, tempStr);
		
		JE_updateStream();
		if (netQuit)
			exit(0);
	}
	
	if (totalEnemy == 0)
	{
		temp = 0;
	} else {
		temp = round(enemyKilled * 100 / totalEnemy);
	}
	sprintf(tempStr, "%s %d%%", miscText[63-1], temp);
	JE_outTextGlow(40, 90, tempStr);
	
	if (!constantPlay)
	{
		editorLevel += temp / 5;
	}
	
	JE_updateStream();
	if (netQuit)
		exit(0);
	
	if (!onePlayerAction && !twoPlayerMode)
	{
		JE_outTextGlow(30, 120, miscText[4-1]);   /*Cubes*/
		
		JE_updateStream();
		if (netQuit)
			exit(0);
		
		if (cubeMax > 0)
		{
			if (cubeMax > 4)
			{
				cubeMax = 4;
			}
			if (frameCountMax != 0)
			{
				frameCountMax = 1;
			}
			for (temp = 1; temp <= cubeMax; temp++)
			{
				JE_playSampleNum(18);
				x = 20 + 30 * temp;
				y = 135;
				JE_drawCube(x, y, 9, 0);
				JE_showVGA();
				JE_updateStream();
				if (netQuit)
					exit(0);
				
				for (i = -15; i <= 10; i++)
				{
					setjasondelay(frameCountMax);
					tempScreenSeg = VGAScreenSeg; /* sega000 */
					JE_newDrawCShapeAdjustNum(OPTION_SHAPES, 26, x, y, 9, i);
					if (JE_anyButton())
					{
						frameCountMax = 0;
					}
					JE_waitRetrace();
					JE_showVGA();
					
					JE_updateStream();
					if (netQuit)
						exit(0);
					
					int delaycount_temp;
					if ((delaycount_temp = target - SDL_GetTicks()) > 0)
						SDL_Delay(delaycount_temp);
				}
				for (i = 10; i >= 0; i--)
				{
					setjasondelay(frameCountMax);
					tempScreenSeg = VGAScreenSeg; /* sega000 */
					JE_newDrawCShapeAdjustNum(OPTION_SHAPES, 26, x, y, 9, i);
					if (JE_anyButton())
					{
						frameCountMax = 0;
					}
					JE_waitRetrace();
					JE_showVGA();
					
					JE_updateStream();
					if (netQuit)
						exit(0);
					
					int delaycount_temp;
					if ((delaycount_temp = target - SDL_GetTicks()) > 0)
						SDL_Delay(delaycount_temp);
				}
			}
		} else {
			JE_outTextGlow(50, 135, miscText[15-1]);
		}
		
	}

	JE_updateStream();
	if (netQuit)
		exit(0);
	
	if (frameCountMax != 0)
	{
		frameCountMax = 6;
		temp = 1;
	} else {
		temp = 0;
	}
	temp2 = twoPlayerMode ? 150 : 160;
	JE_outTextGlow(90, temp2, miscText[5-1]);
	JE_updateStream();
	if (netQuit)
		exit(0);
	
	if (!constantPlay)
	{
		do
		{
			setjasondelay(1);
			JE_waitRetrace();
			
			JE_updateStream();
			if (netQuit)
				exit(0);
			
			int delaycount_temp;
			if ((delaycount_temp = target - SDL_GetTicks()) > 0)
				SDL_Delay(delaycount_temp);
		} while (!(JE_anyButton() || (frameCountMax == 0 && temp == 1)));
	}
	
	/*Synchronize the network*/
	if (isNetworkGame)
	{
		tempScreenSeg = VGAScreenSeg; /* sega000 */
		
		frameCountMax = 0;
		JE_outTextGlow(10, 165, "Waiting for other player.");
		
		exchangeCount = 1;
		
		/* TODO
		REPEAT
			SetNetByte (249);
			framecount := 1;
			UpdateStream;
			IF NetQuit THEN
				EXIT;
			REPEAT
			UNTIL framecount = 0;
		UNTIL (InPacket^.Data [0] = 249) AND (UseOutPacket.Data [0] = 249);
		*/
	} else {
		if (isNetworkActive)
		{
			/* TODO ArenaPoll;*/
		}
	}
	
	JE_fadeBlack(15);
	JE_clr256();
}

void JE_drawCube( JE_word x, JE_word y, JE_byte filter, JE_byte brightness )
{
	JE_newDrawCShapeDarken((*shapeArray)[OPTION_SHAPES][26-1], shapeX[OPTION_SHAPES][26-1],
	  shapeY[OPTION_SHAPES][26 - 1], x + 4, y + 4);
	JE_newDrawCShapeDarken((*shapeArray)[OPTION_SHAPES][26-1], shapeX[OPTION_SHAPES][26-1],
	  shapeY[OPTION_SHAPES][26 - 1], x + 3, y + 3);
	JE_newDrawCShapeAdjustNum(OPTION_SHAPES, 26, x, y, filter, brightness);
}

void JE_handleChat( void )
{
	// STUB(); Annoying piece of crap =P
}

JE_boolean JE_getNumber( char *s, JE_byte *x )
{
	JE_boolean getNumber = false;
	JE_integer code;
	char buf[256];

	while (strlen(s) > 0)
	{
		if (s[0] == ' ')
		{
			strcpy(buf, s+1);
			strcpy(s, buf);
		} else {
			char *tmp;

			*x = strtol(s, &tmp, 10);
			if (s != tmp)
			{
				getNumber = true;
			}
			strcpy(buf, tmp);
			strcpy(s, buf);
			goto end_loop;
		}
	}

end_loop:
	return getNumber;
}

void JE_operation( JE_byte slot )
{
	JE_byte flash;
	JE_boolean quit;
	char stemp[21];
	char tempStr[51];
	
	if (!performSave) {
		if (saveFiles[slot-1].level > 0)
		{
			gameJustLoaded = true;
			JE_loadGame(slot);
			gameLoaded = true;
			quit = true;
			
			if (pItems[3-1] > 0)
			{
				onePlayerAction = true;
				if (pItems[3-1] == 255)
				{
					superTyrian = true;
				} else {
					superArcadeMode = pItems[3-1];
				}
			}
		}
	} else if (slot % 11 != 0) {
		quit = false;
		strcpy(stemp, "              ");
		memcpy(stemp, saveFiles[slot-1].name, strlen(saveFiles[slot-1].name));
		temp = strlen(stemp);
		while (stemp[temp-1] == ' ' && --temp);
		
		flash = 8 * 16 + 10;
		
		while (JE_mousePosition(&tempX, &tempY) != 0); /* TODO non-busy wait */
		
		JE_barShade(65, 55, 255, 155);
		
		do {
			JE_newDrawCShapeNum(OPTION_SHAPES, 36, 50, 50);
			
			JE_textShade(60, 55, miscText[1-1], 11, 4, DARKEN);
			JE_textShade(70, 70, levelName, 11, 4, DARKEN);
			
			do {
				
				flash = (flash == 8 * 16 + 10) ? 8 * 16 + 2 : 8 * 16 + 10;
				temp3 = (temp3 == 6) ? 2 : 6;
				
				strcpy(tempStr, miscText[2-1]);
				strncat(tempStr, stemp, temp);
				JE_outText(65, 89, tempStr, 8, 3);
				tempW = 65 + JE_textWidth(tempStr, TINY_FONT);
				JE_barShade(tempW + 2, 90, tempW + 6, 95);
				JE_bar(tempW + 1, 89, tempW + 5, 94, flash);
				
				JE_showVGA();
				
			} while (!JE_waitAction(14, false));
			
			if (mouseButton > 0)
			{
				if (mouseX > 56 && mouseX < 142 && mouseY > 123 && mouseY < 149)
				{
					quit = true;
					JE_saveGame(slot, stemp);
					JE_playSampleNum(SELECT);
				} else if (mouseX > 151 && mouseX < 237 && mouseY > 123 && mouseY < 149) {
					quit = true;
					JE_playSampleNum(ESC);
				}
			} else {
				
				if (newkey)
				{
					bool validkey = false;
					lastkey_char = toupper(lastkey_char);
					switch(lastkey_char)
					{
						case ' ':
						case '-':
						case '.':
						case ',':
						case ':':
						case '!':
						case '?':
						case '#':
						case '@':
						case '$':
						case '%':
						case '*':
						case '(':
						case ')':
						case '/':
						case '=':
						case '+':
						case '<':
						case '>':
						case ';':
						case '"':
							validkey = true;
						default:
							if (temp < 14 && (validkey || (lastkey_char >= 'A' && lastkey_char <= 'Z') || (lastkey_char >= '0' && lastkey_char <= '9')))
							{
								JE_playSampleNum(CURSOR_MOVE);
								stemp[temp] = lastkey_char;
								temp++;
							}
							break;
						case 8:
							if (temp)
							{
								temp--;
								stemp[temp] = ' ';
								JE_playSampleNum(CLICK);
							}
							break;
						case 27:
							quit = true;
							JE_playSampleNum(ESC);
							break;
						case 13:
							quit = true;
							JE_saveGame(slot, stemp);
							drawGameSaved = true;
							JE_playSampleNum(SELECT);
							break;
					}
				}
				
		  }
		
		} while (!quit);
	}
	
	while (JE_mousePosition(&tempX, &tempY) != 0); /* TODO non-busy wait */
}

void JE_inGameDisplays( void )
{
	char stemp[21];
	JE_byte temp;
	
	char tempstr[256];

	sprintf(tempstr, "%d", score);
	JE_textShade(30, 175, tempstr, 2, 4, FULL_SHADE);
	if (twoPlayerMode && !galagaMode)
	{
		sprintf(tempstr, "%d", score2);
		JE_textShade(230, 175, tempstr, 2, 4, FULL_SHADE);
	}

	/*Special Weapon?*/
	if (pItems[11-1] > 0)
	{
		JE_drawShape2x2(25, 1, special[pItems[11-1]].itemgraphic, eShapes6);
	}

	/*Lives Left*/
	if (onePlayerAction || twoPlayerMode)
	{
		
		for (temp = 0; temp < (onePlayerAction ? 1 : 2); temp++)
		{
			temp5 = (temp == 0 && pItems[11-1] > 0) ? 35 : 15;
			tempW = (temp == 0) ? 30: 270;
			
			if (portPower[temp] > 5)
			{
				JE_drawShape2(tempW, temp5, 285, shapes9);
				tempW = (temp == 0) ? 45 : 250;
				sprintf(tempstr, "%d", portPower[temp] - 1);
				JE_textShade(tempW, temp5 + 3, tempstr, 15, 1, FULL_SHADE);
			} else if (portPower[temp] > 1) {
				for (temp2 = 1; temp2 < portPower[temp]; temp2++)
				{
					JE_drawShape2(tempW, temp5, 285, shapes9);
					tempW = (temp == 0) ? (tempW + 12) : (tempW - 12);
				}
			}
			
			strcpy(stemp, (temp == 0) ? miscText[49-1] : miscText[50-1]);
			if (isNetworkGame)
			{
				strcpy(stemp, JE_getName(temp+1));
			}
			
			tempW = (temp == 0) ? 28 : (285 - JE_textWidth(stemp, TINY_FONT));
			JE_textShade(tempW, temp5 - 7, stemp, 2, 6, FULL_SHADE);
			
		}
		
	}

	/*Super Bombs!!*/
	for (temp = 0; temp < 2; temp++)
	{
		if (superBomb[temp] > 0)
		{
			tempW = (temp == 0) ? 30 : 270;
			
			for (temp2 = 0; temp2 < superBomb[temp]; temp2++)
			{
				JE_drawShape2(tempW, 160, 304, shapes9);
				tempW = (temp == 0) ? (tempW + 12) : (tempW - 12);
			}
		}
	}

	if (youAreCheating)
	{
		JE_outText(90, 170, "Cheaters always prosper.", 3, 4);
	}
}

void JE_mainKeyboardInput( void )
{
	tempB = JE_gammaCheck();

	/* { Network Request Commands } */

	if (!isNetworkGame || gameQuitDelay == 0)
	{
		/* { Edited Ships } for Player 1 */
		if (extraAvail && keysactive[SDLK_TAB] && !isNetworkGame && !superTyrian)
		{
			for (x = 0; x < 10; x++)
			{
				if (0 /* keyactive[x+1] The number keys, TODO: doesn't exactly work like that under SDL */)
				{
					/* TODO */
				}
			}
		}

		/* for Player 2 */
		if (extraAvail && keysactive[SDLK_CAPSLOCK] && !isNetworkGame && !superTyrian)
		{
			for (x = 0; x < 10; x++)
			{
				if (0 /* keyactive[x+1] The number keys, TODO: doesn't exactly work like that under SDL */)
				{
					/* TODO */
				}
			}
		}
	}

	/* { In-Game Help } */
	if (keysactive[SDLK_F1])
	{
		if (isNetworkGame)
		{
			helpRequest = true;
		} else {
			JE_inGameHelp();
			skipStarShowVGA = true;
		}
	}

	/* {!Activate Nort Ship!} */
	if (keysactive[SDLK_F2] && keysactive[SDLK_F4] && keysactive[SDLK_F6] && keysactive[SDLK_F7] &&
	    keysactive[SDLK_F9] /*&& keysactive[43] missing from scancode table??? TODO*/ && keysactive[SDLK_SLASH])
	{
		if (isNetworkGame)
		{
			nortShipRequest = true;
		} else {
			pItems[11] = 12;
			pItems[10] = 13;
			pItems[1] = 36;
			pItems[2] = 37;
			shipGr = 1;
		}
	}

	/* {Cheating} */
	if (!isNetworkGame && !twoPlayerMode && !superTyrian && superArcadeMode == 0)
	{
		if (keysactive[SDLK_F2] && keysactive[SDLK_F3] && keysactive[SDLK_F6])
		{
			youAreCheating = !youAreCheating;
			keysactive[SDLK_F2] = false;
		}

		if (keysactive[SDLK_F2] && keysactive[SDLK_F3] && (keysactive[SDLK_F4] || keysactive[SDLK_F5]) && !superTyrian)
		{
			armorLevel = 0;
			armorLevel2 = 0;
			youAreCheating = !youAreCheating;
			JE_drawTextWindow(miscText[63-1]);
		}

		if (constantPlay && keysactive[SDLK_c] && !superTyrian && superArcadeMode == 0)
		{
			youAreCheating = !youAreCheating;
			keysactive[SDLK_c] = false;
		}
	}

	if (superTyrian)
	{
		youAreCheating = false;
	}

	/* {Personal Commands} */

	/* {DEBUG} */
	if (keysactive[SDLK_F10] && keysactive[SDLK_BACKSPACE])
	{
		keysactive[SDLK_F10] = false;
		debug = !debug;

		debugHist = 1;
		debugHistCount = 1;

		/* YKS: clock ticks since midnight replaced by SDL_GetTicks */
		lastDebugTime = SDL_GetTicks();
	}

	/* {CHEAT-SKIP LEVEL} */
	if (keysactive[SDLK_F2] && keysactive[SDLK_F6] && (keysactive[SDLK_F7] || keysactive[SDLK_F8]) &&
	    !keysactive[SDLK_F9] && !superTyrian && superArcadeMode == 0)
	{
		if (isNetworkGame)
		{
			skipLevelRequest = true;
		} else {
			levelTimer = true;
			levelTimerCountdown = 0;
			endLevel = true;
			levelEnd = 40;
		}
	}

	/* { Pause Game } */
	if (keysactive[SDLK_p])
	{
		if (isNetworkGame)
		{
			pauseRequest = true;
		} else {
			JE_pauseGame();
		}
	}

	/* {In-Game Setup} */
	if (keysactive[SDLK_ESCAPE])
	{
		yourInGameMenuRequest = true;
		if (isNetworkGame)
		{
			inGameMenuRequest = true;
		} else {
			JE_doInGameSetup();
			skipStarShowVGA = true;
		}
	}

	/* {MUTE SOUND} */
	if (keysactive[SDLK_s])
	{
		keysactive[SDLK_s] = false;
		soundActive = !soundActive;
		if (soundActive)
		{
			JE_drawTextWindow(miscText[18]);
		} else {
			JE_drawTextWindow(miscText[17]);
		}
		JE_setNewGameVol();
	}

	/* {MUTE MUSIC} */
	if (keysactive[SDLK_m])
	{
		keysactive[SDLK_m] = false;
		musicActive = !musicActive;
		if (musicActive)
		{
			JE_drawTextWindow(miscText[36]);
			JE_selectSong(2);
		} else {
			JE_drawTextWindow(miscText[35]);
			JE_stopSong();
		}
	}

	if (keysactive[SDLK_BACKSPACE])
	{
		/* {SCREENSHOT PAUSE} */
		if (keysactive[SDLK_NUMLOCK])
		{
			superPause = !superPause;
		}

		/* {SMOOTHIES} */
		if (keysactive[SDLK_F12] && keysactive[SDLK_SCROLLOCK])
		{
			for (temp = SDLK_2; temp <= SDLK_9; temp++)
			{
				if (keysactive[temp])
				{
					smoothies[temp-SDLK_2] = !smoothies[temp-SDLK_2];
				}
			}
			if (keysactive[SDLK_0])
			{
				smoothies[8] = !smoothies[8];
			}
		} else

		/* {CYCLE THROUGH FILTER COLORS} */
		if (keysactive[SDLK_MINUS])
		{
			if (levelFilter == -99)
			{
				levelFilter = 0;
			} else {
				levelFilter++;
				if (levelFilter == 16)
				{
					levelFilter = -99;
				}
			}
		} else

		/* {HYPER-SPEED} */
		if (keysactive[SDLK_1])
		{
			fastPlay++;
			if (fastPlay > 2)
			{
				fastPlay = 0;
			}
			keysactive[SDLK_1] = false;
			JE_setNewGameSpeed();
		}

		/* {IN-GAME RANDOM MUSIC SELECTION} */
		if (keysactive[SDLK_SCROLLOCK])
		{
			tempW = (rand() % MUSIC_NUM)+1;
			JE_playSong(tempW);
		}
	}

	newkey = false;
}

void JE_pauseGame( void )
{
	JE_boolean done;
	JE_word mouseX, mouseY;
	
	SDL_Surface *temp_surface;

	tempScreenSeg = VGAScreenSeg; // sega000
	if (!superPause)
	{
		JE_dString(120, 90, miscText[22], FONT_SHAPES);
		
		temp_surface = VGAScreen;  /* side-effect of game_screen */
		VGAScreen = VGAScreenSeg;
		JE_showVGA();
		VGAScreen = temp_surface;
	}
	JE_setVol((tyrMusicVolume >> 1) + 10, fxVolume);

	newkey = false;

	done = false;

	if (isNetworkGame)
	{
		JE_setNetByte(0);
		// outPacket->length = 2; TODO
	}

	newkey = false;

	do
	{
		setjasondelay(2);

		JE_joystick2();

		if (isNetworkActive)
		{
			if (!isNetworkGame)
			{
				// JE_arenaPoll(); TODO
			}
			JE_handleChat();
		}

		if (superPause)
		{
			if ((newkey && (lastkey_sym != SDLK_F11) && !(lastkey_sym == SDLK_LALT) && !(lastkey_sym == SDLK_c) && !(lastkey_sym == SDLK_SPACE)) || (JE_mousePosition(&mouseX, &mouseY) > 0) || button[1] || button[2] || button[3]) 
			{
				if (isNetworkGame)
				{
					JE_setNetByte(10);
					// outPacket->length = 2; TODO
				} else {
					done = true;
				}
			}
		} else {
			if ((newkey && lastkey_sym != SDLK_F11) || JE_mousePosition(&mouseX, &mouseY) > 0 || button[0] || button[1] || button[2] || button[3])
			{
				if (isNetworkGame)
				{
					JE_setNetByte(10);
					// outPacket->length = 2; TODO
				} else {
					done = true;
				}
			}
		}

		if (isNetworkGame)
		{
			JE_updateStream();
			if (netQuit)
			{
				JE_tyrianHalt(6);
			}
			done = JE_scanNetByte(10);
			JE_setNetByte(0);
			// outPacket->length = 2; TODO
		}

		if (lastkey_sym == SDLK_p)
		{
			repause = true;
		}

		wait_delay();
	} while (!done);

	JE_setVol(tyrMusicVolume, fxVolume);

	// gameQuitDelay = streamLagFrames + 4; TODO
	tempScreenSeg = VGAScreen;
	skipStarShowVGA = true;
}

void JE_playerMovement( JE_byte inputDevice_,
                        JE_byte playerNum_,
                        JE_word shipGr_,
                        JE_byte *shapes9ptr_,
                        JE_integer *armorLevel_, JE_integer *baseArmor_,
                        JE_shortint *shield_, JE_shortint *shieldMax_,
                        JE_word *playerInvulnerable_,
                        JE_integer *PX_, JE_integer *PY_,
                        JE_integer *lastPX_, JE_integer *lastPY_,
                        JE_integer *lastPX2_, JE_integer *lastPY2_,
                        JE_integer *PXChange_, JE_integer *PYChange_,
                        JE_integer *lastTurn_, JE_integer *lastTurn2_, JE_integer *tempLastTurn2_,
                        JE_byte *stopWaitX_, JE_byte *stopWaitY_,
                        JE_word *mouseX_, JE_word *mouseY_,
                        JE_boolean *playerAlive_,
                        JE_byte *playerStillExploding_,
                        JE_PItemsType pItems_ )
{
	JE_integer mouseXC, mouseYC;
	JE_integer accelXC, accelYC;
	JE_byte leftOptionIsSpecial = 0;
	JE_byte rightOptionIsSpecial = 0;

	if (playerNum_ == 2 || !twoPlayerMode)
	{
		if (playerNum_ == 2)
		{
			tempW = weaponPort[pItemsPlayer2[2-1]].opnum;
		} else {
			tempW = weaponPort[pItems_[2-1]].opnum;
		}
		if (portConfig[2-1] > tempW)
		{
			portConfig[2-1] = 1;
		}
	}

redo:

	if (isNetworkGame && thisPlayerNum == playerNum_)
	{
		/* TODO
		Outpacket^.Data [0] := 0;
		Outpacket^.Data [1] := 0;
		Outpacket^.Data [2] := 0;
		Outpacket^.Data [3] := 0;
		Outpacket^.Data [4] := 0;
		*/
	}

	if (isNetworkGame)
	{
		inputDevice_ = 0;
	}

	mouseXC = 0;
	mouseYC = 0;
	accelXC = 0;
	accelYC = 0;

	/* Draw Player */
	if (!*playerAlive_)
	{
		if (*playerStillExploding_ > 0)
		{

			(*playerStillExploding_)--;

			if (levelEndFxWait > 0)
			{
				levelEndFxWait--;
			} else {
				levelEndFxWait = (rand() % 6) + 3;
				if ((rand() % 3) == 1)
					soundQueue[6] = 9;
				else
					soundQueue[5] = 11;
			}
			tempW = *PX_ + (rand() % 32) - 16;
			tempW2 = *PY_ + (rand() % 32) - 16;

			JE_setupExplosionLarge(false, 0, *PX_ + (rand() % 32) - 16, *PY_ + (rand() % 32) - 16 + 7);
			JE_setupExplosionLarge(false, 0, *PX_, *PY_ + 7);

			if (levelEnd > 0)
				levelEnd--;
		} else {

			if (twoPlayerMode || onePlayerAction)
			{
				if (portPower[playerNum_-1] > 1)
				{
					reallyEndLevel = false;
					shotMultiPos[playerNum_-1] = 0;
					portPower[playerNum_-1]--;
					JE_calcPurpleBall(playerNum_);
					twoPlayerLinked = false;
					if (galagaMode)
						twoPlayerMode = false;
					*PY_ = 160;
					*playerInvulnerable_ = 100;
					*playerAlive_ = true;
					endLevel = false;

					if (galagaMode || episodeNum == 4)
						*armorLevel_ = *baseArmor_;
					else
						*armorLevel_ = *baseArmor_ / 2;

					if (galagaMode)
						*shield_ = 0;
					else
						*shield_ = *shieldMax_ / 2;

					VGAScreen = VGAScreenSeg; /* side-effect of game_screen */
					JE_drawArmor();
					JE_drawShield();
					VGAScreen = game_screen; /* side-effect of game_screen */
					goto redo;
				} else {
					if (galagaMode)
						twoPlayerMode = false;
					if (allPlayersGone && isNetworkGame)
						reallyEndLevel = true;
				}

			}
		}
	} else
	if (constantDie)
	{
		if (*playerStillExploding_ == 0)
		{

			*shield_ = 0;
			if (*armorLevel_ > 0)
				(*armorLevel_)--;
			else {
				gameQuitDelay = 60;
				*playerAlive_ = false;
				*playerStillExploding_ = 60;
				levelEnd = 40;
			}

			JE_wipeShieldArmorBars();
			VGAScreen = VGAScreenSeg; /* side-effect of game_screen */
			JE_drawArmor();
			VGAScreen = game_screen; /* side-effect of game_screen */
			if (portPower[1-1] < 11)
				portPower[1-1]++;
		}
	}


	if (*playerAlive_)
	{  /*Global throughout entire remaining part of routine*/
		if (!endLevel)
		{

			*mouseX_ = *PX_;
			*mouseY_ = *PY_;
			button[1-1] = false;
			button[2-1] = false;
			button[3-1] = false;
			button[4-1] = false;

			/* --- Movement Routine Beginning --- */

			if (!isNetworkGame || playerNum_ == thisPlayerNum)
			{

				if (endLevel)
					*PY_ -= 2;
				else {
					service_SDL_events(false);
					/* ---- Mouse Input ---- */
					if ((inputDevice_ == 2 || inputDevice_ == 0)
					    && !playDemo && mouseInstalled && !recordDemo)
					{
						/* Get Mouse Status */
						button[1-1] = mouse_pressed[0];
						button[2-1] = mouse_pressed[1];
						button[3-1] = mouse_threeButton ? mouse_pressed[2] : mouse_pressed[1];

#ifdef NDEBUG
						mouseXC = mouse_x - 159;
						mouseYC = mouse_y - 100;
#endif

						if (( inputDevice_ == 2 || inputDevice_ == 0 ) && mouseInstalled
						    && (!isNetworkGame || playerNum_ == thisPlayerNum)
						    && (!galagaMode || (playerNum_ == 2 || !twoPlayerMode || playerStillExploding2 > 0)))
							JE_setMousePosition(159, 100);

					}

					memcpy(tempButton, button, sizeof(button));

					/* End of Mouse Routine */

					/* ==== Joystick Input ==== */
					if ((inputDevice_ == 3 || inputDevice_ == 0)
					    && joystick_installed && !playDemo && !recordDemo)
					{  /* Start of Joystick Routine */
						JE_joystick1();
						JE_updateButtons();

						button[1-1] = button[1-1] | tempButton[1-1];
						button[2-1] = button[2-1] | tempButton[2-1];
						button[3-1] = button[3-1] | tempButton[3-1];

						if (joyY < jCenterY / 2)
							*PY_ -= CURRENT_KEY_SPEED;
						if (joyY > jCenterY + jCenterY / 2)
							*PY_ += CURRENT_KEY_SPEED;

						if (joyX < jCenterX / 2)
							*PX_ -= CURRENT_KEY_SPEED;
						if (joyX > jCenterX + jCenterX / 2)
							*PX_ += CURRENT_KEY_SPEED;
					}


					/* ---- Keyboard Input ---- */
					if (inputDevice_ == 1 || inputDevice_ == 0)
					{

						if (playDemo)
						{
							if (feof(recordFile))
							{
								endLevel = true;
								levelEnd = 40;
							} else {

								JE_doDemoKeys();
								if (lastMoveWait > 1)
									lastMoveWait--;
								else {
									JE_readDemoKeys();
								}

							}

						} else {
							if (keysactive[keySettings[1-1]])
								*PY_ -= CURRENT_KEY_SPEED;
							if (keysactive[keySettings[2-1]])
								*PY_ += CURRENT_KEY_SPEED;

							if (keysactive[keySettings[3-1]])
								*PX_ -= CURRENT_KEY_SPEED;
							if (keysactive[keySettings[4-1]])
								*PX_ += CURRENT_KEY_SPEED;

							if (keysactive[keySettings[5-1]])
								button[1-1] = true;
							if (keysactive[keySettings[6-1]])
								button[4-1] = true;
							if (keysactive[keySettings[7-1]])
								button[2-1] = true;
							if (keysactive[keySettings[8-1]])
								button[3-1] = true;
						}

						if (constantPlay)
						{
							button[1-1] = true;
							button[2-1] = true;
							button[3-1] = true;
							button[4-1] = true;
							(*PY_)++;
							*PX_ += constantLastX;
						}


						if (recordDemo)
						{
							tempB = false;
							for (temp = 0; temp < 8; temp++)
								if (lastKey[temp] != keysactive[keySettings[temp]])
									tempB = true;

							lastMoveWait++;
							if (tempB)
							{
								fputc(lastMoveWait >> 8, recordFile);
								fputc(lastMoveWait && 0xff, recordFile);

								for (temp = 0; temp < 8; temp++)
									lastKey[temp] = keysactive[keySettings[temp]];
								temp = (lastKey[1-1]     ) + (lastKey[2-1] << 1) + (lastKey[3-1] << 2) + (lastKey[4-1] << 3) +
								       (lastKey[5-1] << 4) + (lastKey[6-1] << 5) + (lastKey[7-1] << 6) + (lastKey[8-1] << 7);

								fputc(temp, recordFile);

								lastMoveWait = 0;
							}
						}
					}

					if (smoothies[9-1])
					{
						*mouseY_ = *PY_ - (*mouseY_ - *PY_);
						mouseYC = -mouseYC;
					}

					accelXC += *PX_ - *mouseX_;
					accelYC += *PY_ - *mouseY_;

					if (mouseXC > 30)
						mouseXC = 30;
					else
						if (mouseXC < -30)
							mouseXC = -30;
					if (mouseYC > 30)
						mouseYC = 30;
					else
						if (mouseYC < -30)
							mouseYC = -30;

					if (mouseXC > 0)
						*PX_ += (mouseXC + 3 ) / 4;
					else
						if (mouseXC < 0)
							*PX_ += (mouseXC - 3 ) / 4;
					if (mouseYC > 0)
						*PY_ += (mouseYC + 3 ) / 4;
					else
						if (mouseYC < 0)
							*PY_ += (mouseYC - 3 ) / 4;

					if (makeMouseDelay)
					{
						if (mouseXC > 3)
							accelXC++;
						else
							if (mouseXC < -2)
								accelXC--;
						if (mouseYC > 2)
							accelYC++;
						else
							if (mouseYC < -2)
								accelYC--;
					}

				}   /*endLevel*/

				if (isNetworkGame && playerNum_ == thisPlayerNum)
				{
					/* TODO */;
				}
			}  /*isNetworkGame*/

			/* --- Movement Routine Ending --- */

			moveOk = true;

			if (isNetworkGame)
			{
				/* TODO */;
			}

			/*Street-Fighter codes*/
			JE_SFCodes(playerNum_, *PX_, *PY_, *mouseX_, *mouseY_, pItems_);

			if (moveOk)
			{

				/* END OF MOVEMENT ROUTINES */

				/*Linking Routines*/

				if (twoPlayerMode && !twoPlayerLinked && *PX_ == *mouseX_ && *PY_ == *mouseY_
				    && abs(PX - PXB) < 8 && abs(PY - PYB) < 8
				    && playerAlive && playerAliveB && !galagaMode)
				{
					twoPlayerLinked = true;
				}

				if (playerNum_ == 1 && (button[3-1] || button[2-1]) && !galagaMode)
					twoPlayerLinked = false;

				if (twoPlayerMode && twoPlayerLinked && playerNum_ == 2
				    && (*PX_ != *mouseX_ || *PY_ != *mouseY_))
				{

					if (button[1-1])
					{
						if (abs(*PX_ - *mouseX_) > abs(*PY_ - *mouseY_))
						{
							if (*PX_ - *mouseX_ > 0)
								tempR = 1.570796;
							else
								tempR = 4.712388;
						} else {
							if (*PY_ - *mouseY_ > 0)
								tempR = 0;
							else
								tempR = 3.14159265;
						}

						tempR2 = linkGunDirec - tempR;

						if (fabs(linkGunDirec - tempR) < 0.3)
							linkGunDirec = tempR;
						else {

							if (linkGunDirec < tempR && linkGunDirec - tempR > -3.24)
								linkGunDirec += 0.2;
							else
								if (linkGunDirec - tempR < M_PI)
									linkGunDirec -= 0.2;
								else
									linkGunDirec += 0.2;

						}

						if (linkGunDirec >= 2 * M_PI)
							linkGunDirec -= 2 * M_PI;
						if (linkGunDirec <  0)
							linkGunDirec += 2 * M_PI;

					} else
					if (!galagaMode)
						twoPlayerLinked = false;
				}

				leftOptionIsSpecial  = options[option1Item].tr;
				rightOptionIsSpecial = options[option2Item].tr;

			} /*if (*playerAlive_) ...*/
		} /*if (!endLevel) ...*/


		if (levelEnd > 0 &&
		    !*playerAlive_ && (!twoPlayerMode || !playerAliveB))
			reallyEndLevel = true;
		/* End Level Fade-Out */
		if (*playerAlive_ && endLevel)
		{

			if (levelEnd == 0)
				reallyEndLevel = true;
			else {
				*PY_ -= levelEndWarp;
				if (*PY_ < -200)
					reallyEndLevel = true;

				tempI = 1;
				tempW2 = *PY_;
				tempI2 = abs(41 - levelEnd);
				if (tempI2 > 20)
					tempI2 = 20;

				for (z = 1; z <= tempI2; z++)
				{
					tempW2 += tempI;
					tempI++;
				}

				for (z = 1; z <= tempI2; z++)
				{
					tempW2 -= tempI;
					tempI--;
					if (tempW2 > 0 && tempW2 < 170)
					{
						if (shipGr_ == 0)
						{
							JE_drawShape2x2(*PX_ - 17, tempW2 - 7, 13, shapes9ptr_);
							JE_drawShape2x2(*PX_ + 7 , tempW2 - 7, 51, shapes9ptr_);
						} else
							if (shipGr_ == 1)
							{
								JE_drawShape2x2(*PX_ - 17, tempW2 - 7, 220, shapes9ptr_);
								JE_drawShape2x2(*PX_ + 7 , tempW2 - 7, 222, shapes9ptr_);
							} else
								JE_drawShape2x2(*PX_ - 5, tempW2 - 7, shipGr_, shapes9ptr_);
					}
				}

			}
		}

		if (playDemo)
			JE_dString(115, 10, miscText[8-1], SMALL_FONT_SHAPES);

		if (*playerAlive_ && !endLevel)
		{

			if (!twoPlayerLinked || playerNum_ < 2)
			{

				if (!twoPlayerMode || shipGr2 != 0)
				{
					option1X = *mouseX_ - 14;
					option1Y = *mouseY_;

					if (rightOptionIsSpecial == 0)
					{
						option2X = *mouseX_ + 16;
						option2Y = *mouseY_;
					}
				}

				if (*stopWaitX_ > 0)
					(*stopWaitX_)--;
				else {
					*stopWaitX_ = 2;
					if (*lastTurn_ < 0)
						(*lastTurn_)++;
					else
						if (*lastTurn_ > 0)
							(*lastTurn_)--;
				}

				if (*stopWaitY_ > 0)
					(*stopWaitY_)--;
				else {
					*stopWaitY_ = 1;
					if (*lastTurn2_ < 0)
						(*lastTurn2_)++;
					else
						if (*lastTurn2_ > 0)
							(*lastTurn2_)--;
				}

				*lastTurn_ += accelYC;
				*lastTurn2_ += accelXC;

				if (*lastTurn2_ < -4)
					*lastTurn2_ = -4;
				if (*lastTurn2_ > 4)
					*lastTurn2_ = 4;
				if (*lastTurn_ < -4)
					*lastTurn_ = -4;
				if (*lastTurn_ > 4)
					*lastTurn_ = 4;

				*PX_ += *lastTurn2_;
				*PY_ += *lastTurn_;


				/*Option History for special new sideships*/
				if (playerHNotReady)
					JE_resetPlayerH();
				else
					if ((playerNum_ == 1 && !twoPlayerMode)
					    || (playerNum_ == 2 && twoPlayerMode))
						if (*PX_ - *mouseX_ != 0 || *PY_ - *mouseY_ != 0)
						{ /*Option History*/
							for (temp = 0; temp < 19; temp++)
							{
								playerHX[temp] = playerHX[temp + 1];
								playerHY[temp] = playerHY[temp + 1];
							}
							playerHX[20-1] = *PX_;
							playerHY[20-1] = *PY_;
						}

			} else {  /*twoPlayerLinked*/
				if (shipGr_ == 0)
					*PX_ = PX - 1;
				else
					*PX_ = PX;
				*PY_ = PY + 8;
				*lastTurn2_ = lastTurn2;
				*lastTurn_ = 4;

				shotMultiPos[5-1] = 0;
				JE_initPlayerShot(0, 5, *PX_ + 1 + round(sin(linkGunDirec + 0.2) * 26), *PY_ + round(cos(linkGunDirec + 0.2) * 26),
				                  *mouseX_, *mouseY_, 148, playerNum_);
				shotMultiPos[5-1] = 0;
				JE_initPlayerShot(0, 5, *PX_ + 1 + round(sin(linkGunDirec - 0.2) * 26), *PY_ + round(cos(linkGunDirec - 0.2) * 26),
				                  *mouseX_, *mouseY_, 148, playerNum_);
				shotMultiPos[5-1] = 0;
				JE_initPlayerShot(0, 5, *PX_ + 1 + round(sin(linkGunDirec) * 26), *PY_ + round(cos(linkGunDirec) * 26),
				                  *mouseX_, *mouseY_, 147, playerNum_);

				if (shotRepeat[2-1] > 0)
					shotRepeat[2-1]--;
				else
					if (button[1-1])
					{
						shotMultiPos[2-1] = 0;
						JE_initPlayerShot(0, 2, *PX_ + 1 + round(sin(linkGunDirec) * 20), *PY_ + round(cos(linkGunDirec) * 20),
						                  *mouseX_, *mouseY_, linkGunWeapons[pItems_[2-1]-1], playerNum_);
						playerShotData[b].shotXM = -round(sin(linkGunDirec) * playerShotData[b].shotYM);
						playerShotData[b].shotYM = -round(cos(linkGunDirec) * playerShotData[b].shotYM);

						switch (pItems_[2-1])
						{
							case 27:
							case 32:
							case 10:
								temp = round(linkGunDirec * 2.54647908948);  /*16 directions*/
								playerShotData[b].shotGr = linkMultiGr[temp];
								break;
							case 28:
							case 33:
							case 11:
								temp = round(linkGunDirec * 2.54647908948);  /*16 directions*/
								playerShotData[b].shotGr = linkSonicGr[temp];
								break;
							case 30:
							case 35:
							case 14:
								if (linkGunDirec > M_PI_2 && linkGunDirec < M_PI + M_PI / 2)
								{
									playerShotData[b].shotYC = 1;
								}
								break;
							case 38:
							case 22:
								temp = round(linkGunDirec * 2.54647908948);  /*16 directions*/
								playerShotData[b].shotGr = linkMult2Gr[temp];
								break;
						}

					}
			}

		}  /*moveOK*/

		if (!endLevel)
		{
			if (*PX_ > 256)
			{
				*PX_ = 256;
				constantLastX = -constantLastX;
			}
			if (*PX_ < 40)
			{
				*PX_ = 40;
				constantLastX = -constantLastX;
			}

			if (isNetworkGame && playerNum_ == 1)
			{
				if (*PY_ > 154)
					*PY_ = 154;
			} else {
				if (*PY_ > 160)
					*PY_ = 160;
			}

			if (*PY_ < 10)
				*PY_ = 10;

			tempI2 = *lastTurn2_ / 2;
			tempI2 += (*PX_ - *mouseX_) / 6;

			if (tempI2 < -2)
				tempI2 = -2;
			else
				if (tempI2 > 2)
					tempI2 = 2;


			tempI  = tempI2 * 2 + shipGr_;

			tempI4 = *PY_ - *lastPY2_;
			if (tempI4 < 1)
				tempI4 = 0;
			explosionFollowAmount = *PX_ - *lastPX2_ + tempI4 * 320;
			*lastPX2_ = *PX_;
			*lastPY2_ = *PY_;

			if (shipGr_ == 0)
			{
				if (background2)
				{
					JE_drawShape2x2Shadow(*PX_ - 17 - mapX2Ofs + 30, *PY_ - 7 + shadowyDist, tempI + 13, shapes9ptr_);
					JE_drawShape2x2Shadow(*PX_ + 7 - mapX2Ofs + 30, *PY_ - 7 + shadowyDist, tempI + 51, shapes9ptr_);
					if (superWild)
					{
						JE_drawShape2x2Shadow(*PX_ - 16 - mapX2Ofs + 30, *PY_ - 7 + shadowyDist, tempI + 13, shapes9ptr_);
						JE_drawShape2x2Shadow(*PX_ + 6 - mapX2Ofs + 30, *PY_ - 7 + shadowyDist, tempI + 51, shapes9ptr_);
					}
				}
			} else
				if (shipGr_ == 1)
				{
					if (background2)
					{
						JE_drawShape2x2Shadow(*PX_ - 17 - mapX2Ofs + 30, *PY_ - 7 + shadowyDist, 220, shapes9ptr_);
						JE_drawShape2x2Shadow(*PX_ + 7 - mapX2Ofs + 30, *PY_ - 7 + shadowyDist, 222, shapes9ptr_);
					}
				} else {
					if (background2)
					{
						JE_drawShape2x2Shadow(*PX_ - 5 - mapX2Ofs + 30, *PY_ - 7 + shadowyDist, tempI, shapes9ptr_);
						if (superWild)
						{
							JE_drawShape2x2Shadow(*PX_ - 4 - mapX2Ofs + 30, *PY_ - 7 + shadowyDist, tempI, shapes9ptr_);
						}
					}
				}

			if (*playerInvulnerable_ > 0)
			{
				(*playerInvulnerable_)--;

				if (shipGr_ == 0)
				{
					JE_superDrawShape2x2(*PX_ - 17, *PY_ - 7, tempI + 13, shapes9ptr_);
					JE_superDrawShape2x2(*PX_ + 7 , *PY_ - 7, tempI + 51, shapes9ptr_);
				} else
					if (shipGr_ == 1)
					{
						JE_superDrawShape2x2(*PX_ - 17, *PY_ - 7, 220, shapes9ptr_);
						JE_superDrawShape2x2(*PX_ + 7 , *PY_ - 7, 222, shapes9ptr_);
					} else
						JE_superDrawShape2x2(*PX_ - 5, *PY_ - 7, tempI, shapes9ptr_);

			} else {
				if (shipGr_ == 0)
				{
					JE_drawShape2x2(*PX_ - 17, *PY_ - 7, tempI + 13, shapes9ptr_);
					JE_drawShape2x2(*PX_ + 7, *PY_ - 7, tempI + 51, shapes9ptr_);
				} else
					if (shipGr_ == 1)
					{
						JE_drawShape2x2(*PX_ - 17, *PY_ - 7, 220, shapes9ptr_);
						JE_drawShape2x2(*PX_ + 7, *PY_ - 7, 222, shapes9ptr_);
						switch (tempI)
						{
							case 5:
								JE_drawShape2(*PX_ - 17, *PY_ + 7, 40, shapes9ptr_);
								tempW = *PX_ - 7;
								tempI2 = -2;
								break;
							case 3:
								JE_drawShape2(*PX_ - 17, *PY_ + 7, 39, shapes9ptr_);
								tempW = *PX_ - 7;
								tempI2 = -1;
								break;
							case 1:
								tempI2 = 0;
								break;
							case -1:
								JE_drawShape2(*PX_ + 19, *PY_ + 7, 58, shapes9ptr_);
								tempW = *PX_ + 9;
								tempI2 = 1;
								break;
							case -3:
								JE_drawShape2(*PX_ + 19, *PY_ + 7, 59, shapes9ptr_);
								tempW = *PX_ + 9;
								tempI2 = 2;
								break;
						}
						if (tempI2 != 0)
						{  /*NortSparks*/
							if (shotRepeat[10-1] > 0)
								shotRepeat[10-1]--;
							else {
									JE_initPlayerShot(0, 10, tempW + (rand() % 8) - 4, (*PY_) + (rand() % 8) - 4, *mouseX_, *mouseY_, 671, 1);
									shotRepeat[10-1] = abs(tempI2) - 1;
							}
						}
					} else
						JE_drawShape2x2(*PX_ - 5, *PY_ - 7, tempI, shapes9ptr_);
			}
		}  /*endLevel*/

		/*Options Location*/
		if (playerNum_ == 2 && shipGr_ == 0)
		{
			if (rightOptionIsSpecial == 0)
			{
				option2X = *PX_ + 17 + tempI;
				option2Y = *PY_;
			}

			option1X = *PX_ - 14 + tempI;
			option1Y = *PY_;
		}

		if (moveOk)
		{

			if (*playerAlive_)
			{
				if (!endLevel)
				{
					*PXChange_ = *PX_ - lastPXShotMove;
					*PYChange_ = *PY_ - lastPYShotMove;

					/* PLAYER SHOT Change */
					if (button[4-1])
					{
						portConfigChange = true;
						if (portConfigDone)
						{

							shotMultiPos[2-1] = 0;

							if (superArcadeMode > 0 && superArcadeMode <= SA)
							{
								shotMultiPos[9-1] = 0;
								shotMultiPos[11-1] = 0;
								if (pItems[11-1] == SASpecialWeapon[superArcadeMode-1])
								{
									pItems[11-1] = SASpecialWeaponB[superArcadeMode-1];
									portConfig[2-1] = 2;
								} else {
									pItems[11-1] = SASpecialWeapon[superArcadeMode-1];
									portConfig[2-1] = 1;
								}
							} else {
								portConfig[2-1]++;
								JE_portConfigs();
								if (portConfig[2-1] > tempW)
								{
									portConfig[2-1] = 1;
								}

							}

							JE_drawPortConfigButtons();
							portConfigDone = false;
						}
					}

					/* PLAYER SHOT Creation */

					/*SpecialShot*/
					if (!galagaMode)
						JE_doSpecialShot(playerNum_, armorLevel_, shield_);

					/*Normal Main Weapons*/
					if (!(twoPlayerLinked && playerNum_ == 2))
					{
						if (!twoPlayerMode)
						{
							min = 1;
							max = 2;
						} else
							switch (playerNum_)
							{
								case 1:
									min = 1;
									max = 1;
									break;
								case 2:
									min = 2;
									max = 2;
									break;
							}
							for (temp = min - 1; temp < max; temp++)
								if (pItems_[temp] > 0)
								{
									if (shotRepeat[temp] > 0)
										shotRepeat[temp]--;
									else
										if (button[1-1])
											JE_initPlayerShot(pItems_[temp], temp + 1, *PX_, *PY_, *mouseX_, *mouseY_,
											                  weaponPort[pItems_[temp]].op[portConfig[temp]-1]
											                  [portPower[temp] * !galagaMode + galagaMode - 1],
											                  playerNum_);
								}
					}

					/*Super Charge Weapons*/
					if (playerNum_ == 2)
					{

						if (!twoPlayerLinked)
							JE_drawShape2(*PX_ + (shipGr_ == 0) + 1, *PY_ - 13, 77 + chargeLevel + chargeGr * 19, eShapes6);

						if (chargeGrWait > 0)
						{
							chargeGrWait--;
						} else {
							chargeGr++;
							if (chargeGr == 4)
								chargeGr = 0;
							chargeGrWait = 3;
						}

						if (chargeLevel > 0)
						{
							JE_c_bar(269, 107 + (chargeLevel - 1) * 3, 275, 108 + (chargeLevel - 1) * 3, 193);
						}

						if (chargeWait > 0)
						{
							chargeWait--;
						} else {
							if (chargeLevel < chargeMax)
								chargeLevel++;
							chargeWait = 28 - portPower[2-1] * 2;
							if (difficultyLevel > 3)
								chargeWait -= 5;
						}

						if (chargeLevel > 0)
						{
							JE_c_bar(269, 107 + (chargeLevel - 1) * 3, 275, 108 + (chargeLevel - 1) * 3, 204);
						}

						if (shotRepeat[6-1] > 0)
							shotRepeat[6-1]--;
						else
							if (button[1-1]
							    && (!twoPlayerLinked || chargeLevel > 0))
							{
								shotMultiPos[6-1] = 0;
								JE_initPlayerShot(16, 6, *PX_, *PY_, *mouseX_, *mouseY_,
								                  chargeGunWeapons[pItemsPlayer2[2-1]-1] + chargeLevel, playerNum_);

								if (chargeLevel > 0)
								{
									JE_c_bar(269, 107 + (chargeLevel - 1) * 3, 275, 108 + (chargeLevel - 1) * 3, 193);
								}

								chargeLevel = 0;
								chargeWait = 30 - portPower[2-1] * 2;
							}
					}


					/*SUPER BOMB*/
					temp = playerNum_;
					if (temp == 0)
						temp = 1;  /*Get whether player 1 or 2*/

					if (superBomb[temp-1] > 0)
					{
						if (shotRepeat[temp-1 + 6] > 0)
							shotRepeat[temp-1 + 6]--;
						else {
							if (button[3-1] || button[2-1])
							{
								superBomb[temp-1]--;
								shotMultiPos[temp-1 + 6] = 0;
								JE_initPlayerShot(16, temp + 6, *PX_, *PY_, *mouseX_, *mouseY_, 535, playerNum_);
							}
						}
					}


					/*Special option following*/
					switch (leftOptionIsSpecial)
					{
						case 1:
						case 3:
							option1X = playerHX[10-1];
							option1Y = playerHY[10-1];
							break;
						case 2:
							option1X = *PX_;
							option1Y = *PY_ - 20;
							if (option1Y < 10)
								option1Y = 10;
							break;
						case 4:
							if (rightOptionIsSpecial == 4)
								optionSatelliteRotate += 0.2;
							else
								optionSatelliteRotate += 0.15;
							option1X = *PX_ + round(sin(optionSatelliteRotate) * 20);
							option1Y = *PY_ + round(cos(optionSatelliteRotate) * 20);
							break;
					}


					switch (rightOptionIsSpecial)
					{
	                    case 4:
							if (leftOptionIsSpecial != 4)
								optionSatelliteRotate += 0.15;
							option2X = *PX_ - round(sin(optionSatelliteRotate) * 20);
							option2Y = *PY_ - round(cos(optionSatelliteRotate) * 20);
							break;
						case 1:
						case 3:
							option2X = playerHX[1-1];
							option2Y = playerHY[1-1];
							break;
						case 2:
							if (!optionAttachmentLinked)
							{
								option2Y += optionAttachmentMove / 2;
								if (optionAttachmentMove >= -2)
								{

									if (optionAttachmentReturn)
										temp = 2;
									else
										temp = 0;
									if (option2Y > (*PY_ - 20) + 5)
									{
										temp = 2;
										optionAttachmentMove -= 1 + optionAttachmentReturn;
									} else
										if (option2Y > (*PY_ - 20) - 0)
										{
											temp = 3;
											if (optionAttachmentMove > 0)
												optionAttachmentMove--;
											else
												optionAttachmentMove++;
										} else
											if (option2Y > (*PY_ - 20) - 5)
											{
												temp = 2;
												optionAttachmentMove++;
											} else
												if (optionAttachmentMove < 2 + optionAttachmentReturn * 4)
													optionAttachmentMove += 1 + optionAttachmentReturn;

									if (optionAttachmentReturn)
										temp = temp * 2;
									if (abs(option2X - *PX_ < temp))
										temp = 1;

									if (option2X > *PX_)
										option2X -= temp;
									else
										if (option2X < *PX_)
											option2X += temp;

									if (abs(option2Y - (*PY_ - 20)) + abs(option2X - *PX_) < 8)
									{
										optionAttachmentLinked = true;
										soundQueue[3-1] = 23;
									}

									if (button[3-1])
										optionAttachmentReturn = true;
								} else {
									optionAttachmentMove += 1 + optionAttachmentReturn;
									JE_setupExplosion(option2X + 1, option2Y + 10, 0);
								}

							} else {
								option2X = *PX_;
								option2Y = *PY_ - 20;
								if (button[3-1])
								{
									optionAttachmentLinked = false;
									optionAttachmentReturn = false;
									optionAttachmentMove = -20;
									soundQueue[3] = 26;
								}
							}

							if (option2Y < 10)
								option2Y = 10;
							break;
					}

					if (playerNum_ == 2 || !twoPlayerMode)
					{
						if (options[option1Item].wport > 0)
						{
							if (shotRepeat[3-1] > 0)
								shotRepeat[3-1]--;
							else {

								if (option1Ammo >= 0)
								{
									if (option1AmmoRechargeWait > 0)
										option1AmmoRechargeWait--;
									else {
										option1AmmoRechargeWait = option1AmmoRechargeWaitMax;
										if (option1Ammo < options[option1Item].ammo)
											option1Ammo++;
										JE_barDrawDirect (284, option1Draw + 13, option1AmmoMax, 112, option1Ammo, 2, 2); /*Option1Ammo*/
									}
								}

								if (option1Ammo > 0)
								{
									if (button[2-1])
									{
										JE_initPlayerShot(options[option1Item].wport, 3,
														option1X, option1Y,
														*mouseX_, *mouseY_,
														options[option1Item].wpnum + optionCharge1,
														playerNum_);
										if (optionCharge1 > 0)
											shotMultiPos[3-1] = 0;
										optionAni1Go = true;
										optionCharge1Wait = 20;
										optionCharge1 = 0;
										option1Ammo--;
										JE_c_bar(284, option1Draw + 13, 312, option1Draw + 15, 0);
										JE_barDrawDirect(284, option1Draw + 13, option1AmmoMax, 112, option1Ammo, 2, 2);
									}
								} else
									if (option1Ammo < 0)
									{
										if (button[1-1] || button[2-1])
										{
											JE_initPlayerShot(options[option1Item].wport, 3,
															option1X, option1Y,
															*mouseX_, *mouseY_,
															options[option1Item].wpnum + optionCharge1,
															playerNum_);
											if (optionCharge1 > 0)
												shotMultiPos[3-1] = 0;
											optionCharge1Wait = 20;
											optionCharge1 = 0;
											optionAni1Go = true;
										}
									}
							}
						}

						if (options[option2Item].wport > 0)
						{
							if (shotRepeat[4-1] > 0)
								shotRepeat[4-1]--;
							else {

								if (option2Ammo >= 0)
								{
									if (option2AmmoRechargeWait > 0)
										option2AmmoRechargeWait--;
									else {
										option2AmmoRechargeWait = option2AmmoRechargeWaitMax;
										if (option2Ammo < options[option2Item].ammo)
											option2Ammo++;
										JE_barDrawDirect(284, option2Draw + 13, option2AmmoMax, 112, option2Ammo, 2, 2);
									}
								}

								if (option2Ammo > 0)
								{
									if (button[3-1])
									{
										JE_initPlayerShot(options[option2Item].wport, 4, option2X, option2Y,
														*mouseX_, *mouseY_,
														options[option2Item].wpnum + optionCharge2,
														playerNum_);
										if (optionCharge2 > 0)
										{
											shotMultiPos[4-1] = 0;
											optionCharge2 = 0;
										}
										optionCharge2Wait = 20;
										optionCharge2 = 0;
										optionAni2Go = true;
										option2Ammo--;
										JE_c_bar(284, option2Draw + 13, 312, option2Draw + 15, 0);
										JE_barDrawDirect(284, option2Draw + 13, option2AmmoMax, 112, option2Ammo, 2, 2);
									}
								} else
									if (option2Ammo < 0)
										if (button[1-1] || button[3-1])
										{
											JE_initPlayerShot(options[option2Item].wport, 4, option2X, option2Y,
														*mouseX_, *mouseY_,
														options[option2Item].wpnum + optionCharge2,
														playerNum_);
											if (optionCharge2 > 0)
											{
												shotMultiPos[4-1] = 0;
												optionCharge2 = 0;
											}
											optionCharge2Wait = 20;
											optionAni2Go = true;
										}
							}
						}
					}
				}  /* !endLevel */
			}

		}

		/* Draw Floating Options */
		if ((playerNum_ == 2 || !twoPlayerMode) && !endLevel)
		{
			if (options[option1Item].option > 0)
			{

				if (optionAni1Go)
				{
					optionAni1++;
					if (optionAni1 > options[option1Item].ani)
					{
						optionAni1 = 1;
						optionAni1Go = options[option1Item].option == 1;
					}
				}

				if (leftOptionIsSpecial == 1 || leftOptionIsSpecial == 2)
					JE_drawShape2x2(option1X - 6, option1Y, options[option1Item].gr[optionAni1-1] + optionCharge1,
					                eShapes6);
				else
					JE_drawShape2(option1X, option1Y, options[option1Item].gr[optionAni1-1] + optionCharge1,
					              shapes9);
			}

			if (options[option2Item].option > 0)
			{

				if (optionAni2Go)
				{
					optionAni2++;
					if (optionAni2 > options[option2Item].ani)
					{
						optionAni2 = 1;
						optionAni2Go = options[option2Item].option == 1;
					}
				}

				if (rightOptionIsSpecial == 1 || rightOptionIsSpecial == 2)
					JE_drawShape2x2(option2X - 6, option2Y, options[option2Item].gr[optionAni2-1] + optionCharge2,
					                eShapes6);
				else
					JE_drawShape2(option2X, option2Y, options[option2Item].gr[optionAni2-1] + optionCharge2,
					              shapes9);
			}

			optionCharge1Wait--;
			if (optionCharge1Wait == 0)
			{
				if (optionCharge1 < options[option1Item].pwr)
					optionCharge1++;
				optionCharge1Wait = 20;
			}
			optionCharge2Wait--;
			if (optionCharge2Wait == 0)
			{
				if (optionCharge2 < options[option2Item].pwr)
					optionCharge2++;
				optionCharge2Wait = 20;
			}
		}

	} else
		explosionFollowAmount = 0; /*if (*playerAlive_)...*/

}

void JE_mainGamePlayerFunctions( void )
{
	/*PLAYER MOVEMENT/MOUSE ROUTINES*/

	useButtonAssign = true;

	if (endLevel && levelEnd > 0)
	{
		levelEnd--;
		levelEndWarp++;
	}

	/*Reset Street-Fighter commands*/
	memset(SFExecuted, 0, sizeof(SFExecuted));

	makeMouseDelay = true;
	portConfigChange = false;

	if (twoPlayerMode)
	{

		JE_playerMovement(playerDevice1 * !galagaMode, 1, shipGr, shipGrPtr,
		                  &armorLevel, &baseArmor,
		                  &shield, &shieldMax,
		                  &playerInvulnerable1,
		                  &PX, &PY, &lastPX, &lastPY, &lastPX2, &lastPY2, &PXChange, &PYChange,
		                  &lastTurn, &lastTurn2, &tempLastTurn2, &stopWaitX, &stopWaitY,
		                  &mouseX, &mouseY,
		                  &playerAlive, &playerStillExploding, pItems);
		JE_playerMovement(playerDevice2 * !galagaMode, 2, shipGr2, shipGr2ptr,
		                  &armorLevel2, &baseArmor2,
		                  &shield2, &shieldMax2,
		                  &playerInvulnerable2,
		                  &PXB, &PYB, &lastPXB, &lastPYB, &lastPX2B, &lastPY2B, &PXChangeB, &PYChangeB,
		                  &lastTurnB, &lastTurn2B, &tempLastTurn2B, &stopWaitXB, &stopWaitYB,
		                  &mouseXB, &mouseYB,
		                  &playerAliveB, &playerStillExploding2, pItemsPlayer2);
	} else {
		JE_playerMovement(0, 1, shipGr, shipGrPtr,
		                  &armorLevel, &baseArmor,
		                  &shield, &shieldMax,
		                  &playerInvulnerable1,
		                  &PX, &PY, &lastPX, &lastPY, &lastPX2, &lastPY2, &PXChange, &PYChange,
		                  &lastTurn, &lastTurn2, &tempLastTurn2, &stopWaitX, &stopWaitY,
		                  &mouseX, &mouseY,
		                  &playerAlive, &playerStillExploding, pItems);
	}

	/*-----Horizontal Map Scrolling-----*/
	if (twoPlayerMode)
	{
		tempX = (PX + PXB) / 2;
	} else {
		tempX = PX;
	}

	tempW = floor((260.0f - (tempX - 36.0f)) / (260.0f - 36.0f) * (24.0f * 3.0f) - 1.0f);
	mapX3Ofs   = tempW;
	mapX3Pos   = mapX3Ofs % 24;
	mapX3bpPos = 1 - (mapX3Ofs / 24);

	mapX2Ofs   = (tempW * 2) / 3;
	mapX2Pos   = mapX2Ofs % 24;
	mapX2bpPos = 1 - (mapX2Ofs / 24);

	oldMapXOfs = mapXOfs;
	mapXOfs    = mapX2Ofs / 2;
	mapXPos    = mapXOfs % 24;
	mapXbpPos  = 1 - (mapXOfs / 24);

	if (background3x1)
	{
		mapX3Ofs = mapXOfs;
		mapX3Pos = mapXPos;
		mapX3bpPos = mapXbpPos - 1;
	}
}

char *JE_getName( JE_byte pnum )
{
	STUB();

	return NULL;
}

void JE_playerCollide( JE_integer *PX_, JE_integer *PY_, JE_integer *lastTurn_, JE_integer *lastTurn2_,
                       JE_longint *score_, JE_integer *armorLevel_, JE_shortint *shield_, JE_boolean *playerAlive_,
                       JE_byte *playerStillExploding_, JE_byte playerNum_, JE_byte playerInvulnerable_ )
{
	char tempStr[256];
	
	for (z = 0; z < 100; z++)
	{
		if (enemyAvail[z] != 1)
		{
			tempI3 = enemy[z].ex + enemy[z].mapoffset;
			
			if (abs(*PX_ - tempI3) < 12 && abs(*PY_ - enemy[z].ey) < 14)
			{   /*Collide*/
				tempI4 = enemy[z].evalue;
				if (tempI4 > 29999)
				{
					if (tempI4 == 30000)
					{
						*score_ += 100;
						
						if (!galagaMode)
						{
							if (purpleBallsRemaining[playerNum_ - 1] > 1)
								purpleBallsRemaining[playerNum_ - 1]--;
							else {
								JE_powerUp(playerNum_);
								JE_calcPurpleBall(playerNum_);
							}
						} else {
							if (twoPlayerMode)
								*score_ += 2400;
							twoPlayerMode = true;
							twoPlayerLinked = true;
							portPower[2-1] = 1;
							armorLevel2 = 10;
							playerAliveB = true;
						}
						enemyAvail[z] = 1;
						soundQueue[7] = 29;
					} else if (superArcadeMode > 0 && tempI4 > 30000) {
						shotMultiPos[1-1] = 0;
						shotRepeat[1-1] = 10;
						
						tempW = SAWeapon[superArcadeMode-1][tempI4 - 30000-1];
						
						if (tempW == pItems[1-1])
						{
							*score_ += 1000;
							if (portPower[1-1] < 11)
								JE_powerUp(1);
							JE_calcPurpleBall(playerNum_);
						} else if (purpleBallsRemaining[playerNum_-1] > 1)
							purpleBallsRemaining[playerNum_-1]--;
						else {
							JE_powerUp(playerNum_);
							JE_calcPurpleBall(playerNum_);
						}
						
						pItems[1-1] = tempW;
						*score_ += 200;
						soundQueue[7] = 29;
						enemyAvail[z] = 1;
					} else if (tempI4 > 32100) {
						if (playerNum_ == 1)
						{
							*score_ += 250;
							pItems[11-1] = tempI4 - 32100;
							shotMultiPos[9-1] = 0;
							shotRepeat[9-1] = 10;
							shotMultiPos[11-1] = 0;
							shotRepeat[11-1] = 0;
							
							if (isNetworkGame)
							{
								sprintf(tempStr, "%s %s %s", JE_getName(1), miscTextB[4-1], special[tempI4 - 32100].name);
							} else if (twoPlayerMode) {
								sprintf(tempStr, "%s %s", miscText[43-1], special[tempI4 - 32100].name);
							} else {
								sprintf(tempStr, "%s %s", miscText[64-1], special[tempI4 - 32100].name);
							}
							JE_drawTextWindow(tempStr);
							soundQueue[7] = 29;
							enemyAvail[z] = 1;
						}
					} else if (tempI4 > 32000) {
						if (playerNum_ == 2)
						{
							enemyAvail[z] = 1;
							if (isNetworkGame)
							{
								sprintf(tempStr, "%s %s %s", JE_getName(2), miscTextB[4-1], options[tempI4 - 32000].name);
							} else {
								sprintf(tempStr, "%s %s", miscText[44-1], options[tempI4 - 32000].name);
							}
							JE_drawTextWindow(tempStr);
							
							if (tempI4 - 32000 != pItemsPlayer2[8-1])
								pItemsPlayer2[7-1] = 100;
							pItemsPlayer2[8-1] = tempI4 - 32000;
							
							if (pItemsPlayer2[7-1] < 103)
								pItemsPlayer2[7-1]++;
							
							temp = pItemsPlayer2[7-1] - 100;
							pItemsPlayer2[4-1] = optionSelect[pItemsPlayer2[8-1]][temp-1][1-1];
							pItemsPlayer2[5-1] = optionSelect[pItemsPlayer2[8-1]][temp-1][2-1];
							
							
							shotMultiPos[3-1] = 0;
							shotMultiPos[4-1] = 0;
							tempScreenSeg = VGAScreenSeg;
							JE_drawOptions();
							soundQueue[7] = 29;
							tempScreenSeg = VGAScreen;
						} else if (onePlayerAction) {
							enemyAvail[z] = 1;
							sprintf(tempStr, "%s %s", miscText[64-1], options[tempI4 - 32000].name);
							JE_drawTextWindow(tempStr);
							pItems[4-1] = tempI4 - 32000;
							pItems[5-1] = tempI4 - 32000;
							shotMultiPos[3-1] = 0;
							shotMultiPos[4-1] = 0;
							tempScreenSeg = VGAScreenSeg;
							JE_drawOptions();
							soundQueue[7] = 29;
						}
						if (enemyAvail[z] == 1)
							*score_ += 250;
					} else if (tempI4 > 31000) {
						*score_ += 250;
						if (playerNum_ == 2)
						{
							if (isNetworkGame)
							{
								sprintf(tempStr, "%s %s %s", JE_getName(2), miscTextB[4-1], weaponPort[tempI4 - 31000].name);
							} else {
								sprintf(tempStr, "%s %s", miscText[44-1], weaponPort[tempI4 - 31000].name);
							}
							JE_drawTextWindow(tempStr);
							pItemsPlayer2[2-1] = tempI4 - 31000;
							shotMultiPos[2-1] = 0;
							enemyAvail[z] = 1;
							soundQueue[7] = 29;
						} else if (onePlayerAction) {
							sprintf(tempStr, "%s %s", miscText[64-1], weaponPort[tempI4 - 31000].name);
							JE_drawTextWindow(tempStr);
							pItems[2-1] = tempI4 - 31000;
							shotMultiPos[2] = 0;
							enemyAvail[z] = 1;
							soundQueue[7] = 29;
							if (portPower[2-1] == 0)
								portPower[2-1] = 1;
						}
					} else if (tempI4 > 30000) {
						
						if (playerNum_ == 1 && twoPlayerMode)
						{
							if (isNetworkGame)
							{
								sprintf(tempStr, "%s %s %s", JE_getName(1), miscTextB[4-1], weaponPort[tempI4 - 30000].name);
							} else {
								sprintf(tempStr, "%s %s", miscText[43-1], weaponPort[tempI4 - 30000].name);
							}
							JE_drawTextWindow(tempStr);
							pItems[1-1] = tempI4 - 30000;
							shotMultiPos[1-1] = 0;
							enemyAvail[z] = 1;
							soundQueue[7] = 29;
						} else if (onePlayerAction) {
							sprintf(tempStr, "%s %s", miscText[64-1], weaponPort[tempI4 - 30000].name);
							JE_drawTextWindow(tempStr);
							pItems[1-1] = tempI4 - 30000;
							shotMultiPos[1-1] = 0;
							enemyAvail[z] = 1;
							soundQueue[7] = 29;
						}
						
						if (enemyAvail[z] == 1)
						{
							pItems[11-1] = specialArcadeWeapon[tempI4 - 30000-1];
							if (pItems[11-1] > 0)
							{
								shotMultiPos[9-1] = 0;
								shotRepeat[9-1] = 0;
								shotMultiPos[11-1] = 0;
								shotRepeat[11-1] = 0;
							}
							*score_ += 250;
						}
						
					}
				} else if (tempI4 > 20000) {
					if (twoPlayerLinked)
					{
						armorLevel += (tempI4 - 20000) / 2;
						if (armorLevel > 28)
							armorLevel = 28;
						armorLevel2 += (tempI4 - 20000) / 2;
						if (armorLevel2 > 28)
							armorLevel2 = 28;
					} else {
						*armorLevel_ += tempI4 - 20000;
						if (*armorLevel_ > 28)
							*armorLevel_ = 28;
					}
					enemyAvail[z] = 1;
					VGAScreen = VGAScreenSeg; /* side-effect of game_screen */
					JE_drawArmor();
					VGAScreen = game_screen; /* side-effect of game_screen */
					soundQueue[7] = 29;
				} else if (tempI4 > 10000 && enemyAvail[z] == 2) {
					if (!bonusLevel)
					{
						JE_playSong(31);  /*Zanac*/
						bonusLevel = true;
						nextLevel = tempI4 - 10000;
						enemyAvail[z] = 1;
						displayTime = 150;
					}
				} else if (enemy[z].scoreitem) {
					enemyAvail[z] = 1;
					soundQueue[7] = 18;
					if (tempI4 == 1)
					{
						cubeMax++;
						soundQueue[3] = V_DATA_CUBE;
					} else if (tempI4 == -1) {
						if (isNetworkGame)
						{
							sprintf(tempStr, "%s %s %s", JE_getName(1), miscTextB[4-1], miscText[45-1]);
						} else if (twoPlayerMode) {
							sprintf(tempStr, "%s %s", miscText[43-1], miscText[45-1]);
						} else {
							strcpy(tempStr, miscText[45-1]);
						}
						JE_drawTextWindow(tempStr);
						JE_powerUp(1);
						soundQueue[7] = 29;
					} else if (tempI4 == -2) {
						if (isNetworkGame)
						{
							sprintf(tempStr, "%s %s %s", JE_getName(2), miscTextB[4-1], miscText[46-1]);
						} else if (twoPlayerMode) {
							sprintf(tempStr, "%s %s", miscText[44-1], miscText[46-1]);
						} else {
							strcpy(tempStr, miscText[46-1]);
						}
						JE_drawTextWindow(tempStr);
						JE_powerUp(2);
						soundQueue[7] = 29;
					} else if (tempI4 == -3) {
						shotMultiPos[5-1] = 0;
						JE_initPlayerShot(0, 5, *PX_, *PY_, mouseX, mouseY, 104, playerNum_);
						shotAvail[z] = 0;
					} else if (tempI4 == -4) {
						if (superBomb[playerNum_-1] < 10)
							superBomb[playerNum_-1]++;
					} else if (tempI4 == -5) {
						pItems[1-1] = 25;         /*HOT DOG!*/
						pItems[2-1] = 26;
						pItemsPlayer2[2-1] = 26;
						memcpy(pItemsBack2, pItems, sizeof(pItemsBack2));
						portConfig[2-1] = 1;
						memset(shotMultiPos, 0, sizeof(shotMultiPos));
					} else if (twoPlayerLinked) {
						score += tempI4 / 2;
						score2 += tempI4 / 2;
					} else
						*score_ += tempI4;
					fixedExplosions = true;
					JE_setupExplosion(tempI3, enemy[z].ey, enemyDat[enemy[z].enemytype].explosiontype);
					fixedExplosions = false;
				} else
					if (playerInvulnerable_ == 0 && enemyAvail[z] == 0 &&
					    enemyDat[enemy[z].enemytype].explosiontype % 2 == 0)
					{
						
						tempI3 = enemy[z].armorleft;
						if (tempI3 > damageRate)
							tempI3 = damageRate;
						
						JE_playerDamage(tempW, tempW, tempI3, PX_, PY_, playerAlive_, playerStillExploding_, armorLevel_, shield_);
						
						if (enemy[z].armorleft > 0)
						{
							*lastTurn2_ += (enemy[z].exc * enemy[z].armorleft) / 2;
							*lastTurn_  += (enemy[z].eyc * enemy[z].armorleft) / 2;
						}
						
						tempI = enemy[z].armorleft;
						if (tempI == 255)
							tempI = 30000;
						
						temp = enemy[z].linknum;
						if (temp == 0)
							temp = 255;
						
						b = z;
						
						if (tempI > tempI2)
						{
							if (enemy[z].armorleft != 255)
								enemy[z].armorleft -= tempI3;
							soundQueue[5] = 3;
						} else {
							
							for (temp2 = 0; temp2 < 100; temp2++)
							{
								if (enemyAvail[temp2] != 1)
								{
									temp3 = enemy[temp2].linknum;
									if (temp2 == b ||
										(temp != 255 &&
										 (temp == temp3 || temp - 100 == temp3
										  || (temp3 > 40 && temp3 / 20 == temp / 20 && temp3 <= temp))))
										
									{
										
										tempI3 = enemy[temp2].ex + enemy[temp2].mapoffset;
										
										enemy[temp2].linknum = 0;
										
										enemyAvail[temp2] = 1;
										
										if (enemyDat[enemy[temp2].enemytype].esize == 1)
										{
											JE_setupExplosionLarge(enemy[temp2].enemyground, enemy[temp2].explonum, tempI3, enemy[temp2].ey);
											soundQueue[6] = 9;
										} else {
											JE_setupExplosion(tempI3, enemy[temp2].ey, 1);
											soundQueue[5] = 4;
										}
									}
								}
							}
							enemyAvail[z] = 1;
						}
					}
			}

		}
	}
}


/*void JE_textMenuWait ( JE_word waittime, JE_boolean dogamma )
{
	STUB();
}*/
