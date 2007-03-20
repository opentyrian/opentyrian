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

#define NO_EXTERNS
#include "nortsong.h"
#undef NO_EXTERNS

#include "SDL.h"

Uint32 target;

void setdelay( JE_byte delay )
{
    target = (delay << 4)+SDL_GetTicks(); /* delay << 4 == delay * 16 */
}

void waitdelay( void )
{
    int ticks;

    while ((ticks = SDL_GetTicks()) < target)
    {
        if (ticks % 5 == 0)
        {
            service_SDL_events();
        }
    }
}

void waitdelayorkey( void )
{
    int ticks;

    newkey = newmouse = FALSE;
    while ((ticks = SDL_GetTicks()) < target && !newmouse && !newkey)
    {
        if (ticks % 5 == 0)
        {
            service_SDL_events();
        }
    }
}
