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
#include "varz.h"
#include "fonthand.h"
#include "vga256d.h"
#include "helptext.h"
#include "picload.h"
#include "starfade.h"
#include "newshape.h"
#include "nortsong.h"
#include "keyboard.h"
#include "config.h"
#include "episodes.h"
#include "setup.h"
#include "helptext.h"
#include "sndmast.h"
#include "shpmast.h"
#include "error.h"
#include "params.h"
#include "joystick.h"
#include "network.h"
#include "pcxload.h"

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
void JE_drawTextWindow( JE_string text )
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

void JE_outCharGlow( JE_word x, JE_word y, JE_string s )
{
	JE_integer maxloc, loc, z;
	JE_shortint glowcol[60]; /* [1..60] */
	JE_shortint glowcolc[60]; /* [1..60] */
	JE_word textloc[60]; /* [1..60] */
	JE_byte b, bank;

	setjasondelay2(1);

	/* TODO JE_setNetByte(0);*/

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

					/* TODO JE_updateStream();*/
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
					do
						if (levelWarningDisplay)
						{
							JE_updateWarning();
						}
						/* TODO JE_updateStream();*/
					while (!(delaycount() == 0 || ESCPressed));
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

					tempw = 0;
					JE_textMenuWait(&tempw, FALSE);
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
		tempw = 0;
		JE_textMenuWait(&tempw, FALSE);

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
		tempw = 0;
		JE_textMenuWait(&tempw, FALSE);

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
		tempw = 0;
		JE_textMenuWait(&tempw, FALSE);

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
	fread(*shapes, 1, shapeSize, f);
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
		JE_resetFileExt(&f, "TYRIANC.SHP", FALSE);
	} else {
		JE_resetFileExt(&f, "TYRIAN.SHP", FALSE);
	}

	fread(&shpNumb, 2, 1, f);
	for (x = 0; x < shpNumb; x++)
	{
		fread(&shpPos[x], sizeof(shpPos[x]), 1, f);
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
	JE_byte sel, screen, min, max;
	JE_string tempstr;
	JE_string tempstr2;
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
					temp2 = 250; /* - BYTE (saveFiles [x] .level = 0) SHL 1; */
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
		
		tempw = 0;
		JE_textMenuWait(&tempw, FALSE);
		
		
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
					if (FALSE) /* (saveFiles[sel.level > 0) */
					{
                        /*playsamplenum (_Select);
                        performsave := FALSE;
                        Operation (sel);*/
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
