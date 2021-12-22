/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
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
#include "pcxmast.h"

#include "opentyr.h"

const JE_byte pcxpal[PCX_NUM] = /* [1..PCXnum] */
{ 0, 7, 5, 8, 10, 5, 18, 19, 19, 20, 21, 22, 5};

/*FACEMAX*/
const JE_byte facepal[12] = /* [1..12] */
{ 1, 2, 3, 4, 6, 9, 11, 12, 16, 13, 14, 15};

JE_pcxpostype pcxpos;

