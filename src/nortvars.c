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
#include "opentyr.h"
#include "nortvars.h"

/* File constants for Saving ShapeFile */
const JE_byte NV_shapeactive   = 0x01;
const JE_byte NV_shapeinactive = 0x00;

JE_boolean ScanForJoystick;
JE_word z, y;
JE_boolean InputDetected;
JE_word LastMouseX, LastMouseY;

/*Mouse Data*/  /*Mouse_Installed is in VGA256d*/
JE_byte MouseCursor;
JE_boolean Mouse_ThreeButton;
JE_word MouseX, MouseY, MouseButton;

/* TODO */
