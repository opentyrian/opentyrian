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
#include "opentyr.h"

#define NO_EXTERNS
#include "episodes.h"
#undef NO_EXTERNS

/* MAIN Weapons Data */
JE_WeaponPortType *weaponPort;
JE_WeaponType     *weapons;

/* Items */
JE_PowerType   *powerSys;
JE_ShipType    *ships;
JE_OptionType  *options;
JE_ShieldType  *shields;
JE_SpecialType *special;

/* Enemy data */
JE_EnemyDatType *enemyDat;

/* EPISODE variables */
JE_byte    episodeNum = 0;
JE_boolean episodeAvail[EpisodeMax]; /* [1..episodemax] */
char       macroFile[13], CubeFile[13]; /* string [12] */

JE_longint episode1DataLoc;

/* Tells the game whether the level currently loaded is a bonus level. */
JE_boolean bonusLevel;

/* Tells if the game jumped back to Episode 1 */
JE_boolean jumpBackToEpisode1;
