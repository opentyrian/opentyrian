/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
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
#include "starlib.h"

#include "keyboard.h"
#include "mtrand.h"
#include "video.h"

#include <ctype.h>

#define starlib_MAX_STARS 1000
#define MAX_TYPES 14

struct JE_StarType
{
	JE_integer spX, spY, spZ;
	JE_integer lastX, lastY;
};

static int tempX, tempY;
static JE_boolean run;
static struct JE_StarType star[starlib_MAX_STARS];

static JE_byte setup;
static JE_word stepCounter;

static JE_word nsp2;
static JE_shortint nspVar2Inc;

/* JE: new sprite pointer */
static JE_real nsp;
static JE_real nspVarInc;
static JE_real nspVarVarInc;

static JE_word changeTime;
static JE_boolean doChange;

static JE_boolean grayB;

static JE_integer starlib_speed;
static JE_shortint speedChange;

static JE_byte pColor;


void JE_starlib_main( void )
{
	int off;
	JE_word i;
	JE_integer tempZ;
	JE_byte tempCol;
	struct JE_StarType *stars;
	Uint8 *surf;

	JE_wackyCol();

	grayB = false;

	starlib_speed += speedChange;


	for(stars = star, i = starlib_MAX_STARS; i > 0; stars++, i--)
	{
		/* Make a pointer to the screen... */
		surf = VGAScreen->pixels;

		/* Calculate the offset to where we wish to draw */
		off = (stars->lastX)+(stars->lastY)*320;


		/* We don't want trails in our star field.  Erase the old graphic */
		if (off >= 640 && off < (320*200)-640)
		{
			surf[off] = 0; /* Shade Level 0 */

			surf[off-1] = 0; /* Shade Level 1, 2 */
			surf[off+1] = 0;
			surf[off-2] = 0;
			surf[off+2] = 0;

			surf[off-320] = 0;
			surf[off+320] = 0;
			surf[off-640] = 0;
			surf[off+640] = 0;
		}

		/* Move star */
		tempZ = stars->spZ;
		tempX = (stars->spX / tempZ) + 160;
		tempY = (stars->spY / tempZ) + 100;
		tempZ -=  starlib_speed;


		/* If star is out of range, make a new one */
		if (tempZ <=  0 ||
		    tempY ==  0 || tempY > 198 ||
		    tempX > 318 || tempX <   1)
		{
			stars->spZ = 500;

			JE_newStar();

			stars->spX = tempX;
			stars->spY = tempY;
		}
		else /* Otherwise, update & draw it */
		{
			stars->lastX = tempX;
			stars->lastY = tempY;
			stars->spZ = tempZ;

			off = tempX+tempY*320;

			if (grayB)
			{
				tempCol = tempZ >> 1;
			} else {
				tempCol = pColor+((tempZ >> 4) & 31);
			}

			/* Draw the pixel! */
			if (off >= 640 && off < (320*200)-640)
			{
				surf[off] = tempCol;

				tempCol += 72;
				surf[off-1] = tempCol;
				surf[off+1] = tempCol;
				surf[off-320] = tempCol;
				surf[off+320] = tempCol;

				tempCol += 72;
				surf[off-2] = tempCol;
				surf[off+2] = tempCol;
				surf[off-640] = tempCol;
				surf[off+640] = tempCol;
			}
		}
	}

	if (newkey)
	{
		char key = 0;

		if ((lastkey_mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_GUI)) == KMOD_NONE)
		{
			switch (lastkey_scan)
			{
			case SDL_SCANCODE_C: key = 'c'; break;
			case SDL_SCANCODE_P: key = 'p'; break;
			case SDL_SCANCODE_S: key = 's'; break;
			case SDL_SCANCODE_X: key = 'x'; break;
			case SDL_SCANCODE_1: key = '1'; break;
			case SDL_SCANCODE_2: key = '2'; break;
			case SDL_SCANCODE_3: key = '3'; break;
			case SDL_SCANCODE_4: key = '4'; break;
			case SDL_SCANCODE_5: key = '5'; break;
			case SDL_SCANCODE_6: key = '6'; break;
			case SDL_SCANCODE_7: key = '7'; break;
			case SDL_SCANCODE_8: key = '8'; break;
			case SDL_SCANCODE_9: key = '9'; break;
			case SDL_SCANCODE_0: key = '0'; break;
			case SDL_SCANCODE_ESCAPE: key = 27; break;
			case SDL_SCANCODE_MINUS: key = '-'; break;
			case SDL_SCANCODE_LEFTBRACKET: key = '['; break;
			case SDL_SCANCODE_RIGHTBRACKET: key = ']'; break;
			case SDL_SCANCODE_GRAVE: key = '`'; break;
			case SDL_SCANCODE_KP_MINUS: key = '-'; break;
			case SDL_SCANCODE_KP_PLUS: key = '+'; break;
			case SDL_SCANCODE_KP_1: key = '1'; break;
			case SDL_SCANCODE_KP_2: key = '2'; break;
			case SDL_SCANCODE_KP_3: key = '3'; break;
			case SDL_SCANCODE_KP_4: key = '4'; break;
			case SDL_SCANCODE_KP_5: key = '5'; break;
			case SDL_SCANCODE_KP_6: key = '6'; break;
			case SDL_SCANCODE_KP_7: key = '7'; break;
			case SDL_SCANCODE_KP_8: key = '8'; break;
			case SDL_SCANCODE_KP_9: key = '9'; break;
			case SDL_SCANCODE_KP_0: key = '0'; break;
			default: break;
			}
		}
		else if ((lastkey_mod & KMOD_SHIFT) != KMOD_NONE &&
		         (lastkey_mod & (KMOD_CTRL | KMOD_ALT | KMOD_GUI)) == KMOD_NONE)
		{
			switch (lastkey_scan)
			{
			case SDL_SCANCODE_C: key = 'C'; break;
			case SDL_SCANCODE_P: key = 'P'; break;
			case SDL_SCANCODE_S: key = 'S'; break;
			case SDL_SCANCODE_X: key = 'X'; break;
			case SDL_SCANCODE_1: key = '!'; break;
			case SDL_SCANCODE_2: key = '@'; break;
			case SDL_SCANCODE_3: key = '#'; break;
			case SDL_SCANCODE_4: key = '$'; break;
			case SDL_SCANCODE_EQUALS: key = '+'; break;
			case SDL_SCANCODE_LEFTBRACKET: key = '{'; break;
			case SDL_SCANCODE_RIGHTBRACKET: key = '}'; break;
			default: break;
			}
		}

		switch (toupper(key))
		{
			case '+':
				starlib_speed++;
				speedChange = 0;
				break;
			case '-':
				starlib_speed--;
				speedChange = 0;
				break;
			case '1':
				JE_changeSetup(1);
				break;
			case '2':
				JE_changeSetup(2);
				break;
			case '3':
				JE_changeSetup(3);
				break;
			case '4':
				JE_changeSetup(4);
				break;
			case '5':
				JE_changeSetup(5);
				break;
			case '6':
				JE_changeSetup(6);
				break;
			case '7':
				JE_changeSetup(7);
				break;
			case '8':
				JE_changeSetup(8);
				break;
			case '9':
				JE_changeSetup(9);
				break;
			case '0':
				JE_changeSetup(10);
				break;
			case '!':
				JE_changeSetup(11);
				break;
			case '@':
				JE_changeSetup(12);
				break;
			case '#':
				JE_changeSetup(13);
				break;
			case '$':
				JE_changeSetup(14);
				break;

			case 'C':
				JE_resetValues();
				break;
			case 'S':
				nspVarVarInc = mt_rand_1() * 0.01f - 0.005f;
				break;
			case 'X':
			case 27:
				run = false;
				break;
			case '[':
				pColor--;
				break;
			case ']':
				pColor++;
				break;
			case '{':
				pColor -= 72;
				break;
			case '}':
				pColor += 72;
				break;
			case '`':
				doChange = !doChange;
				break;
			case 'P':
				wait_noinput(true, false, false);
				wait_input(true, false, false);
				break;
			default:
				break;
		}
	}

	if (doChange)
	{
		stepCounter++;
		if (stepCounter > changeTime)
		{
			JE_changeSetup(0);
		}
	}

	if ((mt_rand() % 1000) == 1)
	{
		nspVarVarInc = mt_rand_1() * 0.01f - 0.005f;
	}

	nspVarInc += nspVarVarInc;
}

void JE_wackyCol( void )
{
	/* YKS: Does nothing */
}

void JE_starlib_init( void )
{
	static JE_boolean initialized = false;

	if (!initialized)
	{
		initialized = true;

		JE_resetValues();
		JE_changeSetup(2);
		doChange = true;

		/* RANDOMIZE; */
		for (int x = 0; x < starlib_MAX_STARS; x++)
		{
			star[x].spX = (mt_rand() % 64000) - 32000;
			star[x].spY = (mt_rand() % 40000) - 20000;
			star[x].spZ = x+1;
		}
	}
}

void JE_resetValues( void )
{
	nsp2 = 1;
	nspVar2Inc = 1;
	nspVarInc = 0.1f;
	nspVarVarInc = 0.0001f;
	nsp = 0;
	pColor = 32;
	starlib_speed = 2;
	speedChange = 0;
}

void JE_changeSetup( JE_byte setupType )
{
	stepCounter = 0;
	changeTime = (mt_rand() % 1000);

	if (setupType > 0)
	{
		setup = setupType;
	} else {
		setup = mt_rand() % (MAX_TYPES + 1);
	}

	if (setup == 1)
	{
		nspVarInc = 0.1f;
	}
	if (nspVarInc > 2.2f)
	{
		nspVarInc = 0.1f;
	}
}

void JE_newStar( void )
{
	if (setup == 0)
	{
		tempX = (mt_rand() % 64000) - 32000;
		tempY = (mt_rand() % 40000) - 20000;
	} else {
		nsp = nsp + nspVarInc; /* YKS: < lol */
		switch (setup)
		{
			case 1:
				tempX = (int)(sinf(nsp / 30) * 20000);
				tempY = (mt_rand() % 40000) - 20000;
				break;
			case 2:
				tempX = (int)(cosf(nsp) * 20000);
				tempY = (int)(sinf(nsp) * 20000);
				break;
			case 3:
				tempX = (int)(cosf(nsp * 15) * 100) * ((int)(nsp / 6) % 200);
				tempY = (int)(sinf(nsp * 15) * 100) * ((int)(nsp / 6) % 200);
				break;
			case 4:
				tempX = (int)(sinf(nsp / 60) * 20000);
				tempY = (int)(cosf(nsp) * (int)(sinf(nsp / 200) * 300) * 100);
				break;
			case 5:
				tempX = (int)(sinf(nsp / 2) * 20000);
				tempY = (int)(cosf(nsp) * (int)(sinf(nsp / 200) * 300) * 100);
				break;
			case 6:
				tempX = (int)(sinf(nsp) * 40000);
				tempY = (int)(cosf(nsp) * 20000);
				break;
			case 8:
				tempX = (int)(sinf(nsp / 2) * 40000);
				tempY = (int)(cosf(nsp) * 20000);
				break;
			case 7:
				tempX = mt_rand() % 65535;
				if ((mt_rand() % 2) == 0)
				{
					tempY = (int)(cosf(nsp / 80) * 10000) + 15000;
				} else {
					tempY = 50000 - (int)(cosf(nsp / 80) * 13000);
				}
				break;
			case 9:
				nsp2 += nspVar2Inc;
				if ((nsp2 == 65535) || (nsp2 == 0))
				{
					nspVar2Inc = -nspVar2Inc;
				}
				tempX = (int)(cosf(sinf(nsp2 / 10.0f) + (nsp / 500)) * 32000);
				tempY = (int)(sinf(cosf(nsp2 / 10.0f) + (nsp / 500)) * 30000);
				break;
			case 10:
				nsp2 += nspVar2Inc;
				if ((nsp2 == 65535) || (nsp2 == 0))
				{
					nspVar2Inc = -nspVar2Inc;
				}
				tempX = (int)(cosf(sinf(nsp2 / 5.0f) + (nsp / 100)) * 32000);
				tempY = (int)(sinf(cosf(nsp2 / 5.0f) + (nsp / 100)) * 30000);
				break;;
			case 11:
				nsp2 += nspVar2Inc;
				if ((nsp2 == 65535) || (nsp2 == 0))
				{
					nspVar2Inc = -nspVar2Inc;
				}
				tempX = (int)(cosf(sinf(nsp2 / 1000.0f) + (nsp / 2)) * 32000);
				tempY = (int)(sinf(cosf(nsp2 / 1000.0f) + (nsp / 2)) * 30000);
				break;
			case 12:
				if (nsp != 0)
				{
					nsp2 += nspVar2Inc;
					if ((nsp2 == 65535) || (nsp2 == 0))
					{
						nspVar2Inc = -nspVar2Inc;
					}
					tempX = (int)(cosf(sinf(nsp2 / 2.0f) / (sqrtf(fabsf(nsp)) / 10.0f + 1) + (nsp2 / 100.0f)) * 32000);
					tempY = (int)(sinf(cosf(nsp2 / 2.0f) / (sqrtf(fabsf(nsp)) / 10.0f + 1) + (nsp2 / 100.0f)) * 30000);
				}
				break;
			case 13:
				if (nsp != 0)
				{
					nsp2 += nspVar2Inc;
					if ((nsp2 == 65535) || (nsp2 == 0))
					{
						nspVar2Inc = -nspVar2Inc;
					}
					tempX = (int)(cosf(sinf(nsp2 / 10.0f) / 2 + (nsp / 20)) * 32000);
					tempY = (int)(sinf(sinf(nsp2 / 11.0f) / 2 + (nsp / 20)) * 30000);
				}
				break;
			case 14:
				nsp2 += nspVar2Inc;
				tempX = (int)((sinf(nsp) + cosf(nsp2 / 1000.0f) * 3) * 12000);
				tempY = (int)(cosf(nsp) * 10000) + nsp2;
				break;
		}
	}
}

