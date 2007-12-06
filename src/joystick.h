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
#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "opentyr.h"

#ifndef TARGET_GP2X
typedef JE_boolean JE_ButtonType[4]; /* [1..4] */
typedef JE_byte JE_ButtonAssign[4]; /* [1..4] */
#else  /* TARGET_GP2X */
typedef JE_boolean JE_ButtonType[19];
typedef JE_byte JE_ButtonAssign[19];

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

extern const JE_ButtonAssign defaultJoyButtonAssign;
extern JE_ButtonType tempButton, button, joyButton;
extern JE_boolean buttonHeld;
extern JE_ButtonAssign joyButtonAssign;
extern JE_boolean useButtonAssign;
extern Sint16 jMinX, jMaxX, jMinY, jMaxY, jCenterX, jCenterY, joyX, joyY;
extern JE_byte joystickError;
extern JE_boolean joystickUp, joystickDown, joystickLeft, joystickRight, joystickInput;
extern JE_boolean joystick_installed;
extern JE_word joystickWait, joystickWaitMax;

void JE_joystick1( void ); /*procedure to get JoyX, JoyY, Button1, Button2 of Joystick 1*/
void JE_joystick2( void );
JE_boolean JE_nextJoystickCheck( void );
JE_boolean JE_joystickTranslate( void );
void JE_joystickInit( void );
JE_boolean JE_joystickNotHeld( void );
void JE_updateButtons( void ); /*Uses ButtonAssign to find out*/

void joystick_init( void );

#endif /* JOYSTICK_H */
