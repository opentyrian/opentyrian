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
#ifndef PICLOAD2_H
#define PICLOAD2_H

#include "opentyr.h"

#include "pcxmast.h"
#include "nortvars.h"


extern JE_ColorType colors2;
extern JE_boolean overrideblack;
extern JE_char c2;

void JE_analyzePic( void );

void JE_unpackPCX( void );

/* void JE_updatePCXColorsSlow ( void* colorbuffer ); */

/*void JE_loadPic( JE_byte pcxnumber, JE_boolean storepalette );*/

#endif /* PICLOAD2_H */
