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
#include "opentyr.h"

JE_boolean timMode         = FALSE;
JE_boolean richMode        = FALSE;
JE_boolean recordDemo      = FALSE;

JE_boolean robertWeird     = TRUE;
JE_boolean constantPlay    = FALSE;
JE_boolean constantDie;

JE_boolean scanForJoystick = TRUE;
JE_boolean useBios         = FALSE;
JE_boolean NOSOUND         = FALSE;
JE_boolean quikJuke        = FALSE;
JE_boolean noRetrace       = FALSE;
JE_boolean joyMax          = FALSE;
JE_boolean forceAveraging  = TRUE;

JE_boolean forceMaxVolume  = FALSE;
JE_boolean soundInfoReport = FALSE;
JE_boolean stupidWindows;

/* Special Note:
 * The two booleans to detect network play for Tim's stuff.
 * It's in here because of the dumb port 60 bug.
 */
JE_boolean isNetworkGame, isNetworkActive;

JE_boolean tyrianXmas;

/* TODO */
