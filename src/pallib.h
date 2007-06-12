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
#ifndef PALLIB_H
#define PALLIB_H

#include "opentyr.h"

#include "nortvars.h"


#define MAX_PAL 23

typedef JE_ColorType JE_PalType[MAX_PAL]; /* [1..maxpal] */

#ifndef NO_EXTERNS
extern JE_PalType palettes;
extern JE_word palNum;
#endif

void JE_loadPals( void );
void JE_zPal( JE_byte palette );

#endif /* PALLIB_H */
