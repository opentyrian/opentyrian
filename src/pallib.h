/*
 * OpenTyrian Classic: A moden cross-platform port of Tyrian
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

#define maxpal 23

typedef JE_colortype JE_paltype[maxpal]; /* [1..maxpal] */

#define PALLIB_EXTERNS \
extern JE_paltype palettes; \
extern JE_word palnum;

void loadpals( void );
void ZPal( JE_byte palette );
void pallib_init( void );

#endif /* PALLIB_H */
