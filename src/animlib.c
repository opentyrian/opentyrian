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

#include "error.h"
#include "joystick.h"
#include "network.h"
#include "nortsong.h"
#include "nortvars.h"
#include "pcxload.h"
#include "starfade.h"
#include "vga256d.h"

#define NO_EXTERNS
#include "animlib.h"
#undef NO_EXTERNS


JE_word currentpageofs;
JE_word currentpageseg;

void JE_loadPage( JE_word pagenumber )
{
	STUB(JE_loadPage);
}

void JE_drawFrame( JE_word framenumber )
{
	STUB(JE_drawFrame);
}

JE_word JE_findPage ( JE_word framenumber )
{
	STUB(JE_findPage );
	return -1;
}

void JE_renderFrame( JE_word framenumber )
{
	STUB(JE_renderFrame);
}

void JE_playAnim( JE_char *animfile, JE_byte startingframe, JE_boolean keyhalt, JE_byte speed )
{
	STUB(JE_playAnim);
}

void JE_loadAnim( JE_char *filename )
{
	STUB(JE_loadAnim);
}

void JE_closeAnim( void )
{
	STUB(JE_closeAnim);
}

void JE_playRunSkipDump( JE_word bufferoffset )
{
	STUB(JE_playRunSkipDump);
}
