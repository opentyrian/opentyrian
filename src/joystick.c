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
#include "params.h"
#include "vga256d.h"
#include "keyboard.h"

#define NO_EXTERNS
#include "joystick.h"
#undef NO_EXTERNS

#include "SDL.h"

const JE_ButtonAssign defaultJoyButtonAssign = {1, 4, 5, 5};

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

    if(isNetworkGame)
        forceAveraging = TRUE;

    joyX = SDL_JoystickGetAxis(joystick, 0);
    joyY = SDL_JoystickGetAxis(joystick, 1);

    for(i = 0; i < 4; i++)
       joyButton[i] = SDL_JoystickGetButton(joystick, i);

    if(forceAveraging)
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
    1 : Fire Normal
    2 : Fire Left Sidekick
    3 : Fire Right Sidekick
    4 : Switch Rear Weapon */

    memset(button, 0, sizeof(button));

    for(i = 0; i < 4; i++)
        switch(joyButtonAssign[i])
        {
            case 1:
                button[1] |= joyButton[i];
                break;
            case 2:
                button[2] |= joyButton[i];
                break;
            case 3:
                button[3] |= joyButton[i];
                break;
            case 4:
                button[2] |= joyButton[i];
                button[3] |= joyButton[i];
                break;
            case 5:
                button[4] |= joyButton[i];
                break;
        }
}

void JE_joystick2( void )
{
    joystickInput = FALSE;

    if(joystick_installed)
    {
        JE_joystick1();
        memcpy(button, joyButton, sizeof(button));
        /*JE_UpdateButtons;*/

        if(!joyMax)
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

        joystickInput = joystickUp | joystickDown | joystickLeft | joystickRight | button[0] | button[1] | button[2] | button[3];
    }
}

JE_boolean JE_nextJoystickCheck( void )
{
    JE_joystick2();
    if(joystickInput)
    {
        if(joystickWait > 0)
            joystickWait--;
        if(button[0] || button[1] || button[2] || button[3])
        {
            return(TRUE);
        } else {
            if(joystickWait == 0)
            {
                joystickWait = joystickWaitMax;
                return(TRUE);
            }
        }
    } else {
        joystickWait = 0;
    }
    return(FALSE);
}

JE_boolean JE_joystickTranslate( void )
{
    JE_boolean tempb;

    joystickUp = FALSE;
    joystickDown = FALSE;
    joystickLeft = FALSE;
    joystickRight = FALSE;

    tempb = JE_nextJoystickCheck();

    if(tempb)
    {
        if(joystickUp)
            lastkey_sym = SDLK_UP; /* scancode = 72; */
        if(joystickDown)
            lastkey_sym = SDLK_DOWN; /* scancode = 80; */
        if(joystickLeft)
            lastkey_sym = SDLK_LEFT; /* scancode = 75; */
        if(joystickRight)
            lastkey_sym = SDLK_RIGHT; /* scancode = 77; */
        if(button[0])
            lastkey_sym = SDLK_RETURN; /* k = 13; */
        if(button[1])
            lastkey_sym = SDLK_ESCAPE; /* k = 27; */
    }

    return(tempb);
}

JE_boolean JE_joystickNotHeld( void )
{
    if(JE_joystickTranslate())
    {

        if((k == 13) || (k == 27))
        {
            if(buttonHeld)
            {
                k = 0;
            } else  {
                buttonHeld = TRUE;
                return(TRUE);
            }
        } else {
            buttonHeld = FALSE;
            return(TRUE);
        }

    } else {
        buttonHeld = FALSE;
    }

    return(FALSE);
}


void JE_joystickInit( void )
{
    joystick_installed = FALSE;

    if(scanForJoystick)
    {
        SDL_InitSubSystem(SDL_INIT_JOYSTICK);
        if(SDL_NumJoysticks())
        {
            joystick = SDL_JoystickOpen(0);

            if(joystick)
            {
                if(SDL_JoystickNumButtons(joystick) >= 4 && SDL_JoystickNumAxes(joystick) >= 2)
                {
                    joystick_installed = TRUE;
                }
            }
        }
    }

    if(joystick_installed)
    {
        JE_joystick1();
        if(forceAveraging)
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
