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
#include "keyboard.h"

#include "joystick.h"
#include "mouse.h"
#include "network.h"
#include "opentyr.h"
#include "video.h"
#include "video_scale.h"

#include "SDL.h"

#include <stdio.h>

JE_boolean ESCPressed;

JE_boolean newkey, newmouse, keydown, mousedown;
SDL_Scancode lastkey_scan;
SDL_Keymod lastkey_mod;
Uint8 lastmouse_but;
Sint32 lastmouse_x, lastmouse_y;
JE_boolean mouse_pressed[3] = {false, false, false};
Sint32 mouse_x, mouse_y;

bool windowHasFocus;

Uint8 keysactive[SDL_NUM_SCANCODES];

bool new_text;
char last_text[SDL_TEXTINPUTEVENT_TEXT_SIZE];

static bool mouseRelativeEnabled;

// Relative mouse position in window coordinates.
static Sint32 mouseWindowXRelative;
static Sint32 mouseWindowYRelative;

void flush_events_buffer(void)
{
	SDL_Event ev;

	while (SDL_PollEvent(&ev));
}

void wait_input(JE_boolean keyboard, JE_boolean mouse, JE_boolean joystick)
{
	service_SDL_events(false);
	while (!((keyboard && keydown) || (mouse && mousedown) || (joystick && joydown)))
	{
		SDL_Delay(SDL_POLL_INTERVAL);
		push_joysticks_as_keyboard();
		service_SDL_events(false);

#ifdef WITH_NETWORK
		if (isNetworkGame)
			network_check();
#endif
	}
}

void wait_noinput(JE_boolean keyboard, JE_boolean mouse, JE_boolean joystick)
{
	service_SDL_events(false);
	while ((keyboard && keydown) || (mouse && mousedown) || (joystick && joydown))
	{
		SDL_Delay(SDL_POLL_INTERVAL);
		poll_joysticks();
		service_SDL_events(false);

#ifdef WITH_NETWORK
		if (isNetworkGame)
			network_check();
#endif
	}
}

void init_keyboard(void)
{
	//SDL_EnableKeyRepeat(500, 60); TODO Find if SDL2 has an equivalent.

	newkey = newmouse = false;
	keydown = mousedown = false;

	SDL_ShowCursor(SDL_FALSE);

#if SDL_VERSION_ATLEAST(2, 26, 0)
	SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SYSTEM_SCALE, "1");
#endif
}

void mouseSetRelative(bool enable)
{
	SDL_SetRelativeMouseMode(enable && windowHasFocus);

	mouseRelativeEnabled = enable;

	mouseWindowXRelative = 0;
	mouseWindowYRelative = 0;
}

JE_word JE_mousePosition(JE_word *mouseX, JE_word *mouseY)
{
	service_SDL_events(false);
	*mouseX = mouse_x;
	*mouseY = mouse_y;
	return mousedown ? lastmouse_but : 0;
}

void mouseGetRelativePosition(Sint32 *const out_x, Sint32 *const out_y)
{
	service_SDL_events(false);

	scaleWindowDistanceToScreen(&mouseWindowXRelative, &mouseWindowYRelative);
	*out_x = mouseWindowXRelative;
	*out_y = mouseWindowYRelative;

	mouseWindowXRelative = 0;
	mouseWindowYRelative = 0;
}

void service_SDL_events(JE_boolean clear_new)
{
	SDL_Event ev;

	if (clear_new)
	{
		newkey = false;
		newmouse = false;
		new_text = false;
	}

	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
			case SDL_WINDOWEVENT:
				switch (ev.window.event)
				{
				case SDL_WINDOWEVENT_FOCUS_LOST:
					windowHasFocus = false;

					mouseSetRelative(mouseRelativeEnabled);
					break;

				case SDL_WINDOWEVENT_FOCUS_GAINED:
					windowHasFocus = true;

					mouseSetRelative(mouseRelativeEnabled);
					break;

				case SDL_WINDOWEVENT_RESIZED:
					video_on_win_resize();
					break;
				}
				break;

			case SDL_KEYDOWN:
				/* <alt><enter> toggle fullscreen */
				if (ev.key.keysym.mod & KMOD_ALT && ev.key.keysym.scancode == SDL_SCANCODE_RETURN)
				{
					toggle_fullscreen();
					break;
				}

				keysactive[ev.key.keysym.scancode] = 1;

				newkey = true;
				lastkey_scan = ev.key.keysym.scancode;
				lastkey_mod = ev.key.keysym.mod;
				keydown = true;

				mouseInactive = true;
				return;

			case SDL_KEYUP:
				keysactive[ev.key.keysym.scancode] = 0;
				keydown = false;
				return;

			case SDL_MOUSEMOTION:
				mouse_x = ev.motion.x;
				mouse_y = ev.motion.y;
				mapWindowPointToScreen(&mouse_x, &mouse_y);

				if (mouseRelativeEnabled && windowHasFocus)
				{
					mouseWindowXRelative += ev.motion.xrel;
					mouseWindowYRelative += ev.motion.yrel;
				}

				// Show system mouse pointer if outside screen.
				SDL_ShowCursor(mouse_x < 0 || mouse_x >= vga_width ||
				               mouse_y < 0 || mouse_y >= vga_height ? SDL_TRUE : SDL_FALSE);

				if (ev.motion.xrel != 0 || ev.motion.yrel != 0)
					mouseInactive = false;
				break;

			case SDL_MOUSEBUTTONDOWN:
				mouseInactive = false;

				// fall through
			case SDL_MOUSEBUTTONUP:
				mapWindowPointToScreen(&ev.button.x, &ev.button.y);
				if (ev.type == SDL_MOUSEBUTTONDOWN)
				{
					newmouse = true;
					lastmouse_but = ev.button.button;
					lastmouse_x = ev.button.x;
					lastmouse_y = ev.button.y;
					mousedown = true;
				}
				else
				{
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

			case SDL_TEXTINPUT:
				SDL_strlcpy(last_text, ev.text.text, COUNTOF(last_text));
				new_text = true;
				break;

			case SDL_TEXTEDITING:
				break;

			case SDL_QUIT:
				/* TODO: Call the cleanup code here. */
				exit(0);
				break;
		}
	}
}

void JE_clearKeyboard(void)
{
	// /!\ Doesn't seems important. I think. D:
}
