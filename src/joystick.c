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
#include "joystick.h"

#include "keyboard.h"
#include "params.h"
#include "vga256d.h"


#include "SDL.h"

#ifndef TARGET_GP2X
const JE_ButtonAssign defaultJoyButtonAssign = {1, 4, 5, 5};
#else  /* TARGET_GP2X */
const JE_ButtonAssign defaultJoyButtonAssign = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 1, 4, 5, 5, 0, 0, 0};
#endif  /* TARGET_GP2X */

JE_ButtonType tempButton, button, joyButton;
JE_boolean buttonHeld;

JE_ButtonAssign joyButtonAssign;

JE_boolean useButtonAssign = false;

/* Joystick Data */
Sint16 joyX, joyY;
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
		forceAveraging = true;
	}

	for (i = 0; i < COUNTOF(joyButton); i++)
	{
		joyButton[i] = SDL_JoystickGetButton(joystick, i);
	}

#ifndef TARGET_GP2X
	joyX = SDL_JoystickGetAxis(joystick, 0);
	joyY = SDL_JoystickGetAxis(joystick, 1);
#else  /* TARGET_GP2X */
	joyX = - (joyButton[GP2X_VK_LEFT]  || joyButton[GP2X_VK_UP_LEFT]  || joyButton[GP2X_VK_DOWN_LEFT])
	       + (joyButton[GP2X_VK_RIGHT] || joyButton[GP2X_VK_UP_RIGHT] || joyButton[GP2X_VK_DOWN_RIGHT]);
	joyY = - (joyButton[GP2X_VK_UP]   || joyButton[GP2X_VK_UP_LEFT]   || joyButton[GP2X_VK_UP_LEFT])
	       + (joyButton[GP2X_VK_DOWN] || joyButton[GP2X_VK_DOWN_LEFT] || joyButton[GP2X_VK_DOWN_RIGHT]);
#endif  /* TARGET_GP2X */

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
	 * 4 : Fire Both Sidekicks
	 * 5 : Switch Rear Weapon
	 */

	memset(button, 0, sizeof(button));

	for (i = 0; i < COUNTOF(joyButton); i++)
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
	joystickInput = false;

	if (joystick_installed)
	{
		JE_joystick1();
#ifndef TARGET_GP2X
		memcpy(button, joyButton, sizeof(button));
#else  /* TARGET_GP2X */
		memcpy(button, joyButton + 12, 4);
#endif  /* TARGET_GP2X */
		/*JE_UpdateButtons;*/

#ifndef TARGET_GP2X
		if (!joyMax)
		{
			joystickUp    = joyY < -32768 / 2;
			joystickDown  = joyY > 32767 / 2;
			
			joystickLeft  = joyX < -32768 / 2;
			joystickRight = joyX > 32767 / 2;
		} else {
			joystickUp    = joyY < -32768 / 5;
			joystickDown  = joyY > 32767 / 5;
			
			joystickLeft  = joyX < -32768 / 5;
			joystickRight = joyX > 32767 / 5;
		}
		
		joystickInput = joystickUp || joystickDown || joystickLeft || joystickRight || button[0] || button[1] || button[2] || button[3];
#else  /* TARGET_GP2X */
		joystickLeft  = joyButton[GP2X_VK_LEFT]  || joyButton[GP2X_VK_UP_LEFT]  || joyButton[GP2X_VK_DOWN_LEFT];
		joystickRight = joyButton[GP2X_VK_RIGHT] || joyButton[GP2X_VK_UP_RIGHT] || joyButton[GP2X_VK_DOWN_RIGHT];
		joystickUp    = joyButton[GP2X_VK_UP]   || joyButton[GP2X_VK_UP_LEFT]   || joyButton[GP2X_VK_UP_LEFT];
		joystickDown  = joyButton[GP2X_VK_DOWN] || joyButton[GP2X_VK_DOWN_LEFT] || joyButton[GP2X_VK_DOWN_RIGHT];
		
		for (int i = 0; i < COUNTOF(joyButton) && joystickInput == 0; i++)
		{
			joystickInput = joystickInput || joyButton[i];
		}
#endif  /* TARGET_GP2X */
	}
}

JE_boolean JE_nextJoystickCheck( void )
{
	JE_joystick2();
	if (joystickInput)
	{
		if (joystickWait > 0)
			joystickWait--;
		
		for (int i = 0; i < COUNTOF(joyButton); i++)
		{
#ifndef TARGET_GP2X
			if (joyButton[i])
#else  /* TARGET_GP2X */
			if (joyButton[i] && i != GP2X_VK_LEFT && i != GP2X_VK_RIGHT && i != GP2X_VK_UP && i != GP2X_VK_DOWN)
#endif  /* TARGET_GP2X */
				return true;
		}
		
		if (joystickWait == 0)
		{
			joystickWait = joystickWaitMax;
			return true;
		}
	} else {
		joystickWait = 0;
	}
	return false;
}

JE_boolean JE_joystickTranslate( void )
{
	JE_boolean tempb;

	joystickUp = false;
	joystickDown = false;
	joystickLeft = false;
	joystickRight = false;

	tempb = JE_nextJoystickCheck();

	if (tempb)
	{
		if (joystickUp)
		{
			newkey = true;
			lastkey_char = lastkey_sym = SDLK_UP;
		}
		if (joystickDown)
		{
			newkey = true;
			lastkey_char = lastkey_sym = SDLK_DOWN;
		}
		if (joystickLeft)
		{
			newkey = true;
			lastkey_char = lastkey_sym = SDLK_LEFT;
		}
		if (joystickRight)
		{
			newkey = true;
			lastkey_char = lastkey_sym = SDLK_RIGHT;
		}
		if (button[0])
		{
			newkey = true;
			lastkey_char = lastkey_sym = SDLK_RETURN;
		}
		if (button[1])
		{
			newkey = true;
			lastkey_char = lastkey_sym = SDLK_ESCAPE;
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
				newkey = false;
			} else {
				buttonHeld = true;
				return true;
			}
		} else {
			buttonHeld = false;
			return true;
		}
	} else {
		buttonHeld = false;
	}

	return false;
}


void JE_joystickInit( void )
{
	joystick_installed = false;

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
					joystick_installed = true;
				}
#else  /* TARGET_GP2X */
				joystick_installed = true;
#endif  /* TARGET_GP2X */
			}
		}
	}
	
	memset(button, 0, sizeof(button));
	
	if (joystick_installed)
	{
		JE_joystick1();
		if (forceAveraging)
		{
			JE_joystick1();
			JE_joystick1();
			JE_joystick1();
		}

		joystickUp = false;
		joystickDown = false;
		joystickLeft = false;
		joystickRight = false;
	}
}
