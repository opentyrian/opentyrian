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
	STUB(JE_makeGray);	
}

void JE_wackyCol( void )
{
	JE_byte a, b, c;
	/* YKS: Does nothing */
}

void JE_starlib_init( void )
{
	STUB(JE_starlib_init);
}

void JE_resetValues( void )
{
	STUB(JE_resetValues);	
}

void JE_changeSetup( JE_byte setupType )
{
	STUB(JE_changeSetup);	
}

void JE_newStar( void )
{
	STUB(JE_newStar);	
}
