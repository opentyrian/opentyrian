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
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "opentyr.h"

#include "SDL.h"

#define KEY_COMBO(mod, scancode) ((Uint32)(scancode) | \
	(((mod) & KMOD_SHIFT ? (Uint32)KMOD_SHIFT : 0) | \
	 ((mod) & KMOD_CTRL ? (Uint32)KMOD_CTRL : 0) | \
	 ((mod) & KMOD_ALT ? (Uint32)KMOD_ALT : 0) | \
	 ((mod) & KMOD_GUI ? (Uint32)KMOD_GUI : 0)) << 16)

typedef struct KeyboardInput
{
	SDL_Keycode sym;  // Used for secret text input (ex. super arcade codes).
	Uint16 scancode;  // SDL_Scancode; used for non-text input.
	Uint16 mod;  // SDL_Keymod
	Uint8 ch;  // CP437 character; used for text input (ex. save file name).
} KeyboardInput;

typedef struct MouseInput
{
	Sint32 x;
	Sint32 y;
	Uint8 button;
} MouseInput;

typedef enum InputFlags
{
	INPUT_ANY = 0,
	INPUT_NO_MOTION = 1,  // Ignore mouse motion.
} InputFlags;

extern JE_boolean ESCPressed;  // TODO: Implement this.

extern bool windowHasFocus;

extern bool keysactive[SDL_NUM_SCANCODES];

extern const SDL_Keycode lordKeySyms[4];
extern bool lordKeySymsDown[4];

extern Sint32 mouseX;
extern Sint32 mouseY;
extern Uint8 mouseButtonsDown;

void init_keyboard(void);

bool keyboardHasInput(void);
bool keyboardGetInput(KeyboardInput *out_input);
void keyboardClearInput(void);

bool mouseHasInput(InputFlags flags);
bool mouseGetInput(InputFlags flags, MouseInput *out_input);
void mouseClearInput(void);

void mouseSetRelative(bool enable);
void mouseGetRelativePosition(Sint32 *out_x, Sint32 *out_y);

void handleSdlEvents(void);

bool hasInput(InputFlags flags);
bool getInput(void);

void waitUntilHasInput(InputFlags flags);
void waitUntilGetInput(void);

void waitUntilElapsed(void);
bool waitUntilHasInputOrElapsed(void);
bool waitUntilGetInputOrElapsed(void);

#endif /* KEYBOARD_H */
