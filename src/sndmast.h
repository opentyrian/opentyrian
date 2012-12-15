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
#ifndef SNDMAST_H
#define SNDMAST_H

#include "opentyr.h"

#define SAMPLE_COUNT 38



enum
{
	S_NONE = 0,
	S_WEAPON_1,
	S_WEAPON_2,
	S_ENEMY_HIT,
	S_EXPLOSION_4,
	S_WEAPON_5,
	S_WEAPON_6,
	S_WEAPON_7,
	S_SELECT, // S_EXPLOSION_8
	S_EXPLOSION_9,
	S_WEAPON_10,
	S_EXPLOSION_11,
	S_EXPLOSION_12,
	S_WEAPON_13,
	S_WEAPON_14,
	S_WEAPON_15,
	S_SPRING,
	S_WARNING,
	S_ITEM,
	S_HULL_HIT,
	S_MACHINE_GUN,
	S_SOUL_OF_ZINGLON,
	S_EXPLOSION_22,
	S_CLINK,
	S_CLICK,
	S_WEAPON_25,
	S_WEAPON_26,
	S_SHIELD_HIT,
	S_CURSOR,
	S_POWERUP,
	V_CLEARED_PLATFORM, // 30
	V_BOSS,
	V_ENEMIES,
	V_GOOD_LUCK,
	V_LEVEL_END,
	V_DANGER,
	V_SPIKES,
	V_DATA_CUBE,
	V_ACCELERATE
};

extern const char soundTitle[SAMPLE_COUNT][9];
extern const JE_byte windowTextSamples[9];

#endif /* SNDMAST_H */

