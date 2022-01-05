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
#ifndef MOUSE_H
#define MOUSE_H

#include "opentyr.h"

#include "SDL.h"

enum
{
	MOUSE_POINTER_NORMAL = 0,
	MOUSE_POINTER_UP,
	MOUSE_POINTER_DOWN,
	MOUSE_POINTER_LEFT,
	MOUSE_POINTER_RIGHT,
};

extern bool has_mouse;

extern bool mouseInactive;
extern JE_byte mouseCursor;
extern JE_word mouseX, mouseY, mouseButton;
extern JE_word mouseXB, mouseYB;

void JE_mouseStart( void );
void JE_mouseStartFilter(Uint8 filter);
void JE_mouseReplace( void );

#endif /* MOUSE_H */

