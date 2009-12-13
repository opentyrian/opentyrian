/* 
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
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
#ifndef PLAYER_H
#define PLAYER_H

#include "config.h"
#include "opentyr.h"

enum
{
	FRONT_WEAPON = 0,
	REAR_WEAPON = 1
};

enum
{
	LEFT_SIDEKICK = 0,
	RIGHT_SIDEKICK = 1
};

typedef struct
{
	uint ship;
	uint generator;
	uint shield;
	struct { uint id; uint power; } weapon[2];
	uint sidekick[2];
	uint special;
	
	// Dragonwing only:
	// repeatedly collecting the same powerup gives a series of sidekick upgrades
	uint sidekick_series;
	uint sidekick_level;
	
	// Single-player only
	uint super_arcade_mode;  // stored as an item for compatibility :(
}
PlayerItems;

typedef struct
{
	ulong cash;
	
	PlayerItems items, last_items;
	
	bool is_dragonwing;  // i.e., is player 2
	uint *lives;
	
	// calculatables
	uint shield_max;
	uint initial_armor;
	
	// state
	bool is_alive;
	uint invulnerable_ticks;
	uint exploding_ticks;
	uint shield;
	uint armor;
	uint weapon_mode;
	uint superbombs;
	uint purple_balls_needed;
	
	int x, y;
	int old_x[20], old_y[20];
	
	int delta_x_shot_move, delta_y_shot_move;
	
	int last_x_shot_move, last_y_shot_move;
	int last_x_explosion_follow, last_y_explosion_follow;
}
Player;

extern Player player[2];

static inline bool all_players_dead( void )
{
	return (!player[0].is_alive && (!twoPlayerMode || !player[1].is_alive));
}

static inline bool all_players_alive( void )
{
	return (player[0].is_alive && (!twoPlayerMode || player[1].is_alive));
}

void calc_purple_balls_needed( Player * );
bool power_up_weapon( Player *, uint port );
void handle_got_purple_ball( Player * );

#endif // PLAYER_H
