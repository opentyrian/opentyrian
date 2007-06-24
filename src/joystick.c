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
#include "opentyr.h"

#include "keyboard.h"
#include "params.h"
#include "vga256d.h"

#define NO_EXTERNS
#include "joystick.h"
#undef NO_EXTERNS

#include "SDL.h"

#ifndef TARGET_GP2X
const JE_ButtonAssign defaultJoyButtonAssign = {1, 4, 5, 5};
#else
const JE_ButtonAssign defaultJoyButtonAssign = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 5, 5, 0, 0, 0};
#ifndef GP2X_VK_UP
#define GP2X_VK_UP              0
#define GP2X_VK_DOWN            4
#define GP2X_VK_LEFT            2
#define GP2X_VK_RIGHT           6
#define GP2X_VK_UP_LEFT         1
#define GP2X_VK_UP_RIGHT        7
#define GP2X_VK_DOWN_LEFT       3
#define GP2X_VK_DOWN_RIGHT      5
#define GP2X_VK_CLICK           18
#define GP2X_VK_FA              12
#define GP2X_VK_FB              13
#define GP2X_VK_FX              15
#define GP2X_VK_FY              14
#define GP2X_VK_FL              10
#define GP2X_VK_FR              11
#define GP2X_VK_START           8
#define GP2X_VK_SELECT          9
#define GP2X_VK_VOL_UP          16
#define GP2X_VK_VOL_DOWN        17
#endif  /* GP2X_VK_UP */
#endif  /* TARGET_GP2X */

JE_ButtonType tempButton, button, joyButton;
JE_boolean buttonHeld;

JE_ButtonAssign joyButtonAssign;

JE_boolean useButtonAssign = FALSE;

/* Joystick Data */
Sint16 jMinX, jMaxX, jMinY, jMaxY, jCenterX, jCenterY, joyX, joyY;
Sint16 lastJoyX,  lastJoyY;
Sint16 lastJoyXb, lastJoyYb;
Sint16 lastJoyXc, lastJoyYc;
Sint16 lastJoyXd, lastJoyYd;

JE_byte joystickError;
JE_boolean joystickUp, joystickDown, joystickLeft, joystickRight, joystickInput;
JE_boolean joystick_installed;
JE_word joystickWait, joystickWaitMax;

/*JE_byte SJoyCD;*/ /*NortVars anybutton check - problems with checking too fast*/

SDL_Joystick *joystick = NULL;

void JE_joystick1( void ) /* procedure to get x and y */
{
	int i;

	if (isNetworkGame)
	{
		forceAveraging = TRUE;
	}

	for (i = 0; i < COUNTOF(joyButton); i++)
	{
		joyButton[i] = SDL_JoystickGetButton(joystick, i);
	}

#ifndef TARGET_GP2X
	joyX = SDL_JoystickGetAxis(joystick, 0);
	joyY = SDL_JoystickGetAxis(joystick, 1);
#else
	joyX = - (joyButton[GP2X_VK_LEFT]  || joyButton[GP2X_VK_UP_LEFT]  || joyButton[GP2X_VK_DOWN_LEFT])
	       + (joyButton[GP2X_VK_RIGHT] || joyButton[GP2X_VK_UP_RIGHT] || joyButton[GP2X_VK_DOWN_RIGHT]);
	joyX = - (joyButton[GP2X_VK_UP]   || joyButton[GP2X_VK_UP_LEFT]   || joyButton[GP2X_VK_UP_LEFT])
	       + (joyButton[GP2X_VK_DOWN] || joyButton[GP2X_VK_DOWN_LEFT] || joyButton[GP2X_VK_DOWN_RIGHT]);
#endif

	if (forceAveraging)
	{
		lastJoyXd = lastJoyXc;
		lastJoyXc = lastJoyXb;
		lastJoyXb = lastJoyX;
		lastJoyX = joyX;
		joyX = (joyX + lastJoyX + lastJoyXb + lastJoyXc + lastJoyXd) / 5;

		lastJoyYd = lastJoyYc;
		lastJoyYc = lastJoyYb;
		lastJoyYb = lastJoyY;
		lastJoyY = joyY;
		joyY = (joyY + lastJoyY + lastJoyYb + lastJoyYc + lastJoyYd) / 5;
	}
}

void JE_updateButtons( void ) /* Uses ButtonAssign to find out */
{
	int i;

	/* Button Functions:
	 * 1 : Fire Normal
	 * 2 : Fire Left Sidekick
	 * 3 : Fire Right Sidekick
	 * 4 : Switch Rear Weapon
	 */

	memset(button, 0, sizeof(button));

	for (i = 0; i < 4; i++)
	{
		switch (joyButtonAssign[i])
		{
			case 1:
				button[1-1] |= joyButton[i];
				break;
			case 2:
				button[2-1] |= joyButton[i];
				break;
			case 3:
				button[3-1] |= joyButton[i];
				break;
			case 4:
				button[2-1] |= joyButton[i];
				button[3-1] |= joyButton[i];
				break;
			case 5:
				button[4-1] |= joyButton[i];
				break;
		}
	}
}

void JE_joystick2( void )
{
	joystickInput = FALSE;

	if (joystick_installed)
	{
		JE_joystick1();
#ifndef TARGET_GP2X
		memcpy(button, joyButton, sizeof(button));
#else
		memcpy(button, joyButton + 12, 4);
#endif
		/*JE_UpdateButtons;*/

#ifndef TARGET_GP2X
		if (!joyMax)
		{
			joystickUp    = joyY < (jCenterY - jCenterY / 2);
			joystickDown  = joyY > (jCenterY + jCenterY / 2);

			joystickLeft  = joyX < (jCenterX - jCenterY / 2);
			joystickRight = joyX > (jCenterX + jCenterX / 2);
		} else {
			joystickUp    = joyY < (jCenterY - jCenterY / 5);
			joystickDown  = joyY > (jCenterY + jCenterY / 5);

			joystickLeft  = joyX < (jCenterX - jCenterY / 5);
			joystickRight = joyX > (jCenterX + jCenterX / 5);
		}
#else
		joystickLeft  = joyButton[GP2X_VK_LEFT]  || joyButton[GP2X_VK_UP_LEFT]  || joyButton[GP2X_VK_DOWN_LEFT];
		joystickRight = joyButton[GP2X_VK_RIGHT] || joyButton[GP2X_VK_UP_RIGHT] || joyButton[GP2X_VK_DOWN_RIGHT];
		joystickUp    = joyButton[GP2X_VK_UP]   || joyButton[GP2X_VK_UP_LEFT]   || joyButton[GP2X_VK_UP_LEFT];
		joystickDown  = joyButton[GP2X_VK_DOWN] || joyButton[GP2X_VK_DOWN_LEFT] || joyButton[GP2X_VK_DOWN_RIGHT];
#endif

		joystickInput = joystickUp | joystickDown | joystickLeft | joystickRight | button[0] | button[1] | button[2] | button[3];
	}
}

JE_boolean JE_nextJoystickCheck( void )
{
	JE_joystick2();
	if (joystickInput)
	{
		if (joystickWait > 0)
			joystickWait--;
		if (button[0] || button[1] || button[2] || button[3])
		{
			return TRUE;
		} else {
			if (joystickWait == 0)
			{
				joystickWait = joystickWaitMax;
				return TRUE;
			}
		}
	} else {
		joystickWait = 0;
	}
	return FALSE;
}

JE_boolean JE_joystickTranslate( void )
{
	JE_boolean tempb;

	joystickUp = FALSE;
	joystickDown = FALSE;
	joystickLeft = FALSE;
	joystickRight = FALSE;

	tempb = JE_nextJoystickCheck();

	if (tempb)
	{
		if (joystickUp)
		{
			newkey = TRUE;
			lastkey_sym = SDLK_UP;
		}
		if (joystickDown)
		{
			newkey = TRUE;
			lastkey_sym = SDLK_DOWN;
		}
		if (joystickLeft)
		{
			newkey = TRUE;
			lastkey_sym = SDLK_LEFT;
		}
		if (joystickRight)
		{
			newkey = TRUE;
			lastkey_sym = SDLK_RIGHT;
		}
		if (button[0])
		{
			newkey = TRUE;
			lastkey_sym = SDLK_RETURN;
		}
		if (button[1])
		{
			newkey = TRUE;
			lastkey_sym = SDLK_ESCAPE;
		}
	}

	return tempb;
}

JE_boolean JE_joystickNotHeld( void )
{
	if (JE_joystickTranslate())
	{
		if (newkey && (lastkey_sym == SDLK_RETURN || lastkey_sym == SDLK_ESCAPE))
		{
			if (buttonHeld)
			{
				newkey = FALSE;
			} else {
				buttonHeld = TRUE;
				return TRUE;
			}
		} else {
			buttonHeld = FALSE;
			return TRUE;
		}
	} else {
		buttonHeld = FALSE;
	}

	return FALSE;
}


void JE_joystickInit( void )
{
	joystick_installed = FALSE;

	if (scanForJoystick)
	{
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK))
		{
			printf("Failed to initialize joystick: %s\n", SDL_GetError());
		}
		if (SDL_NumJoysticks())
		{
			joystick = SDL_JoystickOpen(0);

			if (joystick)
			{
#ifndef TARGET_GP2X
				if (SDL_JoystickNumButtons(joystick) >= 4 && SDL_JoystickNumAxes(joystick) >= 2)
				{
					joystick_installed = TRUE;
				}
#else
				joystick_installed = TRUE;
#endif
			}
		}
	}

	if (joystick_installed)
	{
		JE_joystick1();
		if (forceAveraging)
		{
			JE_joystick1();
			JE_joystick1();
			JE_joystick1();
		}

		jCenterX = joyX;
		jCenterY = joyY;
		button[0] = FALSE;
		button[1] = FALSE;
		button[2] = FALSE;
		button[3] = FALSE;
		joystickUp = FALSE;
		joystickDown = FALSE;
		joystickLeft = FALSE;
		joystickRight = FALSE;
	}
}

void joystick_init( void )
{
	isNetworkGame = FALSE;
}
