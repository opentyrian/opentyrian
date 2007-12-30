/* vim: set noet:
 *
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Development Team
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
#include "tyrian2.h"

#include "animlib.h"
#include "backgrnd.h"
#include "destruct.h"
#include "episodes.h"
#include "error.h"
#include "fonthand.h"
#include "helptext.h"
#include "joystick.h"
#include "keyboard.h"
#include "loudness.h"
#include "lvllib.h"
#include "lvlmast.h"
#include "mainint.h"
#include "network.h"
#include "newshape.h"
#include "nortsong.h"
#include "pallib.h"
#include "params.h"
#include "pcxmast.h"
#include "picload.h"
#include "setup.h"
#include "sndmast.h"
#include "starfade.h"
#include "varz.h"
#include "vga256d.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


JE_word statDmg[2]; /* [1..2] */
JE_byte planetAni, planetAniWait;
JE_byte currentDotNum, currentDotWait;
JE_real navX, navY, newNavX, newNavY;
JE_integer tempNavX, tempNavY;
JE_byte planetDots[5]; /* [1..5] */
JE_integer planetDotX[5][10], planetDotY[5][10]; /* [1..5, 1..10] */

/* Level Event Data */
JE_boolean quit, first, loadLevelOk, refade;
/*JE_byte newPal, curPal, oldPal;*/
int newPal, curPal, oldPal; /* SYN: Originally bytes, I hope this doesn't break anything */

JE_word yLoc;
JE_shortint yChg;

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

void JE_starShowVGA( void )
{
	JE_byte *src;
	Uint8 *s = NULL; /* screen pointer, 8-bit specific */

	int x, y, lightx, lighty, lightdist;
	
	if (!playerEndLevel && !skipStarShowVGA)
	{

		s = VGAScreenSeg->pixels;

		src = game_screen->pixels;
		src += 24;

		if (smoothScroll != 0 && thisPlayerNum != 2)
		{
			int delaycount_temp;
			if ((delaycount_temp = target - SDL_GetTicks()) > 0)
				SDL_Delay(delaycount_temp);
			setjasondelay(frameCountMax);
		}

		if (starShowVGASpecialCode == 1)
		{
			src += game_screen->w * 183;
			for (y = 0; y < 184; y++)
			{
				memmove(s, src, 264);
				s += VGAScreenSeg->w;
				src -= game_screen->w;
			}
		} else if (starShowVGASpecialCode == 2 && processorType >= 2) {
			lighty = 172 - PY;
			lightx = 281 - PX;
			
			for (y = 184; y; y--)
			{
				if (lighty > y)
				{
					for (x = 320 - 56; x; x--)
					{
						*s = (*src & 0xf0) | ((*src >> 2) & 0x03);
						s++;
						src++;
					}
				} else {
					for (x = 320 - 56; x; x--)
					{
						lightdist = abs(lightx - x) + lighty;
						if (lightdist < y)
						{
							*s = *src;
						} else if (lightdist - y <= 5) {
							*s = (*src & 0xf0) | (((*src & 0x0f) + (3 * (5 - (lightdist - y)))) / 4);
						} else {
							*s = (*src & 0xf0) | ((*src & 0x0f) >> 2);
						}
						s++;
						src++;
					}
				}
				s += 56 + VGAScreenSeg->w - 320;
				src += 56 + VGAScreenSeg->w - 320;
			}
		} else {
			for (y = 0; y < 184; y++)
			{
				memmove(s, src, 264);
				s += VGAScreenSeg->w;
				src += game_screen->w;
			}
		}
		JE_showVGA();
	}

	quitRequested = false;
	skipStarShowVGA = false;
}

void JE_newEnemy( int enemyOffset )
{
	int i;

	b = 0;

	for(i = enemyOffset; i < enemyOffset + 25; i++)
	{
		if (enemyAvail[i] == 1)
		{
			b = i + 1;
			break;
		}
	}

	if (b == 0)
	{
		return;
	}

	JE_makeEnemy(&enemy[b-1]);
	enemyAvail[b-1] = a;
}

void JE_drawEnemy( int enemyOffset )
{
	int i, j;

	JE_byte *p; /* shape pointer */
	Uint8 *s; /* screen pointer, 8-bit specific */
	Uint8 *s_limit; /* buffer boundary */

	PX -= 25;

	for (i = enemyOffset - 25; i < enemyOffset; i++)
	{
		if (enemyAvail[i] != 1)
		{
			enemy[i].mapoffset = tempMapXOfs;

			if (enemy[i].xaccel && enemy[i].xaccel - 89 > rand() % 11)
			{
				if (PX > enemy[i].ex)
				{
					if (enemy[i].exc < enemy[i].xaccel - 89)
					{
						enemy[i].exc++;
					}
				} else {
					if (enemy[i].exc >= 0 || -enemy[i].exc < enemy[i].xaccel - 89)
					{
						enemy[i].exc--;
					}
				}
			}

			if (enemy[i].yaccel && enemy[i].yaccel - 89 > rand() % 11)
			{
				if (PY > enemy[i].ey)
				{
					if (enemy[i].eyc < enemy[i].yaccel - 89)
					{
						enemy[i].eyc++;
					}
				} else {
					if (enemy[i].eyc >= 0 || -enemy[i].eyc < enemy[i].yaccel - 89)
					{
						enemy[i].eyc--;
					}
				}
			}

 			if (enemy[i].ex + tempMapXOfs > -29 && enemy[i].ex + tempMapXOfs < 300)
			{
				if (enemy[i].aniactive == 1)
				{
					enemy[i].enemycycle++;

					if (enemy[i].enemycycle == enemy[i].animax)
					{
						enemy[i].aniactive = enemy[i].aniwhenfire;
					} else {
						if (enemy[i].enemycycle > enemy[i].ani)
						{
							enemy[i].enemycycle = enemy[i].animin;
						}
					}
				}

				if (enemy[i].egr[enemy[i].enemycycle - 1] == 999)
				{
					goto enemy_gone;
				}

				if (enemy[i].size == 1)
				{
					/* -------------------------  LARGE ENEMY -- Draw a 2x2 enemy ---------- */

					if (enemy[i].ey > -13)
					{
						s = (Uint8 *)VGAScreen->pixels;
						s += (enemy[i].ey - 7) * VGAScreen->w + (enemy[i].ex - 6) + tempMapXOfs;

						s_limit = (Uint8 *)VGAScreen->pixels;
						s_limit += VGAScreen->h * VGAScreen->w;

						p = enemy[i].shapeseg;
						p += SDL_SwapLE16(((JE_word *)p)[enemy[i].egr[enemy[i].enemycycle - 1] - 1]);

						while (*p != 0x0f)
						{
							s += *p & 0x0f;
							j = (*p & 0xf0) >> 4;
							if (j)
							{
								while (j--)
								{
									p++;
									if (s >= s_limit)
										goto enemy_draw_overflow;
									if ((void *)s >= VGAScreen->pixels)
										*s = (enemy[i].filter == 0) ? *p : (*p & 0x0f) | enemy[i].filter;
									s++;
								}
							} else {
								s -= 12;
								s += VGAScreen->w;
							}
							p++;
						}

						s = (Uint8 *)VGAScreen->pixels;
						s += (enemy[i].ey - 7) * VGAScreen->w + (enemy[i].ex + 6) + tempMapXOfs;

						p = enemy[i].shapeseg;
						p += SDL_SwapLE16(((JE_word *)p)[enemy[i].egr[enemy[i].enemycycle - 1]]);

						while (*p != 0x0f)
						{
							s += *p & 0x0f;
							j = (*p & 0xf0) >> 4;
							if (j)
							{
								while (j--)
								{
									p++;
									if (s >= s_limit)
										goto enemy_draw_overflow;
									if ((void *)s >= VGAScreen->pixels)
										*s = (enemy[i].filter == 0) ? *p : (*p & 0x0f) | enemy[i].filter;
									s++;
								}
							} else {
								s -= 12;
								s += VGAScreen->w;
							}
							p++;
						}
					}

					if (enemy[i].ey > -26 && enemy[i].ey < 182)
					{
						s = (Uint8 *)VGAScreen->pixels;
						s += (enemy[i].ey + 7) * VGAScreen->w + (enemy[i].ex - 6) + tempMapXOfs;

						s_limit = (Uint8 *)VGAScreen->pixels;
						s_limit += VGAScreen->h * VGAScreen->w;

						p = enemy[i].shapeseg;
						p += SDL_SwapLE16(((JE_word *)p)[enemy[i].egr[enemy[i].enemycycle - 1] + 18]);

						while (*p != 0x0f)
						{
							s += *p & 0x0f;
							j = (*p & 0xf0) >> 4;
							if (j)
							{
								while (j--)
								{
									p++;
									if (s >= s_limit)
										goto enemy_draw_overflow;
									if ((void *)s >= VGAScreen->pixels)
										*s = (enemy[i].filter == 0) ? *p : (*p & 0x0f) | enemy[i].filter;
									s++;
								}
							} else {
								s -= 12;
								s += VGAScreen->w;
							}
							p++;
						}

						s = (Uint8 *)VGAScreen->pixels;
						s += (enemy[i].ey + 7) * VGAScreen->w + (enemy[i].ex + 6) + tempMapXOfs;

						p = enemy[i].shapeseg;
						p += SDL_SwapLE16(((JE_word *)p)[enemy[i].egr[enemy[i].enemycycle - 1] + 19]);

						while (*p != 0x0f)
						{
							s += *p & 0x0f;
							j = (*p & 0xf0) >> 4;
							if (j)
							{
								while (j--)
								{
									p++;
									if (s >= s_limit)
										goto enemy_draw_overflow;
									if ((void *)s >= VGAScreen->pixels)
										*s = (enemy[i].filter == 0) ? *p : (*p & 0x0f) | enemy[i].filter;
									s++;
								}
							} else {
								s -= 12;
								s += VGAScreen->w;
							}
							p++;
						}
					}

				} else {
					if (enemy[i].ey > -13)
					{
						s = (Uint8 *)VGAScreen->pixels;
						s += enemy[i].ey * VGAScreen->w + enemy[i].ex + tempMapXOfs;

						s_limit = (Uint8 *)VGAScreen->pixels;
						s_limit += VGAScreen->h * VGAScreen->w;

						p = enemy[i].shapeseg;
						p += SDL_SwapLE16(((JE_word *)p)[enemy[i].egr[enemy[i].enemycycle - 1] - 1]);

						while (*p != 0x0f)
						{
							s += *p & 0x0f;
							j = (*p & 0xf0) >> 4;
							if (j)
							{
								while (j--)
								{
									p++;
									if (s >= s_limit)
										goto enemy_draw_overflow;
									if ((void *)s >= VGAScreen->pixels)
										*s = (enemy[i].filter == 0) ? *p : (*p & 0x0f) | enemy[i].filter;
									s++;
								}
							} else {
								s -= 12;
								s += VGAScreen->w;
							}
							p++;
						}
					}
				}
enemy_draw_overflow:
				enemy[i].filter = 0;
			}

			if (enemy[i].excc)
			{
				if (--enemy[i].exccw <= 0)
				{
					if (enemy[i].exc == enemy[i].exrev)
					{
						enemy[i].excc = -enemy[i].excc;
						enemy[i].exrev = -enemy[i].exrev;
						enemy[i].exccadd = -enemy[i].exccadd;
					} else {
						enemy[i].exc += enemy[i].exccadd;
						enemy[i].exccw = enemy[i].exccwmax;
						if (enemy[i].exc == enemy[i].exrev)
						{
							enemy[i].excc = -enemy[i].excc;
							enemy[i].exrev = -enemy[i].exrev;
							enemy[i].exccadd = -enemy[i].exccadd;
						}
					}
				}
			}

			if (enemy[i].eycc)
			{
				if (--enemy[i].eyccw <= 0)
				{
					if (enemy[i].eyc == enemy[i].eyrev)
					{
						enemy[i].eycc = -enemy[i].eycc;
						enemy[i].eyrev = -enemy[i].eyrev;
						enemy[i].eyccadd = -enemy[i].eyccadd;
					} else {
						enemy[i].eyc += enemy[i].eyccadd;
						enemy[i].eyccw = enemy[i].eyccwmax;
						if (enemy[i].eyc == enemy[i].eyrev)
						{
							enemy[i].eycc = -enemy[i].eycc;
							enemy[i].eyrev = -enemy[i].eyrev;
							enemy[i].eyccadd = -enemy[i].eyccadd;
						}
					}
				}
			}

			enemy[i].ey += enemy[i].fixedmovey;

			enemy[i].ex += enemy[i].exc;
			if (enemy[i].ex < -80 || enemy[i].ex > 340)
			{
				goto enemy_gone;
			}

			enemy[i].ey += enemy[i].eyc;
			if (enemy[i].ey < -112 || enemy[i].ey > 190)
			{
				goto enemy_gone;
			}

			goto enemy_still_exists;

enemy_gone:
			/* TODO ? */
			enemyAvail[i] = 1;
			goto draw_enemy_end;

enemy_still_exists:

			/*X bounce*/
			if (enemy[i].ex <= enemy[i].xminbounce || enemy[i].ex >= enemy[i].xmaxbounce)
			{
				enemy[i].exc = -enemy[i].exc;
			}

			/*Y bounce*/
			if (enemy[i].ey <= enemy[i].yminbounce || enemy[i].ey >= enemy[i].ymaxbounce)
			{
				enemy[i].eyc = -enemy[i].eyc;
			}

			/* Evalue != 0 - score item at boundary */
			if (enemy[i].scoreitem)
			{
				if (enemy[i].ex < -5)
				{
					enemy[i].ex++;
				}
				if (enemy[i].ex > 245)
				{
					enemy[i].ex--;
				}
			}

			enemy[i].ey += tempBackMove;

			if (enemy[i].ex <= -24 || enemy[i].ex >= 296)
				goto draw_enemy_end;

			tempX = enemy[i].ex;
			tempY = enemy[i].ey;

			temp = enemy[i].enemytype;

			/* Enemy Shots */
			if (enemy[i].edamaged == 1)
				goto draw_enemy_end;

			enemyOnScreen++;

			if (enemy[i].iced)
			{
				enemy[i].iced--;
				if (enemy[i].enemyground != 0)
				{
					enemy[i].filter = 9;
				}
				goto draw_enemy_end;
			}

			for (j = 3; j > 0; j--)
			{
				if (enemy[i].freq[j-1])
				{
					temp3 = enemy[i].tur[j-1];

					if (--enemy[i].eshotwait[j-1] == 0 && temp3)
					{
						enemy[i].eshotwait[j-1] = enemy[i].freq[j-1];
						if (difficultyLevel > 2)
						{
							enemy[i].eshotwait[j-1] = (enemy[i].eshotwait[j-1] / 2) + 1;
							if (difficultyLevel > 7)
							{
								enemy[i].eshotwait[j-1] = (enemy[i].eshotwait[j-1] / 2) + 1;
							}
						}

						if (galagaMode && (enemy[i].eyc == 0 || (rand() % 400) >= galagaShotFreq))
							goto draw_enemy_end;

						/* <MXD> replace with a switch */
						if (temp3 == 252) /* Savara Boss DualMissile */
						{
							if (enemy[i].ey > 20)
							{
								explosionMoveUp = -640;
								JE_setupExplosion(tempX - 8 + tempMapXOfs, tempY - 20 - backMove * 8, 6);
								JE_setupExplosion(tempX + 4 + tempMapXOfs, tempY - 20 - backMove * 8, 6);
								explosionMoveUp = 0;
							}
						} else if (temp3 > 250) {
							if (temp3 == 251) /* Suck-O-Magnet */
							{
								tempI4 = 4 - (abs(PX - tempX) + abs(PY - tempY)) / 100;
								if (PX > tempX)
								{
									lastTurn2 -= tempI4;
								} else {
									lastTurn2 += tempI4;
								}
							} else if (temp3 == 253) /* Left ShortRange Magnet */
							{
								if (abs(PX + 25 - 14 - tempX) < 24 && abs(PY - tempY) < 28)
								{
									lastTurn2 += 2;
								}
								if (twoPlayerMode &&
								   (abs(PXB - 14 - tempX) < 24 && abs(PYB - tempY) < 28))
								{
									lastTurn2B += 2;
								}
							} else if (temp3 == 254) /* Left ShortRange Magnet */
							{
								if (abs(PX + 25 - 14 - tempX) < 24 && abs(PY - tempY) < 28)
								{
									lastTurn2 -= 2;
								}
								if (twoPlayerMode &&
								   (abs(PXB - 14 - tempX) < 24 && abs(PYB - tempY) < 28))
								{
									lastTurn2B -= 2;
								}
							} else if (temp3 == 255) /* Magneto RePulse!! */
							{
								if (difficultyLevel != 1) /*DIF*/
								{
									if (j == 3)
									{
										enemy[i].filter = 112;
									} else {
										tempI4 = 4 - (abs(PX - tempX) + abs(PY - tempY)) / 20;
										if (tempI4 > 0)
										{
											if (PX > tempX)
											{
												lastTurn2 += tempI4;
											} else {
												lastTurn2 -= tempI4;
											}
										}
									}
								}
							}
						} else {

							/*Rot*/
							for (tempCount = weapons[temp3].multi; tempCount > 0; tempCount--)
							{
								for (b = 0; b < ENEMY_SHOT_MAX; b++)
								{
									if (enemyShotAvail[b] == 1)
									{
										break;
									}
								}
								if (b == ENEMY_SHOT_MAX)
								{
									goto draw_enemy_end;
								}

								enemyShotAvail[b]--;

								if (weapons[temp3].sound > 0) {
									do {
										temp = rand() % 8;
									} while (temp == 3);
									soundQueue[temp] = weapons[temp3].sound;
								}

								tempPos = weapons[temp3].max;

								if (enemy[i].aniactive == 2)
								{
									enemy[i].aniactive = 1;
								}

								if (++enemy[i].eshotmultipos[j-1] > tempPos)
								{
									enemy[i].eshotmultipos[j-1] = 1;
								}
								tempPos = enemy[i].eshotmultipos[j-1];

								if (j == 1)
								{
									temp2 = 4;
								}

								enemyShot[b].sx = tempX + weapons[temp3].bx[tempPos-1] + tempMapXOfs;
								enemyShot[b].sy = tempY + weapons[temp3].by[tempPos-1];
								enemyShot[b].sdmg = weapons[temp3].attack[tempPos-1];
								enemyShot[b].tx = weapons[temp3].tx;
								enemyShot[b].ty = weapons[temp3].ty;
								enemyShot[b].duration = weapons[temp3].del[tempPos-1];
								enemyShot[b].animate = 0;
								enemyShot[b].animax = weapons[temp3].weapani;

								enemyShot[b].sgr = weapons[temp3].sg[tempPos-1];
								switch (j)
								{
									case 1:
										enemyShot[b].syc = weapons[temp3].acceleration;
										enemyShot[b].sxc = weapons[temp3].accelerationx;

										enemyShot[b].sxm = weapons[temp3].sx[tempPos-1];
										enemyShot[b].sym = weapons[temp3].sy[tempPos-1];
										break;
									case 3:
										enemyShot[b].sxc = -weapons[temp3].acceleration;
										enemyShot[b].syc = weapons[temp3].accelerationx;

										enemyShot[b].sxm = -weapons[temp3].sy[tempPos-1];
										enemyShot[b].sym = -weapons[temp3].sx[tempPos-1];
										break;
									case 2:
										enemyShot[b].sxc = weapons[temp3].acceleration;
										enemyShot[b].syc = -weapons[temp3].acceleration;

										enemyShot[b].sxm = weapons[temp3].sy[tempPos-1];
										enemyShot[b].sym = -weapons[temp3].sx[tempPos-1];
										break;
								}

								if (weapons[temp3].aim > 0)
								{
									temp4 = weapons[temp3].aim;

									/*DIF*/
									if (difficultyLevel > 2)
									{
										temp4 += difficultyLevel - 2;
									}

									tempX2 = PX;
									tempY2 = PY;

									if (twoPlayerMode)
									{

										if (playerAliveB && !playerAlive)
										{
											temp = 1;
										} else if (playerAlive && !playerAliveB)
										{
											temp = 0;
										} else {
											temp = rand() % 2;
										}

										if (temp == 1)
										{
											tempX2 = PXB - 25;
											tempY2 = PYB;
										}
									}

									tempI = (tempX2 + 25) - tempX - tempMapXOfs - 4;
									if (tempI == 0)
									{
										tempI++;
									}
									tempI2 = tempY2 - tempY;
									if (tempI2 == 0)
									{
										tempI2++;
									}
									if (abs(tempI) > abs(tempI2))
									{
										tempI3 = abs(tempI);
									} else {
										tempI3 = abs(tempI2);
									}
									enemyShot[b].sxm = round(((float)tempI / tempI3) * temp4);
									enemyShot[b].sym = round(((float)tempI2 / tempI3) * temp4);
								}
							}

						}
					}
				}
			}

			/* Enemy Launch Routine */
			if (enemy[i].launchfreq)
			{
				if (--enemy[i].launchwait == 0)
				{
					enemy[i].launchwait = enemy[i].launchfreq;

					if (enemy[i].launchspecial != 0)
					{
						/*Type  1 : Must be inline with player*/
						if (abs(enemy[i].ey - PY) > 5)
							goto draw_enemy_end;
					}

					if (enemy[i].aniactive == 2)
					{
						enemy[i].aniactive = 1;
					}

					if (enemy[i].launchtype == 0)
						goto draw_enemy_end;

					tempW = enemy[i].launchtype;
					JE_newEnemy(enemyOffset == 50 ? 75 : enemyOffset - 25);

					/*Launch Enemy Placement*/
					if (b > 0)
					{
						tempI = tempX;
						tempI2 = tempY + enemyDat[enemy[b-1].enemytype].startyc;
						if (enemy[b-1].size == 0)
						{
							tempI -= 0;
							tempI2 -= 7;
						}
						if (enemy[b-1].launchtype > 0 && enemy[b-1].launchfreq == 0)
						{

							if (enemy[b-1].launchtype > 90)
							{
								tempI += rand() % ((enemy[b-1].launchtype - 90) * 4) - (enemy[b-1].launchtype - 90) * 2;
							} else {
								tempI4 = (PX + 25) - tempX - tempMapXOfs - 4;
								if (tempI4 == 0)
								{
									tempI4++;
								}
								tempI5 = PY - tempY;
								if (tempI5 == 0)
								{
									tempI5++;
								}
								if (abs(tempI4) > abs(tempI5))
								{
									tempI3 = abs(tempI4);
								} else {
									tempI3 = abs(tempI5);
								}
								enemy[b-1].exc = round(((float)tempI4 / tempI3) * enemy[b-1].launchtype);
								enemy[b-1].eyc = round(((float)tempI5 / tempI3) * enemy[b-1].launchtype);
							}
						}

						do {
							temp = rand() % 8;
						} while (temp == 3);
						soundQueue[temp] = randomEnemyLaunchSounds[(rand() % 3)];

						if (enemy[i].launchspecial == 1
							&& enemy[i].linknum < 100)
						{
							enemy[b-1].linknum = enemy[i].linknum;
						}

						enemy[b-1].ex = tempI;
						enemy[b-1].ey = tempI2;
					}
				}
			}
		}
draw_enemy_end:
		;
	}

	PX += 25;
}

void JE_main( void )
{
	int i, j, l;
	JE_byte **bp;

	JE_byte *p; /* source/shape pointer */
	Uint8 *s; /* screen pointer, 8-bit specific */
	Uint8 *s_limit; /* buffer boundary */

	SDL_Surface *swapper;

	char buffer[256];

	int lastEnemyOnScreen;

	loadTitleScreen = true;

	/* Setup Player Items/General Data */
	fixedExplosions = false;
	explosionMoveUp = 0;
	for (z = 0; z < 12; z++)
	{
		pItems[z] = 0;
	}
	shieldSet = 5;

	/* Setup Graphics */
	JE_updateColorsFast(&black);

	/*debuginfo('Initiating Configuration');*/


	/* Setup Input Device */
	/*JConfigure:=false;*/

	debug = false;

	/* NOTE: BEGIN MAIN PROGRAM HERE AFTER LOADING A GAME OR STARTING A NEW ONE */

	/* ----------- GAME ROUTINES ------------------------------------- */
	/* We need to jump to the beginning to make space for the routines */
	/* --------------------------------------------------------------- */
	goto start_level_first;

	/* TODO */

	/*------------------------------GAME LOOP-----------------------------------*/


	/* Startlevel is called after a previous level is over.  If the first level
	   is started for a gaming session, startlevelfirst is called instead and
	   this code is skipped.  The code here finishes the level and prepares for
	   the loadmap function. */

start_level:

	if (galagaMode)
	{
		twoPlayerMode = false;
	}

	if (playDemo)
	{
		JE_selectSong(0);
	}

	useButtonAssign = true; /*Joystick button remapping*/

	/*Set keyboard to NO BIOS*/
	/* TODO callBIOShandler = true; */
	/* TODO JE_clearKeyboard(); */

	if (eShapes1 != NULL)
	{
		free(eShapes1);
		eShapes1 = NULL;
	}
	if (eShapes2 != NULL)
	{
		free(eShapes2);
		eShapes2 = NULL;
	}
	if (eShapes3 != NULL)
	{
		free(eShapes3);
		eShapes3 = NULL;
	}
	if (eShapes4 != NULL)
	{
		free(eShapes4);
		eShapes4 = NULL;
	}

	/* Normal speed */
	if (fastPlay != 0)
	{
		smoothScroll = true;
		speed = 0x4300;
		JE_resetTimerInt();
		JE_setTimerInt();
	}

	if (recordDemo || playDemo)
	{
		fclose(recordFile);
		if (playDemo)
		{
			JE_fadeBlack(10);
			/* JE_wipekey();*/
			wait_noinput(true, true, joystick_installed);
		}
	}

	if (isNetworkGame)
	{
		netQuit = false;
	}

	difficultyLevel = oldDifficultyLevel;   /*Return difficulty to normal*/

	if (!playDemo)
	{
		if (((playerAlive || (twoPlayerMode && playerAliveB))
		   || normalBonusLevelCurrent || bonusLevelCurrent)
		   && !playerEndLevel)
		{
			mainLevel = nextLevel;
			JE_endLevelAni();
			JE_selectSong(0xC001);  /*Fade song out*/

		} else {

			outputData[0] = 127;   /*Send 127 value if Game ends before next level*/
			/* TODO JE_exchangePacket(1);*/
			JE_selectSong(0xC001);  /*Fade song out*/

			JE_fadeBlack(10);
			if (twoPlayerMode)
			{
				temp = 22;
			} else {
				temp = 11;
			}
			JE_loadGame(temp);
			if (doNotSaveBackup)
			{
				superTyrian = false;
				onePlayerAction = false;
				pItems[2] = 0;
			}
			if (bonusLevelCurrent && !playerEndLevel)
			{
				mainLevel = nextLevel;
			}
		}
	}
	doNotSaveBackup = false;

start_level_first:

	/*stopsequence;*/
	/*debuginfo('Setting Master Sound Volume');*/
	JE_setVol(tyrMusicVolume, fxVolume);

	JE_loadCompShapes(&shapes6, &shapes6Size, '1');  /* Items */

	endLevel = false;
	reallyEndLevel = false;
	playerEndLevel = false;
	extraGame = false;

	/*debuginfo('Loading LEVELS.DAT');*/

	doNotSaveBackup = false;
	JE_loadMap();
	JE_selectSong(0xC001); /*Fade song out*/

	playerAlive = true;
	playerAliveB = true;
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
		JE_destructGame();
		loadDestruct = false;
		loadTitleScreen = true;
		goto start_level_first;
	}

	PX = 100;
	PY = 180;

	PXB = 190;
	PYB = 180;

	playerHNotReady = true;

	lastPXShotMove = PX;
	lastPYShotMove = PY;

	if (twoPlayerMode)
	{
		JE_loadPic(6, false);
	} else {
		JE_loadPic(3, false);
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
	JE_gammaCorrect(&colors, gammaCorrection);
	JE_fadeColor(50);

	JE_loadCompShapes(&shapes6, &shapes6Size, '6'); /* Explosions */

	/* MAPX will already be set correctly */
	mapY = 300 - 8;
	mapY2 = 600 - 8;
	mapY3 = 600 - 8;
	mapYPos = &megaData1->mainmap[mapY][0] - 1;
	mapY2Pos = &megaData2->mainmap[mapY2][0] - 1;
	mapY3Pos = &megaData3->mainmap[mapY3][0] - 1;
	mapXPos = 0;
	mapXOfs = 0;
	mapX2Pos = 0;
	mapX3Pos = 0;
	mapX3Ofs = 0;
	mapXbpPos = 0;
	mapX2bpPos = 0;
	mapX3bpPos = 0;

	map1YDelay = 1;
	map1YDelayMax = 1;
	map2YDelay = 1;
	map2YDelayMax = 1;

	musicFade = false;

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

	newkey = false;

	/* Initialize Level Data and Debug Mode */
	levelEnd = 255;
	levelEndWarp = -4;
	levelEndFxWait = 0;
	warningCol = 120;
	warningColChange = 1;
	warningSoundDelay = 0;
	armorShipDelay = 50;

	bonusLevel = false;
	readyToEndLevel = false;
	firstGameOver = true;
	eventLoc = 1;
	curLoc = 0;
	backMove = 1;
	backMove2 = 2;
	backMove3 = 3;
	explodeMove = 640;
	enemiesActive = true;
	for(temp = 0; temp < 3; temp++)
	{
		button[temp] = false;
	}
	stopBackgrounds = false;
	stopBackgroundNum = 0;
	background3x1   = false;
	background3x1b  = false;
	background3over = 0;
	background2over = 1;
	topEnemyOver = false;
	skyEnemyOverAll = false;
	smallEnemyAdjust = false;
	starActive = true;
	enemyContinualDamage = false;
	levelEnemyFrequency = 96;
	quitRequested = false;

	memset(statBar, 0, sizeof(statBar));

	forceEvents = false;  /*Force events to continue if background movement = 0*/

	uniqueEnemy = false;  /*Look in MakeEnemy under shape bank*/

	superEnemy254Jump = 0;   /*When Enemy with PL 254 dies*/

	/* Filter Status */
	filterActive = true;
	filterFade = true;
	filterFadeStart = false;
	levelFilter = -99;
	levelBrightness = -14;
	levelBrightnessChg = 1;

	background2notTransparent = false;

	/* Initially erase power bars */
	lastPower = power / 10;

	/* Initial Text */
	JE_drawTextWindow(miscText[20]);

	/* Setup Armor/Shield Data */
	shieldWait = 1;
	shield     = shields[pItems[10-1]].mpwr;
	shieldT    = shields[pItems[10-1]].tpwr * 20;
	shieldMax  = shield * 2;
	shield2    = shields[pItemsPlayer2[10-1]].mpwr;
	shieldMax2 = shield * 2;
	JE_drawShield();
	JE_drawArmor();

	superBomb[0] = 0;
	superBomb[1] = 0;

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

	playerFollow = false;

	/*if not JConfigure and (InputDevice=1) then CalibrateJoy;*/

	JE_drawPortConfigButtons();

	/* --- MAIN LOOP --- */

	newkey = false;

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

	/* JE_setVol(tyrMusicVolume, fxPlayVol >> 2); NOTE: MXD killed this because it was broken */

	/*Save backup game*/
	if (!playDemo && !doNotSaveBackup)
	{
		temp = twoPlayerMode ? 22 : 11;
		JE_saveGame(temp, "LAST LEVEL    ");
	}

	/*Set keyboard to NO BIOS*/
	/* NOTE callBIOShandler = false; */

	memset(lastKey, 0, sizeof(lastKey));
	if (recordDemo && !playDemo)
	{
		dont_die = true;
		do
		{
			sprintf(tempStr, "demorec.%d", recordFileNum);
			tempb = JE_find(tempStr);
			if (tempb)
			{
				recordFileNum++;
			}
		} while (tempb);
		dont_die = false;

		recordFile = fopen_check(tempStr, "wb");
		if (!recordFile)
		{
			exit(1);
		}

		efwrite(&episodeNum, 1, 1, recordFile);
		efwrite(levelName, 1, 10, recordFile);
		efwrite(&lvlFileNum, 1, 1, recordFile);
		efwrite(pItems, 1, 12, recordFile);
		efwrite(portPower, 1, 5, recordFile);
		efwrite(&levelSong, 1, 1, recordFile);

		lastMoveWait = 0;
	}

	twoPlayerLinked = false;
	linkGunDirec = M_PI;

	JE_calcPurpleBall(1);
	JE_calcPurpleBall(2);

	damageRate = 2;  /*Normal Rate for Collision Damage*/

	chargeWait   = 5;
	chargeLevel  = 0;
	chargeMax    = 5;
	chargeGr     = 0;
	chargeGrWait = 3;

	portConfigChange = false;

	makeMouseDelay = false;

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

	yourInGameMenuRequest = false;

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
	for (i = 0; i < COUNTOF(enemyShotAvail); i++)
		enemyShotAvail[i] = 1;

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

	memset(SFCurrentCode,    0, sizeof(SFCurrentCode));
	memset(SFExecuted,       0, sizeof(SFExecuted));

	zinglonDuration = 0;
	specialWait = 0;
	nextSpecialWait = 0;
	optionAttachmentMove  = 0;    /*Launch the Attachments!*/
	optionAttachmentLinked = true;

	editShip1 = false;
	editShip2 = false;

	memset(smoothies, 0, sizeof(smoothies));

	levelTimer = false;
	randomExplosions = false;

	lastSP = 0;
	memset(SPZ, 0, sizeof(SPZ));

	returnActive = false;

	galagaShotFreq = 0;

	if (galagaMode)
	{
		difficultyLevel = 2;
	}
	galagaLife = 10000;

	JE_drawOptionLevel();

	BKwrap2 = &megaData2->mainmap[0][0];
	BKwrap2to = &megaData2->mainmap[0][0];

/* TODO <MXD> remove these */
debugHist = 1;
debugHistCount = 1;
lastDebugTime = SDL_GetTicks();
/* </MXD> */

level_loop:

	tempScreenSeg = game_screen; /* side-effect of game_screen */

	if (isNetworkGame)
	{
		smoothies[9-1] = false;
		smoothies[6-1] = false;
	} else {
		starShowVGASpecialCode = smoothies[9-1] + (smoothies[6-1] << 1);
	}

	/*Background Wrapping*/
	if (mapY2Pos <= BKwrap2)
	{
		mapY2Pos = BKwrap2to;
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
			musicFade = false;
		}
	}

	if (!allPlayersGone && levelEnd > 0 && endLevel)
	{
		JE_playSong(10);
		musicFade = false;
	} else {
		if (!playing && musicActive && firstGameOver)
		{
			JE_playSong(levelSong);
			playing = true;
		}
	}


	/* use game_screen for all the generic drawing functions */
	VGAScreen = game_screen;

	if (!endLevel)
	{    /*MAIN DRAWING IS STOPPED STARTING HERE*/
		
		/*-----------------------Message Bar------------------------*/
		if (textErase > 0)
		{
			if (--textErase == 0)
			{
				tempScreenSeg = VGAScreenSeg; /* <MXD> SEGa000 */
				JE_newDrawCShapeNum(OPTION_SHAPES, 37, 16, 189);
			}
		}

		VGAScreen = VGAScreenSeg; /* side-effect of game_screen */
		
		/*------------------------Shield Gen-------------------------*/
		if (galagaMode)
		{
			shield = 0;
			shield2 = 0;
			if (portPower[2-1] == 0 || armorLevel2 == 0)
			{
				twoPlayerMode = false;
			}
			if (score >= galagaLife)
			{
				soundQueue[6] = 11;
				soundQueue[7] = 21;
				if (portPower[1-1] < 11)
				{
					portPower[1-1]++;
				} else {
					score += 1000;
				}
				if (galagaLife == 10000)
				{
					galagaLife = 20000;
				} else {
					galagaLife += 25000;
				}
			}
		} else { /*GALAGA mode*/
			if (twoPlayerMode)
			{
				if (--shieldWait == 0)
				{
					shieldWait = 20 - shieldSet;
					if (shield < shieldMax && playerAlive)
					{
						shield++;
					}
					if (shield2 < shieldMax2 && playerAliveB)
					{
						shield2++;
					}
					JE_drawShield();
				}
			} else {
				if (playerAlive && shield < shieldMax && power > shieldT)
				{
					if (--shieldWait == 0)
					{
						shieldWait = 20 - shieldSet;
						power -= shieldT;
						shield++;
						if (shield2 < shieldMax)
						{
							shield2++;
						}
						JE_drawShield();
					}
				}
			}
		}
		
		/*---------------------Weapon Display-------------------------*/
		if (lastPortPower[1-1] != portPower[1-1])
		{
			lastPortPower[1-1] = portPower[1-1];
			
			if (twoPlayerMode)
			{
				tempW2 = 6;
				tempW = 286;
			} else {
				tempW2 = 17;
				tempW = 289;
			}
			
			JE_c_bar(tempW, tempW2, tempW + 1 + 10 * 2, tempW2 + 2, 0);
			
			for (temp = 1; temp <= portPower[1-1]; temp++)
			{
				JE_rectangle(tempW, tempW2, tempW + 1, tempW2 + 2, 115 + temp); /* <MXD> SEGa000 */
				tempW += 2;
			}
		}
		
		if (lastPortPower[2-1] != portPower[2-1])
		{
			lastPortPower[2-1] = portPower[2-1];
			
			if (twoPlayerMode)
			{
				tempW2 = 100;
				tempW = 286;
			} else {
				tempW2 = 38;
				tempW = 289;
			}
			
			JE_c_bar(tempW, tempW2, tempW + 1 + 10 * 2, tempW2 + 2, 0);
			
			for (temp = 1; temp <= portPower[2-1]; temp++)
			{
				JE_rectangle(tempW, tempW2, tempW + 1, tempW2 + 2, 115 + temp); /* <MXD> SEGa000 */
				tempW += 2;
			}
		}
		
		/*------------------------Power Bar-------------------------*/
		if (twoPlayerMode || onePlayerAction)
		{
			power = 900;
		} else {
			power = power + powerAdd;
			if (power > 900)
			{
				power = 900;
			}
			
			temp = power / 10;
			
			if (temp != lastPower)
			{
				if (temp > lastPower)
				{
					JE_c_bar(269, 113 - 11 - temp, 276, 114 - 11 - lastPower, 113 + temp / 7);
				} else {
					JE_c_bar(269, 113 - 11 - lastPower, 276, 114 - 11 - temp, 0);
				}
			}
			
			lastPower = temp;
		}

		oldMapX3Ofs = mapX3Ofs;

		enemyOnScreen = 0;
		
		VGAScreen = game_screen; /* side-effect of game_screen */

	}    /*MAIN DRAWING IS STOPPED ENDING   HERE*/

	/*---------------------------EVENTS-------------------------*/
	while (eventRec[eventLoc-1].eventtime <= curLoc && eventLoc <= maxEvent)
	{
		JE_eventSystem();
	}

	if (isNetworkGame && reallyEndLevel)
	{
		goto start_level;
	}


	/* SMOOTHIES! */
	JE_checkSmoothies();
	if (anySmoothies)
	{
		VGAScreen = smoothiesScreen;
	}

	/* --- BACKGROUNDS --- */
	/* --- BACKGROUND 1 --- */

	if (forceEvents && !backMove)
	{
		curLoc++;
	}

	if (map1YDelayMax > 1 && backMove < 2)
	{
		if (map1YDelay == 1)
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

		s = (Uint8 *)VGAScreen->pixels;

		/* Offset for top */
		s += 11 * 24;
		s += mapXPos;

		/* Map location number in BP */
		bp = mapYPos;
		bp += mapXbpPos;

		if (backPos)
		{
			/* --- BACKGROUND 1 TOP --- */
			for (i = 12; i; i--)
			{

				/* move to previous map X location */
				bp--;

				p = *bp;
				p += (28 - backPos) * 24;

				for (j = backPos; j; j--)
				{
					memcpy(s, p, 24);

					s += VGAScreen->w;
					p += 24;
				}

				s -= backPos * VGAScreen->w + 24;
			}

			s += 24 * 11;
			s += backPos * VGAScreen->w + 24;

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
				p = *bp;

				for (l = 0; l < 28; l++)
				{
					memcpy(s, p, 24);

					s += VGAScreen->w;
					p += 24;
				}

				s -= VGAScreen->w * 28 + 24;
			}

			/* Increment Map Location for next line */
			bp += 14 + 14 - 2;  /* (Map Width) */

			s += VGAScreen->w * 28 + 24 * 12;
		}

		if (backPos <= 15)
		{
			/* --- BACKGROUND 1 BOTTOM --- */
			for (i = 12; i; i--)
			{
				/* move to previous map X location */
				bp--;
				p = *bp;

				for (j = 15 - backPos + 1; j; j--)
				{
					memcpy(s, p, 24);

					s += VGAScreen->w;
					p += 24;
				}

				s -= (15 - backPos + 1) * VGAScreen->w + 24;
			}
		}
	} else {
		JE_clr256();
	}

	/*Set Movement of background 1*/
	if (--map1YDelay == 0)
	{
		map1YDelay = map1YDelayMax;

		curLoc += backMove;

		backPos += backMove;

		if (backPos > 27)
		{
			backPos -= 28;
			mapY--;
			mapYPos -= 14;  /*Map Width*/
		}
	}

	/*---------------------------STARS--------------------------*/
	/* DRAWSTARS */
	if (starActive || astralDuration > 0)
	{
		s = (Uint8 *)VGAScreen->pixels;
		
		for (i = MAX_STARS; i--; )
		{
			starDat[i].sLoc += starDat[i].sMov + starY;
			if (starDat[i].sLoc < 177 * VGAScreen->w)
			{
				if (*(s + starDat[i].sLoc) == 0)
				{
					*(s + starDat[i].sLoc) = starDat[i].sC;
				}
				if (starDat[i].sC - 4 >= 9 * 16)
				{
					if (*(s + starDat[i].sLoc + 1) == 0)
					{
						*(s + starDat[i].sLoc + 1) = starDat[i].sC - 4;
					}
					if (starDat[i].sLoc > 0 && *(s + starDat[i].sLoc - 1) == 0)
					{
						*(s + starDat[i].sLoc - 1) = starDat[i].sC - 4;
					}
					if (*(s + starDat[i].sLoc + VGAScreen->w) == 0)
					{
						*(s + starDat[i].sLoc + VGAScreen->w) = starDat[i].sC - 4;
					}
					if (starDat[i].sLoc >= VGAScreen->w && *(s + starDat[i].sLoc - VGAScreen->w) == 0)
					{
						*(s + starDat[i].sLoc - VGAScreen->w) = starDat[i].sC - 4;
					}
				}
			}
		}
	}

	if (processorType > 1 && smoothies[5-1])
	{
		JE_smoothies3();
	}

	/*-----------------------BACKGROUNDS------------------------*/
	/*-----------------------BACKGROUND 2------------------------*/
	if (background2over == 3)
	{
		JE_drawBackground2();
		background2 = true;
	}

	if (background2over == 0)
	{
		if (!(smoothies[2-1] && processorType < 4) && !(smoothies[1-1] && processorType == 3))
		{
			if (wild && !background2notTransparent)
			{
				JE_superBackground2();
			} else {
				JE_drawBackground2();
			}
		}
	}

	if (smoothies[1-1] && processorType > 2 && SDAT[1-1] == 0)
	{
		JE_smoothies1();
	}
	if (smoothies[2-1] && processorType > 2)
	{
		JE_smoothies2();
	}

	/*-----------------------Ground Enemy------------------------*/
	lastEnemyOnScreen = enemyOnScreen;

	tempMapXOfs = mapXOfs;
	tempBackMove = backMove;
	JE_drawEnemy(50);
	JE_drawEnemy(100);

	if (enemyOnScreen == 0 || enemyOnScreen == lastEnemyOnScreen)
	{
		if (stopBackgroundNum == 1)
		{
			stopBackgroundNum = 9;
		}
	}

	if (smoothies[1-1] && processorType > 2 && SDAT[1-1] > 0)
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
	if (!(smoothies[2-1] && processorType < 4) &&
	    !(smoothies[1-1] && processorType == 3))
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
		tempW = levelEnemy[rand() % levelEnemyMax];
		if (tempW == 2)
		{
			soundQueue[3] = 7;
		}
		JE_newEnemy(0);
	}

	if (processorType > 1 && smoothies[3-1])
	{
		JE_smoothies3();
	}
	if (processorType > 1 && smoothies[4-1])
	{
		JE_smoothies4();
	}

	/* Draw Sky Enemy */
	if (!skyEnemyOverAll)
	{
		lastEnemyOnScreen = enemyOnScreen;

		tempMapXOfs = mapX2Ofs;
		tempBackMove = 0;
		JE_drawEnemy(25);

		if (enemyOnScreen == lastEnemyOnScreen)
		{
			if (stopBackgroundNum == 2)
			{
				stopBackgroundNum = 9;
			}
		}
	}

	if (background3over == 0)
	{
		JE_drawBackground3();
	}

	/* Draw Top Enemy */
	if (!topEnemyOver)
	{
		tempMapXOfs = (background3x1 == 0) ? oldMapX3Ofs : mapXOfs;
		tempBackMove = backMove3;
		JE_drawEnemy(75);
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

				if (playerShotData[z].shotYM > 100)
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
					goto draw_player_shot_loop_end;
				}

				if (playerShotData[z].shotTrail != 255)
				{
					if (playerShotData[z].shotTrail == 98)
					{
						JE_setupExplosion(playerShotData[z].shotX - playerShotData[z].shotXM, playerShotData[z].shotY - playerShotData[z].shotYM, playerShotData[z].shotTrail);
					} else {
						JE_setupExplosion(playerShotData[z].shotX, playerShotData[z].shotY, playerShotData[z].shotTrail);
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

				tempW = playerShotData[z].shotGr + playerShotData[z].shotAni;
				if (++playerShotData[z].shotAni == playerShotData[z].shotAniMax)
				{
					playerShotData[z].shotAni = 0;
				}

				tempI2 = playerShotData[z].shotDmg;
				temp2 = playerShotData[z].shotBlastFilter;
				chain = playerShotData[z].chainReaction;
				playerNum = playerShotData[z].playerNumber;

				tempSpecial = tempW > 60000;

				if (tempSpecial)
				{
					JE_newDrawCShapeTrickNum(OPTION_SHAPES, tempW - 60000, tempShotX+1, tempShotY);
					tempX2 = shapeX[OPTION_SHAPES][tempW - 60000 - 1] >> 1;
					tempY2 = shapeY[OPTION_SHAPES][tempW - 60000 - 1] >> 1;
				} else {
					if (tempW > 1000)
					{
						JE_doSP(tempShotX+1 + 6, tempShotY + 6, 5, 3, (tempW / 1000) << 4);
						tempW = tempW % 1000;
					}
					if (tempW > 500)
					{
						if (background2 && tempShotY + shadowyDist < 190 && tempI4 < 100)
						{
							JE_drawShape2Shadow(tempShotX+1, tempShotY + shadowyDist, tempW - 500, shapesW2);
						}
						JE_drawShape2(tempShotX+1, tempShotY, tempW - 500, shapesW2);
					} else {
						if (background2 && tempShotY + shadowyDist < 190 && tempI4 < 100)
						{
							JE_drawShape2Shadow(tempShotX+1, tempShotY + shadowyDist, tempW, shapesC1);
						}
						JE_drawShape2(tempShotX+1, tempShotY, tempW, shapesC1);
					}
				}

			}

			for (b = 0; b < 100; b++)
			{
				if (enemyAvail[b] == 0)
				{
					if (z == MAX_PWEAPON)
					{
						temp = 25 - abs(zinglonDuration - 25);
						tempB = abs(enemy[b].ex + enemy[b].mapoffset - (PX + 7)) < temp;
						temp2 = 9;
						chain = 0;
						tempI2 = 10;
					} else if (tempSpecial) {
						tempB = ((enemy[b].enemycycle == 0) &&
						        (abs(enemy[b].ex + enemy[b].mapoffset - tempShotX - tempX2) < (25 + tempX2)) &&
						        (abs(enemy[b].ey - tempShotY - 12 - tempY2)                 < (29 + tempY2))) ||
						        ((enemy[b].enemycycle > 0) &&
						        (abs(enemy[b].ex + enemy[b].mapoffset - tempShotX - tempX2) < (13 + tempX2)) &&
						        (abs(enemy[b].ey - tempShotY - 6 - tempY2)                  < (15 + tempY2)));
					} else {
						tempB = ((enemy[b].enemycycle == 0) &&
						        (abs(enemy[b].ex + enemy[b].mapoffset - tempShotX) < 25) && (abs(enemy[b].ey - tempShotY - 12) < 29)) ||
						        ((enemy[b].enemycycle > 0) &&
						        (abs(enemy[b].ex + enemy[b].mapoffset - tempShotX) < 13) && (abs(enemy[b].ey - tempShotY - 6) < 15));
					}
					
					if (tempB)
					{
						if (chain > 0)
						{
							shotMultiPos[5-1] = 0;
							JE_initPlayerShot(0, 5, tempShotX, tempShotY, mouseX, mouseY, chain, playerNum);
							shotAvail[z] = 0;
							goto draw_player_shot_loop_end;
						}
						
						infiniteShot = false;
						
						if (tempI2 == 99)
						{
							tempI2 = 0;
							doIced = 40;
							enemy[b].iced = 40;
						} else {
							doIced = 0;
							if (tempI2 >= 250) {
								tempI2 = tempI2 - 250;
								infiniteShot = true;
							}
						}
						
						tempI = enemy[b].armorleft;
						
						temp = enemy[b].linknum;
						if (temp == 0)
						{
							temp = 255;
						}
						
						if (enemy[b].armorleft < 255)
						{
							if (temp == statBar[1-1])
							{
								statCol[1-1] = 6;
							}
							if (temp == statBar[2-1])
							{
								statCol[2-1] = 6;
							}
							if (enemy[b].enemyground)
							{
								enemy[b].filter = temp2;
							}
							for (i = 0; i < 100; i++)
							{
								if (enemy[i].linknum == temp &&
								    enemyAvail[i] != 1 &&
								    enemy[i].enemyground != 0)
								{
									if (doIced)
									{
										enemy[i].iced = doIced;
									}
									enemy[i].filter = temp2;
								}
							}
						}
						
						if (tempI > tempI2)
						{
							if (z != MAX_PWEAPON)
							{
									if (enemy[b].armorleft != 255)
									{
										enemy[b].armorleft -= tempI2;
										JE_setupExplosion(tempShotX, tempShotY, 0);
									} else {
									JE_doSP(tempShotX + 6, tempShotY + 6, tempI2 / 2 + 3, tempI2 / 4 + 2, temp2);
								}
							}
							
							soundQueue[5] = 3;
							
							if ((tempI - tempI2 <= enemy[b].edlevel) &&
							    ((!enemy[b].edamaged) ^ (enemy[b].edani < 0)))
							{
								
								for (temp3 = 0; temp3 < 100; temp3++)
								{
									if (enemyAvail[temp3] != 1)
									{
										temp4 = enemy[temp3].linknum;
										if ((temp3 == b) ||
										    ((temp != 255) &&
										    (((enemy[temp3].edlevel > 0) && (temp4 == temp)) ||
										    ((enemyContinualDamage && (temp - 100 == temp4)) ||
										    ((temp4 > 40) && (temp4 / 20 == temp / 20) && (temp4 <= temp))))))
										{
										
											enemy[temp3].enemycycle = 1;
											
											enemy[temp3].edamaged = !enemy[temp3].edamaged;
											
											if (enemy[temp3].edani != 0)
											{
												enemy[temp3].ani = abs(enemy[temp3].edani);
												enemy[temp3].aniactive = 1;
												enemy[temp3].animax = 0;
												enemy[temp3].animin = enemy[temp3].edgr;
												enemy[temp3].enemycycle = enemy[temp3].animin - 1;
												
											} else if (enemy[temp3].edgr > 0)
											{
												enemy[temp3].egr[1-1] = enemy[temp3].edgr;
												enemy[temp3].ani = 1;
												enemy[temp3].aniactive = 0;
												enemy[temp3].animax = 0;
												enemy[temp3].animin = 1;
											}
											else
											{
												enemyAvail[temp3] = 1;
												enemyKilled++;
											}
											
											enemy[temp3].aniwhenfire = 0;
											
											if (enemy[temp3].armorleft > enemy[temp3].edlevel)
											{
												enemy[temp3].armorleft = enemy[temp3].edlevel;
											}
											
											tempX = enemy[temp3].ex + enemy[temp3].mapoffset;
											tempY = enemy[temp3].ey;
											
											if (enemyDat[enemy[temp3].enemytype].esize != 1)
											{
												JE_setupExplosion(tempX, tempY - 6, 1);
											} else {
												JE_setupExplosionLarge(enemy[temp3].enemyground, enemy[temp3].explonum / 2, tempX, tempY);
											}
										}
									}
								}
							}
						} else {
							
							if ((temp == 254) && (superEnemy254Jump > 0))
							{
								JE_eventJump(superEnemy254Jump);
							}
						
							for (temp2 = 0; temp2 < 100; temp2++)
							{
								if (enemyAvail[temp2] != 1)
								{
									temp3 = enemy[temp2].linknum;
									if ((temp2 == b) || (temp == 254) ||
									    ((temp != 255) && ((temp == temp3) || (temp - 100 == temp3)
									    || ((temp3 > 40) && (temp3 / 20 == temp / 20) && (temp3 <= temp)))))
									{
										
										tempI3 = enemy[temp2].ex + enemy[temp2].mapoffset;
										
										if (enemy[temp2].special)
										{
											globalFlags[enemy[temp2].flagnum] = enemy[temp2].setto;
										}
										
										if ((enemy[temp2].enemydie > 0) &&
											!((superArcadeMode > 0) &&
											(enemyDat[enemy[temp2].enemydie].value == 30000)))
										{
											zz = b;
											tempW = enemy[temp2].enemydie;
											tempW2 = temp2 - (temp2 % 25);
											if (enemyDat[tempW].value > 30000)
											{
												tempW2 = 0;
											}
											JE_newEnemy(tempW2);
											if ((superArcadeMode > 0) && (enemy[b-1].evalue > 30000))
											{
												superArcadePowerUp++;
												if (superArcadePowerUp > 5)
												{
													superArcadePowerUp = 1;
												}
												enemy[b-1].egr[1-1] = 5 + superArcadePowerUp * 2;
												enemy[b-1].evalue = 30000 + superArcadePowerUp;
											}
											
											if (enemy[b-1].evalue != 0)
											{
												enemy[b-1].scoreitem = true;
											} else {
												enemy[b-1].scoreitem = false;
											}
											enemy[b-1].ex = enemy[temp2].ex;
											enemy[b-1].ey = enemy[temp2].ey;
											b = zz;
										}
										
										if ((enemy[temp2].evalue > 0) && (enemy[temp2].evalue < 10000))
										{
											if (enemy[temp2].evalue == 1)
											{
												cubeMax++;
											} else {
												if ((playerNum < 2) || galagaMode)
												{
													score += enemy[temp2].evalue;
												} else {
													score2 += enemy[temp2].evalue;
												}
											}
										}
										
										if ((enemy[temp2].edlevel == -1) && (temp == temp3))
										{
											enemy[temp2].edlevel = 0;
											enemyAvail[temp2] = 2;
											enemy[temp2].egr[1-1] = enemy[temp2].edgr;
											enemy[temp2].ani = 1;
											enemy[temp2].aniactive = 0;
											enemy[temp2].animax = 0;
											enemy[temp2].animin = 1;
											enemy[temp2].edamaged = true;
											enemy[temp2].enemycycle = 1;
										} else {
											enemyAvail[temp2] = 1;
											enemyKilled++;
										}
										
										if (enemyDat[enemy[temp2].enemytype].esize == 1)
										{
											JE_setupExplosionLarge(enemy[temp2].enemyground, enemy[temp2].explonum, tempI3, enemy[temp2].ey);
											soundQueue[6] = 9;
										} else {
											JE_setupExplosion(tempI3, enemy[temp2].ey, 1);
											soundQueue[6] = 8;
										}
									}
								}
							}
						}
						
						if (infiniteShot)
						{
							tempI2 += 250;
						} else {
							if (z != MAX_PWEAPON)
							{
								if (tempI2 <= tempI)
								{
									shotAvail[z] = 0;
									goto draw_player_shot_loop_end;
								} else {
									playerShotData[z].shotDmg -= tempI;
								}
							}
						}
					}
				}
			}

draw_player_shot_loop_end:
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
	netSuccess = false;

	if (!endLevel)
	{    /*MAIN DRAWING IS STOPPED STARTING HERE*/

		/* Draw Enemy Shots */
		for (z = 0; z < ENEMY_SHOT_MAX; z++)
		{
			if (enemyShotAvail[z] == 0)
			{
				enemyShot[z].sxm += enemyShot[z].sxc;
				enemyShot[z].sx += enemyShot[z].sxm;

				if (enemyShot[z].tx != 0)
				{
					if (enemyShot[z].sx > PX)
					{
						if (enemyShot[z].sxm > -enemyShot[z].tx)
						{
							enemyShot[z].sxm--;
						}
					} else {
						if (enemyShot[z].sxm < enemyShot[z].tx)
						{
							enemyShot[z].sxm++;
						}
					}
				}

				enemyShot[z].sym += enemyShot[z].syc;
				enemyShot[z].sy += enemyShot[z].sym;

				if (enemyShot[z].ty != 0)
				{
					if (enemyShot[z].sy > PY)
					{
						if (enemyShot[z].sym > -enemyShot[z].ty)
						{
							enemyShot[z].sym--;
						}
					} else {
						if (enemyShot[z].sym < enemyShot[z].ty)
						{
							enemyShot[z].sym++;
						}
					}
				}

				if (enemyShot[z].duration-- == 0 || enemyShot[z].sy > 190 || enemyShot[z].sy <= -14 || enemyShot[z].sx > 275 || enemyShot[z].sx <= 0)
				{
					enemyShotAvail[z] = 1;
				} else {
					
					if (((temp3 = 1)
					     && playerAlive != 0
					     && enemyShot[z].sx - PX > sAniXNeg && enemyShot[z].sx - PX < sAniX
					     && enemyShot[z].sy - PY > sAniYNeg && enemyShot[z].sy - PY < sAniY)
					 || ((temp3 = 2)
					     && twoPlayerMode != 0
					     && playerAliveB != 0
					     && enemyShot[z].sx - PXB > sAniXNeg && enemyShot[z].sx - PXB < sAniX
					     && enemyShot[z].sy - PYB > sAniYNeg && enemyShot[z].sy - PYB < sAniY))
					{
						tempX = enemyShot[z].sx;
						tempY = enemyShot[z].sy;
						temp = enemyShot[z].sdmg;
						
						enemyShotAvail[z] = 1;
						
						JE_setupExplosion(tempX, tempY, 0);
						
						switch (temp3)
						{
							case 1:
								if (playerInvulnerable1 == 0)
								{
									if ((temp = JE_playerDamage(tempX, tempY, temp, &PX, &PY, &playerAlive, &playerStillExploding, &armorLevel, &shield)) > 0)
									{
										lastTurn2 += (enemyShot[z].sxm * temp) / 2;
										lastTurn  += (enemyShot[z].sym * temp) / 2;
									}
								}
								break;
							case 2:
								if (playerInvulnerable2 == 0)
								{
									if ((temp = JE_playerDamage(tempX, tempY, temp, &PXB, &PYB, &playerAliveB, &playerStillExploding2, &armorLevel2, &shield2)) > 0)
									{
										lastTurn2B += (enemyShot[z].sxm * temp) / 2;
										lastTurnB  += (enemyShot[z].sym * temp) / 2;
									}
								}
								break;
						}
					} else {
						s = (Uint8 *)VGAScreen->pixels;
						s += enemyShot[z].sy * VGAScreen->w + enemyShot[z].sx;

						s_limit = (Uint8 *)VGAScreen->pixels;
						s_limit += VGAScreen->h * VGAScreen->w;

						if (enemyShot[z].animax != 0)
						{
							if (++enemyShot[z].animate >= enemyShot[z].animax)
							{
								enemyShot[z].animate = 0;
							}
						}

						if (enemyShot[z].sgr >= 500)
						{
							p = shapesW2;
							p += SDL_SwapLE16(((JE_word *)p)[enemyShot[z].sgr + enemyShot[z].animate - 500 - 1]);
						} else {
							p = shapesC1;
							p += SDL_SwapLE16(((JE_word *)p)[enemyShot[z].sgr + enemyShot[z].animate - 1]);
						}

						while (*p != 0x0f)
						{
							s += *p & 0x0f;
							i = (*p & 0xf0) >> 4;
							if (i)
							{
								while (i--)
								{
									p++;
									if (s >= s_limit)
										goto enemy_shot_draw_overflow;
									if ((void *)s >= VGAScreen->pixels)
										*s = *p;
									s++;
								}
							} else {
								s -= 12;
								s += VGAScreen->w;
							}
							p++;
						}
					}

				}

enemy_shot_draw_overflow:
				;
			}
		}
	}

	if (background3over == 1)
	{
		JE_drawBackground3();
	}

	/* Draw Top Enemy */
	if (topEnemyOver)
	{
		tempMapXOfs = (background3x1 == 0) ? oldMapX3Ofs : oldMapXOfs;
		tempBackMove = backMove3;
		JE_drawEnemy(75);
	}

	/* Draw Sky Enemy */
	if (skyEnemyOverAll)
	{
		lastEnemyOnScreen = enemyOnScreen;

		tempMapXOfs = mapX2Ofs;
		tempBackMove = 0;
		JE_drawEnemy(25);

		if (enemyOnScreen == lastEnemyOnScreen)
		{
			if (stopBackgroundNum == 2)
			{
				stopBackgroundNum = 9;
			}
		}
	}

	/*-------------------------- Sequenced Explosions -------------------------*/
	enemyStillExploding = false;
	for (tempREX = 0; tempREX < 20; tempREX++)
	{
		if (REXavail[tempREX] != 0)
		{
			enemyStillExploding = true;
			if (REXdat[tempREX].delay > 0)
			{
				REXdat[tempREX].delay--;
			} else {
				REXdat[tempREX].ey += backMove2 + 1;
				tempX = REXdat[tempREX].ex + (rand() % 24) - 12;
				tempY = REXdat[tempREX].ey + (rand() % 27) - 24;
				if (REXdat[tempREX].big)
				{
					JE_setupExplosionLarge(false, 2, tempX, tempY);
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
	for (j = 0; j < EXPLOSION_MAX; j++)
	{
		if (explodeAvail[j] != 0)
		{
			l = 0;
			if (explosions[j].followPlayer == 1)
			{
				l = explosionFollowAmount;
			}
			if (explosions[j].fixedExplode != 1)
			{
				explosions[j].explodeGr++;
				l = explodeMove;
			}
			explosions[j].explodeLoc += l + explosions[j].fixedMovement;
			
			s = (Uint8 *)VGAScreen->pixels;
			s += explosions[j].explodeLoc;
			
			s_limit = (Uint8 *)VGAScreen->pixels;
			s_limit += VGAScreen->h * VGAScreen->w;
			
			if (s + VGAScreen->w * 14 > s_limit)
			{
				explodeAvail[j] = 0;
			} else {
				p = shapes6;
				p += SDL_SwapLE16(((JE_word *)p)[explosions[j].explodeGr - 1]);
				
				if (explosionTransparent)
				{
					while (*p != 0x0f)
					{
						s += *p & 0x0f;
						i = (*p & 0xf0) >> 4;
						if (i)
						{
							while (i--)
							{
								p++;
								if (s >= s_limit)
									goto explosion_draw_overflow;
								if ((void *)s >= VGAScreen->pixels)
									*s = (((*p & 0x0f) + (*s & 0x0f)) >> 1) | (*p & 0xf0);
								s++;
							}
						} else {
							s -= 12;
							s += VGAScreen->w;
						}
						p++;
					}
				} else {
					while (*p != 0x0f)
					{
						s += *p & 0x0f;
						i = (*p & 0xf0) >> 4;
						if (i)
						{
							while (i--)
							{
								p++;
								if (s >= s_limit)
									goto explosion_draw_overflow;
								if ((void *)s >= VGAScreen->pixels)
									*s = *p;
								s++;
							}
						} else {
							s -= 12;
							s += VGAScreen->w;
						}
						p++;
					}
				}
explosion_draw_overflow:
				
				explodeAvail[j]--;
			}
		}
	}

	if (!portConfigChange)
	{
		portConfigDone = true;
	}


	/*-----------------------BACKGROUNDS------------------------*/
	/*-----------------------BACKGROUND 2------------------------*/
	if (!(smoothies[2-1] && processorType < 4) &&
	    !(smoothies[1-1] && processorType == 3))
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
		if (playerAlive && armorLevel < 6)
		{
			tempW2 = armorLevel;
		} else {
			tempW2 = armorLevel2;
		}
		
		if (armorShipDelay > 0)
		{
			armorShipDelay--;
		} else {
			tempW = 560;
			JE_newEnemy(50);
			if (b > 0)
			{
				enemy[b-1].enemydie = 560 + (rand() % 3) + 1;
				enemy[b-1].eyc -= backMove3;
				enemy[b-1].armorleft = 4;
			}
			armorShipDelay = 500;
		}
		
		if ((playerAlive && armorLevel < 6 && (!isNetworkGame || thisPlayerNum == 1))
		    || (twoPlayerMode && playerAliveB && armorLevel2 < 6 && (!isNetworkGame || thisPlayerNum == 2)))
		{
				
			tempW = tempW2 * 4 + 8;
			if (warningSoundDelay > tempW)
			{
				warningSoundDelay = tempW;
			}
			
			if (warningSoundDelay > 1)
			{
				warningSoundDelay--;
			} else {
				soundQueue[7] = 17;
				warningSoundDelay = tempW;
			}
			
			warningCol += warningColChange;
			if (warningCol > 113 + (14 - (tempW2 * 2)))
			{
				warningColChange = -warningColChange;
				warningCol = 113 + (14 - (tempW2 * 2));
			} else if (warningCol < 113) {
				warningColChange = -warningColChange;
			}
			JE_bar(24, 181, 138, 183, warningCol);
			JE_bar(175, 181, 287, 183, warningCol);
			JE_bar(24, 0, 287, 3, warningCol);
			
			JE_outText(140, 178, "WARNING", 7, (warningCol % 16) / 2);
			
		}
	}

	/*------- Random Explosions --------*/
	if (randomExplosions)
	{
		if (rand() % 10 == 1)
		{
			JE_setupExplosionLarge(false, 20, rand() % 280, rand() % 180);
		}
	}


	/*=================================*/
	/*=======The Sound Routine=========*/
	/*=================================*/
	if (soundEffects > 0 && soundActive && firstGameOver)
	{
		temp = 0;
		for (temp2 = 0; temp2 < SFX_CHANNELS; temp2++)
		{
			if (soundQueue[temp2] > 0)
			{
				temp = soundQueue[temp2];
				if (temp2 == 3)
				{
					temp3 = fxPlayVol;
				} else {
					if (temp == 15)
					{
						temp3 = fxPlayVol / 4;
					} else {   /*Lightning*/
						temp3 = fxPlayVol / 2;
					}
				}
				JE_multiSamplePlay(digiFx[temp-1], fxSize[temp-1], temp2, temp3);
				soundQueue[temp2] = 0;
			}
		}
	}

	if (returnActive && enemyOnScreen == 0)
	{
		JE_eventJump(65535);
		returnActive = false;
	}

	/*-------      DEbug      ---------*/
	debugTime = SDL_GetTicks();
	tempW = lastmouse_but;
	tempX = mouse_x;
	tempY = mouse_y;

	if (debug)
	{
		strcpy(tempStr, "");
		for (temp = 0; temp < 9; temp++)
		{
			sprintf(tempStr, "%s%c", tempStr,  smoothies[temp] + 48);
		}
		sprintf(buffer, "SM = %s", tempStr);
		JE_outText(30, 70, buffer, 4, 0);

		sprintf(buffer, "Memory left = %d", -1);
		JE_outText(30, 80, buffer, 4, 0);
		sprintf(buffer, "Enemies onscreen = %d", enemyOnScreen);
		JE_outText(30, 90, buffer, 6, 0);

		debugHist = debugHist + abs((JE_longint)debugTime - (JE_longint)lastDebugTime);
		debugHistCount++;
		sprintf(tempStr, "%2.3f", 1000.0f / round(debugHist / debugHistCount));
		sprintf(buffer, "X:%d Y:%-5d  %s FPS  %d %d %d %d", (mapX - 1) * 12 + PX, curLoc, tempStr, lastTurn2, lastTurn, PX, PY);
		JE_outText(45, 175, buffer, 15, 3);
		lastDebugTime = debugTime;
	}

	if (displayTime > 0)
	{
		displayTime--;
		JE_outTextAndDarken(90, 10, miscText[59], 15, (JE_byte)flash - 8, FONT_SHAPES);
		flash += flashChange;
		if (flash > 4 || flash == 0)
		{
			flashChange = -flashChange;
		}
	}

	/*Pentium Speed Mode?*/
	if (pentiumMode)
	{
		if (frameCountMax == 2)
		{
			frameCountMax = 3;
		} else {
			frameCountMax = 2;
		}
	}

	/*--------  Level Timer    ---------*/
	if (levelTimer)
	{
		if (levelTimerCountdown > 0)
		{
			levelTimerCountdown--;
			if (levelTimerCountdown == 0)
			{
				JE_eventJump(levelTimerJumpTo);
			}

			if (levelTimerCountdown > 200)
			{
				if (levelTimerCountdown % 100 == 0)
				{
					soundQueue[7] = 17;
				}

				if (levelTimerCountdown % 10 == 0)
				{
					soundQueue[6] = 24;  /*28 or 24*/
				}
			} else {
				if (levelTimerCountdown % 20 == 0)
				{
					soundQueue[7] = 17;
				}
			}

			JE_textShade (140, 6, miscText[66], 7, (levelTimerCountdown % 20) / 3, FULL_SHADE);
			sprintf(buffer, "%.1f", levelTimerCountdown / 100.0f);
			JE_dString (100, 2, buffer, SMALL_FONT_SHAPES);
		}
	}

	/*GAME OVER*/
	if (!constantPlay && !constantDie)
	{
		if (allPlayersGone)
		{
			if (!(playerStillExploding == 0 && playerStillExploding2 == 0))
			{
				if (galagaMode)
				{
					playerStillExploding2 = 0;
				}
				musicFade = true;
			} else {
				if (playDemo || normalBonusLevelCurrent || bonusLevelCurrent)
				{
					reallyEndLevel = true;
				} else {
					JE_dString(120, 60, miscText[21], FONT_SHAPES);
				}
				JE_setMousePosition(159, 100);
				if (firstGameOver)
				{
					if (!playDemo)
					{
						JE_playSong(SONG_GAMEOVER);
						JE_setVol(tyrMusicVolume, fxVolume);
					}
					firstGameOver = false;
				}

				if (!playDemo)
				{
					service_SDL_events(true);
					JE_joystick2();
					if (playDemo ||
					   (newkey || button[0] || button[1] || button[2]) ||
					   (newmouse))
					{
						reallyEndLevel = true;
					}
				}

				if (isNetworkGame)
				{
					JE_setNetByte(0);
					for (x = 0; x < 1/* TODO streamLagFrames*/; x++)
					{
						JE_updateStream();
					}
					JE_setNetByte(253);
					do
					{
						JE_updateStream();
						if (netQuit)
						{
							/* TODO JE_arenaExit("*Other player stopped responding");*/
							JE_tyrianHalt(5);
						}
					} while (!JE_scanNetByte(253));
					reallyEndLevel = true;
				}

			}
		}
	}

	/* Call Keyboard input handler */
	service_SDL_events(false);
	if (playDemo)
	{
		if (newkey || JE_anyButton() || button[0])
		{
			reallyEndLevel = true;
			stoppedDemo = true;
		}
	} else {
		if (newkey)
		{
			skipStarShowVGA = false;
			JE_mainKeyboardInput();
			newkey = false;
			if (useBios)
			{
				while (JE_keyPressed(&k));
			}
			if (skipStarShowVGA)
			{
				goto level_loop;
			}
		}
	}
	
	/*Network Update*/
	if (isNetworkGame)
	{
		/* TODO JE_doNetwork();*/
	}

	/** Test **/
	JE_drawSP();

	/*Filtration*/
	if (filterActive)
	{
		JE_filterScreen(levelFilter, levelBrightness);
	}

	/** Statbar **/
	if (statBar[1-1] > 0 || statBar[2-1] > 0)
	{
		JE_doStatBar();
	}

	JE_inGameDisplays();

	VGAScreen = VGAScreenSeg; /* side-effect of game_screen */

	JE_starShowVGA();

	/*??*/
	if (repause && superPause)
	{
		repause = false;
		JE_pauseGame();
		keysactive[SDLK_p] = false;
	}

	/*Start backgrounds if no enemies on screen
	  End level if number of enemies left to kill equals 0.*/
	if (stopBackgroundNum == 9 && backMove == 0 && !enemyStillExploding)
	{
		backMove = 1;
		backMove2 = 2;
		backMove3 = 3;
		explodeMove = 640;
		stopBackgroundNum = 0;
		stopBackgrounds = false;
		if (waitToEndLevel)
		{
			endLevel = true;
			levelEnd = 40;
		}
		if (allPlayersGone)
		{
			reallyEndLevel = true;
		}
	}

	if (!endLevel && enemyOnScreen == 0)
	{
		if (readyToEndLevel && !enemyStillExploding)
		{
			if (levelTimerCountdown > 0)
			{
				levelTimer = false;
			}
			readyToEndLevel = false;
			endLevel = true;
			levelEnd = 40;
			if (allPlayersGone)
			{
				reallyEndLevel = true;
			}
		}
		if (stopBackgrounds)
		{
			stopBackgrounds = false;
			backMove = 1;
			backMove2 = 2;
			backMove3 = 3;
			explodeMove = 640;
		}
	}


	exchangeCount = 20;

	/*Other Network Functions*/
	JE_handleChat();

	if (reallyEndLevel)
	{
		goto start_level;
	}
	goto level_loop;
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


	/*JE_word yLoc;*/
	JE_shortint yChg;

	JE_byte mapBuf[15 * 600]; /* [1..15 * 600] */
	JE_word bufLoc;

	char buffer[256];
	int i;
	Uint8 pic_buffer[320*200]; /* screen buffer, 8-bit specific */
	Uint8 *vga, *pic, *vga2; /* screen pointer, 8-bit specific */

	lastCubeMax = cubeMax;

	/*Defaults*/
	songBuy = DEFAULT_SONG_BUY;  /*Item Screen default song*/

	if (loadTitleScreen || playDemo)
	{
		JE_openingAnim();
		JE_titleScreen(true);
		loadTitleScreen = false;
	}

	/* Load LEVELS.DAT - Section = MAINLEVEL */
	saveLevel = mainLevel;

new_game:
	galagaMode  = false;
	useLastBank = false;
	extraGame   = false;
	haltGame = false;
	if (loadTitleScreen)
	{
		JE_titleScreen(true);
		loadTitleScreen = false;
	}

	gameLoaded = false;


	first = true;

	if (!playDemo && !loadDestruct)
	{
		do
		{
			JE_resetFile(&lvlFile, macroFile);

			x = 0;
			jumpSection = false;
			loadLevelOk = false;

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

			ESCPressed = false;

			do
			{

				if (gameLoaded)
				{
					if (mainLevel == 0)
					{
						loadTitleScreen = true;
					}
					fclose(lvlFile);
					goto new_game;
				}

				strcpy(s, " ");
				JE_readCryptLn(lvlFile, s);

				switch (s[0])
				{
					case ']':
						switch (s[1])
						{
							case 'A':
								JE_playAnim("tyrend.anm", 1, true, 7);
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
								galagaMode = true;   /*GALAGA mode*/
								memcpy(&pItemsPlayer2, &pItems, sizeof(pItemsPlayer2));
								pItemsPlayer2[1] = 15; /*Player 2 starts with 15 - MultiCannon and 2 single shot options*/
								pItemsPlayer2[3] = 0;
								pItemsPlayer2[4] = 0;
								break;

							case 'x':
								extraGame = true;
								break;

							case 'e': /*ENGAGE mode*/
								doNotSaveBackup = true;
								constantDie = false;
								onePlayerAction = true;
								superTyrian = true;
								twoPlayerMode = false;

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
								jumpSection = true;
								break;
							case '2':
								temp = atoi(strnztcpy(buffer, s + 3, 3));
								if (twoPlayerMode || onePlayerAction)
								{
									mainLevel = temp;
									jumpSection = true;
								}
								break;
							case 'w':
								temp = atoi(strnztcpy(buffer, s + 3, 3));   /*Allowed to go to Time War?*/
								if (pItems[11] == 13)
								{
									mainLevel = temp;
									jumpSection = true;
								}
								break;
							case 't':
								temp = atoi(strnztcpy(buffer, s + 3, 3));
								if (levelTimer && levelTimerCountdown == 0)
								{
									mainLevel = temp;
									jumpSection = true;
								}
								break;
							case 'l':
								temp = atoi(strnztcpy(buffer, s + 3, 3));
								if (!playerAlive || (twoPlayerMode && !playerAliveB))
								{
									mainLevel = temp;
									jumpSection = true;
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
								JE_saveGame(11, "LAST LEVEL    ");
								break;

							case 'i':
								temp = atoi(strnztcpy(buffer, s + 3, 3));
								songBuy = temp;
								break;
							case 'I': /*Load Items Available Information*/

								memset(&itemAvail, 0, sizeof(itemAvail));

								for (temp = 0; temp < 9; temp++)
								{
									char buf[256];

									JE_readCryptLn(lvlFile, s);

									sprintf(buf, "%s ", (strlen(s) > 8 ? s+8 : ""));
									/*strcat(strcpy(s, s + 8), " ");*/
									temp2 = 0;
									while (JE_getNumber(buf, &itemAvail[temp][temp2]))
									{
										temp2++;
									}

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
								loadLevelOk = true;
								bonusLevelCurrent = (strlen(s) > 28) & (s[28] == '$');
								normalBonusLevelCurrent = (strlen(s) > 27) & (s[27] == '$');
								gameJustLoaded = false;
								break;

							case '@':
								useLastBank = !useLastBank;
								break;

							case 'Q':
								ESCPressed = false;
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

									JE_loadPic(1, false);
									JE_clr256();

									if (superTyrian)
									{
										if (initialDifficulty == 8)
										{
											superArcadeMode = SA + 1;
										} else {
											superArcadeMode = 1;
										}

										jumpSection = true;
										loadTitleScreen = true;
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
											JE_drawShape2x2(148, 70, ships[SAShip[SANextShip[superArcadeMode]-1]].shipgraphic, shapes9);
										} else {
											if (SANextShip[superArcadeMode] == 7)
											{
												trentWin = true;
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

									jumpSection = true;
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
										memcpy(colors, palettes[pcxpal[tempX-1 - 900]], sizeof(colors));
										JE_clr256();
										JE_showVGA();
										JE_fadeColor(1);
									} else {
										if (tempX == 0)
										{
											/* TODO JE_loadPcx("tshp2.pcx", false);*/
										} else {
											JE_loadPic(tempX, false);
										}
										JE_showVGA();
										JE_fadeColor(10);
									}
								}
								break;

							case 'U':
								if (!constantPlay)
								{
									JE_setNetByte(0);
									memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

									tempX = atoi(strnztcpy(buffer, s + 3, 3));
									JE_loadPic(tempX, false);
									memcpy(pic_buffer, VGAScreen->pixels, sizeof(pic_buffer));

									service_SDL_events(true);
									for (z = 0; z <= 199; z++)
									{
										service_SDL_events(false);
										if (!newkey && !ESCPressed)
										{
											vga = VGAScreen->pixels;
											vga2 = VGAScreen2->pixels;
											pic = pic_buffer + (199 - z) * 320;

											setjasondelay(1); /* attempting to emulate JE_waitRetrace();*/
											for (y = 0; y < 199; y++)
											{
												if (y <= z)
												{
													memcpy(vga, pic, 320);
													pic += 320;
												} else {
													memcpy(vga, vga2, VGAScreen->w);
													vga2 += VGAScreen->w;
												}
												vga += VGAScreen->w;
											}
											JE_showVGA();
											wait_delay();

											if (isNetworkGame)
											{
												JE_updateStream();
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
									JE_setNetByte(0);
									memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

									tempX = atoi(strnztcpy(buffer, s + 3, 3));
									JE_loadPic(tempX, false);
									memcpy(pic_buffer, VGAScreen->pixels, sizeof(pic_buffer));

									service_SDL_events(true);
									for (z = 0; z <= 199; z++)
									{
										service_SDL_events(false);
										if (!newkey && !ESCPressed)
										{
											vga = VGAScreen->pixels;
											vga2 = VGAScreen2->pixels;
											pic = pic_buffer;

											setjasondelay(1); /* attempting to emulate JE_waitRetrace();*/
											for (y = 0; y < 199; y++)
											{
												if (y <= 199 - z)
												{
													memcpy(vga, vga2, VGAScreen->w);
													vga2 += VGAScreen->w;
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
												JE_updateStream();
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
									JE_setNetByte(0);
									memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

									tempX = atoi(strnztcpy(buffer, s + 3, 3));
									JE_loadPic(tempX, false);
									memcpy(pic_buffer, VGAScreen->pixels, sizeof(pic_buffer));

									service_SDL_events(true);
									for (z = 0; z <= 318; z++)
									{
										service_SDL_events(false);
										if (!newkey && !ESCPressed)
										{
											vga = VGAScreen->pixels;
											vga2 = VGAScreen2->pixels;
											pic = pic_buffer;

											setjasondelay(1); /* attempting to emulate JE_waitRetrace();*/
											for(y = 0; y < 200; y++)
											{
												memcpy(vga, vga2 + z, 319 - z);
												vga += 320 - z;
												vga2 += VGAScreen2->w;
												memcpy(vga, pic, z + 1);
												vga += z;
												pic += 320;
											}
											JE_showVGA();
											wait_delay();

											if (isNetworkGame)
											{
												JE_updateStream();
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
										newkey = false;
									}
								}
								break;

							case 'H':
								if (initialDifficulty < 3)
								{
									mainLevel = atoi(strnztcpy(buffer, s + 4, 3));
									jumpSection = true;
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
								ESCPressed = false;
								break;

							case 'M':
								temp = atoi(strnztcpy(buffer, s + 3, 3));
								JE_playSong(temp);
								break;
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
			sprintf(tempStr, "demo.%d", playDemoNum);
			JE_resetFile(&recordFile, tempStr);

			bonusLevelCurrent = false;

			temp = fgetc(recordFile);
			JE_initEpisode(temp);
			efread(levelName, 1, 10, recordFile); levelName[10] = '\0';
			lvlFileNum = fgetc(recordFile);
			efread(pItems, sizeof(JE_byte), 12, recordFile);
			efread(portPower, sizeof(JE_byte), 5, recordFile);
			levelSong = fgetc(recordFile);

			temp = fgetc(recordFile);
			temp2 = fgetc(recordFile);
			lastMoveWait = (temp << 8) | temp2;
			nextDemoOperation = fgetc(recordFile);

			firstEvent = true;

			/*debuginfo('Demo loaded.');*/
		} else {
			JE_fadeColors(&colors, &black, 0, 255, 50);
		}


		JE_resetFile(&lvlFile, levelFile);
		fseek(lvlFile, lvlPos[(lvlFileNum-1) * 2], SEEK_SET);

		char_mapFile = fgetc(lvlFile);
		char_shapeFile = fgetc(lvlFile);
		efread(&mapX,  sizeof(JE_word), 1, lvlFile);
		efread(&mapX2, sizeof(JE_word), 1, lvlFile);
		efread(&mapX3, sizeof(JE_word), 1, lvlFile);

		efread(&levelEnemyMax, sizeof(JE_word), 1, lvlFile);
		for (x = 0; x < levelEnemyMax; x++)
		{
			efread(&levelEnemy[x], sizeof(JE_word), 1, lvlFile);
		}

		efread(&maxEvent, sizeof(JE_word), 1, lvlFile);
		for (x = 0; x < maxEvent; x++)
		{
			efread(&eventRec[x].eventtime, sizeof(JE_word), 1, lvlFile);
			efread(&eventRec[x].eventtype, sizeof(JE_byte), 1, lvlFile);
			efread(&eventRec[x].eventdat,  sizeof(JE_integer), 1, lvlFile);
			efread(&eventRec[x].eventdat2, sizeof(JE_integer), 1, lvlFile);
			efread(&eventRec[x].eventdat3, sizeof(JE_shortint), 1, lvlFile);
			efread(&eventRec[x].eventdat5, sizeof(JE_shortint), 1, lvlFile);
			efread(&eventRec[x].eventdat6, sizeof(JE_shortint), 1, lvlFile);
			efread(&eventRec[x].eventdat4, sizeof(JE_byte), 1, lvlFile);
		}
		eventRec[x].eventtime = 65500;  /*Not needed but just in case*/

		/*debuginfo('Level loaded.');*/

		/*debuginfo('Loading Map');*/

		/* MAP SHAPE LOOKUP TABLE - Each map is directly after level */
		efread(mapSh, sizeof(JE_word), sizeof(mapSh) / sizeof(JE_word), lvlFile);
		for (temp = 0; temp < 3; temp++)
		{
			for (temp2 = 0; temp2 < 128; temp2++)
			{
				mapSh[temp][temp2] = SDL_Swap16(mapSh[temp][temp2]);
			}
		}

		/* Read Shapes.DAT */
		sprintf(tempStr, "shapes%c.dat", tolower(char_shapeFile));
		JE_resetFile(&shpFile, tempStr);

		for (z = 0; z < 600; z++)
		{
			shapeBlank = fgetc(shpFile);

			if (shapeBlank)
			{
				memset(shape, 0, sizeof(shape));
			} else {
				efread(shape, sizeof(JE_byte), sizeof(shape), shpFile);
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

		efread(mapBuf, sizeof(JE_byte), 14 * 300, lvlFile);
		bufLoc = 0;              /* MAP NUMBER 1 */
		for (y = 0; y < 300; y++)
		{
			for (x = 0; x < 14; x++)
			{
				megaData1->mainmap[y][x] = ref[0][mapBuf[bufLoc]];
				bufLoc++;
			}
		}

		efread(mapBuf, sizeof(JE_byte), 14 * 600, lvlFile);
		bufLoc = 0;              /* MAP NUMBER 2 */
		for (y = 0; y < 600; y++)
		{
			for (x = 0; x < 14; x++)
			{
				megaData2->mainmap[y][x] = ref[1][mapBuf[bufLoc]];
				bufLoc++;
			}
		}

		efread(mapBuf, sizeof(JE_byte), 15 * 600, lvlFile);
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
	JE_byte nameGo[SA + 2] = {0}; /* [1..SA+2] */
	JE_word waitForDemo;
	JE_byte menu = 0;
	JE_boolean redraw = true,
	           fadeIn = false,
	           first = true;
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

	playDemo = false;

	stoppedDemo = false;

	first  = true;
	redraw = true;
	fadeIn = false;

	gameLoaded = false;
	jumpSection = false;

	joystickWaitMax = 80;
	joystickWait = 0;

	/* If IsNetworkActive { TODO We're not having networking, are we? } else { */

	do
	{
		defaultBrightness = -3;

		/* Animate instead of quickly fading in */
		if (redraw)
		{
			if (currentSong != SONG_TITLE) JE_playSong(SONG_TITLE);

			menu = 0;
			redraw = false;
			if (animate)
			{
				if (fadeIn)
				{
					JE_fadeBlack(10);
				}
				JE_loadPic(4, false);

				memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

				if (moveTyrianLogoUp)
				{
					temp = 62;
				} else {
					temp = 4;
				}

				JE_newDrawCShapeNum(PLANET_SHAPES, 147, 11, temp);


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

				fadeIn = false;

				if (moveTyrianLogoUp)
				{
					for (temp = 61; temp >= 4; temp -= 2)
					{
						int i;

						setjasondelay(2);
						memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);

						JE_newDrawCShapeNum(PLANET_SHAPES, 147, 11, temp);

						JE_showVGA();
						wait_delay();
					}
				}
				moveTyrianLogoUp = false;

				/* Draw Menu Text on Screen */
				for (temp = 0; temp < menunum; temp++)
				{
					tempX = 104+(temp)*13;
					if (temp == 4) /* OpenTyrian override */
					{
						tempY = JE_fontCenter(opentyrian_str, SMALL_FONT_SHAPES);

						JE_outTextAdjust(tempY-1,tempX-1,opentyrian_str,15,-10,SMALL_FONT_SHAPES,false);
						JE_outTextAdjust(tempY+1,tempX+1,opentyrian_str,15,-10,SMALL_FONT_SHAPES,false);
						JE_outTextAdjust(tempY+1,tempX-1,opentyrian_str,15,-10,SMALL_FONT_SHAPES,false);
						JE_outTextAdjust(tempY-1,tempX+1,opentyrian_str,15,-10,SMALL_FONT_SHAPES,false);
						JE_outTextAdjust(tempY,tempX,opentyrian_str,15,-3,SMALL_FONT_SHAPES,false);
					} else {
						tempY = JE_fontCenter(menuText[temp],SMALL_FONT_SHAPES);

						JE_outTextAdjust(tempY-1,tempX-1,menuText[temp],15,-10,SMALL_FONT_SHAPES,false);
						JE_outTextAdjust(tempY+1,tempX+1,menuText[temp],15,-10,SMALL_FONT_SHAPES,false);
						JE_outTextAdjust(tempY+1,tempX-1,menuText[temp],15,-10,SMALL_FONT_SHAPES,false);
						JE_outTextAdjust(tempY-1,tempX+1,menuText[temp],15,-10,SMALL_FONT_SHAPES,false);
						JE_outTextAdjust(tempY,tempX,menuText[temp],15,-3,SMALL_FONT_SHAPES,false);
					}
				}
				JE_showVGA();

				JE_fadeColors(&colors, &colors2, 0, 255, 20);

				memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);
			}
		}

		memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);

		for (temp = 0; temp < menunum; temp++)
		{
			if (temp == 4) /* OpenTyrian override */
			{
				JE_outTextAdjust(JE_fontCenter(opentyrian_str, SMALL_FONT_SHAPES), 104+temp*13,
				                 opentyrian_str, 15, -3+((temp == menu) * 2), SMALL_FONT_SHAPES, false);
			} else {
				JE_outTextAdjust(JE_fontCenter(menuText[temp], SMALL_FONT_SHAPES), 104+temp*13,
				                 menuText[temp], 15, -3+((temp == menu) * 2), SMALL_FONT_SHAPES, false);
			}
		}

		JE_showVGA();

		first = false;

		if (trentWin)
		{
			quit = true;
			goto trentWinsGame;
		}

		waitForDemo = -1; /* TODO Uh, fix this? */
		JE_textMenuWait(&waitForDemo, false);

		if (waitForDemo == 1)
		{
			playDemo = true;
			playDemoNum++;
			if (playDemoNum > 5)
				playDemoNum = 1;
		}

		if (keysactive[SDLK_LALT] && keysactive[SDLK_x])
		{
			quit = true;
		}

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
				default:
					break;
			}
		}

		for (z = 0; z < SA+2; z++)
		{
			if (specialName[z][nameGo[z]] == toupper(lastkey_char))
			{
				nameGo[z]++;
				if (strlen(specialName[z]) == nameGo[z])
				{
					if (z == SA)
					{
						loadDestruct = true;
					} else if (z == SA+1) {
						/* SuperTyrian */

						JE_playSampleNum(37);
						JE_whoa();
						JE_clr256();
						JE_outText(10, 10, "Cheat codes have been disabled.", 15, 4);

						if (keysactive[SDLK_SCROLLOCK])
						{
							initialDifficulty = 6;
						} else {
							initialDifficulty = 8;
						}

						if (initialDifficulty == 8)
						{
							JE_outText(10, 20, "Difficulty level has been set to Lord of Game.", 15, 4);
						} else {
							JE_outText(10, 20, "Difficulty level has been set to Suicide.", 15, 4);
						}
						JE_outText(10, 30, "It is imperitive that you discover the special codes.", 15, 4);
						if (initialDifficulty == 8)
						{
							JE_outText(10, 40, "(Next time, for an easier challenge hold down SCROLL LOCK.)", 15, 4);
						}
						JE_outText(10, 60, "Prepare to play...", 15, 4);

						char buf[10+1+15+1];
						snprintf(buf, sizeof(buf), "%s %s", miscTextB[4], pName[0]);
						JE_dString(JE_fontCenter(buf, FONT_SHAPES), 110, buf, FONT_SHAPES);
						JE_playSong(17);
						JE_playSampleNum(35);
						JE_showVGA();

						JE_wipeKey();

						wait_input(true, true, true);

						constantDie = false;
						JE_initEpisode(1);
						superTyrian = true;
						onePlayerAction = true;
						pItems[11] = 13;
						pItems[0] = 39;
						pItems[2] = 254;
						gameLoaded = true;
						difficultyLevel = initialDifficulty;
						score = 0;
					} else {
						pItems[2] = z+1;
						pItems[11] = SAShip[z];
						JE_fadeBlack(10);
						if (JE_episodeSelect() && JE_difficultySelect())
						{
							/* Start special mode! */
							JE_fadeBlack(10);
							JE_loadPic(1, false);
							JE_clr256();
							JE_dString(JE_fontCenter(superShips[0], FONT_SHAPES), 30, superShips[0], FONT_SHAPES);
							JE_dString(JE_fontCenter(superShips[z+1], SMALL_FONT_SHAPES), 100, superShips[z+1], SMALL_FONT_SHAPES);
							tempW = ships[pItems[11]].shipgraphic;
							if (tempW != 1)
							{
								JE_drawShape2x2(148, 70, tempW, shapes9);
							}

							JE_showVGA();
							JE_fadeColor(50);

							wait_input(true, true, true);

							twoPlayerMode = false;
							onePlayerAction = true;
							superArcadeMode = z+1;
							gameLoaded = true;
							score = 0;
							pItems[0] = SAWeapon[z][0];
							pItems[10] = SASpecialWeapon[z];
							if (z+1 == SA)
							{
								pItems[3] = 24;
								pItems[4] = 24;
							}
							difficultyLevel++;
							initialDifficulty = difficultyLevel;
						} else {
							redraw = true;
							fadeIn = true;
						}
					}
					newkey = false;
				}
			} else {
				nameGo[z] = 0;
			}
		}
		lastkey_char = '\0';

		if (newkey)
		{
			switch (lastkey_sym)
			{
				case SDLK_ESCAPE:
					quit = true;
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
									gameLoaded = true;
								} else {
									redraw = true;
									fadeIn = true;
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
							fadeIn = true;
							break;
						case 1: /* Load game */
							JE_loadScreen();
							if (!gameLoaded)
							{
								redraw = true;
							}
							fadeIn = true;
							break;
						case 2: /* High scores */
							JE_highScoreScreen();
							fadeIn = true;
							break;
						case 3: /* Instructions */
							JE_helpSystem(1);
							redraw = true;
							fadeIn = true;
							break;
						case 4: /* Ordering info, now OpenTyrian menu*/
							opentyrian_menu();
							redraw = true;
							fadeIn = true;
							break;
						case 5: /* Demo */
							JE_initPlayerData();
							playDemo = true;
							if (playDemoNum++ > 4)
							{
								playDemoNum = 1;
							}
							break;
						case 6: /* Quit */
							quit = true;
							break;
					}
					redraw = true;
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

	moveTyrianLogoUp = true;

	if (!isNetworkGame && !stoppedDemo)
	{
		memcpy(colors, black, sizeof(colors));
		memset(black, 63, sizeof(black));
		JE_fadeColors(&colors, &black, 0, 255, 50);

		JE_loadPic(10, false);
		JE_showVGA();

		JE_fadeColors(&black, &colors, 0, 255, 50);
		memset(black, 0, sizeof(black));

		setjasondelay(200);
		while (!JE_anyButton() && (signed int)(target - SDL_GetTicks()) > 0)
			SDL_Delay(16);

		JE_fadeBlack(15);

		JE_loadPic(12, false);
		JE_showVGA();

		memcpy(colors, palettes[pcxpal[12-1]], sizeof(colors));
		JE_fadeColor(10);

		setjasondelay(200);
		while (!JE_anyButton() && (signed int)(target - SDL_GetTicks()) > 0)
			SDL_Delay(16);

		JE_fadeBlack(10);
	}
}

void JE_readTextSync( void )
{
	JE_clr256();
	JE_showVGA();
	JE_loadPic(1, true);

	JE_barShade(3, 3, 316, 196);
	JE_barShade(1, 1, 318, 198);
	JE_dString(10, 160, "Waiting for other player.", SMALL_FONT_SHAPES);
	JE_showVGA();

	JE_setNetByte(251);

	do
	{
		setjasondelay(2);

		JE_updateStream();
		if (netQuit)
		{
			JE_tyrianHalt(5);
		}

		int delaycount_temp;
		if ((delaycount_temp = target - SDL_GetTicks()) > 0)
			SDL_Delay(delaycount_temp);

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
	tempW = 184;
	if (warningRed)
	{
		tempW = 7 * 16 + 6;
	}

	JE_outCharGlow(JE_fontCenter(miscText[4], TINY_FONT), tempW, miscText[4]);

	do
	{
		if (levelWarningDisplay)
		{
			JE_updateWarning();
		}

		setjasondelay(1);

		JE_setNetByte(0);
		JE_updateStream();
		if (netQuit)
		{
			JE_tyrianHalt(5);
		}

		int delaycount_temp;
		if ((delaycount_temp = target - SDL_GetTicks()) > 0)
			SDL_Delay(delaycount_temp);

	} while (!(JE_anyButton() || (frameCountMax == 0 && temp == 1) || ESCPressed));
	levelWarningDisplay = false;
}


void JE_makeEnemy( struct JE_SingleEnemyType *enemy )
{
	JE_byte temp;
	int t = 0;

	if (superArcadeMode > 0 && tempW == 534)
	{
		tempW = 533;
	}

	enemyShapeTables[5-1] = 21;   /*Coins&Gems*/
	enemyShapeTables[6-1] = 26;   /*Two-Player Stuff*/

	if (uniqueEnemy)
	{
		temp = tempI2;
		uniqueEnemy = false;
	} else {
		temp = enemyDat[tempW].shapebank;
	}

	for (a = 0; a < 6; a++)
	{
		if (temp == enemyShapeTables[a])
		{
			switch (a)
			{
				case 0:
					enemy->shapeseg = eShapes1;
					break;
				case 1:
					enemy->shapeseg = eShapes2;
					break;
				case 2:
					enemy->shapeseg = eShapes3;
					break;
				case 3:
					enemy->shapeseg = eShapes4;
					break;
				case 4:
					enemy->shapeseg = eShapes5;
					break;
				case 5:
					enemy->shapeseg = eShapes6;
					break;
			}
		}
	}

	enemy->enemydatofs = &enemyDat[tempW];

	enemy->mapoffset = 0;

	for (a = 0; a < 3; a++)
	{
		enemy->eshotmultipos[a] = 0;
	}

	temp4 = enemyDat[tempW].explosiontype;
	enemy->enemyground = ((temp4 & 0x01) == 0);
	enemy->explonum = temp4 / 2;

	enemy->launchfreq = enemyDat[tempW].elaunchfreq;
	enemy->launchwait = enemyDat[tempW].elaunchfreq;
	enemy->launchtype = enemyDat[tempW].elaunchtype % 1000;
	enemy->launchspecial = enemyDat[tempW].elaunchtype / 1000;

	enemy->xaccel = enemyDat[tempW].xaccel;
	enemy->yaccel = enemyDat[tempW].yaccel;

	enemy->xminbounce = -10000;
	enemy->xmaxbounce = 10000;
	enemy->yminbounce = -10000;
	enemy->ymaxbounce = 10000;
	/*Far enough away to be impossible to reach*/

	for (a = 0; a < 3; a++)
	{
		enemy->tur[a] = enemyDat[tempW].tur[a];
	}

	enemy->ani = enemyDat[tempW].ani;
	enemy->animin = 1;

	switch (enemyDat[tempW].animate)
	{
		case 0:
			enemy->enemycycle = 1;
			enemy->aniactive = 0;
			enemy->animax = 0;
			enemy->aniwhenfire = 0;
			break;
		case 1:
			enemy->enemycycle = 0;
			enemy->aniactive = 1;
			enemy->animax = 0;
			enemy->aniwhenfire = 0;
			break;
		case 2:
			enemy->enemycycle = 1;
			enemy->aniactive = 2;
			enemy->animax = enemy->ani;
			enemy->aniwhenfire = 2;
			break;
	}

	if (enemyDat[tempW].startxc != 0)
	{
		enemy->ex = enemyDat[tempW].startx + (rand() % (enemyDat[tempW].startxc * 2)) - enemyDat[tempW].startxc + 1;
	} else {
		enemy->ex = enemyDat[tempW].startx + 1;
	}

	if (enemyDat[tempW].startyc != 0)
	{
		enemy->ey = enemyDat[tempW].starty + (rand() % (enemyDat[tempW].startyc * 2)) - enemyDat[tempW].startyc + 1;
	} else {
		enemy->ey = enemyDat[tempW].starty + 1;
	}

	enemy->exc = enemyDat[tempW].xmove;
	enemy->eyc = enemyDat[tempW].ymove;
	enemy->excc = enemyDat[tempW].xcaccel;
	enemy->eycc = enemyDat[tempW].ycaccel;
	enemy->exccw = abs(enemy->excc);
	enemy->exccwmax = enemy->exccw;
	enemy->eyccw = abs(enemy->eycc);
	enemy->eyccwmax = enemy->eyccw;
	if (enemy->excc > 0)
	{
		enemy->exccadd = 1;
	} else {
		enemy->exccadd = -1;
	}
	if (enemy->eycc > 0)
	{
		enemy->eyccadd = 1;
	} else {
		enemy->eyccadd = -1;
	}

	enemy->special = false;
	enemy->iced = 0;

	if (enemyDat[tempW].xrev == 0)
	{
		enemy->exrev = 100;
	} else {
		if (enemyDat[tempW].xrev == -99)
		{
			enemy->exrev = 0;
		} else {
			enemy->exrev = enemyDat[tempW].xrev;
		}
	}
	if (enemyDat[tempW].yrev == 0)
	{
		enemy->eyrev = 100;
	} else {
		if (enemyDat[tempW].yrev == -99)
		{
			enemy->eyrev = 0;
		} else {
			enemy->eyrev = enemyDat[tempW].yrev;
		}
	}
	if (enemy->xaccel > 0)
	{
		enemy->exca = 1;
	} else {
		enemy->exca = -1;
	}
	if (enemy->yaccel > 0)
	{
		enemy->eyca = 1;
	} else {
		enemy->eyca = - 1;
	}

	enemy->enemytype = tempW;

	for (a = 0; a < 3; a++)
	{
		if (enemy->tur[a] == 252)
		{
			enemy->eshotwait[a] = 1;
		} else {
			if (enemy->tur[a] > 0)
			{
				enemy->eshotwait[a] = 20;
			} else {
				enemy->eshotwait[a] = 255;
			}
		}
	}
	for (a = 0; a < 20; a++)
	{
		enemy->egr[a] = enemyDat[tempW].egraphic[a];
	}
	enemy->size = enemyDat[tempW].esize;
	enemy->linknum = 0;
	if (enemyDat[tempW].dani < 0)
	{
		enemy->edamaged = true;
	} else {
		enemy->edamaged = false;
	}
	enemy->enemydie = enemyDat[tempW].eenemydie;

	enemy->freq[1-1] = enemyDat[tempW].freq[1-1];
	enemy->freq[2-1] = enemyDat[tempW].freq[2-1];
	enemy->freq[3-1] = enemyDat[tempW].freq[3-1];

	enemy->edani   = enemyDat[tempW].dani;
	enemy->edgr    = enemyDat[tempW].dgr;
	enemy->edlevel = enemyDat[tempW].dlevel;

	enemy->fixedmovey = 0;

	enemy->filter = 0x00;

	if (enemyDat[tempW].value > 1 && enemyDat[tempW].value < 10000)
	{
		switch (difficultyLevel)
		{
			case -1:
			case 0:
				t = enemyDat[tempW].value * 0.75f;
				break;
			case 1:
			case 2:
				t = enemyDat[tempW].value;
				break;
			case 3:
				t = enemyDat[tempW].value * 1.125f;
				break;
			case 4:
				t = enemyDat[tempW].value * 1.5f;
				break;
			case 5:
				t = enemyDat[tempW].value * 2;
				break;
			case 6:
				t = enemyDat[tempW].value * 2.5f;
				break;
			case 7:
			case 8:
				t = enemyDat[tempW].value * 4;
				break;
			case 9:
			case 10:
				t = enemyDat[tempW].value * 8;
				break;
		}
		if (t > 10000)
		{
			t = 10000;
		}
		enemy->evalue = t;
	} else {
		enemy->evalue = enemyDat[tempW].value;
	}

	t = 1;
	if (enemyDat[tempW].armor > 0)
	{

		if (enemyDat[tempW].armor != 255)
		{

			switch (difficultyLevel)
			{
				case -1:
				case 0:
					t = enemyDat[tempW].armor * 0.5f + 1;
					break;
				case 1:
					t = enemyDat[tempW].armor * 0.75f + 1;
					break;
				case 2:
					t = enemyDat[tempW].armor;
					break;
				case 3:
					t = enemyDat[tempW].armor * 1.2f;
					break;
				case 4:
					t = enemyDat[tempW].armor * 1.5f;
					break;
				case 5:
					t = enemyDat[tempW].armor * 1.8f;
					break;
				case 6:
					t = enemyDat[tempW].armor * 2;
					break;
				case 7:
					t = enemyDat[tempW].armor * 3;
					break;
				case 8:
					t = enemyDat[tempW].armor * 4;
					break;
				case 9:
				case 10:
					t = enemyDat[tempW].armor * 8;
					break;
			}

			if (t > 254)
			{
				t = 254;
			}

		} else {
			t = 255;
		}

		enemy->armorleft = t;
		
		a = 0;
		enemy->scoreitem = false;
	} else {
		a = 2;
		enemy->armorleft = 255;
		if (enemy->evalue != 0)
		{
			enemy->scoreitem = true;
		}
	}
	/*The returning A value indicates what to set ENEMYAVAIL to */

	if (!enemy->scoreitem)
	{
		totalEnemy++;  /*Destruction ratio*/
	}
}

void JE_createNewEventEnemy( JE_byte enemyTypeOfs, JE_word enemyOffset )
{
	int i;

	b = 0;

	for(i = enemyOffset; i < enemyOffset + 25; i++)
	{
		if (enemyAvail[i] == 1)
		{
			b = i + 1;
			break;
		}
	}

	if (b == 0)
	{
		return;
	}

	tempW = eventRec[eventLoc-1].eventdat + enemyTypeOfs;

	JE_makeEnemy(&enemy[b-1]);

	enemyAvail[b-1] = a;

	if (eventRec[eventLoc-1].eventdat2 != -99)
	{
		switch (enemyOffset)
		{
			case 0:
				enemy[b-1].ex = eventRec[eventLoc-1].eventdat2 - (mapX - 1) * 24;
				enemy[b-1].ey -= backMove2;
				break;
			case 25:
			case 75:
				enemy[b-1].ex = eventRec[eventLoc-1].eventdat2 - (mapX - 1) * 24 - 12;
				enemy[b-1].ey -= backMove;
				break;
			case 50:
				if (background3x1)
				{
					enemy[b-1].ex = eventRec[eventLoc-1].eventdat2 - (mapX - 1) * 24 - 12;
				} else {
					enemy[b-1].ex = eventRec[eventLoc-1].eventdat2 - mapX3 * 24 - 24 * 2 + 6;
				}
				enemy[b-1].ey -= backMove3;

				if (background3x1b)
				{
					enemy[b-1].ex -= 6;
				}
				break;
		}
		enemy[b-1].ey = -28;
		if (background3x1b && enemyOffset == 50)
		{
			enemy[b-1].ey += 4;
		}
	}

	if (smallEnemyAdjust && enemy[b-1].size == 0)
	{
		enemy[b-1].ex -= 10;
		enemy[b-1].ey -= 7;
	}

	enemy[b-1].ey += eventRec[eventLoc-1].eventdat5;
	enemy[b-1].eyc += eventRec[eventLoc-1].eventdat3;
	enemy[b-1].linknum = eventRec[eventLoc-1].eventdat4;
	enemy[b-1].fixedmovey = eventRec[eventLoc-1].eventdat6;
}

void JE_eventJump( JE_word jump )
{
	JE_word tempW;

	if (jump == 65535)
	{
		curLoc = returnLoc;
	} else {
		returnLoc = curLoc + 1;
		curLoc = jump;
	}
	tempW = 0;
	do {
		tempW++;
	} while (!(eventRec[tempW-1].eventtime >= curLoc));
	eventLoc = tempW - 1;
}

JE_boolean JE_searchFor/*enemy*/( JE_byte PLType )
{
	JE_boolean tempb = false;
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
			tempb = true;
		}
	}
	return tempb;
}

void JE_eventSystem( void )
{
	JE_boolean tempb;
	
	switch (eventRec[eventLoc-1].eventtype)
	{
		case 1:
			starY = eventRec[eventLoc-1].eventdat * 320;
			break;
		case 2:
			map1YDelay = 1;
			map1YDelayMax = 1;
			map2YDelay = 1;
			map2YDelayMax = 1;

			backMove = eventRec[eventLoc-1].eventdat;
			backMove2 = eventRec[eventLoc-1].eventdat2;
			if (backMove2 > 0)
			{
				explodeMove = backMove2 * 320;
			} else {
				explodeMove = backMove * 320;
			}
			backMove3 = eventRec[eventLoc-1].eventdat3;

			if (backMove > 0)
			{
				stopBackgroundNum = 0;
			}
			break;
		case 3:
			backMove = 1;
			map1YDelay = 3;
			map1YDelayMax = 3;
			backMove2 = 1;
			map2YDelay = 2;
			map2YDelayMax = 2;
			backMove3 = 1;
			break;
		case 4:
			stopBackgrounds = true;
			switch (eventRec[eventLoc-1].eventdat)
			{
				case 0:
				case 1:
					stopBackgroundNum = 1;
					break;
				case 2:
					stopBackgroundNum = 2;
					break;
				case 3:
					stopBackgroundNum = 3;
					break;
			}
			break;
		case 5:
			if (enemyShapeTables[1-1] != eventRec[eventLoc-1].eventdat)
			{
				if (eventRec[eventLoc-1].eventdat > 0)
				{
					JE_loadCompShapes(&eShapes1, &eShapes1Size, shapeFile[eventRec[eventLoc-1].eventdat -1]);      /* Enemy Bank 1 */
					enemyShapeTables[1-1] = eventRec[eventLoc-1].eventdat;
				} else if (eShapes1 != NULL) {
					free(eShapes1);
					eShapes1 = NULL;
					enemyShapeTables[1-1] = 0;
				}
			}
			if (enemyShapeTables[2-1] != eventRec[eventLoc-1].eventdat2)
			{
				if (eventRec[eventLoc-1].eventdat2 > 0)
				{
					JE_loadCompShapes(&eShapes2, &eShapes2Size, shapeFile[eventRec[eventLoc-1].eventdat2-1]);      /* Enemy Bank 2 */
					enemyShapeTables[2-1] = eventRec[eventLoc-1].eventdat2;
				} else if (eShapes2 != NULL) {
					free(eShapes2);
					eShapes2 = NULL;
					enemyShapeTables[2-1] = 0;
				}
			}
			if (enemyShapeTables[3-1] != eventRec[eventLoc-1].eventdat3)
			{
				if (eventRec[eventLoc-1].eventdat3 > 0)
				{
					JE_loadCompShapes(&eShapes3, &eShapes3Size, shapeFile[eventRec[eventLoc-1].eventdat3-1]);      /* Enemy Bank 3 */
					enemyShapeTables[3-1] = eventRec[eventLoc-1].eventdat3;
				} else if (eShapes3 != NULL) {
					free(eShapes3);
					eShapes3 = NULL;
					enemyShapeTables[3-1] = 0;
				}
			}
			if (enemyShapeTables[4-1] != eventRec[eventLoc-1].eventdat4)
			{
				if (eventRec[eventLoc-1].eventdat4 > 0)
				{
					JE_loadCompShapes(&eShapes4, &eShapes4Size, shapeFile[eventRec[eventLoc-1].eventdat4-1]);      /* Enemy Bank 4 */
					enemyShapeTables[4-1] = eventRec[eventLoc-1].eventdat4;
					enemyShapeTables[5-1] = 21;
				} else if (eShapes4 != NULL) {
					free(eShapes4);
					eShapes4 = NULL;
					enemyShapeTables[4-1] = 0;
				}
			}
			break;
		case 6: /* Ground Enemy */
			JE_createNewEventEnemy(0, 25);
			break;
		case 7: /* Top Enemy */
			JE_createNewEventEnemy(0, 50);
			break;
		case 8:
			starActive = false;
			break;
		case 9:
			starActive = true;
			break;
		case 10: /* Ground Enemy 2 */
			JE_createNewEventEnemy(0, 75);
			break;
		case 11:
			if (allPlayersGone || eventRec[eventLoc-1].eventdat == 1)
				reallyEndLevel = true;
			else
				if (!endLevel)
				{
					readyToEndLevel = false;
					endLevel = true;
					levelEnd = 40;
				}
			break;
		case 12: /* Custom 4x4 Ground Enemy */
			switch (eventRec[eventLoc-1].eventdat6)
			{
				case 0:
				case 1:
					tempW4 = 25;
					break;
				case 2:
					tempW4 = 0;
					break;
				case 3:
					tempW4 = 50;
					break;
				case 4:
					tempW4 = 75;
					break;
			}
			eventRec[eventLoc-1].eventdat6 = 0;   /* We use EVENTDAT6 for the background */
			JE_createNewEventEnemy(0, tempW4);
			JE_createNewEventEnemy(1, tempW4);
			enemy[b-1].ex += 24;
			JE_createNewEventEnemy(2, tempW4);
			enemy[b-1].ey -= 28;
			JE_createNewEventEnemy(3, tempW4);
			enemy[b-1].ex += 24;
			enemy[b-1].ey -= 28;
			break;
		case 13:
			enemiesActive = false;
			break;
		case 14:
			enemiesActive = true;
			break;
		case 15: /* Sky Enemy */
			JE_createNewEventEnemy(0, 0);
			break;
		case 16:
			if (eventRec[eventLoc-1].eventdat > 9)
			{
				printf("-!- EVENT 16: BAD EVENT DATA\n");
			} else {
				JE_drawTextWindow(outputs[eventRec[eventLoc-1].eventdat-1]);
				soundQueue[3] = windowTextSamples[eventRec[eventLoc-1].eventdat-1];
			}
			break;
		case 17: /* Ground Bottom */
			JE_createNewEventEnemy(0, 25);
			if (b > 0)
			{
				enemy[b-1].ey = 190 + eventRec[eventLoc-1].eventdat5;
			}
			break;

		case 18: /* Sky Enemy on Bottom */
			JE_createNewEventEnemy(0, 0);
			if (b > 0)
			{
				enemy[b-1].ey = 190 + eventRec[eventLoc-1].eventdat5;
			}
			break;

		case 19: /* Enemy Global Move */
			if (eventRec[eventLoc-1].eventdat3 > 79 && eventRec[eventLoc-1].eventdat3 < 90)
			{
				temp2 = 1;
				temp3 = 100;
				temp4 = 0;
				eventRec[eventLoc-1].eventdat4 = newPL[eventRec[eventLoc-1].eventdat3 - 80];
			} else {
				switch (eventRec[eventLoc-1].eventdat3)
				{
					case 0:
						temp2 = 1;
						temp3 = 100;
						temp4 = 0;
						break;
					case 2:
						temp2 = 1;
						temp3 = 25;
						temp4 = 1;
						break;
					case 1:
						temp2 = 26;
						temp3 = 50;
						temp4 = 1;
						break;
					case 3:
						temp2 = 51;
						temp3 = 75;
						temp4 = 1;
						break;
					case 99:
						temp2 = 1;
						temp3 = 100;
						temp4 = 1;
						break;
				}
			}

			for (temp = temp2-1; temp < temp3; temp++)
			{
				if (temp4 == 1 || enemy[temp].linknum == eventRec[eventLoc-1].eventdat4)
				{
					if (eventRec[eventLoc-1].eventdat != -99)
					{
						enemy[temp].exc = eventRec[eventLoc-1].eventdat;
					}
					if (eventRec[eventLoc-1].eventdat2 != -99)
					{
						enemy[temp].eyc = eventRec[eventLoc-1].eventdat2;
					}
					if (eventRec[eventLoc-1].eventdat6 != 0)
					{
						enemy[temp].fixedmovey = eventRec[eventLoc-1].eventdat6;
					}
					if (eventRec[eventLoc-1].eventdat6 == -99)
					{
						enemy[temp].fixedmovey = 0;
					}
					if (eventRec[eventLoc-1].eventdat5 > 0)
					{
						enemy[temp].enemycycle = eventRec[eventLoc-1].eventdat5;
					}
				}
			}
			break;
		case 20: /* Enemy Global Accel */
			if (eventRec[eventLoc-1].eventdat3 > 79 && eventRec[eventLoc-1].eventdat3 < 90)
			{
				eventRec[eventLoc-1].eventdat4 = newPL[eventRec[eventLoc-1].eventdat3 - 80];
			}
			for (temp = 0; temp < 100; temp++)
			{
				if (enemyAvail[temp] != 1
				    && (enemy[temp].linknum == eventRec[eventLoc-1].eventdat4 || eventRec[eventLoc-1].eventdat4 == 0))
				{
					if (eventRec[eventLoc-1].eventdat != -99)
					{
						enemy[temp].excc = eventRec[eventLoc-1].eventdat;
						enemy[temp].exccw = abs(eventRec[eventLoc-1].eventdat);
						enemy[temp].exccwmax = abs(eventRec[eventLoc-1].eventdat);
						if (eventRec[eventLoc-1].eventdat > 0)
						{
							enemy[temp].exccadd = 1;
						} else {
							enemy[temp].exccadd = -1;
						}
					}

					if (eventRec[eventLoc-1].eventdat2 != -99)
					{
						enemy[temp].eycc = eventRec[eventLoc-1].eventdat2;
						enemy[temp].eyccw = abs(eventRec[eventLoc-1].eventdat2);
						enemy[temp].eyccwmax = abs(eventRec[eventLoc-1].eventdat2);
						if (eventRec[eventLoc-1].eventdat2 > 0)
						{
							enemy[temp].eyccadd = 1;
						} else {
							enemy[temp].eyccadd = -1;
						}
					}

					if (eventRec[eventLoc-1].eventdat5 > 0)
					{
						enemy[temp].enemycycle = eventRec[eventLoc-1].eventdat5;
					}
					if (eventRec[eventLoc-1].eventdat6 > 0)
					{
						enemy[temp].ani = eventRec[eventLoc-1].eventdat6;
						enemy[temp].animin = eventRec[eventLoc-1].eventdat5;
						enemy[temp].animax = 0;
						enemy[temp].aniactive = 1;
					}
				}
			}
			break;
		case 21:
			background3over = 1;
			break;
		case 22:
			background3over = 0;
			break;
		case 23: /* Sky Enemy on Bottom */
			JE_createNewEventEnemy(0, 50);
			if (b > 0)
			{
				enemy[b-1].ey = 180 + eventRec[eventLoc-1].eventdat5;
			}
			break;

		case 24: /* Enemy Global Animate */
			for (temp = 0; temp < 100; temp++)
			{
				if (enemy[temp].linknum == eventRec[eventLoc-1].eventdat4)
				{
					enemy[temp].aniactive = 1;
					enemy[temp].aniwhenfire = 0;
					if (eventRec[eventLoc-1].eventdat2 > 0)
					{
						enemy[temp].enemycycle = eventRec[eventLoc-1].eventdat2;
						enemy[temp].animin = enemy[temp].enemycycle;
					} else {
						enemy[temp].enemycycle = 0;
					}
					if (eventRec[eventLoc-1].eventdat > 0)
					{
						enemy[temp].ani = eventRec[eventLoc-1].eventdat;
					}
					if (eventRec[eventLoc-1].eventdat3 == 1)
					{
						enemy[temp].animax = enemy[temp].ani;
					} else {
						if (eventRec[eventLoc-1].eventdat3 == 2)
						{
							enemy[temp].aniactive = 2;
							enemy[temp].animax = enemy[temp].ani;
							enemy[temp].aniwhenfire = 2;
						}
					}
				}
			}
			break;
		case 25: /* Enemy Global Damage change */
			for (temp = 0; temp < 100; temp++)
			{
				if (eventRec[eventLoc-1].eventdat4 == 0 || enemy[temp].linknum == eventRec[eventLoc-1].eventdat4)
				{
					enemy[temp].armorleft = eventRec[eventLoc-1].eventdat;
					if (galagaMode)
					{
						enemy[temp].armorleft = round(eventRec[eventLoc-1].eventdat * (difficultyLevel / 2));
					}
				}
			}
			break;
		case 26:
			smallEnemyAdjust = eventRec[eventLoc-1].eventdat;
			break;
		case 27: /* Enemy Global AccelRev */
			if (eventRec[eventLoc-1].eventdat3 > 79 && eventRec[eventLoc-1].eventdat3 < 90)
			{
				eventRec[eventLoc-1].eventdat4 = newPL[eventRec[eventLoc-1].eventdat3 - 80];
			}
			for (temp = 0; temp < 100; temp++)
			{
				if (eventRec[eventLoc-1].eventdat4 == 0 || enemy[temp].linknum == eventRec[eventLoc-1].eventdat4)
				{
					if (eventRec[eventLoc-1].eventdat != -99)
					{
						enemy[temp].exrev = eventRec[eventLoc-1].eventdat;
					}
					if (eventRec[eventLoc-1].eventdat2 != -99)
					{
						enemy[temp].eyrev = eventRec[eventLoc-1].eventdat2;
					}
					if (eventRec[eventLoc-1].eventdat3 != 0 && eventRec[eventLoc-1].eventdat3 < 17)
					{
						enemy[temp].filter = eventRec[eventLoc-1].eventdat3;
					}
				}
			}
			break;
		case 28:
			topEnemyOver = false;
			break;
		case 29:
			topEnemyOver = true;
			break;
		case 30:
			map1YDelay = 1;
			map1YDelayMax = 1;
			map2YDelay = 1;
			map2YDelayMax = 1;

			backMove = eventRec[eventLoc-1].eventdat;
			backMove2 = eventRec[eventLoc-1].eventdat2;
			explodeMove = backMove2 * 320;
			backMove3 = eventRec[eventLoc-1].eventdat3;
			break;
		case 31: /* Enemy Fire Override */
			for (temp = 0; temp < 100; temp++)
			{
				if (eventRec[eventLoc-1].eventdat4 == 99 || enemy[temp].linknum == eventRec[eventLoc-1].eventdat4)
				{
					enemy[temp].freq[1-1] = eventRec[eventLoc-1].eventdat ;
					enemy[temp].freq[2-1] = eventRec[eventLoc-1].eventdat2;
					enemy[temp].freq[3-1] = eventRec[eventLoc-1].eventdat3;
					for (temp2 = 0; temp2 < 3; temp2++)
					{
						enemy[temp].eshotwait[temp2] = 1;
					}
					if (enemy[temp].launchtype > 0)
					{
						enemy[temp].launchfreq = eventRec[eventLoc-1].eventdat5;
						enemy[temp].launchwait = 1;
					}
				}
			}
			break;
		case 32:
			JE_createNewEventEnemy(0, 50);
			if (b > 0)
			{
				enemy[b-1].ey = 190;
			}
			break;
		case 33: /* Enemy From other Enemies */
			if (!((eventRec[eventLoc-1].eventdat == 512 || eventRec[eventLoc-1].eventdat == 513) && (twoPlayerMode || onePlayerAction || superTyrian)))
			{
				if (superArcadeMode > 0)
				{
					if (eventRec[eventLoc-1].eventdat == 534)
					{
						eventRec[eventLoc-1].eventdat = 827;
					}
				} else {
					if (eventRec[eventLoc-1].eventdat == 533
					    && (portPower[1-1] == 11 || (rand() % 15) < portPower[1-1])
					    && !superTyrian)
					{
						eventRec[eventLoc-1].eventdat = 829 + (rand() % 6);
					}
				}
				if (eventRec[eventLoc-1].eventdat == 534 && superTyrian)
				{
					eventRec[eventLoc-1].eventdat = 828 + superTyrianSpecials[rand() % 4];
				}

				for (temp = 0; temp < 100; temp++)
				{
					if (enemy[temp].linknum == eventRec[eventLoc-1].eventdat4)
					{
						enemy[temp].enemydie = eventRec[eventLoc-1].eventdat;
					}
				}
			}
			break;
		case 34: /* Start Music Fade */
			if (firstGameOver)
			{
				musicFade = true;
				tempVolume = tyrMusicVolume;
			}
			break;
		case 35: /* Play new song */
			if (firstGameOver)
			{
				JE_playSong(eventRec[eventLoc-1].eventdat);
				if (!musicActive)
				{
					JE_selectSong (0);
				}
				JE_setVol(tyrMusicVolume, fxVolume);
			}
			musicFade = false;
			break;
		case 36:
			readyToEndLevel = true;
			break;
		case 37:
			levelEnemyFrequency = eventRec[eventLoc-1].eventdat;
			break;
		case 38:
			curLoc = eventRec[eventLoc-1].eventdat;
			tempW2 = 1;
			for (tempW = 0; tempW < maxEvent; tempW++)
			{
				if (eventRec[tempW].eventtime <= curLoc)
				{
					tempW2 = tempW+1 - 1;
				}
			}
			eventLoc = tempW2;
			break;
		case 39: /* Enemy Global Linknum Change */
			for (temp = 0; temp < 100; temp++)
			{
				if (enemy[temp].linknum == eventRec[eventLoc-1].eventdat)
				{
					enemy[temp].linknum = eventRec[eventLoc-1].eventdat2;
				}
			}
			break;
		case 40: /* Enemy Continual Damage */
			enemyContinualDamage = true;
			break;
		case 41:
			if (eventRec[eventLoc-1].eventdat == 0)
			{
				memset(enemyAvail, 1, sizeof(enemyAvail));
			} else {
				for (x = 0; x <= 24; x++)
				{
					enemyAvail[x] = 1;
				}
			}
			break;
		case 42:
			background3over = 2;
			break;
		case 43:
			background2over = eventRec[eventLoc-1].eventdat;
			break;
		case 44:
			filterActive       = (eventRec[eventLoc-1].eventdat > 0);
			filterFade         = (eventRec[eventLoc-1].eventdat == 2);
			levelFilter        = eventRec[eventLoc-1].eventdat2;
			levelBrightness    = eventRec[eventLoc-1].eventdat3;
			levelFilterNew     = eventRec[eventLoc-1].eventdat4;
			levelBrightnessChg = eventRec[eventLoc-1].eventdat5;
			filterFadeStart    = (eventRec[eventLoc-1].eventdat6 == 0);
			break;
		case 45: /* Two Player Enemy from other Enemies */
			if (!superTyrian)
			{
				if (eventRec[eventLoc-1].eventdat == 533
				    && (portPower[1-1] == 11 || (rand() % 15) < portPower[1-1]))
				{
					eventRec[eventLoc-1].eventdat = 829 + (rand() % 6);
				}
				if (twoPlayerMode || onePlayerAction)
				{
					for (temp = 0; temp < 100; temp++)
					{
						if (enemy[temp].linknum == eventRec[eventLoc-1].eventdat4)
						{
							enemy[temp].enemydie = eventRec[eventLoc-1].eventdat;
						}
					}
				}
			}
			break;
		case 46:
			if (eventRec[eventLoc-1].eventdat3 != 0)
			{
				damageRate = eventRec[eventLoc-1].eventdat3;
			}
			if (eventRec[eventLoc-1].eventdat2 == 0 || twoPlayerMode || onePlayerAction)
			{
				difficultyLevel += eventRec[eventLoc-1].eventdat;
				if (difficultyLevel < 1)
				{
					difficultyLevel = 1;
				}
				if (difficultyLevel > 10)
				{
					difficultyLevel = 10;
				}
			}
			break;
		case 47: /* Enemy Global AccelRev */
			for (temp = 0; temp < 100; temp++)
			{
				if (eventRec[eventLoc-1].eventdat4 == 0 || enemy[temp].linknum == eventRec[eventLoc-1].eventdat4)
				{
					enemy[temp].armorleft = eventRec[eventLoc-1].eventdat;
				}
			}
			break;
		case 48: /* Background 2 Cannot be Transparent */
			background2notTransparent = true;
			break;
		case 49:
		case 50:
		case 51:
		case 52:
			tempDat2 = eventRec[eventLoc-1].eventdat;
			eventRec[eventLoc-1].eventdat = 0;
			tempDat = eventRec[eventLoc-1].eventdat3;
			eventRec[eventLoc-1].eventdat3 = 0;
			tempDat3 = eventRec[eventLoc-1].eventdat6;
			eventRec[eventLoc-1].eventdat6 = 0;
			tempI2 = tempDat;
			enemyDat[0].armor = tempDat3;
			enemyDat[0].egraphic[1-1] = tempDat2;
			switch (eventRec[eventLoc-1].eventtype - 48)
			{
				case 1:
					temp = 25;
					break;
				case 2:
					temp = 0;
					break;
				case 3:
					temp = 50;
					break;
				case 4:
					temp = 75;
					break;
			}
			uniqueEnemy = true;
			JE_createNewEventEnemy(0, temp);
			uniqueEnemy = false;
			eventRec[eventLoc-1].eventdat = tempDat2;
			eventRec[eventLoc-1].eventdat3 = tempDat;
			eventRec[eventLoc-1].eventdat6 = tempDat3;
			break;

		case 53:
			forceEvents = (eventRec[eventLoc-1].eventdat != 99);
			break;
		case 54:
			JE_eventJump(eventRec[eventLoc-1].eventdat);
			break;
		case 55: /* Enemy Global AccelRev */
			if (eventRec[eventLoc-1].eventdat3 > 79 && eventRec[eventLoc-1].eventdat3 < 90)
			{
				eventRec[eventLoc-1].eventdat4 = newPL[eventRec[eventLoc-1].eventdat3 - 80];
			}
			for (temp = 0; temp < 100; temp++)
			{
				if (eventRec[eventLoc-1].eventdat4 == 0 || enemy[temp].linknum == eventRec[eventLoc-1].eventdat4)
				{
					if (eventRec[eventLoc-1].eventdat != -99)
					{
						enemy[temp].xaccel = eventRec[eventLoc-1].eventdat;
					}
					if (eventRec[eventLoc-1].eventdat2 != -99)
					{
						enemy[temp].yaccel = eventRec[eventLoc-1].eventdat2;
					}
				}
			}
			break;
		case 56: /* Ground2 Bottom */
			JE_createNewEventEnemy(0, 75);
			if (b > 0)
			{
				enemy[b-1].ey = 190;
			}
			break;
		case 57:
			superEnemy254Jump = eventRec[eventLoc-1].eventdat;
			break;

		case 60: /*Assign Special Enemy*/
			for (temp = 0; temp < 100; temp++)
			{
				if (enemy[temp].linknum == eventRec[eventLoc-1].eventdat4)
				{
					enemy[temp].special = true;
					enemy[temp].flagnum = eventRec[eventLoc-1].eventdat;
					enemy[temp].setto  = (eventRec[eventLoc-1].eventdat2 == 1);
				}
			}
			break;

		case 61: /*If Flag then...*/
			if (globalFlags[eventRec[eventLoc-1].eventdat] == eventRec[eventLoc-1].eventdat2)
			{
				eventLoc += eventRec[eventLoc-1].eventdat3;
			}
			break;
		case 62: /*Play sound effect*/
			soundQueue[3] = eventRec[eventLoc-1].eventdat;
			break;

		case 63: /*Skip X events if not in 2-player mode*/
			if (!twoPlayerMode && !onePlayerAction)
			{
				eventLoc += eventRec[eventLoc-1].eventdat;
			}
			break;

		case 64:
			if (!(eventRec[eventLoc-1].eventdat == 6 && twoPlayerMode && difficultyLevel > 2))
			{
				smoothies[eventRec[eventLoc-1].eventdat-1] = eventRec[eventLoc-1].eventdat2;
				temp = eventRec[eventLoc-1].eventdat;
				if (temp == 5)
					temp = 3;
				SDAT[temp-1] = eventRec[eventLoc-1].eventdat3;
			}
			break;

		case 65:
			background3x1 = (eventRec[eventLoc-1].eventdat == 0);
			break;
		case 66: /*If not on this difficulty level or higher then...*/
			if (initialDifficulty <= eventRec[eventLoc-1].eventdat)
				eventLoc += eventRec[eventLoc-1].eventdat2;
			break;
		case 67:
			levelTimer = (eventRec[eventLoc-1].eventdat == 1);
			levelTimerCountdown = eventRec[eventLoc-1].eventdat3 * 100;
			levelTimerJumpTo   = eventRec[eventLoc-1].eventdat2;
			break;
		case 68:
			randomExplosions = (eventRec[eventLoc-1].eventdat == 1);
			break;
		case 69:
			playerInvulnerable1 = eventRec[eventLoc-1].eventdat;
			playerInvulnerable2 = eventRec[eventLoc-1].eventdat;
			break;

		case 70:
			if (eventRec[eventLoc-1].eventdat2 == 0)
			{  /*1-10*/
				tempB = false;
				for (temp = 1; temp <= 19; temp++)
				{
					tempB = tempB | JE_searchFor(temp);
				}
				if (!tempB)
				{
					JE_eventJump(eventRec[eventLoc-1].eventdat);
				}
			} else {
				if (!JE_searchFor(eventRec[eventLoc-1].eventdat2)
				    && (eventRec[eventLoc-1].eventdat3 == 0 || !JE_searchFor(eventRec[eventLoc-1].eventdat3))
				    && (eventRec[eventLoc-1].eventdat4 == 0 || !JE_searchFor(eventRec[eventLoc-1].eventdat4)))
					JE_eventJump(eventRec[eventLoc-1].eventdat);
			}
			break;

		case 71:
			printf("-!- EVENT 71: POSSIBLY BAD MAP REPOSITONING\n");
			if (((((intptr_t)mapYPos - (intptr_t)&megaData1->mainmap) / sizeof(JE_byte *)) * 2) <= eventRec[eventLoc-1].eventdat2) /* <MXD> ported correctly? */
			{
				JE_eventJump(eventRec[eventLoc-1].eventdat);
			}
			break;

		case 72:
			background3x1b = (eventRec[eventLoc-1].eventdat == 1);
			break;

		case 73:
			skyEnemyOverAll = (eventRec[eventLoc-1].eventdat == 1);
			break;

		case 74: /* Enemy Global BounceParams */
			for (temp = 0; temp < 100; temp++)
			{
				if (eventRec[eventLoc-1].eventdat4 == 0 || enemy[temp].linknum == eventRec[eventLoc-1].eventdat4)
				{
					if (eventRec[eventLoc-1].eventdat5 != -99)
					{
						enemy[temp].xminbounce = eventRec[eventLoc-1].eventdat5;
					}
					if (eventRec[eventLoc-1].eventdat6 != -99)
					{
						enemy[temp].yminbounce = eventRec[eventLoc-1].eventdat6;
					}
					if (eventRec[eventLoc-1].eventdat != -99)
					{
						enemy[temp].xmaxbounce = eventRec[eventLoc-1].eventdat ;
					}
					if (eventRec[eventLoc-1].eventdat2 != -99)
					{
						enemy[temp].ymaxbounce = eventRec[eventLoc-1].eventdat2;
					}
				}
			}
			break;

		case 75:

			tempB = false;
			for (temp = 0; temp < 100; temp++)
			{
				if (enemyAvail[temp] == 0
				    && enemy[temp].eyc == 0
				    && enemy[temp].linknum >= eventRec[eventLoc-1].eventdat
				    && enemy[temp].linknum <= eventRec[eventLoc-1].eventdat2)
				{
					tempB = true;
				}
			}

			if (tempB)
			{
				do {
					temp = (rand() % (eventRec[eventLoc-1].eventdat2 + 1 - eventRec[eventLoc-1].eventdat)) + eventRec[eventLoc-1].eventdat;
				} while (!(JE_searchFor(temp) && enemy[temp5-1].eyc == 0));

				newPL[eventRec[eventLoc-1].eventdat3 - 80] = temp;
			} else {
				newPL[eventRec[eventLoc-1].eventdat3 - 80] = 255;
				if (eventRec[eventLoc-1].eventdat4 > 0)
				{ /*Skip*/
					curLoc = eventRec[eventLoc-1 + eventRec[eventLoc-1].eventdat4].eventtime - 1;
					eventLoc += eventRec[eventLoc-1].eventdat4 - 1;
				}
			}

			break;

		case 76:
			returnActive = true;
			break;

		case 77:
			printf("-!- EVENT 77: POSSIBLY BAD MAP REPOSITONING\n");
			mapYPos = &megaData1->mainmap[0][0];
			mapYPos += eventRec[eventLoc-1].eventdat / 2;
			if (eventRec[eventLoc-1].eventdat2 > 0)
			{
				mapY2Pos = &megaData2->mainmap[0][0];
				mapY2Pos += eventRec[eventLoc-1].eventdat2 / 2;
			} else {
				mapY2Pos = &megaData2->mainmap[0][0];
				mapY2Pos += eventRec[eventLoc-1].eventdat / 2;
			}
			break;

		case 78:
			if (galagaShotFreq < 10)
			{
				galagaShotFreq++;
			}
			break;

		case 79:
			statBar[1-1] = eventRec[eventLoc-1].eventdat;
			statBar[2-1] = eventRec[eventLoc-1].eventdat2;
			break;

		case 80: /*Skip X events if not in 2-player mode*/
			if (twoPlayerMode)
			{
				eventLoc += eventRec[eventLoc-1].eventdat;
			}
			break;

		case 81: /*WRAP2*/
			printf("-!- EVENT 81: POSSIBLY BAD MAP REPOSITONING\n");
			BKwrap2   = &megaData2->mainmap[0][0];
			BKwrap2   += eventRec[eventLoc-1].eventdat / 2;
			BKwrap2to = &megaData2->mainmap[0][0];
			BKwrap2to += eventRec[eventLoc-1].eventdat2 / 2;
			break;

		case 82: /*Give SPECIAL WEAPON*/
			pItems[11-1] = eventRec[eventLoc-1].eventdat;
			shotMultiPos[9-1] = 0;
			shotRepeat[9-1] = 0;
			shotMultiPos[11-1] = 0;
			shotRepeat[11-1] = 0;
			break;
		default:
			printf("-!- EVENT %d: UNHANDLED EVENT\n", eventRec[eventLoc-1].eventtype);
			break;
	}

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
	JE_longint tempL;
	JE_byte x;
	JE_word itemNum;

	tempL = score;
	itemNum = pItems[pItemButtonMap[curSel[1] - 2] - 1];

	tempL -= JE_getCost(curSel[1], itemNum);

	tempW = 0;

	switch (curSel[1])
	{
		case 3:
		case 4:
			tempW2 = weaponPort[itemNum].cost;
			for (x = 1; x < portPower[curSel[1] - 3]; x++)
			{
				tempW += tempW2 * x;
				tempL -= tempW;
			}
			break;
	}

	return tempL;
}

void JE_itemScreen( void )
{
	char *buf;

	/* SYN: Okay, here's the menu numbers. All are reindexed by -1 from the original code.
		0: full game menu
		1: upgrade ship main
		2: full game options
		3: play next level
		4: upgrade ship submenus
		5: keyboard settings
		6: load/save menu
		7: data cube menu
		8: read data cube
		9: 2 player arcade game menu
		10: 1 player arcade game menu
		11: different options menu
		12: joystick settings
	*/

	JE_loadCubes();

	JE_wipeKey();

	tempScreenSeg = VGAScreen;

	memcpy(menuChoices, menuChoicesDefault, sizeof(menuChoices));

	first = true;
	refade = false;

	joystickWaitMax = 10;
	joystickWait = 0;

	JE_playSong(songBuy);

	JE_loadPic(1, false);

	newPal = 0;
	JE_showVGA();

	JE_updateColorsFast(&colors);

	col = 1;
	gameLoaded = false;
	curItemType = 1;
	cursor = 1;
	curItem = 0;

	for (x = 0; x < MAX_MENU; x++)
	{
		curSel[x] = 2;
	}

	curMenu = 0;
	curX = 1;
	curY = 1;
	curWindow = 1;

	/* JE: (* Check for where Pitems and Select match up - if no match then add to the
     itemavail list *) */
	for (x = 0; x < 7; x++)
	{
		temp = pItemsBack2[pItemButtonMap[x] - 1];
		temp2 = 0;

		for (y = 0; y < itemAvailMax[itemAvailMap[x]-1]; y++)
		{
			if (itemAvail[itemAvailMap[x]-1][y] == temp)
			{
				temp2 = 1;
			}
		}

		if (temp2 == 0)
		{
			itemAvailMax[itemAvailMap[x]-1]++;
			itemAvail[itemAvailMap[x]-1][itemAvailMax[itemAvailMap[x]-1]-1] = temp;
		}
	}

	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

	keyboardUsed = false;
	firstMenu9 = false;
	backFromHelp = false;

item_screen_start:

	do
	{
		JE_getShipInfo();

		quit = false;


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

		paletteChanged = false;

		leftPower = false;
		rightPower = false;

		/* JE: Sort items in merchant inventory */
		for (x = 0; x < 9; x++)
		{
			if (itemAvailMax[x] > 1)
			{
				for (temp = 0; temp < itemAvailMax[x] - 1; temp++)
				{
					for (temp2 = temp; temp2 < itemAvailMax[x]; temp2++)
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

		/* SYN: note reindexing... "firstMenu9" refers to Menu 8 here :( */
		if (curMenu != 8 || firstMenu9)
		{
			memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
		}

		defaultBrightness = -3;

		/* JE: --- STEP I - Draw the menu --- */

		/* play next level menu */
		if (curMenu == 3)
		{
			planetAni = 0;
			keyboardUsed = false;
			currentDotNum = 0;
			currentDotWait = 8;
			planetAniWait = 3;
			JE_updateNavScreen();
		}

		/* Draw menu title for everything but upgrade ship submenus */
		if (curMenu != 4)
		{
			JE_drawMenuHeader();
		}

		/* Draw menu choices for simple menus */
		if ((curMenu >= 0 && curMenu <= 3) || (curMenu >= 9 && curMenu <= 13))
		{
			JE_drawMenuChoices();
		}

		if (skipMove > 0)
		{
			skipMove--;
		}

		/* Data cube icons */
		if (curMenu == 0)
		{
			for (x = 1; x <= cubeMax; x++)
			{
				JE_newDrawCShapeDarkenNum(OPTION_SHAPES, 35, 190 + x*18 + 2, 37+1);
				JE_newDrawCShapeNum(OPTION_SHAPES, 35, 190 + x*18, 37);
			}
		}

		/* Joystick settings menu */
		if (curMenu == 12)
		{
			for (temp = 1; temp <= 4; temp++)
			{
				JE_textShade(214, 34 + temp*24 - 8, joyButtonNames[joyButtonAssign[temp-1]-1], 15, 2, DARKEN);
			}
		}

		/* load/save menu */
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
				/* Highlight if current selection */
				if (x - min + 2 == curSel[curMenu])
				{
					temp2 = 15;
				} else {
					temp2 = 28;
				}

				/* Write save game slot */
				if (x == max)
				{
					strcpy(tempStr, miscText[6-1]);
				} else {
					if (saveFiles[x-2].level == 0)
					{
						strcpy(tempStr, miscText[3-1]);
					} else {
						strcpy(tempStr, saveFiles[x-2].name);
					}
				}

				tempY = 38 + (x - min)*11;

				JE_textShade(163, tempY, tempStr, temp2 / 16, temp2 % 16 - 8, DARKEN);

				/* If selected with keyboard, move mouse pointer to match? Or something. */
				if (x - min + 2 == curSel[curMenu])
				{
					if (keyboardUsed)
					{
						JE_setMousePosition(305, 38 + (x - min) * 11);
					}
				}

				if (x < max) /* x == max isn't a save slot */
				{
					/* Highlight if current selection */
					if (x - min + 2 == curSel[curMenu])
					{
						temp2 = 252;
					} else {
						temp2 = 250;
					}

					if (saveFiles[x-2].level == 0)
					{
						strcpy(tempStr, "-----"); /* Empty save slot */
					} else {
						char buf[20];

						strcpy(tempStr, saveFiles[x-2].levelName);

						snprintf(buf, sizeof buf, "%s%d", miscTextB[1-1], saveFiles[x-2].episode);
						JE_textShade(297, tempY, buf, temp2 / 16, temp2 % 16 - 8, DARKEN);
					}

					JE_textShade(245, tempY, tempStr, temp2 / 16, temp2 % 16 - 8, DARKEN);
				}

				JE_drawMenuHeader();
			}
		}

		/* keyboard settings menu */
		if (curMenu == 5)
		{
			for (x = 2; x <= 11; x++)
			{
				if (x == curSel[curMenu])
				{
					temp2 = 15;
					if (keyboardUsed)
					{
						JE_setMousePosition(305, 38 + (x - 2)*12);
					}
				} else {
					temp2 = 28;
				}

				JE_textShade(166, 38 + (x - 2)*12, menuInt[curMenu][x-1], temp2 / 16, temp2 % 16 - 8, DARKEN);

				if (x < 10) /* 10 = reset to defaults, 11 = done */
				{
					if (x == curSel[curMenu])
					{
						temp2 = 252;
					} else {
						temp2 = 250;
					}
					/*JE_textShade(236, 38 + (x - 2)*12, keyNames[keySettings[x-2]], temp2 / 16, temp2 % 16 - 8, DARKEN);*/
					JE_textShade(236, 38 + (x - 2)*12, SDL_GetKeyName(keySettings[x-2]), temp2 / 16, temp2 % 16 - 8, DARKEN);
				}
			}

			menuChoices[5] = 11;
		}

		/* Upgrade weapon submenus, with weapon sim */
		if (curMenu == 4)
		{
			/* Move cursor until we hit either "Done" or a weapon the player can afford */
			while (curSel[4] < menuChoices[4] && JE_getCost(curSel[1], itemAvail[itemAvailMap[curSel[1]-2]-1][curSel[4]-2]) > score)
			{
				curSel[4] += lastDirection;
				if (curSel[4] < 2)
				{
					curSel[4] = menuChoices[4];
				}
				if (curSel[4] > menuChoices[4])
				{
					curSel[4] = 2;
				}
			}

			if (curSel[4] == menuChoices[4])
			{
				/* If cursor on "Done", use previous weapon */
				pItems[pItemButtonMap[curSel[1]-2]-1] = pItemsBack[pItemButtonMap[curSel[1]-2]-1];
			} else {
				/* Otherwise display the selected weapon */
				pItems[pItemButtonMap[curSel[1]-2]-1] = itemAvail[itemAvailMap[curSel[1]-2]-1][curSel[4]-2];
			}

			/* Get power level info for front and rear weapons */
			if ((curSel[1] == 3 && curSel[4] < menuChoices[4]) || (curSel[1] == 4 && curSel[4] < menuChoices[4]-1))
			{
				if (curSel[1] == 3)
				{
					temp = portPower[0]; /* Front weapon */
				} else {
					temp = portPower[1]; /* Rear weapon */
				}

				/* JE: Only needed if change */
				tempW3 = JE_getCost(curSel[1], itemAvail[itemAvailMap[curSel[1]-2]-1][curSel[5]-2]);

				leftPower  = portPower[curSel[1] - 3] > 1; /* Can downgrade */
				rightPower = portPower[curSel[1] - 3] < 11; /* Can upgrade */

				if (rightPower)
				{
					rightPowerAfford = JE_cashLeft() >= upgradeCost; /* Can player afford an upgrade? */
				}
			} else {
				/* Nothing else can be upgraded / downgraded */
				leftPower = false;
				rightPower = false;
			}

			/* Write title for which submenu this is, e.g., "Left Sidekick" */
			JE_dString(74 + JE_fontCenter(menuInt[1][curSel[1]-1], FONT_SHAPES), 10, menuInt[1][curSel[1]-1], FONT_SHAPES);

			temp2 = pItems[pItemButtonMap[curSel[1]-2]-1]; /* get index into pItems for current submenu  */

			/* Iterate through all submenu options */
			for (tempW = 1; tempW < menuChoices[curMenu]; tempW++)
			{
				tempY = 40 + (tempW-1) * 26; /* Calculate y position */

				/* Is this a item or None/DONE? */
				if (tempW < menuChoices[4] - 1)
				{
					/* Get base cost for choice */
					tempW3 = JE_getCost(curSel[1], itemAvail[itemAvailMap[curSel[1]-2]-1][tempW-1]);
				} else {
					/* "None" is free :) */
					tempW3 = 0;
				}

				if (tempW3 > score) /* Can player afford current weapon at all */
				{
					temp4 = 4;
				} else {
					temp4 = 0;
				}

				temp = itemAvail[itemAvailMap[curSel[1]-2]-1][tempW-1]; /* Item ID */
				switch (curSel[1]-1)
				{
					case 1: /* ship */
						if (temp > 90)
						{
							snprintf(tempStr, sizeof tempStr, "Custom Ship %d", temp - 90);
						} else {
							strcpy(tempStr, ships[temp].name);
						}
						break;
					case 2: /* front and rear weapon */
					case 3:
						strcpy(tempStr, weaponPort[temp].name);
						break;
					case 4: /* shields */
						strcpy(tempStr, shields[temp].name);
						break;
					case 5: /* generator */
						strcpy(tempStr, powerSys[temp].name);
						break;
					case 6: /* sidekicks */
					case 7:
						strcpy(tempStr, options[temp].name);
						break;
				}
				if (tempW == curSel[curMenu]-1)
				{
					if (keyboardUsed)
					{
						JE_setMousePosition(305, tempY+10);
					}
					temp2 = 15;
				} else {
					temp2 = 28;
				}

				JE_getShipInfo();

				/* Owned item bar */
				if (temp == pItemsBack[pItemButtonMap[curSel[1]-2]-1] && temp != 0 && tempW != menuChoices[curMenu]-1)
				{
					JE_bar(160, tempY+7, 300, tempY+11, 227);
					JE_drawShape2(298, tempY+2, 247, shapes9);
				}

				/* Draw DONE */
				if (tempW == menuChoices[curMenu]-1)
				{
					strcpy(tempStr, miscText[13]);
				}
				JE_textShade(185, tempY, tempStr, temp2 / 16, temp2 % 16 -8-temp4, DARKEN);

				/* Draw icon if not DONE. NOTE: None is a normal item with a blank icon. */
				if (tempW < menuChoices[curMenu]-1)
				{
					JE_drawItem(curSel[1]-1, temp, 160, tempY-4);
				}

				/* Make selected text brigther */
				if (tempW == curSel[curMenu]-1)
				{
					temp2 = 15;
				} else {
					temp2 = 28;
				}

				/* Draw Cost: if it's not the DONE option */
				if (tempW != menuChoices[curMenu]-1)
				{
					char buf[20];

					snprintf(buf, sizeof buf, "Cost: %d", tempW3);
					JE_textShade(187, tempY+10, buf, temp2 / 16, temp2 % 16 -8-temp4, DARKEN);
				}
			}
		} /* /weapon upgrade */

		/* Not sure what this is doing */
		if (curMenu == 4 && (curSel[1] >= 2 && curSel[1] <= 7 && curSel[1] != 4))
		{
			joystickWaitMax = 20;
		} else {
			joystickWaitMax = 10;
		}

		/* Draw current money and shield/armor bars, when appropriate */
		/* YKS: Ouch */
		if ((curMenu == 0 || curMenu == 1 || curMenu == 6) || ((curMenu == 10 || curMenu == 11) && onePlayerAction) || ((curMenu == 2 || curMenu == 5) && !twoPlayerMode) || (curMenu == 4 && (curSel[1] >= 1 && curSel[1] <= 6)))
		{
			if (curMenu != 4)
			{
				char buf[20];

				snprintf(buf, sizeof buf, "%d", score);
				JE_textShade(65, 173, buf, 1, 6, DARKEN);
			}
			JE_barDrawShadow(42, 152, 3, 14, armorLevel, 2, 13);
			JE_barDrawShadow(104, 152, 2, 14, shields[pItems[10-1]].mpwr * 2, 2, 13);
		}

		/* Draw crap on the left side of the screen, i.e. two player scores, ship graphic, etc. */
		if (((curMenu >= 0 && curMenu <= 2) || curMenu == 5 || curMenu == 6 || (curMenu >= 9 && curMenu <= 11) || curMenu == 13) || (curMenu == 4 && (curSel[1] == 2 || curSel[1] == 5)))
		{
			if (twoPlayerMode)
			{
				char buf[50];

				snprintf(buf, sizeof buf, "%s %d", miscText[40], score);
				JE_textShade(25, 50, buf, 15, 0, FULL_SHADE);

				snprintf(buf, sizeof buf, "%s %d", miscText[41], score2);
				JE_textShade(25, 60, buf, 15, 0, FULL_SHADE);
			} else if (superArcadeMode > 0 || superTyrian) {
				helpBoxColor = 15;
				helpBoxBrightness = 4;
				if (!superTyrian)
				{
					JE_helpBox(35, 25, superShips[superArcadeMode], 18);
				} else {
					JE_helpBox(35, 25, superShips[SA+3], 18);
				}
				helpBoxBrightness = 1;

				JE_textShade(25, 50, superShips[SA+1], 15, 0, FULL_SHADE);
				JE_helpBox(25, 60, weaponPort[pItems[1 - 1]].name, 22);
				JE_textShade(25, 120, superShips[SA+2], 15, 0, FULL_SHADE);
				JE_helpBox(25, 130, special[pItems[11 - 1]].name, 22);
			} else {
				if (pItems[12-1] > 90)
				{
					temp = 32;
				} else if (pItems[12-1] > 0) {
					temp = ships[pItems[12-1]].bigshipgraphic;
				} else {
					temp = ships[pItemsBack[12-1]].bigshipgraphic;
				}

				switch (temp)
				{
					case 32:
						tempW = 35;
						tempW2 = 33;
						break;
					case 28:
						tempW = 31;
						tempW2 = 36;
						break;
					case 33:
						tempW = 31;
						tempW2 = 35;
						break;
				}

				JE_newDrawCShapeNum(OPTION_SHAPES, temp, tempW, tempW2);

				temp = pItems[5];

				if (temp > 1)
				{
					temp--;
				}

				JE_newDrawCShapeNum(WEAPON_SHAPES, temp + 16, generatorX[temp-1]+1, generatorY[temp-1]+1);

				if (pItems[0] > 0)
				{
					temp = tyrian2_weapons[pItems[0] - 1];
					temp2 = frontWeaponList[pItems[0] - 1] - 1;
					JE_newDrawCShapeNum(WEAPON_SHAPES, temp, frontWeaponX[temp2]+1, frontWeaponY[temp2]);
				}
				if (pItems[1] > 0)
				{
					temp = tyrian2_weapons[pItems[1] - 1];
					temp2 = rearWeaponList[pItems[1] - 1] - 1;
					JE_newDrawCShapeNum(WEAPON_SHAPES, temp, rearWeaponX[temp2], rearWeaponY[temp2]);
				}

				JE_drawItem(6, pItems[3], 3, 84);
				JE_drawItem(7, pItems[4], 129, 84);
				JE_newDrawCShapeAdjustNum(OPTION_SHAPES, 27, 28, 23, 15, shields[pItems[10-1]].mpwr - 10);
			}
		}

		/* Changing the volume? */
		if ((curMenu == 2) || (curMenu == 11))
		{
			JE_barDrawShadow(225, 70, 1, 16, tyrMusicVolume / 12, 3, 13);
			JE_barDrawShadow(225, 86, 1, 16, fxVolume / 12, 3, 13);
		}

		/* 7 is data cubes menu, 8 is reading a data cube, "firstmenu9" refers to menu 8 because of reindexing */
		if ( (curMenu == 7) || ( (curMenu == 8) && firstMenu9) )
		{
			firstMenu9 = false;
			menuChoices[7] = cubeMax + 2;
			JE_bar(1, 1, 145, 170, 0);

			JE_newDrawCShapeNum(OPTION_SHAPES, 21, 1, 1); /* Portrait area background */

			if (curMenu == 7)
			{
				if (cubeMax == 0)
				{
					JE_helpBox(166, 80, miscText[16 - 1], 30);
					tempW = 160;
					temp2 = 252;
				} else {
					for (x = 1; x <= cubeMax; x++)
					{
						JE_drawCube(166, 38 + (x - 1) * 28, 13, 0);
						if (x + 1 == curSel[curMenu])
						{
							if (keyboardUsed)
							{
								JE_setMousePosition(305, 38 + (x - 1) * 28 + 6);
							}
							temp2 = 252;
						} else {
							temp2 = 250;
						}

						helpBoxColor = temp2 / 16;
						helpBoxBrightness = (temp2 % 16) - 8;
						helpBoxShadeType = DARKEN;
						JE_helpBox(192, 44 + (x - 1) * 28, cubeHdr[x - 1], 24);
					}
					x = cubeMax + 1;
					if (x + 1 == curSel[curMenu])
					{
						if (keyboardUsed)
						{
							JE_setMousePosition(305, 38 + (x - 1) * 28 + 6);
						}
						temp2 = 252;
					} else {
						temp2 = 250;
					}
					tempW = 44 + (x - 1) * 28;
				}
			}

			JE_textShade(172, tempW, miscText[6 - 1], temp2 / 16, (temp2 % 16) - 8, DARKEN);

			currentFaceNum = 0;
			if (curSel[7] < menuChoices[7])
			{
				/* SYN: Be careful reindexing some things here, because faceNum 0 is blank, but
				   faceNum 1 is at index 0 in several places! */
				currentFaceNum = faceNum[curSel[7] - 2];

				if (lastSelect != curSel[7] && currentFaceNum > 0)
				{
					faceX = 77 - (shapeX[FACE_SHAPES][currentFaceNum - 1] >> 1);
					faceY = 92 - (shapeY[FACE_SHAPES][currentFaceNum - 1] >> 1);

					paletteChanged = true;
					temp2 = facepal[currentFaceNum - 1];
					newPal = 0;

					for (temp = 1; temp <= 255 - (3 * 16); temp++)
					{
						colors[temp].r = palettes[temp2][temp].r;
						colors[temp].g = palettes[temp2][temp].g;
						colors[temp].b = palettes[temp2][temp].b;
					}
				}
			}

			if (currentFaceNum > 0)
			{
				JE_newDrawCShapeNum(FACE_SHAPES, currentFaceNum, faceX, faceY);
			}

			lastSelect = curSel[7];
		}

		/* 2 player input devices */
		if (curMenu == 9)
		{
			JE_dString(186, 38 + 2 * 16, JE_bright(curSel[3] == x) + inputDevices[inputDevice1-1], SMALL_FONT_SHAPES);
			JE_dString(186, 38 + 4 * 16, JE_bright(curSel[4] == x) + inputDevices[inputDevice2-1], SMALL_FONT_SHAPES);
		}


		/* JE: { - Step VI - Help text for current cursor location } */

		flash = false;
		flashDelay = 5;

		/* scanCode = 0; */
		/* k = 0; */

		/* JE: {Reset player weapons} */
		memset(shotMultiPos, 0, sizeof(shotMultiPos));

		JE_drawScore();

		JE_drawMainMenuHelpText();

		if (newPal > 0) /* can't reindex this :( */
		{
			curPal = newPal;
			memcpy(colors, palettes[newPal - 1], sizeof(colors));
			JE_zPal(newPal);
			newPal = 0;
		}

		/* datacube title */
		if ( ( (curMenu == 7) || (curMenu == 8) ) && (curSel[7] < menuChoices[7]) )
		{
			JE_textShade (75 - JE_textWidth(cubeHdr2[curSel[7] - 2], TINY_FONT) / 2, 173,
				cubeHdr2[curSel[7] - 2], 14, 3, DARKEN);
		}

		/* SYN: Everything above was just drawing the screen. In the rest of it, we process
		   any user input (and do a few other things) */

		/* SYN: Let's start by getting fresh events from SDL */
		service_SDL_events(true);

		if (constantPlay)
		{
			mainLevel = mapSection[mapPNum-1];
			jumpSection = true;
		} else {

			do
			{
			/* Inner loop -- this handles animations on menus that need them and handles
			   some keyboard events. Events it can't handle end the loop and fall through
			   to the main keyboard handler below.

			   Also, I think all timing is handled in here. Somehow. */

				if (isNetworkGame)
				{
					JE_setNetByte(0);
					JE_updateStream();
					if (netQuit)
					{
						JE_tyrianHalt(5);
					}
				}

				mouseCursor = 0;

				col += colC;
				if (col < -2 || col > 6)
				{
					colC = (-1 * colC);
				}

				if (curMenu == 8)
				{
					/* TODO: The text wrapping on the datacubes is incorrect. Not a big deal really, but should
					   probably be fixed at some point. */

					if (mouseX > 164 && mouseX < 299 && mouseY > 47 && mouseY < 153)
					{
						if (mouseY > 100)
						{
							mouseCursor = 2;
						} else {
							mouseCursor = 1;
						}
					}

					JE_bar(160, 49, 310, 158, 228);
					if (yLoc + yChg < 0)
					{
						yChg = 0;
						yLoc = 0;
					}

					yLoc += yChg;
					temp = yLoc / 12;
					temp2 = yLoc % 12;
					tempW = 38 + 12 - temp2;
					temp3 = cubeMaxY[curSel[7] - 2];

					for (x = temp + 1; x <= temp + 10; x++)
					{
						/* if (x <= temp3 && x >= 0) */
						if (x <= temp3)
						{
							JE_outTextAndDarken(161, tempW, cubeText[curSel[7] - 2][x-1], 14, 3, TINY_FONT);
							tempW += 12;
						}
					}

					JE_bar(160, 39, 310, 48, 228);
					JE_bar(160, 157, 310, 166, 228);

					buf = malloc(strlen(miscText[12 - 1]) + 8);
					sprintf(buf, "%s %d%%", miscText[12 - 1], (yLoc * 100) / ((cubeMaxY[currentCube] - 9) * 12));
					JE_outTextAndDarken(176, 160, buf, 14, 1, TINY_FONT);

					JE_dString(260, 160, miscText[13 - 1], SMALL_FONT_SHAPES);

					if (temp2 == 0)
					{
						yChg = 0;
					}

					JE_mouseStart();
					/*if (frameCount == 1)
					{
						/ *JE_waitRetrace();* /
						service_SDL_events(false);
						inputDetected = newkey | mousedown | JE_joystickNotHeld();
					}*/

					JE_showVGA();

					if (backFromHelp)
					{
						JE_fadeColor(10);
						backFromHelp = false;
					}
					JE_mouseReplace();

					setjasondelay(1);
					wait_delay();
				} else {
					/* current menu is not 8 (read data cube) */

					if (curMenu == 3)
					{
						JE_updateNavScreen();
						JE_drawMainMenuHelpText();
						JE_drawMenuHeader();
						JE_drawMenuChoices();
						if (extraGame)
						{
							JE_dString(170, 140, miscText[68 - 1], FONT_SHAPES);
						}
					}

					if (curMenu == 12 && curSel[curMenu] == 6 && joystick_installed)
					{
						JE_drawJoystick();
					}

					if (curMenu == 7 && curSel[7] < menuChoices[7])
					{
						/* Draw flashy cube */
						JE_newDrawCShapeModifyNum(OPTION_SHAPES, 26, 166, 38 + (curSel[7] - 2) * 28, 13, col);
					}

					/* IF (curmenu = 5) AND (cursel [2] IN [3, 4, 6, 7, 8]) */
					if (curMenu == 4 && ( curSel[1] == 3 || curSel[1] == 4 || ( curSel[1] >= 6 && curSel[1] <= 8) ) )
					{
						setjasondelay(3);
						JE_weaponSimUpdate();
						JE_drawScore();
						service_SDL_events(false);

						if (newPal > 0)
						{
							curPal = newPal;
							JE_zPal(newPal);
							newPal = 0;
						}

						JE_mouseStart();

						if (paletteChanged)
						{
							JE_updateColorsFast(&colors);
							paletteChanged = false;
						}

						JE_showVGA(); /* SYN: This is where it updates the screen for the weapon sim */

						if (backFromHelp)
						{
							JE_fadeColor(10);
							backFromHelp = false;
						}

						JE_mouseReplace();
						
						int delaycount_temp;
						if ((delaycount_temp = target - SDL_GetTicks()) > 0)
							SDL_Delay(delaycount_temp);
					} else { /* current menu is anything but weapon sim or datacube */

						if (curMenu == 8) /* SYN: menu 8 is a datacube... this should never happen?! */
						{
							setjasondelay(0);
						} else {
							setjasondelay(2);
						}

						JE_drawScore();
						JE_waitRetrace();

						if (newPal > 0)
						{
							curPal = newPal;
							JE_zPal(newPal);
							newPal = 0;
						}

						JE_mouseStart();

						if (paletteChanged)
						{
							JE_updateColorsFast(&colors);
							paletteChanged = false;
						}

						JE_showVGA(); /* SYN: This is the where the screen updates for most menus */

						JE_mouseReplace();

						if (backFromHelp)
						{
							JE_fadeColor(10);
							backFromHelp = false;
						}
						
						int delaycount_temp;
						if ((delaycount_temp = target - SDL_GetTicks()) > 0)
							SDL_Delay(delaycount_temp);
					}
				}
				
				 do {
					service_SDL_events(false);
					mouseButton = JE_mousePosition(&mouseX, &mouseY); 
					inputDetected = ((mouseButton > 0) || newkey);
					
					if (JE_joystickTranslate())
					{
							if (lastkey_sym == SDLK_RETURN || lastkey_sym == SDLK_ESCAPE)
							{
								if (buttonHeld)
								{
									lastkey_sym = 0;
									inputDetected = false;
								} else {
									buttonHeld = true;
									inputDetected = true;
								}
							} else {
								buttonHeld = false;
								inputDetected = true;
							}
					} else {
						buttonHeld = false;
					}
				} while (!inputDetected && delaycount() != 0);
				
				if (curMenu != 6)
				{
					if (keysactive[SDLK_s] && (keysactive[SDLK_LALT] || keysactive[SDLK_RALT]) )
					{
						if (curMenu == 8 || curMenu == 7)
						{
							curMenu = 0;
						}
						quikSave = true;
						oldMenu = curMenu;
						curMenu = 6;
						performSave = true;
						newPal = 1;
						oldPal = curPal;
					}
					if (keysactive[SDLK_l] && (keysactive[SDLK_LALT] || keysactive[SDLK_RALT]) )
					{
						if (curMenu == 8 || curMenu == 7)
						{
							curMenu = 0;
						}
						quikSave = true;
						oldMenu = curMenu;
						curMenu = 6;
						performSave = false;
						newPal = 1;
						oldPal = curPal;
					}
				}

				/* TODO: need to handle joystick events in here? */
				if (curMenu == 8)
				{
					if (mouseButton > 0 && mouseCursor >= 1)
					{
						inputDetected = false;
						if (mouseCursor == 1)
						{
							yChg = -1;
						} else {
							yChg = 1;
						}
					}

					if (keysactive[SDLK_PAGEUP])
					{
						yChg = -2;
						inputDetected = false;
					}
					if (keysactive[SDLK_PAGEDOWN])
					{
						yChg = 2;
						inputDetected = false;
					}

					if (keysactive[SDLK_UP] || joystickUp)
					{
						yChg = -1;
						inputDetected = false;
					}

					if (keysactive[SDLK_DOWN] || joystickDown)
					{
						yChg = 1;
						inputDetected = false;
					}

					if (yChg < 0 && yLoc == 0)
					{
						yChg = 0;
					}
					if (yChg  > 0 && (yLoc / 12) > cubeMaxY[currentCube] - 10)
					{
						yChg = 0;
					}
				}

			} while (!inputDetected);
		}

		keyboardUsed = false;

		/* The rest of this just grabs input events, handles them, then proceeds on. */

		if (mouseButton > 0)
		{
			lastDirection = 1;

			mouseButton = JE_mousePosition(&mouseX, &mouseY);

			if (curMenu == 7 && cubeMax == 0)
			{
				curMenu = 0;
				JE_playSampleNum(ESC);
				newPal = 1;
				JE_wipeKey();
			}

			if (curMenu == 8)
			{
				if ((mouseX > 258) && (mouseX < 290) && (mouseY > 159) && (mouseY < 171))
				{
					curMenu = 7;
					JE_playSampleNum(ESC);
				}
			}

			tempB = true;

			if (curMenu == 2 || curMenu == 11)
			{
				if ((mouseX >= 221) && (mouseX <= 303) && (mouseY >= 70) && (mouseY <= 82))
				{
					if (!musicActive)
					{
						musicActive = true;
						temp = currentSong;
						currentSong = 0;
						JE_playSong(temp);
					}

					curSel[2] = 4;
					temp = (mouseX - 221) / 4 * 12;

					if (abs(tyrMusicVolume - temp) < 12)
					{
						tyrMusicVolume = temp;
					} else {
						if (tyrMusicVolume < temp)
						{
							tyrMusicVolume += 12;
						} else {
							tyrMusicVolume -= 12;
						}
					}
					tempB = false;
				}

				if ((mouseX >= 221) && (mouseX <= 303) && (mouseY >= 86) && (mouseY <= 98))
				{
					soundActive = true;
					curSel[2] = 5;
					temp = (mouseX - 221) / 4 * 12;
					if (abs(fxVolume - temp) < 12)
					{
						fxVolume = temp;
					} else {
						if (fxVolume < temp)
						{
							fxVolume += 12;
						} else {
							fxVolume -= 12;
						}
					}
				}

				if (fxVolume > 254)
				{
					fxVolume = 254;
				}

				JE_calcFXVol();
				JE_setVol(tyrMusicVolume, fxVolume); /* NOTE: MXD killed this because it was broken */
				JE_playSampleNum(CURSOR_MOVE);
			}

			if (tempB && (mouseY > 20) && (mouseX > 170) && (mouseX < 308) && (curMenu != 8))
			{
				tempI = (mouseY - 38) / mouseSelectionY[curMenu]+2;

				if (curMenu == 9)
				{
					if (tempI > 5)
					{
						tempI--;
					}
					if (tempI > 3)
					{
						tempI--;
					}
				}

				if (curMenu == 0)
				{
					if (tempI > 7)
					{
						tempI = 7;
					}
				}

				if (curMenu == 3)
				{
					if (tempI == menuChoices[curMenu]+1)
					{
						tempI = menuChoices[curMenu];
					}
				}

				if (tempI <= menuChoices[curMenu])
				{
					if ((curMenu == 4) && (tempI == menuChoices[4]))
					{
						score = JE_cashLeft();
						curMenu = 1;
						JE_playSampleNum(ITEM);
					} else {
						JE_playSampleNum(CLICK);
						if (curSel[curMenu] == tempI)
						{
							JE_menuFunction(curSel[curMenu]);
						} else {
							if ((curMenu == 4) && ((curSel[1] == 3) || (curSel[1] == 4)))
							{
								tempPowerLevel[curSel[4]-2] = portPower[curSel[1]-3];
							}
							if ((curMenu == 5) && (JE_getCost(curSel[1], itemAvail[itemAvailMap[curSel[2]-1]][tempI-1]) > score))
							{
								JE_playSampleNum(WRONG);
							} else {
								if (curSel[1] == 4)
								{
									portConfig[1] = 1;
								}
								curSel[curMenu] = tempI;
							}

							if ((curMenu == 4) && ((curSel[1] == 3) || (curSel[1] == 4)))
							{
								portPower[curSel[1]-3] = tempPowerLevel[curSel[4]-2];
							}
						}
					}
				}

				/*do {
					mouseButton = JE_mousePosition(&tempX, &tempY);
				} while (!(mouseButton == 0));*/
				wait_noinput(false, true, false);
			}

			if ((curMenu == 4) && ((curSel[1] == 3) || (curSel[1] == 4)))
			{
				if ((mouseX >= 23) && (mouseX <= 36) && (mouseY >= 149) && (mouseY <= 168))
				{
					JE_playSampleNum(CURSOR_MOVE);
					switch (curSel[1])
					{
						case 3:
							if (leftPower)
							{
								portPower[0]--;
							} else {
								JE_playSampleNum(WRONG);
							}
							break;
						case 4:
							if (leftPower)
							{
								portPower[1]--;
							} else {
								JE_playSampleNum(WRONG);
							}
							break;
					}
					wait_noinput(false, true, false);
				}

				if ((mouseX >= 119) && (mouseX <= 131) && (mouseY >= 149) && (mouseY <= 168))
				{
					JE_playSampleNum(CURSOR_MOVE);
					switch (curSel[1])
					{
						case 3:
							if (rightPower && rightPowerAfford)
							{
								portPower[0]++;
							} else {
								JE_playSampleNum(WRONG);
							}
							break;
						case 4:
							if (rightPower && rightPowerAfford)
							{
								portPower[1]++;
							} else {
								JE_playSampleNum(WRONG);
							}
							break;
					}
					wait_noinput(false, true, false);
				}
			}
		} else {
			if (newkey)
			{
				switch (lastkey_sym)
				{
				case SDLK_SPACE:
					if ( (curMenu == 4) && (curSel[1] == 4))
					{
						portConfig[1]++;
						if (portConfig[1] > weaponPort[pItems[1-1]].opnum)
						{
							portConfig[1] = 1;
						}
					}
					/* SYN: Intentional fall-through! */

				case SDLK_RETURN:
					JE_menuFunction(curSel[curMenu]);
					keyboardUsed = true;
					break;

				case SDLK_ESCAPE:
					JE_playSampleNum(ESC);
					if ( (curMenu == 6) && quikSave)
					{
						curMenu = oldMenu;
						newPal = oldPal;
					}
					else if (menuEsc[curMenu] == 0)
					{
						if (JE_quitRequest(true))
						{
							gameLoaded = true;
							mainLevel = 0;
						}
					} else {
						if (curMenu == 4)
						{
							memcpy(pItems, pItemsBack, sizeof(pItems));
							memcpy(portPower, lastPortPower, sizeof(portPower));
							curSel[4] = lastCurSel;
							score = JE_cashLeft();
						}
						if (curMenu != 8)
						{
							newPal = 1;
						}
						curMenu = menuEsc[curMenu] - 1;

					}
					keyboardUsed = true;
					break;

				case SDLK_F1:
					if (!isNetworkGame)
					{
						JE_helpSystem(2);
						JE_fadeBlack(10);

						JE_playSong(songBuy);

						JE_loadPic(1, false);
						newPal = 1;

						switch (curMenu)
						{
						case 3:
							newPal = 18;
							break;
						case 7:
						case 8:
							lastSelect = 0;
							break;
						}

						memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

						curPal = newPal;
						memcpy(colors, palettes[newPal-1], sizeof(colors));
						JE_showVGA();
						newPal = 0;
						backFromHelp = true;
					}
					break;

				case SDLK_UP:
					keyboardUsed = true;
					lastDirection = -1;
					if (curMenu != 8)
					{
						JE_playSampleNum(CURSOR_MOVE);
					}
					if ( (curMenu == 4) && (curSel[1] == 3 || curSel[1] == 4) )
					{
						tempPowerLevel[curSel[4] - 2] = portPower[curSel[1] - 3];
						if (curSel[1] == 4)
						{
							// If Rear Weapon, reset firing pattern to default
							portConfig[1] = 1;
						}
					}
					curSel[curMenu]--;
					if (curSel[curMenu] < 2)
					{
						curSel[curMenu] = menuChoices[curMenu];
					}
					if ( (curMenu == 4) && (curSel[1] == 3 || curSel[1] == 4) )
					{
						portPower[curSel[1] - 3] = tempPowerLevel[curSel[4] - 2];
					}
					break;

				case SDLK_DOWN:
					keyboardUsed = true;
					lastDirection = 1;
					if (curMenu != 8)
					{
						JE_playSampleNum(CURSOR_MOVE);
					}
					if ( (curMenu == 4) && (curSel[1] == 3 || curSel[1] == 4) )
					{
						tempPowerLevel[curSel[4] - 2] = portPower[curSel[1] - 3];
						if (curSel[1] == 4)
						{
							// If Rear Weapon, reset firing pattern to default
							portConfig[1] = 1;
						}
					}
					curSel[curMenu]++;
					if (curSel[curMenu] > menuChoices[curMenu])
					{
						curSel[curMenu] = 2;
					}
					if ( (curMenu == 4) && (curSel[1] == 3 || curSel[1] == 4) )
					{
						portPower[curSel[1] - 3] = tempPowerLevel[curSel[4] - 2];
					}
					break;

				case SDLK_HOME:
					if (curMenu == 8)
					{
						yLoc = 0;
					}
					break;

				case SDLK_END:
					if (curMenu == 8)
					{
						yLoc = (cubeMaxY[currentCube] - 9) * 12;
					}
					break;

				case SDLK_x: /* alt-X */
					if (lastkey_mod == SDLK_RALT || lastkey_mod == SDLK_LALT)
					{
						JE_tyrianHalt(0);
					}
					break;

				case SDLK_LEFT:
					if (curMenu == 12 && curSel[curMenu] < 6)
					{
						joyButtonAssign[curSel[curMenu] - 2]--;
						if (joyButtonAssign[curSel[curMenu] - 2] < 1)
						{
							joyButtonAssign[curSel[curMenu] - 2] = 5;
						}
					}

					if (curMenu == 9)
					{
						switch (curSel[curMenu])
						{
						case 3:
							JE_playSampleNum(CURSOR_MOVE);
							do {
								inputDevice1--;
								if (inputDevice1 < 1)
								{
									inputDevice1 = 3;
								}
							} while (inputDevice1 == inputDevice2);
							break;
						case 4:
							JE_playSampleNum(CURSOR_MOVE);
							do {
								inputDevice2--;
								if (inputDevice2 < 1)
								{
									inputDevice2 = 3;
								}
							} while (inputDevice1 == inputDevice2);
							break;
						}
					}

					if (curMenu == 2 || curMenu == 4  || curMenu == 11)
					{
						JE_playSampleNum(CURSOR_MOVE);
					}

					switch (curMenu)
					{
					case 2:
					case 11:
						switch (curSel[curMenu])
						{
						case 4:
							JE_changeVolume(&tyrMusicVolume, -16, &fxVolume, 0);
							if (!musicActive)
							{
								musicActive = true;
								temp = currentSong;
								currentSong = 0;
								JE_playSong(temp);
							}
							break;
						case 5:
							JE_changeVolume(&tyrMusicVolume, 0, &fxVolume, -16);
							soundActive = true;
							break;
						}
						break;
					case 4:
						switch (curSel[1])
						{
						case 3:
							if (leftPower)
							{
								portPower[0]--;
							} else {
								JE_playSampleNum(WRONG);
							}
							break;
						case 4:
							if (leftPower)
							{
								portPower[1]--;
							} else {
								JE_playSampleNum(WRONG);
							}
							break;
						}
						break;
					}
					break;

				case SDLK_RIGHT:
					if (curMenu == 12 && curSel[curMenu] < 6)
					{
						joyButtonAssign[curSel[curMenu] - 2]++;
						if (joyButtonAssign[curSel[curMenu] - 2] > 5)
						{
							joyButtonAssign[curSel[curMenu] - 2] = 1;
						}
					}

					if (curMenu == 9)
					{
						switch (curSel[curMenu])
						{
						case 3:
							JE_playSampleNum(CURSOR_MOVE);
							do {
								inputDevice1++;
								if (inputDevice1 > 3)
								{
									inputDevice1 = 1;
								}
							} while (inputDevice1 == inputDevice2);
							break;
						case 4:
							JE_playSampleNum(CURSOR_MOVE);
							do {
								inputDevice2++;
								if (inputDevice2 > 3)
								{
									inputDevice2 = 1;
								}
							} while (inputDevice1 == inputDevice2);
							break;
						}
					}

					if (curMenu == 2 || curMenu == 4  || curMenu == 11)
					{
						JE_playSampleNum(CURSOR_MOVE);
					}

					switch (curMenu)
					{
					case 2:
					case 11:
						switch (curSel[curMenu])
						{
						case 4:
							JE_changeVolume(&tyrMusicVolume, 16, &fxVolume, 0);
							if (!musicActive)
							{
								musicActive = true;
								temp = currentSong;
								currentSong = 0;
								JE_playSong(temp);
							}
							break;
						case 5:
							JE_changeVolume(&tyrMusicVolume, 0, &fxVolume, 16);
							soundActive = true;
							break;
						}
						break;
					case 4:
						switch (curSel[1])
						{
						case 3:
							if (rightPower && rightPowerAfford)
							{
								portPower[0]++;
							} else {
								JE_playSampleNum(WRONG);
							}
							break;
						case 4:
							if (rightPower && rightPowerAfford)
							{
								portPower[1]++;
							} else {
								JE_playSampleNum(WRONG);
							}
							break;
						}
						break;
					}
					break;

				default:
					break;
				}
			}
		}
		/*JE_showVGA();*/
	} while (!(quit || gameLoaded || jumpSection));

	/* TODO */

}

void JE_loadCubes( void )
{
	char s[256] = "", s2[256] = "", s3[256] = "";
	JE_byte cube;
	JE_word x, y;
	JE_byte startPos, endPos, pos;
	JE_boolean nextLine;
	JE_boolean endString;
	FILE *f;
	JE_byte lastWidth, curWidth;
	JE_boolean pastStringLen, pastStringWidth;
	JE_byte temp;

	char buffer[256] = "";

	memset(cubeText, 0, sizeof(cubeText));

	for (cube = 1; cube <= cubeMax; cube++)
	{
		JE_resetFile(&f, cubeFile);

		tempW = cubeList[cube-1];

		do
		{
			do
			{
				JE_readCryptLn(f, s);
			} while (s[0] != '*');
			tempW--;
		} while (tempW != 0);

		faceNum[cube-1] = atoi(strnztcpy(buffer, s + 4, 2));

		JE_readCryptLn(f, cubeHdr[cube-1]);
		JE_readCryptLn(f, cubeHdr2[cube-1]);

		curWidth = 0;
		x = 5;
		y = 1;
		s3[0] = '\0';

		s2[0] = ' ';

		do
		{
			JE_readCryptLn(f, s2);

			// If we didn't finish this datacube yet
			if (s2[0] != '*')
			{
				sprintf(s, "%s %s", s3, s2);

				pos = 1;
				endPos = 0;
				endString = false;

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

							// Is printable character?
							if (temp > ' ' && temp < 169 && fontMap[temp] > 0 && (*shapeArray)[5][fontMap[temp]] != NULL)
							{
								// Increase width by character's size
								curWidth += shapeX[5][fontMap[temp]] + 1;
							} else {
								// No, so, is it a space?
								if (temp == ' ')
								{
									// Add a fixed width for the space
									curWidth += 6;
								}
							}

							pos++;
							if (pos == strlen(s))
							{
								// Reached end of string, must load more
								endString = true;
								if (((pos - startPos) < CUBE_WIDTH) /*check for string length*/ && (curWidth <= LINE_WIDTH))
								{
									endPos = pos + 1;
									lastWidth = curWidth;
								}
							}
						} while (!(s[pos - 1] == ' ' || endString)); // Loops until end of word or string

						// 
						pastStringLen = (pos - startPos) > CUBE_WIDTH;
						pastStringWidth = (curWidth > LINE_WIDTH);

					// Loop until we need more string or we wrap over to the next line
					} while (!(pastStringLen || pastStringWidth || endString));

					if (!endString || pastStringLen || pastStringWidth)
					{
						/*Start new line*/
						strnztcpy(cubeText[cube-1][y-1], s + startPos - 1, endPos - startPos + 1);
						y++;
						strnztcpy(s3, s + endPos, 255);
						curWidth = curWidth - lastWidth;
						if (s[endPos-1] == ' ')
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
						strcpy(cubeText[cube-1][y-1], s3);
					}
					y++;
					s3[0] = '\0';
				}
			}
		} while (s2[0] != '*');

		strcpy(cubeText[cube-1][y-1], s3);
		while (!strcmp(cubeText[cube-1][y-1], ""))
		{
			y--;
		}
		cubeMaxY[cube-1] = y;

		fclose(f);
	}
}

void JE_drawItem( JE_byte itemType, JE_word itemNum, JE_word x, JE_word y )
{
	JE_word tempW = 0;

	if (itemNum > 0)
	{
		switch (itemType)
		{
			case 2:
			case 3:
				tempW = weaponPort[itemNum].itemgraphic;
				break;
			case 5:
				tempW = powerSys[itemNum].itemgraphic;
				break;
			case 6:
			case 7:
				tempW = options[itemNum].itemgraphic;
				break;
			case 4:
				tempW = shields[itemNum].itemgraphic;
				break;
		}

		if (itemType == 1)
		{
			if (itemNum > 90)
			{
				shipGrPtr = shapes9;
				shipGr = JE_SGr(itemNum - 90, &shipGrPtr);
				JE_drawShape2x2(x, y, shipGr, shipGrPtr);
			} else {
				JE_drawShape2x2(x, y, ships[itemNum].shipgraphic, shapes9);
			}
		} else {
			if (tempW > 0)
			{
				JE_drawShape2x2(x, y, tempW, shapes6);
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
			strcpy(tempStr, menuInt[0][1]);
			break;
		case 6:
			strcpy(tempStr, menuInt[2][performSave + 1]);
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

	for (x = 2; x <= menuChoices[curMenu]; x++)
	{
		if (curMenu == 12)
		{
			tempY = 38 + (x-1) * 24 - 8;
		} else {
			tempY = 38 + (x-1) * 16;
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

		str = malloc(strlen(menuInt[curMenu][x-1])+2);
		if (curSel[curMenu] == x)
		{
			str[0] = '~';
			strcpy(str+1, menuInt[curMenu][x-1]);
		} else {
			strcpy(str, menuInt[curMenu][x-1]);
		}
		JE_dString(166, tempY, str, SMALL_FONT_SHAPES);
		free(str);

		if (keyboardUsed && curSel[curMenu] == x)
		{
			JE_setMousePosition(305, tempY + 6);
		}
	}
}

void JE_updateNavScreen( void )
{
	JE_byte x;

	/* minor issues: */
	/* TODO: The scroll to the new planet is too fast, I think */
	/* TODO: The starting coordinates for the scrolling effect may be wrong, the
	   yellowish planet below Tyrian isn't visible for as many frames as in the
	   original. */

	tempNavX = round(navX);
	tempNavY = round(navY);
	JE_bar(19, 16, 135, 169, 2);
	JE_drawNavLines(true);
	JE_drawNavLines(false);
	JE_drawDots();

	for (x = 0; x < 11; x++)
	{
		JE_drawPlanet(x);
	}

	for (x = 0; x < menuChoices[3]-1; x++)
	{
		if (mapPlanet[x] > 11)
		{
			JE_drawPlanet(mapPlanet[x] - 1);
		}
	}

	if (mapOrigin > 11)
	{
		JE_drawPlanet(mapOrigin - 1);
	}

	JE_newDrawCShapeNum(OPTION_SHAPES, 29, 0, 0);

	if (curSel[3] < menuChoices[3])
	{
		newNavX = (planetX[mapOrigin-1] - shapeX[PLANET_SHAPES][PGR[mapOrigin-1]-1] / 2
		          + planetX[mapPlanet[curSel[3]-2] - 1]
		          - shapeX[PLANET_SHAPES][PGR[mapPlanet[curSel[3]-2] - 1]-1] / 2) / 2.0;
		newNavY = (planetY[mapOrigin-1] - shapeY[PLANET_SHAPES][PGR[mapOrigin-1]-1] / 2
		          + planetY[mapPlanet[curSel[3]-2] - 1]
		          - shapeY[PLANET_SHAPES][PGR[mapPlanet[curSel[3]-2] - 1]-1] / 2) / 2.0;
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

/* SYN: This was originally PROC drawlines... yes, there were two different procs called
   drawlines in different scopes in the same file. Dammit, Jason, why do you do this to me? */

void JE_drawNavLines( JE_boolean dark )
{
	JE_byte x, y;
	JE_integer tempX, tempY;
	JE_integer tempX2, tempY2;
	JE_word tempW, tempW2;

	tempX2 = tempNavX >> 1;
	tempY2 = tempNavY >> 1;

	tempW = 0;
	for (x = 1; x <= 20; x++)
	{
		tempW += 15;
		tempX = tempW - tempX2;

		if (tempX > 18 && tempX < 135)
		{
			if (dark)
			{
				JE_rectangle(tempX + 1, 16, tempX + 1, 169, 1);
			} else {
				JE_rectangle(tempX, 16, tempX, 169, 5);
			}
		}
	}

	tempW = 0;
	for (y = 1; y <= 20; y++)
	{
		tempW += 15;
		tempY = tempW - tempY2;

		if (tempY > 15 && tempY < 169)
		{
			if (dark)
			{
				JE_rectangle(19, tempY + 1, 135, tempY + 1, 1);
			} else {
				JE_rectangle(8, tempY, 160, tempY, 5);
			}

			tempW2 = 0;

			for (x = 0; x < 20; x++)
			{
				tempW2 += 15;
				tempX = tempW2 - tempX2;
				if (tempX > 18 && tempX < 135)
				{
					JE_pix3(tempX, tempY, 7);
				}
			}
		}
	}
}

void JE_drawDots( void )
{
	JE_byte x, y;
	JE_integer tempX, tempY;

	for (x = 0; x < mapPNum; x++)
	{
		for (y = 0; y < planetDots[x]; y++)
		{
			tempX = planetDotX[x][y] - tempNavX + 66 - 2;
			tempY = planetDotY[x][y] - tempNavY + 85 - 2;
			if (tempX > 0 && tempX < 140 && tempY > 0 && tempY < 168)
			{
				if (x == curSel[3]-2 && y < currentDotNum)
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

	tempZ = PGR[planetNum]-1;
	tempX = planetX[planetNum] + 66 - tempNavX - shapeX[PLANET_SHAPES][tempZ] / 2;
	tempY = planetY[planetNum] + 85 - tempNavY - shapeY[PLANET_SHAPES][tempZ] / 2;

	if (tempX > -7 && tempX + shapeX[PLANET_SHAPES][tempZ] < 170 && tempY > 0 && tempY < 160)
	{
		if (PAni[planetNum])
		{
			tempZ += planetAni;
		}
		JE_newDrawCShapeDarken((*shapeArray)[PLANET_SHAPES][tempZ], shapeX[PLANET_SHAPES][tempZ], shapeY[PLANET_SHAPES][tempZ], tempX + 3, tempY + 3);
		JE_newDrawCShapeNum(PLANET_SHAPES, tempZ+1, tempX, tempY);
	}
}

void JE_scaleBitmap( SDL_Surface *bitmap, JE_word x, JE_word y, JE_word x1, JE_word y1, JE_word x2, JE_word y2 )
{
	JE_word w = x2 - x1 + 1,
	        h = y2 - y1 + 1;
	JE_longint sx = x * 0x10000 / w,
	           sy = y * 0x10000 / h,
	           cx, cy = 0;
	
	Uint8 *s = VGAScreen->pixels;  /* 8-bit specific */
	Uint8 *src = bitmap->pixels;  /* 8-bit specific */
	
	s += y1 * VGAScreen->w + x1;
	
	for (; h; h--)
	{
		cx = 0;
		for (int x = w; x; x--)
		{
			*s = *src;
			s++;
			
			cx += sx;
			src += cx >> 16;
			cx &= 0xffff;
		}
		
		s += VGAScreen->w - w;
		
		cy += sy;
		src -= ((sx * w) >> 16);
		src += (cy >> 16) * bitmap->w;
		cy &= 0xffff;
	}
}

void JE_initWeaponView( void )
{
	JE_bar(8, 8, 144, 177, 0);

	option1X = 72 - 15;
	option1Y = 120;
	option2X = 72 + 15;
	option2Y = 120;

	PX    = 72;
	PY    = 110;
	PXChange = 0;
	PYChange = 0;
	lastPX = 72;
	lastPY = 110;
	lastPX2 = 72;
	lastPY2 = 110;
	power = 500;
	lastPower = 500;

	memset(shotAvail, sizeof(shotAvail), 0);

	memset(shotRepeat, sizeof(shotRepeat), 1);
	memset(shotMultiPos, sizeof(shotMultiPos), 0);

	JE_setupStars();
}

void JE_computeDots( void )
{
	JE_integer tempX, tempY;
	JE_longint distX, distY;
	JE_byte x, y;

	for (x = 0; x < mapPNum; x++)
	{
		distX = (int)(planetX[mapPlanet[x]-1]) - (int)(planetX[mapOrigin-1]);
		distY = (int)(planetY[mapPlanet[x]-1]) - (int)(planetY[mapOrigin-1]);
		tempX = abs(distX) + abs(distY);

		if (tempX != 0)
		{
			planetDots[x] = round(sqrt(sqrt((distX * distX) + (distY * distY)))) - 1;
		} else {
			planetDots[x] = 0;
		}

		if (planetDots[x] > 10)
		{
			planetDots[x] = 10;
		}

		for (y = 0; y < planetDots[x]; y++)
		{
			tempX = JE_partWay(planetX[mapOrigin-1], planetX[mapPlanet[x]-1], planetDots[x], y);
			tempY = JE_partWay(planetY[mapOrigin-1], planetY[mapPlanet[x]-1], planetDots[x], y);
			/* ??? Why does it use temp? =P */
			planetDotX[x][y] = tempX;
			planetDotY[x][y] = tempY;
		}
	}
}

JE_integer JE_partWay( JE_integer start, JE_integer finish, JE_byte dots, JE_byte dist )
{
	return (finish - start) / (dots + 2) * (dist + 1) + start;
}

void JE_doFunkyScreen( void )
{
	if (pItems[12-1] > 90)
	{
		temp = 32;
	} else if (pItems[12-1] > 0) {
		temp = ships[pItems[12-1]].bigshipgraphic;
	} else {
		temp = ships[pItemsBack[12-1]].bigshipgraphic;
	}

	switch (temp)
	{
		case 32:
			tempW = 35;
			tempW2 = 33;
			break;
		case 28:
			tempW = 31;
			tempW2 = 36;
			break;
		case 33:
			tempW = 31;
			tempW2 = 35;
			break;
	}
	tempW -= 30;
	
	VGAScreen = tempScreenSeg = VGAScreen2;
	JE_clr256();
	
	JE_newDrawCShapeNum(OPTION_SHAPES, temp, tempW, tempW2);
	JE_funkyScreen();
	
	tempScreenSeg = VGAScreenSeg;
	
	JE_loadPic(1, false);
	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);
}

void JE_drawMainMenuHelpText( void )
{
	char tempStr[67];
	JE_byte temp;

	temp = curSel[curMenu] - 2;
	if (curMenu < 3 || curMenu == 9 || curMenu > 10)
	{
		memcpy(tempStr, mainMenuHelp[(menuHelp[curMenu][temp])-1], sizeof(tempStr));
	} else {
		if (curMenu == 5 && curSel[5] == 10)
		{
			memcpy(tempStr, mainMenuHelp[25-1], sizeof(tempStr));
		}
		else if (leftPower || rightPower)
		{
			memcpy(tempStr, mainMenuHelp[24-1], sizeof(tempStr));
		}
		else if ( (temp == menuChoices[curMenu] - 1) || ( (curMenu == 7) && (cubeMax == 0) ) )
		{
			memcpy(tempStr, mainMenuHelp[12-1], sizeof(tempStr));
		}
		else
		{
			memcpy(tempStr, mainMenuHelp[17 + curMenu - 3], sizeof(tempStr));
		}
	}
	JE_textShade(10, 187, tempStr, 14, 1, DARKEN);
}

void JE_whoa( void )
{
	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen->h * VGAScreen->pitch);
	memset(VGAScreen->pixels, 0, VGAScreen->h * VGAScreen->pitch);

	JE_wipeKey();

	tempW3 = 300;

	do
	{
		setjasondelay(1);

		Uint16 di = 640; // pixel pointer

		Uint8 *vga2pixels = VGAScreen2->pixels;
		for (Uint16 dx = 64000 - 1280; dx != 0; dx--)
		{
			Uint16 si = di + (Uint8)((Uint8)(dx >> 8) >> 5) - 4;

			Uint16 ax = vga2pixels[si] * 12;
			ax += vga2pixels[si-320];
			ax += vga2pixels[si-1];
			ax += vga2pixels[si+1];
			ax += vga2pixels[si+320];
			ax >>= 4;

			vga2pixels[di] = ax;

			di++;
		}

		di = 320 * 4;
		for (Uint16 cx = 64000 - 320*7; cx != 0; cx--)
		{
			((Uint8 *)VGAScreen->pixels)[di] = vga2pixels[di];
			di++;
		}

		tempW3--;

		wait_delay();
		JE_showVGA();
	} while (!(tempW3 == 0 || JE_anyButton()));

	levelWarningLines = 4;
}

JE_boolean JE_quitRequest( JE_boolean useMouse )
{
	JE_boolean retval;
	JE_byte sel;
	JE_boolean quit, select;
	JE_integer col, colC;

	if (useMouse)
	{
		JE_mouseReplace();
	}

	JE_showVGA();

	/* JE: (*Set keyboard to BIOS*) */
	/* callBIOSHandler = true; */
	JE_clearKeyboard();

	JE_getVGA();

	quit = false;
	select = false;

	sel = 1;

	// JE_wipeKey();
	wait_noinput(true, true, joystick_installed);

	JE_barShade(65, 55, 255, 155);

	do {
		col = 8;
		colC = 1;

		do {
			if (netQuit)
			{
				JE_tyrianHalt(0);
			}

			setjasondelay(4);

			/*scanCode = 0;
			k = 0;*/

			JE_newDrawCShapeNum(OPTION_SHAPES, 36, 50, 50);
			JE_textShade(70, 60, miscText[29 - 1], 0, 5, FULL_SHADE);
			JE_helpBox(70, 90, miscText[30 + useMouse - 1], 30);

			col += colC;
			if (col > 8 || col < 2)
			{
				colC *= -1;
			}

			tempW = 54 + 45 - (JE_textWidth(miscText[10 - 1], FONT_SHAPES) >> 1);
			tempW2 = 149 + 45 - (JE_textWidth(miscText[11 - 1], FONT_SHAPES) >> 1);

			if (sel == 1)
			{
				tempI = col - 12;
				tempI2 = -5;
			} else {
				tempI = -5;
				tempI2 = col - 12;
			}

			JE_outTextAdjust(tempW, 128, miscText[10-1], 15, tempI, FONT_SHAPES, true);
			JE_outTextAdjust(tempW2, 128, miscText[11-1], 15, tempI2, FONT_SHAPES, true);

			if (useMouse && mouseInstalled)
			{
				JE_mouseStart();
				JE_showVGA();
				JE_mouseReplace();
			} else {
				JE_showVGA();
			}

			temp = useMouse;

		} while (!JE_waitAction(0, false)); /* JE: {Use previous framecount} */


		if (mouseButton > 0 && useMouse)
		{
			if (mouseX > 56 && mouseX < 142 && mouseY > 123 && mouseY < 149)
			{
				quit = true;
				select = true;
			}
			else if (mouseX > 151 && mouseX < 237 && mouseY > 123 && mouseY < 149)
			{
				quit = true;
			}
		} else {
			if (newkey)
			{
				switch (lastkey_sym)
				{
				case SDLK_LEFT:
				case SDLK_RIGHT:
					sel = !sel;
					JE_playSampleNum(CURSOR_MOVE);
					break;
				case SDLK_TAB:
					sel = !sel;
					break;
				case SDLK_RETURN:
				case SDLK_SPACE:
					quit = true;
					select = (sel == 1);
					break;
				case SDLK_ESCAPE:
					quit = true;
					break;
				default:
					break;
				}
			}
		}

	} while (!quit);

	if (netQuit)
	{
		JE_tyrianHalt(0);
	}

	retval = select;
	if (select)
	{
		JE_playSampleNum(ESC);
	} else {
		JE_playSampleNum(CLICK);
	}

	if (isNetworkGame && select)
	{
		JE_tyrianHalt(0);
	}

	return retval;
}

void JE_barX( JE_word x1, JE_word y1, JE_word x2, JE_word y2, JE_byte col )
{
	JE_bar(x1, y1,     x2, y1,     col + 1);
	JE_bar(x1, y1 + 1, x2, y2 - 1, col    );
	JE_bar(x1, y2,     x2, y2,     col - 1);
}

void JE_doStatBar( void )
{
	for (temp2 = 0; temp2 < 2; temp2++)
	{
		if (statBar[temp2] > 0)
		{
			statDmg[temp2] = 256;  /*Higher than maximum*/
			for (temp = 0; temp < 100; temp++)
			{
				if (enemy[temp].linknum == statBar[temp2] && enemyAvail[temp] != 1)
				{
					if (enemy[temp].armorleft < statDmg[temp2])
					{
						statDmg[temp2] = enemy[temp].armorleft;
					}
				}
			}
			if (statDmg[temp2] == 256 || statDmg[temp2] == 0)
			{
				statBar[temp2] = 0;
			}
		}
	}
	
	if (statBar[1-1] == 0)
	{
		statBar[1-1] = statBar[2-1];
		statDmg[1-1] = statDmg[2-1];
		statBar[2-1] = 0;
	}
	
	if (statBar[2-1] > 0)
	{  /*2 bars*/
		JE_barX(100, 7, 150, 12, 115);
		JE_barX(125 - (statDmg[1-1] / 10), 7, 125 + (statDmg[1-1] + 5) / 10, 12, 118 + statCol[1-1]);
		JE_barX(160, 7, 210, 12, 115);
		JE_barX(185 - (statDmg[2-1] / 10), 7, 185 + (statDmg[2-1] + 5) / 10, 12, 118 + statCol[2-1]);
	} else if (statBar[1-1] > 0) {  /*1 bar*/
		tempW = 155;
		if (levelTimer)
		{
			tempW = 250;
		}
		JE_barX(tempW - 26, 7, tempW + 26, 12, 115);
		JE_barX(tempW - (statDmg[1-1] / 10), 7, tempW + (statDmg[1-1] + 5) / 10, 12, 118 + statCol[1-1]);
	}
	
	if (statCol[1-1] > 0)
	{
		statCol[1-1]--;
	}
	if (statCol[2-1] > 0)
	{
		statCol[2-1]--;
	}
}

void JE_drawScore( void )
{
	char cl[24];
	if (curMenu == 4)
	{
		sprintf(cl, "%d", JE_cashLeft());
		JE_textShade(65, 173, cl, 1, 6, DARKEN);
	}
}

void JE_menuFunction( JE_byte select )
{
	JE_byte x;
	JE_longint tempScore;
	JE_word curSelect;

	col = 0;
	colC = -1;
	JE_playSampleNum(CLICK);

	curSelect = curSel[curMenu];

	switch (curMenu)
	{
	case 0:
		switch (select)
		{
		case 2:
			curMenu = 7;
			lastSelect = 0;
			curSel[7] = 2;
			break;
		case 3:
			JE_doFunkyScreen();
			break;
		case 4:
			curMenu = 1;
			break;
		case 5:
			curMenu = 2;
			break;
		case 6:
			curMenu = 3;
			newPal = 18;
			JE_computeDots();
			navX = planetX[mapOrigin - 1];
			navY = planetY[mapOrigin - 1];
			newNavX = navX;
			newNavY = navY;
			menuChoices[3] = mapPNum + 2;
			curSel[3] = 2;
			strcpy(menuInt[3][0], "Next Level");
			for (x = 0; x < mapPNum; x++)
			{
				temp = mapPlanet[x];
				strcpy(menuInt[3][x + 1], pName[temp-1]);
			}
			strcpy(menuInt[3][x+1], miscText[6-1]);
			break;
		case 7:
			if (JE_quitRequest(true))
			{
				gameLoaded = true;
				mainLevel = 0;
			}
			break;
		}
		break;

	case 1:
		if (select == 9)
		{
			curMenu = 0;
		} else {
			lastDirection = 1;
			skipMove = 0;
			memcpy(lastPortPower, portPower, sizeof(lastPortPower));
			memcpy(pItemsBack, pItems, sizeof(pItemsBack));
			tempScore = score;
			JE_genItemMenu(select);
			JE_initWeaponView();
			curMenu = 4;
			lastCurSel = curSel[4];
			score = tempScore - JE_cashLeft() + tempScore;
		}
		break;

	case 2:
		switch (select)
		{
		case 2:
			curMenu = 6;
			performSave = false;
			quikSave = false;
			break;
		case 3:
			curMenu = 6;
			performSave = true;
			quikSave = false;
			break;
		case 6:
			curMenu = 12;
			break;
		case 7:
			curMenu = 5;
			break;
		case 8:
			curMenu = 0;
			break;
		}
		break;

	case 3:
		if (select == menuChoices[3])
		{
			curMenu = 0;
			newPal = 1;
		} else {
			mainLevel = mapSection[curSelect - 2];
			jumpSection = true;
		}
		break;

	case 4:
		if (curSel[4] < menuChoices[4] && robertWeird)
		{
			tempPowerLevel[curSel[4] - 1] = portPower[curSel[1] - 2];
			curSel[4] = menuChoices[4];
		} else {
			if (curSel[4] == menuChoices[4] && !robertWeird)
			{
				memcpy(pItems, pItemsBack, sizeof(pItems));
				memcpy(portPower, lastPortPower, sizeof(portPower));
				curSel[4] = lastCurSel; /* JE: Cancel */
			} else {
				JE_playSampleNum(ITEM);
			}

			score = JE_cashLeft();
			curMenu = 1;
		}
		break;

	case 5: /* keyboard settings */
		if (curSelect == 10) /* reset to defaults */
		{
			memcpy(keySettings, defaultKeySettings, sizeof(keySettings));
		}
		else if (curSelect == 11) /* done */
		{
			if (isNetworkGame || onePlayerAction)
			{
				curMenu = 11;
			} else {
				curMenu = 2;
			}
		}
		else /* change key */
		{
			temp2 = 254;
			tempY = 38 + (curSelect - 2) * 12;
			/*JE_textShade(236, tempY, keyNames[keySettings[curSelect-2]], (temp2 / 16), (temp2 % 16) - 8, DARKEN);*/
			JE_textShade(236, tempY, SDL_GetKeyName(keySettings[curSelect-2]), (temp2 / 16), (temp2 % 16) - 8, DARKEN);
			JE_showVGA();

			col = 248;
			colC = 1;

			newkey = false;

			/*callBIOSHandler = false;*/
			do {
				col += colC;
				if (col < 243 || col > 248)
				{
					colC *= -1;
				}
				JE_rectangle(230, tempY - 2, 300, tempY + 7, col);

				/* JE_waitRetrace(); */
				setjasondelay(1);
				service_SDL_events(false);

				JE_showVGA();
			} while (!newkey);

			tempB = true;

			printf("key: %d", lastkey_sym);

			for (x = 0; x < 8; x++)
			{
				if (keySettings[x] == lastkey_sym)
				{
					tempB = false;
					JE_playSampleNum(false);
				}
			}

			/* SYN: I don't know why some of these are disallowed */
			if ( lastkey_sym != SDLK_ESCAPE &&
				 lastkey_sym != SDLK_F11 &&
				 lastkey_sym != SDLK_s &&
				 lastkey_sym != SDLK_m &&
				 lastkey_sym != SDLK_p &&
				 tempB )
			{
				JE_playSampleNum(CLICK);
				keySettings[curSelect-2] = lastkey_sym;
				curSelect++;
			}
			/* callBIOSHandler = true; */
			JE_wipeKey();

		}
		break;

	case 6:
		if (curSelect == 13)
		{
			if (quikSave)
			{
				curMenu = oldMenu;
				newPal = oldPal;
			} else {
				curMenu = 2;
			}
		} else {
			if (twoPlayerMode)
			{
				temp = 11;
			} else {
				temp = 0;
			}
			JE_operation(curSelect - 1 + temp);
			if (quikSave)
			{
				curMenu = oldMenu;
				newPal = oldPal;
			}
		}
		break;

	case 7:
		if (curSelect == menuChoices[curMenu])
		{
			curMenu = 0;
			newPal = 1;
		} else {
			if (cubeMax > 0)
			{
				firstMenu9 = true;
				curMenu = 8;
				yLoc = 0;
				yChg = 0;
				currentCube = curSel[7] - 2;
			} else {
				curMenu = 0;
				newPal = 1;
			}
		}
		break;

	case 8:
		curMenu = 7;
		/*scanCode = 0;*/
		joystickUp = false;
		joystickDown = false;
		break;

	case 9:
		switch (curSel[curMenu])
		{
		case 2:
			mainLevel = mapSection[mapPNum-1];
			jumpSection = true;
			break;
		case 3:
			JE_playSampleNum(CURSOR_MOVE);
			do {
				inputDevice1++;
				if (inputDevice1 > 3)
				{
					inputDevice1 = 1;
				}
			} while (inputDevice1 == inputDevice2);
			break;
		case 4:
			JE_playSampleNum(CURSOR_MOVE);
			do {
				inputDevice2++;
				if (inputDevice2 > 3)
				{
					inputDevice2 = 1;
				}
			} while (inputDevice1 == inputDevice2);
			break;
		case 5:
			curMenu = 2;
			break;
		case 6:
			if (JE_quitRequest(true))
			{
				if (isNetworkActive)
				{
					haltGame = true;
				}
				gameLoaded = true;
				mainLevel = 0;
			}
			break;
		}
		break;

	case 10:
		switch (curSel[curMenu])
		{
		case 2:
			mainLevel = mapSection[mapPNum-1];
			jumpSection = true;
			break;
		case 3:
			curMenu = 11;
			break;
		case 4:
			if (JE_quitRequest(true))
			{
				if (isNetworkActive)
				{
					JE_tyrianHalt(0);
				}
				gameLoaded = true;
				mainLevel = 0;
			}
			break;
		}
		break;

	case 11:
		switch (select)
		{
		case 2:
			curMenu = 12;
			break;
		case 3:
			curMenu = 5;
			break;
		case 6:
			curMenu = 10;
			break;
		}
		break;

	case 12:
		switch (select)
		{
		case 2:
		case 3:
		case 4:
		case 5:
			joyButtonAssign[select - 2]++;
			if (joyButtonAssign[select - 2] > 5)
			{
				joyButtonAssign[select - 2] = 1;
			}
			break;
		case 6:
			if (joystick_installed)
			{
				do
				{
					JE_drawJoystick();
					JE_helpBox(35, 35, mainMenuHelp[33 - 1], 18);
					JE_showVGA();
					if ( (joystickUp || joystickDown || joystickLeft || joystickRight) &&
						 !( button[0] || button[1] || button[2] || button[3]) )
					{
						inputDetected = false;
					}
				} while (!JE_anyButton());

				if (button[0] || button[1] || button[2] || button[3])
				{
					jCenterX = joyX;
					jCenterY = joyY;
				}
			}
			break;
		case 7:
			if (isNetworkGame || onePlayerAction)
			{
				curMenu = 11;
			} else {
				curMenu = 2;
			}
			break;
		}
		break;

	case 13:
		switch (curSel[curMenu])
		{
		case 2:
			mainLevel = mapSection[mapPNum-1];
			jumpSection = true;
			break;
		case 3:
			JE_doFunkyScreen();
			break;
		case 4:
			curMenu = 2;
			break;
		case 5:
			if (JE_quitRequest(true))
			{
				if (isNetworkGame)
				{
					JE_tyrianHalt(0);
				}
				gameLoaded = true;
				mainLevel = 0;
			}
		}
		break;
	}

	memcpy(lastPortPower, portPower, sizeof(lastPortPower));
	memcpy(pItemsBack, pItems, sizeof(pItemsBack));

}

void JE_drawJoystick( void )
{
	STUB();
}

void JE_funkyScreen( void )
{
	wait_noinput(true, true, joystick_installed);

	Uint8 *s = game_screen->pixels; /* 8-bit specific */
	Uint8 *src = VGAScreen2->pixels; /* 8-bit specific */
	
	for (int y = 0; y < 200; y++)
	{
		for (int x = 0; x < 320; x++)
		{
			int avg = 0;
			if (y > 0)
				avg += *(src - VGAScreen2->w) & 0x0f;
			if (y < VGAScreen2->h - 1)
				avg += *(src + VGAScreen2->w) & 0x0f;
			if (x > 0)
				avg += *(src - 1) & 0x0f;
			if (x < VGAScreen2->w - 1)
				avg += *(src + 1) & 0x0f;
			avg /= 4;
			
			if ((*src & 0x0f) > avg)
			{
				*s = (*src & 0x0f) | 0xc0;
			} else {
				*s = 0;
			}
			
			src++;
			s++;
		}
	}

	JE_clr256();
	JE_drawLines(true);
	JE_drawLines(false);
	JE_rectangle(0, 0, 319, 199, 37);
	JE_rectangle(1, 1, 318, 198, 35);

	s = VGAScreen2->pixels; /* 8-bit specific */
	src = game_screen->pixels; /* 8-bit specific */
	
	for (int y = 0; y < 200; y++)
	{
		for (int x = 0; x < 320; x++)
		{
			if (*src)
				*s = *src;
			
			src++;
			s++;
		}
	}

	verticalHeight = 9;
	JE_outText(10, 2, ships[pItems[12-1]].name, 12, 3);
	JE_helpBox(100, 20, shipInfo[pItems[12-1]-1][0], 40);
	JE_helpBox(100, 100, shipInfo[pItems[12-1]-1][1], 40);
	verticalHeight = 7;

	JE_outText(JE_fontCenter(miscText[4], TINY_FONT), 190, miscText[4], 12, 2);

	JE_playSampleNum(16);
	
	VGAScreen = VGAScreenSeg;
	JE_scaleInPicture();

	wait_input(true, true, true);
}

void JE_weaponSimUpdate( void )
{
	char buf[32];

	JE_weaponViewFrame(0);

	if ( (curSel[1] == 3 && curSel[4] < menuChoices[4]) || (curSel[1] == 4 && curSel[4] < menuChoices[4] - 1) )
	{

		if (leftPower)
		{
			sprintf(buf, "%d", downgradeCost);
			JE_outText(26, 137, buf, 1, 4);
		} else {
			JE_newDrawCShapeNum(OPTION_SHAPES, 14, 24, 149);
		}

		if (rightPower)
		{
			if (!rightPowerAfford)
			{
				sprintf(buf, "%d", upgradeCost);
				JE_outText(108, 137, buf, 7, 4);
				JE_newDrawCShapeNum(OPTION_SHAPES, 15, 119, 149);
			} else {
				sprintf(buf, "%d", upgradeCost);
				JE_outText(108, 137, buf, 1, 4);
			}
		} else {
			JE_newDrawCShapeNum(OPTION_SHAPES, 15, 119, 149);
		}

		if (curSel[1] == 3)
		{
			temp = portPower[1 - 1];
		} else {
			temp = portPower[2 - 1];
		}
		for (x = 1; x <= temp; x++)
		{
			JE_bar(39 + x * 6, 151, 39 + x * 6 + 4, 151, 251);
			JE_pix(39 + x * 6, 151, 252);
			JE_bar(39 + x * 6, 152, 39 + x * 6 + 4, 164, 250);
			JE_bar(39 + x * 6, 165, 39 + x * 6 + 4, 165, 249);
		}

		sprintf(buf, "POWER: %d", temp);
		JE_outText(58, 137, buf, 15, 4);

	} else {
		leftPower = false;
		rightPower = false;
		JE_newDrawCShapeNum(OPTION_SHAPES, 18, 20, 146);
	}

	JE_drawItem(1, pItems[12 - 1], PX - 5, PY - 7);
}

void JE_weaponViewFrame( JE_byte testshotnum )
{
	Uint8 *s; /* screen pointer, 8-bit specific */
	int i;

	JE_bar(8, 8, 143, 182, 0);

	/* JE: (* Port Configuration Display *)
	(*    drawportconfigbuttons;*/

	/*===========================STARS==========================*/
	/*DRAWSTARS*/

	for (i = MAX_STARS; i--;)
	{
		s = (Uint8 *)VGAScreen->pixels;
		
		starDat[i].sLoc += starDat[i].sMov + VGAScreen->w;
		
		if (starDat[i].sLoc < 177 * VGAScreen->w)
		{
			if (*(s + starDat[i].sLoc) == 0)
			{
				*(s + starDat[i].sLoc) = starDat[i].sC;
			}
			if (starDat[i].sC - 4 >= 9 * 16)
			{
				if (*(s + starDat[i].sLoc + 1) == 0)
				{
					*(s + starDat[i].sLoc + 1) = starDat[i].sC - 4;
				}
				if (starDat[i].sLoc > 0 && *(s + starDat[i].sLoc - 1) == 0)
				{
					*(s + starDat[i].sLoc - 1) = starDat[i].sC - 4;
				}
				if (*(s + starDat[i].sLoc + VGAScreen->w) == 0)
				{
					*(s + starDat[i].sLoc + VGAScreen->w) = starDat[i].sC - 4;
				}
				if (starDat[i].sLoc >= VGAScreen->w && *(s + starDat[i].sLoc - VGAScreen->w) == 0)
				{
					*(s + starDat[i].sLoc - VGAScreen->w) = starDat[i].sC - 4;
				}
			}
		}
	}

	mouseX = PX;
	mouseY = PY;

	/* JE: (* PLAYER SHOT Creation *) */
	for (temp = 0; temp <= 1; temp++)
	{
		if (shotRepeat[temp] > 0)
		{
			shotRepeat[temp]--;
		} else {
			JE_initPlayerShot(pItems[temp], temp + 1, PX, PY, mouseX, mouseY,
			  weaponPort[pItems[temp]].op[portConfig[temp] - 1][portPower[temp] - 1], 1);
		}
	}

	if (options[pItems[4 - 1]].wport > 0)
	{
		if (shotRepeat[3 - 1] > 0)
		{
			shotRepeat[3 - 1]--;
		} else {
			JE_initPlayerShot(options[pItems[4-1]].wport, 3, option1X, option1Y,
			  mouseX, mouseY, options[pItems[4-1]].wpnum, 1);
		}
	}

	if (options[pItems[5-1]].tr == 2)
	{
		option2X = PX;
		option2Y = PY - 20;
		if (option2Y < 10)
		{
			option2Y = 10;
		}
	} else {
		option2X = 72 + 15;
		option2Y = 120;
	}

	if (options[pItems[5 - 1]].wport > 0)
	{
		if (shotRepeat[4 - 1] > 0)
		{
			shotRepeat[4 - 1]--;
		} else {
			JE_initPlayerShot(options[pItems[5-1]].wport, 4, option2X, option2Y,
			  mouseX, mouseY, options[pItems[5-1]].wpnum, 1);
		}
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

				if (playerShotData[z].shotXM <= 100)
				{
					playerShotData[z].shotX += playerShotData[z].shotXM;
				}

				playerShotData[z].shotYM += playerShotData[z].shotYC;
				playerShotData[z].shotY += playerShotData[z].shotYM;

				if (playerShotData[z].shotYM > 100)
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

				if (playerShotData[z].shotX < 0 || playerShotData[z].shotX > 140 ||
				    playerShotData[z].shotY < 0 || playerShotData[z].shotY > 170)
				{
					shotAvail[z] = 0;
					goto draw_player_shot_loop_end;
				}

/*				if (playerShotData[z].shotTrail != 255)
				{
					if (playerShotData[z].shotTrail == 98)
					{
						JE_setupExplosion(playerShotData[z].shotX - playerShotData[z].shotXM, playerShotData[z].shotY - playerShotData[z].shotYM, playerShotData[z].shotTrail);
					} else {
						JE_setupExplosion(playerShotData[z].shotX, playerShotData[z].shotY, playerShotData[z].shotTrail);
					}
				}*/

				tempW = playerShotData[z].shotGr + playerShotData[z].shotAni;
				if (++playerShotData[z].shotAni == playerShotData[z].shotAniMax)
				{
					playerShotData[z].shotAni = 0;
				}

				if (tempW < 6000)
				{
					if (tempW > 1000)
					{
						tempW = tempW % 1000;
					}
					if (tempW > 500)
					{
						JE_drawShape2(tempShotX+1, tempShotY, tempW - 500, shapesW2);
					} else {
						JE_drawShape2(tempShotX+1, tempShotY, tempW, shapesC1);
					}
				}

			}

draw_player_shot_loop_end:
			;
		}
	}

	JE_newDrawCShapeNum(OPTION_SHAPES, 13, 0, 0);


	/*========================Power Bar=========================*/

	power = power + powerAdd;
	if (power > 900)
	{
		power = 900;
	}

	temp = power / 10;

	for (temp = 147 - temp; temp <= 146; temp++)
	{
		temp2 = 113 + (146 - temp) / 9 + 2;
		temp3 = (temp + 1) % 6;
		if (temp3 == 1)
		{
			temp2 += 3;
		}
		else if (temp3 != 0)
		{
			temp2 += 2;
		}

		JE_pix(141, temp, temp2 - 3);
		JE_pix(142, temp, temp2 - 3);
		JE_pix(143, temp, temp2 - 2);
		JE_pix(144, temp, temp2 - 1);
		JE_bar(145, temp, 149, temp, temp2);

		if (temp2 - 3 < 112)
		{
			temp2++;
		}

	}

	temp = 147 - (power / 10);
	temp2 = 113 + (146 - temp) / 9 + 4;

	JE_pix(141, temp - 1, temp2 - 1);
	JE_pix(142, temp - 1, temp2 - 1);
	JE_pix(143, temp - 1, temp2 - 1);
	JE_pix(144, temp - 1, temp2 - 1);

	JE_bar(145, temp-1, 149, temp-1, temp2);

	lastPower = temp;

	JE_waitFrameCount();
}

void JE_genItemMenu( JE_byte itemNum )
{
	menuChoices[4] = itemAvailMax[itemAvailMap[itemNum - 2] - 1] + 2;

	temp3 = 2;
	temp2 = pItems[pItemButtonMap[itemNum - 2] -1];

	strcpy(menuInt[4][0], menuInt[1][itemNum - 1]);

	for (tempW = 0; tempW < itemAvailMax[itemAvailMap[itemNum-2]-1]; tempW++)
	{
		temp = itemAvail[itemAvailMap[itemNum-2]-1][tempW];
		switch (itemNum)
		{
		case 2:
			strcpy(tempStr, ships[temp].name);
			break;
		case 3:
		case 4:
			strcpy(tempStr, weaponPort[temp].name);
			tempPowerLevel[tempW] = 1;
			break;
		case 5:
			strcpy(tempStr, shields[temp].name);
			break;
		case 6:
			strcpy(tempStr, powerSys[temp].name);
			break;
		case 7:
		case 8:
			strcpy(tempStr, options[temp].name);
			break;
		}
		if (temp == temp2)
		{
			temp3 = tempW + 2;
		}
		strcpy(menuInt[4][tempW+1], tempStr);
	}

	strcpy(menuInt[4][tempW+1], miscText[13]);

	// YKS: I have no idea wtf this is doing, but I don't think it matters either, little of this function does
	if (itemNum == 3 || itemNum == 4)
	{
		tempPowerLevel[tempW] = portPower[itemNum-1];
		if (tempPowerLevel[tempW] < 1)
		{
			tempPowerLevel[tempW] = 1;
		}
	}
	curSel[4] = temp3;
}

void JE_doNetwork( void )
{
	STUB();
}

/* located in backgrnd.c
void JE_drawBackground3( void )
{
}*/

void JE_scaleInPicture( void )
{
	for (int i = 2; i <= 160; i += 2)
	{
		if (JE_anyButton())
			i = 160;
		JE_scaleBitmap(VGAScreen2, 320, 200, 160 - i, 0, 160 + i - 1, 100 + round(i * 0.625f) - 1);
		JE_showVGA();
		
		SDL_Delay(1);
	}
}
