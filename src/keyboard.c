/* vim: set noet:
 *
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Development Team
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

#include "SDL.h"


JE_boolean ESCPressed;

JE_boolean newkey, newmouse, keydown, mousedown;
SDLKey lastkey_sym;
SDLMod lastkey_mod;
unsigned char lastkey_char;
Uint8 lastmouse_but;
Uint16 lastmouse_x, lastmouse_y;
JE_boolean mouse_pressed[3] = {false, false, false};
Uint16 mouse_x, mouse_y, mouse_xrel, mouse_yrel;

int numkeys;
Uint8 *keysactive;

void flush_events_buffer( void )
{
	SDL_Event ev;

	while (SDL_PollEvent(&ev));
}

void wait_input( JE_boolean keyboard, JE_boolean mouse, JE_boolean joystick )
{
	service_SDL_events(false);
	while (!((keyboard ? keydown : false) || (mouse ? mousedown : false) || (joystick ? button[0] : false)))
	{
		SDL_Delay(SDL_POLL_INTERVAL);
		if (joystick)
		{
			JE_joystick2();
		}
		service_SDL_events(false);
	}
}

void wait_noinput( JE_boolean keyboard, JE_boolean mouse, JE_boolean joystick )
{
	service_SDL_events(false);
	while ((keydown && keyboard) || (mousedown && mouse) || (button[0] && joystick))
	{
		SDL_Delay(SDL_POLL_INTERVAL);
		if (joystick)
		{
			JE_joystick2();
		}
		service_SDL_events(false);
	}
}

void init_keyboard( void )
{
	keysactive = SDL_GetKeyState(&numkeys);
	SDL_EnableKeyRepeat(500, 60);

	newkey = newmouse = false;
	keydown = mousedown = false;

	SDL_EnableUNICODE(1);

#ifdef NDEBUG
	SDL_WM_GrabInput(SDL_GRAB_ON);
#endif
}

void service_SDL_events( JE_boolean clear_new )
{
	SDL_Event ev;

	if (clear_new)
	{
		newkey = newmouse = false;
	}
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
				/* Emergency kill: Ctrl+Pause */
				if (ev.key.keysym.sym == SDLK_BACKSPACE && (ev.key.keysym.mod & KMOD_CTRL))
				{
					puts("\n\n\nCtrl+Backspace pressed. Doing emergency quit.\n");
					SDL_Quit();
					exit(1);
				}
				newkey = true;
				lastkey_sym = ev.key.keysym.sym;
				lastkey_mod = ev.key.keysym.mod;
				lastkey_char = ev.key.keysym.unicode;
				keydown = true;
				return;
			case SDL_KEYUP:
				keydown = false;
				return;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (ev.type == SDL_MOUSEBUTTONDOWN)
				{
					newmouse = true;
					lastmouse_but = ev.button.button;
					lastmouse_x = ev.button.x;
					lastmouse_y = ev.button.y;
					mousedown = true;
				} else {
					mousedown = false;
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
				exit(0);
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

void JE_clearKeyboard( void )
{
	// /!\ Doesn't seems important. I think. D:
}
