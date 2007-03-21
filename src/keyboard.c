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

JE_boolean newkey, newmouse, keydown, mousedown;
SDLKey lastkey_sym;
SDLMod lastkey_mod;
Uint8 lastmouse_but;
Uint16 lastmouse_x, lastmouse_y;
JE_boolean mouse_pressed[3] = {FALSE, FALSE, FALSE};
Uint16 mouse_x, mouse_y, mouse_xrel, mouse_yrel;

int numkeys;
Uint8 *keysactive;

void flush_events_buffer( void )
{
    SDL_Event ev;

    while (SDL_PollEvent(&ev));
}

void wait_keymouse( void )
{
    newkey = newmouse = FALSE;
    while (!newmouse && !newkey)
    {
        if (SDL_GetTicks() % 5 == 0)
        {
            service_SDL_events();
        }
    }
}

void init_keyboard( void )
{
    keysactive = SDL_GetKeyState(&numkeys);
}

void service_SDL_events( void )
{
    SDL_Event ev;

    while (SDL_PollEvent(&ev))
    {
        switch (ev.type)
        {
            case SDL_MOUSEMOTION:
                mouse_x = ev.motion.x;
                mouse_y = ev.motion.y;
                mouse_xrel = ev.motion.xrel;
                mouse_yrel = ev.motion.yrel;
                break;
            case SDL_KEYDOWN:
                newkey = TRUE;
                lastkey_sym = ev.key.keysym.sym;
                lastkey_mod = ev.key.keysym.mod;
                keydown = TRUE;
                break;
            case SDL_KEYUP:
                keydown = FALSE;
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                if (ev.type == SDL_MOUSEBUTTONDOWN)
                {
                    newmouse = TRUE;
                    lastmouse_but = ev.button.button;
                    lastmouse_x = ev.button.x;
                    lastmouse_y = ev.button.y;
                    mousedown = TRUE;
                } else {
                    mousedown = FALSE;
                }
                switch (ev.button.button)
                {
                    case SDL_BUTTON_LEFT:
                        mouse_pressed[0] = mousedown; break;
                    case SDL_BUTTON_RIGHT:
                        mouse_pressed[1] = mousedown; break;
                    case SDL_BUTTON_MIDDLE:
                        mouse_pressed[2] = mousedown; break;
                }
                break;
            /*case SDL_ACTIVEEVENT:
                if (ev.active.type & SDL_APPACTIVE || !ev.active.gain)
                {
                    sleep_game();
                }
                break;*/
            case SDL_QUIT:
                /* TODO: Call the cleanup code here. */
                break;
        }
    }
}

void sleep_game( void )
{
    SDL_Event ev;

    while (SDL_WaitEvent(&ev))
    {
        if (ev.type == SDL_ACTIVEEVENT /*&& ev.active.state & SDL_APPACTIVE*/ && ev.active.gain)
        {
            return;
        }
    }
}
