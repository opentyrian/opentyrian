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
#include "vga256d.h"
#include "keyboard.h"
#include "joystick.h"
#include "nortvars.h"

#include "setup.h"

#include "SDL.h"

int haltGame = 0, netQuit = 0; /* placeholders */

void JE_textMenuWait( JE_word *waitTime, JE_boolean doGamma )
{
#ifdef NDEBUG /* Disable mouse grabbing/etc in debug builds */
    SDL_WarpMouse(160, 100);
#endif
    lastkey_sym = 0;

    do {
        JE_ShowVGA();

        /*waitretrace;*/

        service_SDL_events();

        inputDetected = keydown | mousedown | JE_joystickNotHeld();

        if(lastkey_sym == SDLK_SPACE)
            lastkey_sym = SDLK_RETURN;

        if(mousedown)
            lastkey_sym = SDLK_RETURN;

#ifdef NDEBUG
        if(mouse_installed)
        {
            if(abs(mouse_y - 100) > 10)
            {
                inputDetected = TRUE;
                if(mouse_y - 100 < 0)
                    lastkey_sym = SDLK_UP;
                else
                    lastkey_sym = SDLK_DOWN;
            }
            if(abs(mouse_x - 160) > 10)
            {
                inputDetected = TRUE;
                if(mouse_x - 160 < 0)
                    lastkey_sym = SDLK_LEFT;
                else
                    lastkey_sym = SDLK_RIGHT;
            }
        }
#endif

        if(*waitTime > 0)
            *waitTime--;

    } while(!inputDetected && (*waitTime != 1) && !haltGame && !netQuit);
}

/* TODO */
