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
#include "destruct.h"

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
#include "vga256d.h"


JE_word tempscreenseg;
JE_byte enddelay;
JE_boolean died;
JE_boolean firsttime;

void JE_superPixel( JE_word loc )
{
	STUB();
}

void JE_pauseScreen( void )
{
	STUB();
}

void JE_eSound( JE_byte sound )
{
	STUB();
}

void JE_helpScreen( void )
{
	STUB();
}

JE_boolean JE_stabilityCheck( JE_integer x, JE_integer y )
{
	STUB();
	return false;
}

JE_byte JE_placementPosition( JE_word x, JE_byte width )
{
	STUB();
	return 0;
}

void JE_destructMain( void )
{
	STUB();
}

void JE_destructGame( void )
{
	STUB();
}

void JE_modeSelect( void )
{
	STUB();
}

void JE_generateTerrain( void )
{
	STUB();
}

void JE_aliasDirt( void )
{
	STUB();
}

void JE_tempScreenChecking( void )
{
	STUB();
}

void JE_makeExplosion( JE_word tempx, JE_word tempy, JE_byte shottype )
{
	STUB();
}

void JE_introScreen( void )
{
	STUB();
}
