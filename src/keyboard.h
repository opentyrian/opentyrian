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
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "opentyr.h"

#include "SDL.h"

#ifndef NO_EXTERNS
extern const char *KeyNames[];
extern JE_boolean newkey;
extern SDLKey key, nkey;
extern int numkeys;
extern Uint8 *keysactive;
#endif

void JE_InitKeyboard( void );
void JE_EndKeyboard( void );
void JE_ClearKeyboard( void );
void JE_WaitForKey( void );
void JE_FlushBIOSBuffer( void );

#endif /* KEYBOARD_H */
