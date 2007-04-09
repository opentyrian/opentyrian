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
#ifndef SHPMAST_H
#define SHPMAST_H

#include "opentyr.h"

#define SHP_NUM 12

typedef JE_longint JE_shppostype[SHP_NUM + 1]; /* [1..SHPnum + 1] */

#ifndef NO_EXTERNS
extern const JE_string shpfile[SHP_NUM];
extern JE_shppostype shppos;
#endif

#endif /* SHPMAST_H */
