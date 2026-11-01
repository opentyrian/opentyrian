/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) The OpenTyrian Development Team
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

#ifndef HOOKS_H
#define HOOKS_H

/* Optional callbacks for code linked into the game, such as testing and
 * debugging tools, or another implementation of the level loop.  They are all
 * NULL unless such code sets them; the game itself never does. */

/* What a level_tick_substitute callback did, and so where JE_main goes
 * next. */
typedef enum
{
	/* The callback didn't substitute for the tick.  JE_main runs the tick, as
	 * it does without the callback. */
	LEVEL_TICK_NOT_SUBSTITUTED,
	/* The callback ran the tick in place of JE_main, and the level goes on.
	 * JE_main skips the tick, both its simulation and its rendering, and goes
	 * back to level_loop for the next tick. */
	LEVEL_TICK_SUBSTITUTED,
	/* The callback ran the tick in place of JE_main, and the tick ended the
	 * level.  JE_main skips the tick, both its simulation and its rendering,
	 * and goes to start_level, which ends the level (calling level_ended) and
	 * goes on to whatever comes next. */
	LEVEL_TICK_STOPPING,
} TickSubstituteResult;

typedef struct
{
	void (*level_initialized)(void);  // level loaded, before its first tick

	/* Called at the top of level_loop, before each tick.  A callback that
	 * only follows the game's progress returns LEVEL_TICK_NOT_SUBSTITUTED.
	 * One can also substitute for the tick, running in place of JE_main what
	 * JE_main does from the level_loop label to its jump back to level_loop,
	 * or to start_level if the level ends: the tick's simulation (its game
	 * logic), its rendering, the wait for the frame deadline, presenting the
	 * frame and polling events, and the pause screen and in-game menu and help
	 * if they open.
	 *
	 * Returning LEVEL_TICK_SUBSTITUTED or LEVEL_TICK_STOPPING makes JE_main
	 * skip all of the tick, both its simulation and its rendering, so a
	 * callback that returns one of them must have done all of it, and left
	 * the game's globals as JE_main would have left them.  JE_main's local
	 * variables (the weapon power that the HUD last drew) are left as they
	 * were, so a callback that substitutes for one of a level's ticks should
	 * substitute for all of them. */
	TickSubstituteResult (*level_tick_substitute)(void);

	void (*level_ended)(void);  // level_loop exited, before the level is torn down
} GameHooks;

extern GameHooks game_hooks;

#endif /* HOOKS_H */
