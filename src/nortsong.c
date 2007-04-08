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

#include "keyboard.h"
#include "joystick.h"

#define NO_EXTERNS
#include "nortsong.h"
#undef NO_EXTERNS

#include "SDL.h"

Uint32 target;

void setdelay( JE_byte delay )
{
    target = (delay << 4)+SDL_GetTicks(); /* delay << 4 == delay * 16 */
}

INLINE int delaycount( void )
{
    return (SDL_GetTicks() < target ? target - SDL_GetTicks() : 0);
}

void wait_delay( void )
{
    Uint32 ticks;

    while ((ticks = SDL_GetTicks()) < target)
    {
        if (ticks % SDL_POLL_INTERVAL == 0)
        {
            service_SDL_events();
        }
    }
}

void wait_delayorinput( JE_boolean keyboard, JE_boolean mouse, JE_boolean joystick )
{
    Uint32 ticks;
    keyboard = !keyboard;
    mouse = !mouse;

    service_SDL_events();
    while ((ticks = SDL_GetTicks()) < target && !(keydown||(keyboard)) && !(mousedown||(mouse)) && !(button[0]||(!joystick)))
    {
        if (ticks % SDL_POLL_INTERVAL == 0)
        {
            if (joystick)
            {
                JE_joystick2();
            }
            service_SDL_events();
        }
    }
}

void JE_playSampleNum( JE_byte samplenum )
{
    /* TODO dummy function */
}
