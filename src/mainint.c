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
#include "episodes.h"
#include "error.h"
#include "fonthand.h"
#include "helptext.h"
#include "helptext.h"
#include "joystick.h"
#include "keyboard.h"
#include "network.h"
#include "newshape.h"
#include "nortsong.h"
#include "nortvars.h"
#include "params.h"
#include "pcxload.h"
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
	tempScreenSeg = VGAScreen; /*sega000*/
	if (textErase > 0)
	{
		JE_newDrawCShapeNum(OPTION_SHAPES, 36, 16, 189);
	}
	textErase = 100;
	tempScreenSeg = VGAScreen; /*sega000*/
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
			tempScreenSeg = VGAScreen; /*sega000*/
			JE_newDrawCShapeNum(OPTION_SHAPES, 18, 285, 44);
			tempScreenSeg = VGAScreen; /*sega000*/
			JE_newDrawCShapeNum(OPTION_SHAPES, 19, 302, 44);
		} else {
			tempScreenSeg = VGAScreen; /*sega000*/
			JE_newDrawCShapeNum(OPTION_SHAPES, 19, 285, 44);
			tempScreenSeg = VGAScreen; /*sega000*/
			JE_newDrawCShapeNum(OPTION_SHAPES, 18, 302, 44);
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
	JE_loadPic(2, FALSE);
	JE_playSong(SONG_MAPVIEW);
	JE_showVGA();
	JE_fadeColor(10);

	memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));

	joystickWaitMax = 120; joystickWait = 0;

	do
	{
		memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));

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
					JE_textMenuWait(&tempW, FALSE);
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
				service_SDL_events(FALSE);
			} while (mousedown);
			while (!JE_waitAction(1, TRUE));

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
					service_SDL_events(FALSE);
				} while (mousedown);
				newkey = TRUE;
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

	JE_loadPic(2, FALSE);
	memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
	JE_showVGA();
	JE_fadeColor(20);
	quit = FALSE;

	sel = 1;
	maxSel = 4;

	do
	{

		JE_dString(JE_fontCenter(playerName[0], FONT_SHAPES), 20, playerName[0], FONT_SHAPES);

		for (temp = 1; temp <= maxSel; temp++)
		{
			JE_outTextAdjust(JE_fontCenter(playerName[temp], SMALL_FONT_SHAPES), temp * 24 + 30, playerName[temp], 15, - 4 + ((sel == temp) << 1), SMALL_FONT_SHAPES, TRUE);
		}

		/*BETA TEST VERSION*/
		/*  JE_Dstring(JE_FontCenter(misctext[34], FONT_SHAPES), 170, misctext[34], FONT_SHAPES);*/

		JE_showVGA();
		tempW = 0;
		JE_textMenuWait(&tempW, FALSE);

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
					quit = TRUE;
					twoPlayerMode = (sel == 3);
					onePlayerAction = (sel == 2);
					JE_playSampleNum(SELECT);
					if (sel == 4)
					{
						netQuit = TRUE;
					}
					break;
				case SDLK_ESCAPE:
				quit = TRUE;
				JE_playSampleNum(ESC);
				return FALSE;
				break;
				default:
					break;
			}
		}

	} while (!quit);

	return TRUE; /*MXD assumes this default return value here*/
}

JE_boolean JE_episodeSelect( void )
{
	JE_byte sel;
	JE_boolean quit;
	JE_byte max;

	max = EPISODE_MAX;

	if (!(episodeAvail[0] && episodeAvail[1] && episodeAvail[2]))
	{
		episodeAvail[3] = FALSE;
	}

	if (episodeAvail[4] == FALSE)
	{
		max = 4;
	}

	/*if (!episodeavail[3]) max = 3;*/

startepisodeselect:
	JE_loadPic(2, FALSE);
	memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
	JE_showVGA();
	JE_fadeColor(10);
	quit = FALSE;

	sel = 1;

	do
	{

		JE_dString(JE_fontCenter(episodeName[0], FONT_SHAPES), 20, episodeName[0], FONT_SHAPES);

		for (temp = 1; temp <= max; temp++)
		{
			JE_outTextAdjust(20, temp * 30 + 20, episodeName[temp], 15, - 4 - (!episodeAvail[temp-1] << 2) + ((sel == temp) << 1), SMALL_FONT_SHAPES, TRUE);
		}

		/*JE_Dstring(JE_fontCenter(misctext[34], FONT_SHAPES), 170, misctext[34], FONT_SHAPES);*/

		JE_showVGA();
		tempW = 0;
		JE_textMenuWait(&tempW, FALSE);

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
	
						quit = TRUE;
						JE_initEpisode(sel);
						return TRUE;
					} else {
						if (sel > 1)
						{
							char buf[] = "EPISODE-.PCX";

							JE_playSampleNum(ESC);
							JE_fadeBlack (10);

							buf[7] = '0' + sel;
							JE_loadPCX(buf, FALSE);
							verticalHeight = 9;
							helpBoxColor = 15;
							helpBoxBrightness = 4;
							helpBoxShadeType = FULL_SHADE;
							JE_helpBox(10, 10, helpTxt[29], 50);
							JE_showVGA();
							JE_fadeColor(10);
							wait_input(TRUE,TRUE,TRUE);
							lastkey_sym = 0;
							JE_fadeBlack(10);
							goto startepisodeselect;
						}
					}
					break;
				case SDLK_ESCAPE:
					quit = TRUE;
					JE_playSampleNum(ESC);
					return FALSE;
					break;
				default:
					break;
			}
		}

	} while (!(quit || haltGame /*|| netQuit*/));
	pItems[8] = episodeNum;

	return FALSE; /*MXD assumes this default return value here*/
}

JE_boolean JE_difficultySelect( void )
{
	JE_byte maxSel;
	JE_byte sel;
	JE_boolean quit;

	JE_loadPic(2, FALSE);
	memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
	JE_showVGA();
	JE_fadeColor(20);
	quit = FALSE;

	sel = 2;
	maxSel = 3;

	do
	{
		JE_dString(JE_fontCenter(difficultyName[0], FONT_SHAPES), 20, difficultyName[0], FONT_SHAPES);

		for (temp = 1; temp <= maxSel; temp++)
		{
			JE_outTextAdjust(JE_fontCenter(difficultyName[temp], SMALL_FONT_SHAPES), temp * 24 + 30, difficultyName[temp], 15, - 4 + ((sel == temp) << 1), SMALL_FONT_SHAPES, TRUE);
		}

		/*BETA TEST VERSION*/
		/*  JE_Dstring(JE_FontCenter(misctext[34], FONT_SHAPES), 170, misctext[34], FONT_SHAPES);*/

		JE_showVGA();
		tempW = 0;
		JE_textMenuWait(&tempW, FALSE);

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
					quit = TRUE;
	
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
					quit = TRUE;
					JE_playSampleNum(ESC);
					return FALSE;
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

	return TRUE; /*MXD assumes this default return value here*/
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
		JE_resetFile(&f, "TYRIANC.SHP");
	} else {
		JE_resetFile(&f, "TYRIAN.SHP");
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
			downgradeCost = JE_powerLevelCost(tempW2, portPower[itemType-2]-1);
			upgradeCost = JE_powerLevelCost(tempW2, portPower[itemType-2]);
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
	JE_boolean mal_str = FALSE;
	int len;

	tempstr = NULL;
	
	/* TODO: Most of the actual save logic, string handling */

	JE_fadeBlack(10);
	JE_loadPic(2, FALSE);
	JE_showVGA();
	JE_fadeColor(10);
	
	screen = 1;
	sel = 1;
	quit = FALSE;
	
	memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
	
	do
	{
		while (mousedown)
		{
			service_SDL_events(FALSE);
			tempX = mouse_x;
			tempY = mouse_y;
		}
		
		memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));
		
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
					mal_str = FALSE;
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
						mal_str = FALSE;
					}
					tempstr = miscText[3 - 1];
				} else {
					if (mal_str)
					{
						free(tempstr);
						mal_str = FALSE;
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
					mal_str = TRUE;
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
		JE_textMenuWait(&tempW, FALSE);
		
		
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
						performSave = FALSE;
                        JE_operation(sel);
                        quit = TRUE;
					} else {
                        JE_playSampleNum (WRONG);
                    }
				} else {
                  quit = TRUE;
				}
				
				
				break;
			case SDLK_ESCAPE:
				quit = TRUE;
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

	tempL += JE_getValue(2, pItems[12]);
	tempL += JE_getValue(3, pItems[1]);
	tempL += JE_getValue(4, pItems[2]);
	tempL += JE_getValue(5, pItems[10]);
	tempL += JE_getValue(6, pItems[6]);
	tempL += JE_getValue(7, pItems[4]);
	tempL += JE_getValue(8, pItems[5]);

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
	STUB(JE_nextEpisode);
	return FALSE;
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
	gameHasRepeated = FALSE;
	onePlayerAction = FALSE;
	superArcadeMode = 0;
	superTyrian = FALSE;
	/* twoplayerarcade:=false; */
	twoPlayerMode = FALSE;

	secretHint = (rand()%3) + 1;

	armorLevel = ships[pItems[11]].dmg;
	portPower[0] = 1;
	portPower[1] = 1;
	portConfig[1] = 1;

	mainLevel = FIRST_LEVEL;
	saveLevel = FIRST_LEVEL;

	strcpy(lastLevelName, miscText[20]);
}

void JE_sortHighScores( void )
{
	STUB(JE_sortHighScores);
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
	JE_loadPic(2, FALSE);
	JE_showVGA();
	JE_fadeColor(10);
	tempScreenSeg = VGAScreen;
	
	while (max < EPISODE_MAX && episodeAvail[max+1])
	{
		max++;
	}
	
	max = 3; /* TODO: Ep. 4 high scores are broked! */
	
	quit = FALSE;
	x = 1;
	chg = 1;
	
	memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
	
	do
	{
		if (episodeAvail[x])
		{
			memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));
			
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
			JE_textMenuWait(&tempW, FALSE);
			
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
				quit = TRUE;
				break;
			default:
				break;
			}
		}
		
	} while (!quit);

}

void JE_gammaCorrect( JE_ColorType *colorBuffer, JE_byte gamma )
{
	STUB(JE_gammaCorrect);
}

JE_boolean JE_gammaCheck( void )
{
	STUB(JE_gammaCheck);
	
	return FALSE;
}

/* void JE_textMenuWait( JE_word *waitTime, JE_boolean doGamma ); /!\ In setup.h */
void JE_loadOrderingInfo( void )
{
	STUB(JE_loadOrderingInfo);
}

void JE_doInGameSetup( void )
{
	STUB(JE_doInGameSetup);
}

JE_boolean JE_inGameSetup( void )
{
	STUB(JE_inGameSetup);
	
	return FALSE;
}

void JE_inGameHelp( void )
{
	STUB(JE_inGameHelp);
}

void JE_highScoreCheck( void )
{
	STUB(JE_highScoreCheck);
}

void JE_setNewGameVol( void )
{
	STUB(JE_setNewGameVol);
}

void JE_changeDifficulty( void )
{
	STUB(JE_changeDifficulty);
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
		button[1-1] = TRUE;
	}
	if (lastKey[5] > 0)
	{
		button[4-1] = TRUE;
	}
	if (lastKey[6] > 0)
	{
		button[2-1] = TRUE;
	}
	if (lastKey[7] > 0)
	{
		button[3-1] = TRUE;
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

void JE_sfCodes( void )
{
	STUB(JE_sfCodes);
}

void JE_func( JE_byte col )
{
	STUB(JE_func);
}

void JE_sort( void )
{
	STUB(JE_sort);
}

JE_boolean JE_getPassword( void )
{
	STUB(JE_getPassword);
	
	return FALSE;
}

void JE_playCredits( void )
{
	STUB(JE_playCredits);
}

void JE_endLevelAni( void )
{
	STUB(JE_endLevelAni);
}

void JE_drawCube( JE_word x, JE_word y, JE_byte filter, JE_byte brightness )
{
	JE_newDrawCShapeDarken((*shapeArray)[OPTION_SHAPES][26-1], shapeX[OPTION_SHAPES][26-1],
	  shapeY[OPTION_SHAPES][26 - 1], x + 4, y + 4);
	JE_newDrawCShapeDarken((*shapeArray)[OPTION_SHAPES][26-1], shapeX[OPTION_SHAPES][26-1],
	  shapeY[OPTION_SHAPES][26 - 1], x + 3, y + 3);
	JE_newDrawCShapeAdjustNum(OPTION_SHAPES, 26 - 1, x, y, filter, brightness);	
}

void JE_handleChat( void )
{
	STUB(JE_handleChat);
}

JE_boolean JE_getNumber( char *s, JE_byte *x )
{
	STUB(JE_getNumber);
	
	return FALSE;
}

void JE_operation( JE_byte slot )
{
	STUB(JE_operation);
}

void JE_inGameDisplays( void )
{
	STUB(JE_inGameDisplays);
}

void JE_mainKeyboardInput( void )
{
	STUB(JE_mainKeyboardInput);
}

void JE_pauseGame( void )
{
	STUB(JE_pauseGame);
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
	JE_byte leftOptionIsSpecial;
	JE_byte rightOptionIsSpecial;

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

			JE_setupExplosionLarge(FALSE, 0, *PX_ + (rand() % 32) - 16, *PY_ + (rand() % 32) - 16 + 7);
			JE_setupExplosionLarge(FALSE, 0, *PX_, *PY_ + 7);

			if (levelEnd > 0)
				levelEnd--;
		} else {

			if (twoPlayerMode || onePlayerAction)
			{
				if (portPower[playerNum_-1] > 1)
				{
					reallyEndLevel = FALSE;
					shotMultiPos[playerNum_-1] = 0;
					portPower[playerNum_-1]--;
					JE_calcPurpleBall(playerNum_);
					twoPlayerLinked = FALSE;
					if (galagaMode)
						twoPlayerMode = FALSE;
					*PY_ = 160;
					*playerInvulnerable_ = 100;
					*playerAlive_ = TRUE;
					endLevel = FALSE;
					
					if (galagaMode || episodeNum == 4)
						*armorLevel_ = *baseArmor_;
					else
						*armorLevel_ = *baseArmor_ / 2;
					
					if (galagaMode)
						*shield_ = 0;
					else
						*shield_ = *shieldMax_ / 2;
					
					JE_drawArmor();
					JE_drawShield();
					goto redo;
				} else {
					if (galagaMode)
						twoPlayerMode = FALSE;
					if (allPlayersGone && isNetworkGame)
						reallyEndLevel = TRUE;
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
				*playerAlive_ = FALSE;
				*playerStillExploding_ = 60;
				levelEnd = 40;
			}

			/* TODO JE_wipeShieldArmorBars();*/
			JE_drawArmor();
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
			button[1-1] = FALSE;
			button[2-1] = FALSE;
			button[3-1] = FALSE;
			button[4-1] = FALSE;
			
			/* --- Movement Routine Beginning --- */
			
			if (!isNetworkGame || playerNum_ == thisPlayerNum)
			{
				
				if (endLevel)
					*PY_ -= 2;
				else {
					service_SDL_events(FALSE);
					/* ---- Mouse Input ---- */
					if ((inputDevice_ == 2 || inputDevice_ == 0)
					    && !playDemo && mouseInstalled && !recordDemo)
					{
						/* Get Mouse Status */
						button[1-1] = mouse_pressed[0];
						button[2-1] = mouse_pressed[1];
						button[3-1] = mouse_threeButton ? mouse_pressed[2] : mouse_pressed[1];
						
						mouseXC = mouse_x - 159; /* <MXD> scale? */
						mouseYC = mouse_y - 100;
						
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
								endLevel = TRUE;
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
								button[1-1] = TRUE;
							if (keysactive[keySettings[6-1]])
								button[4-1] = TRUE;
							if (keysactive[keySettings[7-1]])
								button[2-1] = TRUE;
							if (keysactive[keySettings[8-1]])
								button[3-1] = TRUE;
						}
						
						if (constantPlay)
						{
							button[1-1] = TRUE;
							button[2-1] = TRUE;
							button[3-1] = TRUE;
							button[4-1] = TRUE;
							(*PY_)++;
							*PX_ += constantLastX;
						}
						
						
						if (recordDemo)
						{
							tempB = FALSE;
							for (temp = 0; temp < 8; temp++)
								if (lastKey[temp] != keysactive[keySettings[temp]])
									tempB = TRUE;
							
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
			
			moveOk = TRUE;
			
			if (isNetworkGame)
			{
				/* TODO */;
			}
			
			/*Street-Fighter codes*/
			/* TODO JE_SFCodes();*/
			
			if (moveOk)
			{
				
				/* END OF MOVEMENT ROUTINES */
				
				/*Linking Routines*/
				
				if (twoPlayerMode && !twoPlayerLinked && *PX_ == *mouseX_ && *PY_ == *mouseY_
				    && abs(PX - PXB) < 8 && abs(PY - PYB) < 8
				    && playerAlive && playerAliveB && !galagaMode)
				{
					twoPlayerLinked = TRUE;
				}
				
				if (playerNum_ == 1 && (button[3-1] || button[2-1]) && !galagaMode)
					twoPlayerLinked = FALSE;
				
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
						
						if (abs(linkGunDirec - tempR) < 0.3)
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
						twoPlayerLinked = FALSE;
				}
				
				leftOptionIsSpecial  = options[option1Item].tr;
				rightOptionIsSpecial = options[option2Item].tr;
				
			} /*if (*playerAlive_) ...*/
		} /*if (!endLevel) ...*/


		if (levelEnd > 0 &&
		    !*playerAlive_ && (!twoPlayerMode || !playerAliveB))
			reallyEndLevel = TRUE;
		/* End Level Fade-Out */
		if (*playerAlive_ && endLevel)
		{
			
			if (levelEnd == 0)
				reallyEndLevel = TRUE;
			else {
				*PY_ -= levelEndWarp;
				if (*PY_ < -200)
					reallyEndLevel = TRUE;
				
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
				JE_initPlayerShot(0, 5, *PX_ + 1 + ROUND(sin(linkGunDirec + 0.2) * 26), *PY_ + ROUND(cos(linkGunDirec + 0.2) * 26),
				                  *mouseX_, *mouseY_, 148, playerNum_);
				shotMultiPos[5-1] = 0;
				JE_initPlayerShot(0, 5, *PX_ + 1 + ROUND(sin(linkGunDirec - 0.2) * 26), *PY_ + ROUND(cos(linkGunDirec - 0.2) * 26),
				                  *mouseX_, *mouseY_, 148, playerNum_);
				shotMultiPos[5-1] = 0;
				JE_initPlayerShot(0, 5, *PX_ + 1 + ROUND(sin(linkGunDirec) * 26), *PY_ + ROUND(cos(linkGunDirec) * 26),
				                  *mouseX_, *mouseY_, 147, playerNum_);
				
				if (shotRepeat[2-1] > 0)
					shotRepeat[2-1]--;
				else
					if (button[1-1])
					{
						shotMultiPos[2-1] = 0;
						JE_initPlayerShot(0, 2, *PX_ + 1 + ROUND(sin(linkGunDirec) * 20), *PY_ + ROUND(cos(linkGunDirec) * 20),
						                  *mouseX_, *mouseY_, linkGunWeapons[pItems_[2-1]-1], playerNum_);
						playerShotData[b-1].shotXM = -ROUND(sin(linkGunDirec) * playerShotData[b-1].shotYM);
						playerShotData[b-1].shotYM = -ROUND(cos(linkGunDirec) * playerShotData[b-1].shotYM);
						
						switch (pItems_[2-1])
						{
							case 27:
							case 32:
							case 10:
								temp = ROUND(linkGunDirec * 2.54647908948);  /*16 directions*/
								playerShotData[b-1].shotGr = linkMultiGr[temp];
								break;
							case 28:
							case 33:
							case 11:
								temp = ROUND(linkGunDirec * 2.54647908948);  /*16 directions*/
								playerShotData[b-1].shotGr = linkSonicGr[temp];
								break;
							case 30:
							case 35:
							case 14:
								if (linkGunDirec > M_PI / 2 && linkGunDirec < M_PI + M_PI / 2)
								{
									playerShotData[b-1].shotYC = 1;
								}
								break;
							case 38:
							case 22:
								temp = ROUND(linkGunDirec * 2.54647908948);  /*16 directions*/
								playerShotData[b-1].shotGr = linkMult2Gr[temp];
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
						portConfigChange = TRUE;
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
								/* TODO JE_portConfigs();*/
								if (portConfig[2-1] > tempW)
								{
									portConfig[2-1] = 1;
								}
							
							}
							
							JE_drawPortConfigButtons();
							portConfigDone = FALSE;
						}
					}
					
					/* PLAYER SHOT Creation */
					
					/*SpecialShot*/
					if (!galagaMode)
						/* TODO JE_doSpecialShot(playerNum_, armorLevel_, shield_)*/;
					
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
power = 10000; /** <-- free energy machine **/
							for (temp = min; temp <= max; temp++)
								if (pItems_[temp-1] > 0)
								{
									if (shotRepeat[temp-1] > 0)
										shotRepeat[temp-1]--;
									else
										if (button[1-1])
											JE_initPlayerShot(pItems_[temp-1], temp, *PX_, *PY_, *mouseX_, *mouseY_,
											                  weaponPort[pItems_[temp-1]].op[portConfig[temp-1]-1]
											                  [portPower[temp-1] * !galagaMode + galagaMode - 1],
											                  playerNum_);
								}
					}
					
					/*Super Charge Weapons*/
					if (playerNum_ == 2)
					{
						
						if (!twoPlayerLinked)
							JE_drawShape2(*PX_ + (shipGr_ == 0) + 1, *PY_ - 13, 77 + chargeLevel + chargeGr * 19,
							              eShapes6);
						
						if (chargeGrWait > 0)
							chargeGrWait--;
						else
						{
							chargeGr++;
							if (chargeGr == 4)
								chargeGr = 0;
							chargeGrWait = 3;
						}
						
						if (chargeLevel > 0)
							JE_bar(269, 107 + (chargeLevel - 1) * 3, 275, 108 + (chargeLevel - 1) * 3, 193); /* NOTE vga256c*/
						
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
							JE_bar(269, 107 + (chargeLevel - 1) * 3, 275, 108 + (chargeLevel - 1) * 3, 204); /* NOTE vga256c*/
						
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
									JE_bar(269, 107 + (chargeLevel - 1) * 3, 275, 108 + (chargeLevel - 1) * 3, 193); /* NOTE vga256c*/
								
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
							option1X = *PX_ + ROUND(sin(optionSatelliteRotate) * 20);
							option1Y = *PY_ + ROUND(cos(optionSatelliteRotate) * 20);
							break;
					}
					
					
					switch (rightOptionIsSpecial)
					{
	                    case 4:
							if (leftOptionIsSpecial != 4)
								optionSatelliteRotate += 0.15;
							option2X = *PX_ - ROUND(sin(optionSatelliteRotate) * 20);
							option2Y = *PY_ - ROUND(cos(optionSatelliteRotate) * 20);
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
										optionAttachmentLinked = TRUE;
										soundQueue[3-1] = 23;
									}
									
									if (button[3-1])
										optionAttachmentReturn = TRUE;
								} else {
									optionAttachmentMove += 1 + optionAttachmentReturn;
									JE_setupExplosion(option2X + 1, option2Y + 10, 0);
								}
								
							} else {
								option2X = *PX_;
								option2Y = *PY_ - 20;
								if (button[3-1])
								{
									optionAttachmentLinked = FALSE;
									optionAttachmentReturn = FALSE;
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
										/* TODO JE_barDrawDirect (284, option1Draw + 13, option1AmmoMax, 112, option1Ammo, 2, 2);*/ /*Option1Ammo*/
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
										optionAni1Go = TRUE;
										optionCharge1Wait = 20;
										optionCharge1 = 0;
										option1Ammo--;
										JE_bar(284, option1Draw + 13, 312, option1Draw + 15, 0); /* NOTE vga256c */
										/* TODO JE_barDrawDirect(284, option1Draw + 13, option1AmmoMax, 112, option1Ammo, 2, 2);*/
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
											optionAni1Go = TRUE;
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
										/* TODO JE_barDrawDirect(284, option2Draw + 13, option2AmmoMax, 112, option2Ammo, 2, 2);*/
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
										optionAni2Go = TRUE;
										option2Ammo--;
										JE_bar(284, option2Draw + 13, 312, option2Draw + 15, 0); /* NOTE vga256c */
										/* TODO JE_barDrawDirect(284, option2Draw + 13, option2AmmoMax, 112, option2Ammo, 2, 2);*/
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
											optionAni2Go = TRUE;
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

	useButtonAssign = TRUE;

	if (endLevel && levelEnd > 0)
	{
		levelEnd--;
		levelEndWarp++;
	}

	/*Reset Street-Fighter commands*/
	memset(SFExecuted, 0, sizeof(SFExecuted));

	makeMouseDelay = TRUE;
	portConfigChange = FALSE;
	
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
	STUB(JE_getName);
	
	return NULL;
}

void JE_playerCollide( JE_integer *px, JE_integer *py, JE_integer *lastTurn, JE_integer *lastTurn2,
                       JE_longint *score, JE_integer *armorLevel, JE_shortint *shield, JE_boolean *playerAlive,
                       JE_byte *playerStillExploding, JE_byte playerNum, JE_byte playerInvulnerable )
{
	STUB(JE_playerCollide);
}


/*void JE_textMenuWait ( JE_word waittime, JE_boolean dogamma )
{
	STUB(JE_textMenuWait );
}*/
