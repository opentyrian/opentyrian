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
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "opentyr.h"

#include "SDL.h"


#define SDL_POLL_INTERVAL 5

#ifndef NO_EXTERNS
extern const char *keyNames[];
extern JE_boolean ESCPressed;
extern JE_boolean newkey, newmouse, keydown, mousedown;
extern SDLKey lastkey_sym;
extern SDLMod lastkey_mod;
extern unsigned char lastkey_char;
extern Uint8 lastmouse_but;
extern Uint16 lastmouse_x, lastmouse_y;
extern JE_boolean mouse_pressed[3];
extern Uint16 mouse_x, mouse_y, mouse_xrel, mouse_yrel;
extern int numkeys;
extern Uint8 *keysactive;
#endif

void flush_events_buffer( void );
void wait_input( JE_boolean keyboard, JE_boolean mouse, JE_boolean joystick );
void wait_noinput( JE_boolean keyboard, JE_boolean mouse, JE_boolean joystick );
void init_keyboard( void );
void service_SDL_events( JE_boolean clear_new );
void sleep_game( void );

void JE_clearKeyboard( void );

#endif /* KEYBOARD_H */
