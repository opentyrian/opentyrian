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
#include "error.h"
#include "nortvars.h"

#define NO_EXTERNS
#include "editship.h"
#undef NO_EXTERNS


JE_boolean extraavail;
JE_word extrashapeofs;
JE_ShipsType ships;
void * extrashapes;
JE_word extrashapeseg;
JE_word extrashapesize;

void JE_endShape( void )
{
	STUB(JE_endShape);
}

void JE_decryptShips( void )
{
	STUB(JE_decryptShips);
}

void JE_compressShapeFile( void )
{
	STUB(JE_compressShapeFile);
}

void JE_loadExtraShapes( void )
{
	STUB(JE_loadExtraShapes);
}

void JE_add( JE_byte nextbyte )
{
	STUB(JE_add);
}

void JE_encryptShips( void )
{
	STUB(JE_encryptShips);
}

void JE_buildRec( void )
{
	STUB(JE_buildRec);
}

void JE_startNewShape( void )
{
	STUB(JE_startNewShape);
}
