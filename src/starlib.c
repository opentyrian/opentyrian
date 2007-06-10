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

#define NO_EXTERNS
#include "starlib.h"
#undef NO_EXTERNS

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

JE_integer speed;
JE_shortint speedChange;

JE_byte pColor;


void JE_starlib_main( void )
{
	STUB(JE_starlib_main);
}

void JE_makeGray( void )
{
	/* YKS: Not used anywhere. */
	STUB(JE_makeGray);
}

void JE_wackyCol( void )
{
	JE_byte a, b, c;
	/* YKS: Does nothing */
}

void JE_starlib_init( void )
{
	/* RANDOMIZE; */
	for (x = 0; x < starlib_MAX_STARS; x++)
	{
		star[x].spX = (rand() % 64000) - 32000;
		star[x].spY = (rand() % 40000) - 20000;
		star[x].spZ = x+1;
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
	speed = 2;
	speedChange = 0;
}

void JE_changeSetup( JE_byte setupType )
{
	stepCounter = 0;
	changeTime = (rand() % 1000);

	if (setupType > 0)
	{
		setup = setupType;
	} else {
		setup = rand() % (MAX_TYPES + 1);
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
		tempX = (rand() % 64000) - 32000;
		tempY = (rand() % 40000) - 20000;
	} else {
		nsp = nsp + nspVarInc; /* YKS: < lol */
		switch (setup)
		{
			case 1:
				tempX = (int)(sin(nsp / 30) * 20000);
				tempY = (rand() % 40000) - 20000;
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
				tempX = rand() % 65535;
				if ((rand() % 2) == 0)
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
