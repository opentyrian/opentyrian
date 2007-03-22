/*
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
#ifndef LVLMAST_H
#define LVLMAST_H

#include "opentyr.h"

#define EventMaximum 2500

#define WeapNum    780
#define PortNum    42
#define ArmorNum   4
#define PowerNum   6
#define EngineNum  6
#define OptionNum  30
#define ShipNum    13
#define ShieldNum  10
#define SpecialNum 46

#define EnemyNum   850

#define LVLnum  (18 * 2)
#define LVLnum2 (12 * 2)
#define LVLnum3 (12 * 2)
#define LVLnum4 (20 * 2)
#define LVLnum5 (1 * 2)

typedef JE_longint JE_lvlpostype[LVLnum + 2]; /* [1..LVLnum + 2] */

#ifndef NO_EXTERNS
extern const JE_char ShapeFile[34]; /* [1..34] */
extern const char lvlfile[LVLnum][9]; /* [1..LVLnum] of string [8] */
extern const char lvlfile2[LVLnum2][9]; /* [1..LVLnum2] of string [8] */
extern const char lvlfile3[LVLnum3][9]; /* [1..LVLnum3] of string [8] */
extern const char lvlfile4[LVLnum4][9]; /* [1..LVLnum4] of string [8] */
extern const char lvlfile5[LVLnum5][9]; /* [1..lvlnum5] of string [8] */
extern JE_lvlpostype lvlpos;
#endif

#endif /* LVLMAST_H */
