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
#include "keyboard.h"
#undef NO_EXTERNS

#include "SDL.h"

const char *KeyNames[] = {
    "ESC",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "DASH",
    "EQUALS",
    "BACKSPACE",
    "TAB",
    "Q",
    "W",
    "E",
    "R",
    "T",
    "Y",
    "U",
    "I",
    "O",
    "P",
    "LEFT BRACKET",
    "RIGHT BRACKET",
    "ENTER",
    "CONTROL",
    "A",
    "S",
    "D",
    "F",
    "G",
    "H",
    "J",
    "K",
    "L",
    "SEMI-COLON",
    "SINGLE QUOTE",
    "REVERSE QUOTE",
    "SHIFT",
    "BACKSPACE",
    "Z",
    "X",
    "C",
    "V",
    "B",
    "N",
    "M",
    "COMMA",
    "PERIOD",
    "/",
    "RIGHT SHIFT",
    "*",
    "ALT",
    "SPACEBAR",
    "CAPSLOCK",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "NUMLOCK",
    "SCROLL LOCK",
    "HOME",
    "UP",
    "PAGE UP",
    "-",
    "LEFT",
    "KEYPAD CENTER",
    "RIGHT",
    "+",
    "END",
    "DOWN",
    "PAGE DOWN",
    "INSERT",
    "DEL",
    "??",
    "??",
    "??",
    "F11",
    "F12"
};

JE_boolean newkey;
JE_byte key, nkey;

int numkeys;
Uint8 *keysactive; /* Remember to call SDL_PumpEvents first! */

void updatekeys( void )
{
    SDL_Event ev;

    while (SDL_PollEvent(&ev))
    {
        if (ev.type == SDL_KEYDOWN)
        {
            newkey = TRUE;
            nkey = key = ev.key.keysym.sym;
        }
    }
}

void JE_FlushBIOSBuffer( void )
{
    SDL_Event ev;

    while (SDL_PollEvent(&ev));
}

void JE_WaitForKey( void )
{
    SDL_Event ev;

    JE_FlushBIOSBuffer();
    while (SDL_WaitEvent(&ev))
    {
        if (ev.type == SDL_KEYDOWN)
        {
            break;
        }
    }
    JE_FlushBIOSBuffer();
}

void JE_InitKeyboard( void )
{
    keysactive = SDL_GetKeyState(&numkeys);
}

void JE_EndKeyboard( void )
{
    keysactive = NULL;
    numkeys = 0;
}

void JE_ClearKeyboard( void )
{
    /* Dunno, nop for now. =] */
}
