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

#include "config.h"
#include "fonthand.h"
#include "helptext.h"
#include "keyboard.h"
#include "newshape.h"
#include "nortsong.h"
#include "nortvars.h"
#include "pcxload.h"
#include "picload.h"
#include "picload2.h"
#include "starfade.h"
#include "varz.h"
/* #include "vga256c.h" */
#include "vga256d.h"

#define NO_EXTERNS
#include "destruct.h"
#undef NO_EXTERNS


JE_word tempscreenseg;
JE_byte enddelay;
JE_boolean died;
JE_boolean firsttime;

void JE_superPixel( JE_word loc )
{
	STUB(JE_superPixel);
}

void JE_pauseScreen( void )
{
	STUB(JE_pauseScreen);
}

void JE_eSound( JE_byte sound )
{
	STUB(JE_eSound);
}

void JE_helpScreen( void )
{
	STUB(JE_helpScreen);
}

JE_boolean JE_stabilityCheck( JE_integer x, JE_integer y )
{
	STUB(JE_stabilityCheck);
	return FALSE;
}

JE_byte JE_placementPosition( JE_word x, JE_byte width )
{
	STUB(JE_placementPosition);
	return 0;
}

void JE_destructMain( void )
{
	STUB(JE_destructMain);
}

void JE_destructGame( void )
{
	STUB(JE_destructGame);
}

void JE_modeSelect( void )
{
	STUB(JE_modeSelect);
}

void JE_generateTerrain( void )
{
	STUB(JE_generateTerrain);
}

void JE_aliasDirt( void )
{
	STUB(JE_aliasDirt);
}

void JE_tempScreenChecking( void )
{
	STUB(JE_tempScreenChecking);
}

void JE_makeExplosion( JE_word tempx, JE_word tempy, JE_byte shottype )
{
	STUB(JE_makeExplosion);
}

void JE_introScreen( void )
{
	STUB(JE_introScreen);
}
