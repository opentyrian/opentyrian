/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) The OpenTyrian Development Team
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
#include "nortsong.h"
#include "opentyr.h"
#include "video.h"
#include "video_scale.h"

#include "SDL.h"

#include <stdio.h>

#define SDL_POLL_INTERVAL 10

JE_boolean ESCPressed;

bool windowHasFocus;

bool keysactive[SDL_NUM_SCANCODES];

// There are too many virtual keys, so just keep track of the few we need.
const SDL_Keycode lordKeySyms[] = { SDLK_l, SDLK_o, SDLK_r, SDLK_d };
bool lordKeySymsDown[4] = { 0 };

static KeyboardInput keyboardInputs[32];
static size_t keyboardInputsFront;
static size_t keyboardInputsBack;
static size_t keyboardInputsCount;

Sint32 mouseX;
Sint32 mouseY;
Uint8 mouseButtonsDown;

static MouseInput mouseInputs[4];
static size_t mouseInputsFront;
static size_t mouseInputsBack;
static size_t mouseInputsCount;
static bool mouseHasMotionInput;

static bool mouseRelativeEnabled;

// Relative mouse position in window coordinates.
static Sint32 mouseWindowXRelative;
static Sint32 mouseWindowYRelative;

// Mapping from CP437 to UCS for 0x80 to 0xA8.
static const Uint16 ucsMap[] =
{
	0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
	0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
	0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
	0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
	0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
	0x00BF,
};

void init_keyboard(void)
{
	SDL_StopTextInput();

	SDL_ShowCursor(SDL_FALSE);

#if SDL_VERSION_ATLEAST(2, 26, 0)
	SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SYSTEM_SCALE, "1");
#endif
}

bool keyboardHasInput(void)
{
	return keyboardInputsCount > 0;
}

bool keyboardGetInput(KeyboardInput *out_input)
{
	if (keyboardInputsCount > 0)
	{
		assert(keyboardInputsFront < COUNTOF(keyboardInputs));
		if (out_input != NULL)
			*out_input = keyboardInputs[keyboardInputsFront];
		keyboardInputsFront = keyboardInputsFront == COUNTOF(keyboardInputs) - 1 ? 0 : keyboardInputsFront + 1;
		keyboardInputsCount -= 1;
		return true;
	}

	return false;
}

void keyboardClearInput(void)
{
	keyboardInputsFront = 0;
	keyboardInputsBack = 0;
	keyboardInputsCount = 0;
}

bool mouseHasInput(InputFlags flags)
{
	return mouseInputsCount > 0 || ((flags & INPUT_NO_MOTION) == 0 && mouseHasMotionInput);
}

bool mouseGetInput(InputFlags flags, MouseInput *out_input)
{
	if (mouseInputsCount > 0)
	{
		assert(mouseInputsFront < COUNTOF(mouseInputs));
		if (out_input != NULL)
			*out_input = mouseInputs[mouseInputsFront];
		mouseInputsFront = mouseInputsFront == COUNTOF(mouseInputs) - 1 ? 0 : mouseInputsFront + 1;
		mouseInputsCount -= 1;
		return true;
	}

	if ((flags & INPUT_NO_MOTION) == 0 && mouseHasMotionInput)
	{
		if (out_input != NULL)
		{
			*out_input = (MouseInput)
			{
				.x = mouseX,
				.y = mouseY,
				.button = 0,
			};
		}
		mouseHasMotionInput = false;
		return true;
	}

	return false;
}

void mouseClearInput(void)
{
	mouseInputsFront = 0;
	mouseInputsBack = 0;
	mouseInputsCount = 0;

	mouseHasMotionInput = false;
}

void mouseSetRelative(bool enable)
{
	SDL_SetRelativeMouseMode(enable && windowHasFocus);

	mouseRelativeEnabled = enable;

	mouseWindowXRelative = 0;
	mouseWindowYRelative = 0;
}

void mouseGetRelativePosition(Sint32 *const out_x, Sint32 *const out_y)
{
	scaleWindowDistanceToScreen(&mouseWindowXRelative, &mouseWindowYRelative);
	*out_x = mouseWindowXRelative;
	*out_y = mouseWindowYRelative;

	mouseWindowXRelative = 0;
	mouseWindowYRelative = 0;
}

void handleSdlEvents(void)
{
	SDL_Event ev;

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
				if (ev.key.keysym.mod & KMOD_ALT &&
				    ev.key.keysym.scancode == SDL_SCANCODE_RETURN)
				{
					toggle_fullscreen();
					break;
				}

				if (!ev.key.repeat)
					keysactive[ev.key.keysym.scancode] = true;

				for (size_t i = 0; i < COUNTOF(lordKeySyms); ++i)
					lordKeySymsDown[i] |= ev.key.keysym.sym == lordKeySyms[i];

				if (keyboardInputsCount < COUNTOF(keyboardInputs))
				{
					assert(keyboardInputsBack < COUNTOF(keyboardInputs));
					KeyboardInput *const input = &keyboardInputs[keyboardInputsBack];
					input->sym = ev.key.keysym.sym;
					input->scancode = ev.key.keysym.scancode;
					input->mod = ev.key.keysym.mod;
					input->ch = 0;
					keyboardInputsBack = keyboardInputsBack == COUNTOF(keyboardInputs) - 1 ? 0 : keyboardInputsBack + 1;
					keyboardInputsCount += 1;
				}

				mouseInactive = true;
				break;

			case SDL_KEYUP:
				keysactive[ev.key.keysym.scancode] = false;

				for (size_t i = 0; i < COUNTOF(lordKeySyms); ++i)
					lordKeySymsDown[i] &= ev.key.keysym.sym != lordKeySyms[i];
				break;

			case SDL_TEXTINPUT:
				for (size_t i = 0; i < COUNTOF(ev.text.text); ++i)
				{
					// Decode codepoint from UTF-8.
					Uint16 cp = (unsigned char)ev.text.text[i];
					if (cp == 0)
					{
						break;
					}
					else if (cp < 0x80)
					{
						// ASCII.
					}
					else if (cp < 0xC0)
					{
						// Invalid.
						continue;
					}
					else if (cp < 0xE0)
					{
						if (i + 1 >= COUNTOF(ev.text.text))
							continue;

						cp &= 0x1F;
						cp = (cp << 6) | (ev.text.text[++i] & 0x3F);
					}
					else if (cp < 0xF0)
					{
						if (i + 2 >= COUNTOF(ev.text.text))
							continue;

						cp &= 0x0F;
						cp = (cp << 6) | (ev.text.text[++i] & 0x3F);
						cp = (cp << 6) | (ev.text.text[++i] & 0x3F);
					}
					else
					{
						// Outside the BMP.
						continue;
					}

					// Map codepoint to CP437 character.
					Uint8 ch = 0;
					if (cp < 0x80)
					{
						ch = cp;
					}
					else
					{
						for (size_t j = 0; j < COUNTOF(ucsMap); ++j)
						{
							if (cp == ucsMap[j])
							{
								ch = 0x80 + j;
								break;
							}
						}

						if (ch == 0)
							continue;
					}

					if (keyboardInputsCount < COUNTOF(keyboardInputs))
					{
						assert(keyboardInputsBack < COUNTOF(keyboardInputs));
						KeyboardInput *const input = &keyboardInputs[keyboardInputsBack];
						input->sym = -1;  // Text; not a key.
						input->scancode = -1;  // Text; not a key.
						input->mod = KMOD_NONE;
						input->ch = ch;
						keyboardInputsBack = keyboardInputsBack == COUNTOF(keyboardInputs) - 1 ? 0 : keyboardInputsBack + 1;
						keyboardInputsCount += 1;
					}
				}
				break;

			case SDL_MOUSEMOTION:
				mouseX = ev.motion.x;
				mouseY = ev.motion.y;
				mapWindowPointToScreen(&mouseX, &mouseY);

				mouseHasMotionInput = true;

				if (mouseRelativeEnabled && windowHasFocus)
				{
					mouseWindowXRelative += ev.motion.xrel;
					mouseWindowYRelative += ev.motion.yrel;
				}

				// Show system mouse pointer if outside screen.
				SDL_ShowCursor(mouseX < 0 || mouseX >= vga_width ||
				               mouseY < 0 || mouseY >= vga_height ? SDL_ENABLE : SDL_DISABLE);

				if (ev.motion.xrel != 0 || ev.motion.yrel != 0)
					mouseInactive = false;
				break;

			case SDL_MOUSEBUTTONDOWN:
				mapWindowPointToScreen(&ev.button.x, &ev.button.y);

				if (mouseInputsCount < COUNTOF(mouseInputs))
				{
					assert(mouseInputsBack < COUNTOF(mouseInputs));
					MouseInput *const input = &mouseInputs[mouseInputsBack];
					input->button = ev.button.button;
					input->x = ev.button.x;
					input->y = ev.button.y;
					mouseInputsBack = mouseInputsBack == COUNTOF(mouseInputs) - 1 ? 0 : mouseInputsBack + 1;
					mouseInputsCount += 1;
				}

				mouseButtonsDown |= SDL_BUTTON(ev.button.button);

				mouseInactive = false;
				break;

			case SDL_MOUSEBUTTONUP:
				mapWindowPointToScreen(&ev.button.x, &ev.button.y);

				mouseButtonsDown &= ~SDL_BUTTON(ev.button.button);
				break;

			case SDL_QUIT:
				exit(0);
				break;
		}
	}
}

bool hasInput(InputFlags flags)
{
	return keyboardHasInput() || mouseHasInput(flags);
}

bool getInput(void)
{
	return keyboardGetInput(NULL) || mouseGetInput(INPUT_NO_MOTION, NULL);
}

void waitUntilHasInput(InputFlags flags)
{
	while (true)
	{
		NETWORK_KEEP_ALIVE();

		push_joysticks_as_keyboard();
		handleSdlEvents();

		if (hasInput(flags))
			return;

		SDL_Delay(SDL_POLL_INTERVAL);
	}
}

void waitUntilGetInput(void)
{
	while (true)
	{
		NETWORK_KEEP_ALIVE();

		push_joysticks_as_keyboard();
		handleSdlEvents();

		if (getInput())
			return;

		SDL_Delay(SDL_POLL_INTERVAL);
	}
}

void waitUntilElapsed(void)
{
	while (true)
	{
		NETWORK_KEEP_ALIVE();

		push_joysticks_as_keyboard();
		handleSdlEvents();

		Uint32 delay = getFrameCountTicks();
		if (delay == 0)
			return;

		SDL_Delay(MIN(delay, SDL_POLL_INTERVAL));
	}
}

bool waitUntilHasInputOrElapsed(void)
{
	while (true)
	{
		NETWORK_KEEP_ALIVE();

		push_joysticks_as_keyboard();
		handleSdlEvents();

		if (hasInput(INPUT_NO_MOTION))
			return true;

		Uint32 delay = getFrameCountTicks();
		if (delay == 0)
			return false;

		SDL_Delay(MIN(delay, SDL_POLL_INTERVAL));
	}
}

bool waitUntilGetInputOrElapsed(void)
{
	while (true)
	{
		NETWORK_KEEP_ALIVE();

		push_joysticks_as_keyboard();
		handleSdlEvents();

		if (getInput())
			return true;

		Uint32 delay = getFrameCountTicks();
		if (delay == 0)
			return false;

		SDL_Delay(MIN(delay, SDL_POLL_INTERVAL));
	}
}
