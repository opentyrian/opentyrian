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
#include "animlib.h"

#include "error.h"
#include "joystick.h"
#include "network.h"
#include "nortsong.h"
#include "nortvars.h"
#include "vga256d.h"


JE_word currentpageofs;
JE_word currentpageseg;

void JE_loadPage( JE_word pagenumber )
{
	STUB();
}

void JE_drawFrame( JE_word framenumber )
{
	STUB();
}

JE_word JE_findPage ( JE_word framenumber )
{
	STUB();
	return -1;
}

void JE_renderFrame( JE_word framenumber )
{
	STUB();
}

void JE_playAnim( JE_char *animfile, JE_byte startingframe, JE_boolean keyhalt, JE_byte speed )
{
	STUB();
}

void JE_loadAnim( JE_char *filename )
{
	STUB();
}

void JE_closeAnim( void )
{
	STUB();
}

void JE_playRunSkipDump( JE_word bufferoffset )
{
	STUB();
}

// kate: tab-width 4; vim: set noet:
