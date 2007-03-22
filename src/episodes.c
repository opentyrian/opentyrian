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
JE_WeaponPortType *WeaponPort;
JE_WeaponType     *Weapons;

/* Items */
JE_PowerType   *PowerSys;
JE_ShipType    *Ships;
JE_OptionType  *Options;
JE_ShieldType  *Shields;
JE_SpecialType *Special;

/* Enemy data */
JE_EnemyDatType *EnemyDat;

/* EPISODE variables */
JE_byte    EpisodeNum = 0;
JE_boolean EpisodeAvail[EpisodeMax]; /* [1..episodemax] */
char       MacroFile[13], CubeFile[13]; /* string [12] */

JE_longint Episode1DataLoc;

/* Tells the game whether the level currently loaded is a bonus level. */
JE_boolean BonusLevel;

/* Tells if the game jumped back to Episode 1 */
JE_boolean JumpBacktoEpisode1;
