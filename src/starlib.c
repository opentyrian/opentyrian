/* 
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
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


JE_boolean run;
struct JE_StarType star[starlib_MAX_STARS];

JE_integer tempW;
JE_integer tempX, tempY;

JE_byte setup;
JE_word stepCounter;

JE_word nsp2;
JE_shortint nspVar2Inc;

/* JE: new sprite pointer */
JE_real nsp;
JE_real nspVarInc;
JE_real nspVarVarInc;

JE_word changeTime;
JE_boolean doChange;

JE_boolean grayB;

JE_integer x;

JE_integer starlib_speed;
JE_shortint speedChange;

JE_byte pColor;


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

	/* ASM starts */
	/* ***START THE LOOP*** */
star_loop:
	stars = star;
	i = starlib_MAX_STARS;

	/* ***START OF EACH PIXEL*** */
next_star:
	off = (stars->lastX)+(stars->lastY)*320;

	/* ***CLEAR PIXEL*** */
	surf = VGAScreen->pixels;

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

	tempZ = stars->spZ;

	/* Here's the StarType offsets:
	 *
	 * SPX   0
	 * SPY   2
	 * SPZ   4
	 * LastX 6
	 * LastY 8
	 */

	tempX = (stars->spX / tempZ) + 160;
	tempY = (stars->spY / tempZ) + 100;

	tempZ -= starlib_speed;

	if (tempZ <= 0)
	{
		goto new_star;
	}

	if (tempY == 0 || tempY > 198)
	{
		goto new_star;
	}

	if (tempX > 318 || tempX < 1)
	{
		goto new_star;
	}

	/* Update current star */
	stars->lastX = tempX;
	stars->lastY = tempY;
	stars->spZ = tempZ;

	/* Now, WriteSuperPixel */
	/* Let's find the location */
	off = tempX+tempY*320;

	if (grayB)
	{
		tempCol = tempZ >> 1;
	} else {
		tempCol = pColor+((tempZ >> 4) & 31);
	}

draw_pixel:
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

	goto star_end;

new_star:
	stars->spZ = 500;

	JE_newStar();

	stars->spX = tempX;
	stars->spY = tempY;

star_end:
	stars++;
	i--;
	if (i > 0)
	{
		goto next_star;
	}

	/* ASM ends */

	if (newkey)
	{
		switch (toupper(lastkey_char))
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
				nspVarVarInc = (mt_rand()/(float)RAND_MAX) * 0.01 - 0.005;
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
			case '`': /* ` */
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
		nspVarVarInc = (mt_rand()/(float)RAND_MAX) * 0.01 - 0.005;
	}

	nspVarInc += nspVarVarInc;
}

void JE_makeGray( void )
{
	/* YKS: Not used anywhere. */
	STUB();
}

void JE_wackyCol( void )
{
	JE_byte a, b, c;
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
		for (x = 0; x < starlib_MAX_STARS; x++)
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
	nspVarInc = 0.1;
	nspVarVarInc = 0.0001;
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
		nspVarInc = 0.1;
	}
	if (nspVarInc > 2.2)
	{
		nspVarInc = 0.1;
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
				tempX = (int)(sin(nsp / 30) * 20000);
				tempY = (mt_rand() % 40000) - 20000;
				break;
			case 2:
				tempX = (int)(cos(nsp) * 20000);
				tempY = (int)(sin(nsp) * 20000);
				break;
			case 3:
				tempX = (int)(cos(nsp * 15) * 100) * ((int)(nsp / 6) % 200);
				tempY = (int)(sin(nsp * 15) * 100) * ((int)(nsp / 6) % 200);
				break;
			case 4:
				tempX = (int)(sin(nsp / 60) * 20000);
				tempY = (int)(cos(nsp) * (int)(sin(nsp / 200) * 300) * 100);
				break;
			case 5:
				tempX = (int)(sin(nsp / 2) * 20000);
				tempY = (int)(cos(nsp) * (int)(sin(nsp / 200) * 300) * 100);
				break;
			case 6:
				tempX = (int)(sin(nsp) * 40000);
				tempY = (int)(cos(nsp) * 20000);
				break;
			case 8:
				tempX = (int)(sin(nsp / 2) * 40000);
				tempY = (int)(cos(nsp) * 20000);
				break;
			case 7:
				tempX = mt_rand() % 65535;
				if ((mt_rand() % 2) == 0)
				{
					tempY = (int)(cos(nsp / 80) * 10000) + 15000;
				} else {
					tempY = 50000 - (int)(cos(nsp / 80) * 13000);
				}
				break;
			case 9:
				nsp2 += nspVar2Inc;
				if ((nsp2 == 65535) || (nsp2 == 0))
				{
					nspVar2Inc = -nspVar2Inc;
				}
				tempX = (int)(cos(sin(nsp2 / 10.) + (nsp / 500)) * 32000);
				tempY = (int)(sin(cos(nsp2 / 10.) + (nsp / 500)) * 30000);
				break;
			case 10:
				nsp2 += nspVar2Inc;
				if ((nsp2 == 65535) || (nsp2 == 0))
				{
					nspVar2Inc = -nspVar2Inc;
				}
				tempX = (int)(cos(sin(nsp2 / 5.) + (nsp / 100)) * 32000);
				tempY = (int)(sin(cos(nsp2 / 5.) + (nsp / 100)) * 30000);
				break;;
			case 11:
				nsp2 += nspVar2Inc;
				if ((nsp2 == 65535) || (nsp2 == 0))
				{
					nspVar2Inc = -nspVar2Inc;
				}
				tempX = (int)(cos(sin(nsp2 / 1000.) + (nsp / 2)) * 32000);
				tempY = (int)(sin(cos(nsp2 / 1000.) + (nsp / 2)) * 30000);
				break;
			case 12:
				if (nsp != 0)
				{
					nsp2 += nspVar2Inc;
					if ((nsp2 == 65535) || (nsp2 == 0))
					{
						nspVar2Inc = -nspVar2Inc;
					}
					tempX = (int)(cos(sin(nsp2 / 2.) / (sqrt(abs(nsp)) / 10. + 1) + (nsp2 / 100.)) * 32000);
					tempY = (int)(sin(cos(nsp2 / 2.) / (sqrt(abs(nsp)) / 10. + 1) + (nsp2 / 100.)) * 30000);
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
					tempX = (int)(cos(sin(nsp2 / 10.) / 2 + (nsp / 20)) * 32000);
					tempY = (int)(sin(sin(nsp2 / 11.) / 2 + (nsp / 20)) * 30000);
				}
				break;
			case 14:
				nsp2 += nspVar2Inc;
				tempX = (int)((sin(nsp) + cos(nsp2 / 1000.) * 3) * 12000);
				tempY = (int)(cos(nsp) * 10000) + nsp2;
				break;
		}
	}
}

// kate: tab-width 4; vim: set noet:
