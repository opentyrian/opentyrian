/* 
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
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

#include "config.h"
#include "joystick.h"
#include "keyboard.h"
#include "nortsong.h"
#include "opentyr.h"
#include "params.h"
#include "varz.h"
#include "video.h"

#include <assert.h>

int joystick_axis_threshold( int j, int value );
int check_assigned( SDL_Joystick *joystick_handle, const struct joystick_assignment_struct assignment[2] );

int joystick_repeat_delay = 300; // milliseconds, repeat delay for buttons
bool joydown = false;            // any joystick buttons down, updated by poll_joysticks()
bool ignore_joystick = false;

int joysticks = 0;
struct joystick_struct *joystick = NULL;

const static char joystick_cfg_version = 0;
const static int joystick_analog_max = 32767;

// eliminates axis movement below the threshold
int joystick_axis_threshold( int j, int value )
{
	assert(j < joysticks);
	
	bool negative = value < 0;
	if (negative)
		value = -value;
	
	if (value <= joystick[j].threshold * 1000)
		return 0;
	
	value -= joystick[j].threshold * 1000;
	
	return negative ? -value : value;
}

// converts joystick axis to sane Tyrian-usable value
int joystick_axis_reduce( int j, int value )
{
	assert(j < joysticks);
	
	value = joystick_axis_threshold(j, value);
	
	if (value == 0)
		return 0;
	
	return value / (3000 - 200 * joystick[j].sensitivity);
}

// converts analog joystick axes to an angle
// returns false if axes are centered (there is no angle)
bool joystick_analog_angle( int j, float *angle )
{
	assert(j < joysticks);
	
	float x = joystick_axis_threshold(j, joystick[j].x), y = joystick_axis_threshold(j, joystick[j].y);
	
	if (x != 0)
	{
		*angle += atan(-y / x);
		*angle += (x < 0) ? -M_PI_2 : M_PI_2;
		return true;
	}
	else if (y != 0)
	{
		*angle += y < 0 ? M_PI : 0;
		return true;
	}
	
	return false;
}

int check_assigned( SDL_Joystick *joystick_handle, const struct joystick_assignment_struct assignment[2] )
{
	int result = 0;
	
	for (int i = 0; i < 2; i++)
	{
		if (assignment[i].num == -1)
			continue;
		
		int temp;
		
		if (assignment[i].is_axis)
		{
			temp = SDL_JoystickGetAxis(joystick_handle, assignment[i].num);
			if (assignment[i].axis_negative)
				temp = -temp;
		}
		else // assignment is button
		{
			temp = SDL_JoystickGetButton(joystick_handle, assignment[i].num) == 1 ? joystick_analog_max : 0;
		}
		
		if (temp > result)
			result = temp;
	}
	
	return result;
}

void poll_joystick( int j )
{
	assert(j < joysticks);
	
	if (joystick[j].handle == NULL)
		return;
	
	SDL_JoystickUpdate();
	
	joystick[j].input_pressed = false;
	
	bool repeat = joystick[j].joystick_delay < SDL_GetTicks();
	
	for (int d = 0; d < COUNTOF(joystick[j].direction); d++)
	{
		bool old = joystick[j].direction[d];
		
		joystick[j].analog_direction[d] = check_assigned(joystick[j].handle, joystick[j].assignment[d]);
		joystick[j].direction[d] = joystick[j].analog_direction[d] > (joystick_analog_max / 2);
		joydown |= joystick[j].direction[d];
		
		joystick[j].direction_pressed[d] = joystick[j].direction[d];
		joystick[j].direction_pressed[d] &= (!old || repeat);
		joystick[j].input_pressed |= joystick[j].direction_pressed[d];
	}
	
	joystick[j].x = -joystick[j].analog_direction[3] + joystick[j].analog_direction[1];
	joystick[j].y = -joystick[j].analog_direction[0] + joystick[j].analog_direction[2];
	
	for (int d = 0; d < COUNTOF(joystick[j].action); d++)
	{
		bool old = joystick[j].action[d];
		
		joystick[j].action[d] = check_assigned(joystick[j].handle, joystick[j].assignment[d + COUNTOF(joystick[j].direction)]);
		joydown |= joystick[j].action[d];
		
		joystick[j].action_pressed[d] = joystick[j].action[d];
		joystick[j].action_pressed[d] &= (!old || repeat);
		joystick[j].input_pressed |= joystick[j].action_pressed[d];
	}
	
	joystick[j].confirm = joystick[j].action[0] || joystick[j].action[4];
	joystick[j].cancel = joystick[j].action[1] || joystick[j].action[5];
	
	if (joystick[j].input_pressed) {
		joystick[j].joystick_delay = SDL_GetTicks() + joystick_repeat_delay;
	}
}

void poll_joysticks( void )
{
	joydown = false;
	
	for (int j = 0; j < joysticks; j++)
	{
		poll_joystick(j);
	}
}

// sends SDL KEYDOWN and KEYUP events for a key
void push_key( SDLKey key )
{
	SDL_Event e;
	
	memset(&e.key.keysym, 0, sizeof(e.key.keysym));
	
	e.key.keysym.sym = key;
	e.key.keysym.unicode = key;
	
	e.key.state = SDL_RELEASED;
	
	e.type = SDL_KEYDOWN;
	SDL_PushEvent(&e);
	
	e.type = SDL_KEYUP;
	SDL_PushEvent(&e);
}

// helps us be lazy by pretending the joystick is a keyboard
void push_joysticks_as_keyboard( void )
{
	const SDLKey confirm = SDLK_RETURN, cancel = SDLK_ESCAPE;
	const SDLKey direction[4] = { SDLK_UP, SDLK_RIGHT, SDLK_DOWN, SDLK_LEFT };
	
	poll_joysticks();
	
	for (int j = 0; j < joysticks; j++)
	{
		if (!joystick[j].input_pressed)
			continue;
		
		if (joystick[j].confirm)
			push_key(confirm);
		if (joystick[j].cancel)
			push_key(cancel);
		
		for (int d = 0; d < COUNTOF(joystick[j].direction_pressed); d++)
		{
			if (joystick[j].direction_pressed[d])
				push_key(direction[d]);
		}
	}
}

// initializes SDL joystick system and loads assignments for joysticks found
void init_joysticks( void )
{
	if (ignore_joystick)
		return;
	
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK))
	{
		printf("warning: failed to initialize joystick system: %s\n", SDL_GetError());
		ignore_joystick = true;
		return;
	}
	
	SDL_JoystickEventState(SDL_IGNORE);
	
	joysticks = SDL_NumJoysticks();
	joystick = malloc(joysticks * sizeof(*joystick));
	
	for (int j = 0; j < joysticks; j++)
	{
		memset(&joystick[j], 0, sizeof(*joystick));
		
		joystick[j].handle = SDL_JoystickOpen(j);
		if (joystick[j].handle != NULL)
		{
			printf("joystick detected: %s ", SDL_JoystickName(j));
			printf("(%d axes, %d buttons)\n", SDL_JoystickNumAxes(joystick[j].handle), SDL_JoystickNumButtons(joystick[j].handle));
			
			if (!load_joystick_assignments(j))
				reset_joystick_assignments(j);
		}
	}
	
	if (joysticks == 0)
		printf("no joysticks detected\n");
}

// deinitializes SDL joystick system and saves joystick assignments
void deinit_joysticks( void )
{
	if (ignore_joystick)
		return;
	
	for (int j = 0; j < joysticks; j++)
	{
		if (joystick[j].handle != NULL)
		{
			save_joystick_assignments(j);
			SDL_JoystickClose(joystick[j].handle);
		}
	}
	
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

void reset_joystick_assignments( int j )
{
	assert(j < joysticks);
	
	// defaults: first 2 axes, first 6 buttons
	for (int a = 0; a < COUNTOF(joystick[j].assignment); a++)
	{
		joystick[j].assignment[a][0].is_axis = a < 4;
		joystick[j].assignment[a][0].axis_negative = a == 0 || a == 3;
		joystick[j].assignment[a][0].num = a < 4 ? (a + 1) % 2 : a - 4;
		
		for (int i = 1; i < COUNTOF(joystick[j].assignment[a]); i++)
		{
			joystick[j].assignment[a][i].num = -1;
		}
	}
	
	joystick[j].analog = false;
	joystick[j].sensitivity = 5;
	joystick[j].threshold = 5;
}

bool load_joystick_assignments( int j )
{
	FILE *f = seek_joystick_assignments(j, true);
	
	if (f == NULL)
		return false;
	
	joystick[j].analog = fgetc(f);
	joystick[j].sensitivity = fgetc(f);
	joystick[j].threshold = fgetc(f);
	
	for (int a = 0; a < COUNTOF(joystick[j].assignment); a++)
	{
		for (int i = 0; i < COUNTOF(*joystick[j].assignment); i++)
		{
			joystick[j].assignment[a][i].is_axis = fgetc(f);
			joystick[j].assignment[a][i].axis_negative = fgetc(f);
			joystick[j].assignment[a][i].num = (signed char)fgetc(f);
		}
	}
	
	fclose(f);
	
	return true;
}

bool save_joystick_assignments( int j )
{
	FILE *f = seek_joystick_assignments(j, false);
	
	if (f == NULL)
		return false;
	
	putc(joystick[j].analog, f);
	putc(joystick[j].sensitivity, f);
	putc(joystick[j].threshold, f);
	
	for (int a = 0; a < COUNTOF(joystick[j].assignment); a++)
	{
		for (int i = 0; i < COUNTOF(*joystick[j].assignment); i++)
		{
			fputc(joystick[j].assignment[a][i].is_axis, f);
			fputc(joystick[j].assignment[a][i].axis_negative, f);
			fputc(joystick[j].assignment[a][i].num, f);
		}
	}
	
	fclose(f);
	
	return true;
}

// seeks to a particular joystick's entry in the config file
FILE *seek_joystick_assignments( int j, bool read_only )
{
	assert(j < joysticks);
	
	const int joystick_axes = SDL_JoystickNumAxes(joystick[j].handle), joystick_buttons = SDL_JoystickNumButtons(joystick[j].handle);
	char joystick_xor = 0;
	
	const char *joystick_name = SDL_JoystickName(j);
	for (int i = 0; joystick_name[i] != '\0'; i++)
		joystick_xor ^= joystick_name[i];
	
	char cfg_file[1000];
	snprintf(cfg_file, sizeof(cfg_file), "%s" "joystick.cfg", get_user_directory());
	
	const int entry_size = 3 + 3 + COUNTOF(joystick->assignment) * COUNTOF(*joystick->assignment) * 3;
	
	FILE *f = fopen(cfg_file, read_only ? "rb" : "rb+");
	
	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		if (ftell(f) < 1 || (ftell(f) - 1) % entry_size != 0) // wrong size
		{
			fclose(f);
			f = NULL;
		}
	}
	
	if (f != NULL)
	{
		fseek(f, 0, SEEK_SET);
		if (fgetc(f) != joystick_cfg_version) // version mismatch
		{
			fclose(f);
			f = NULL;
		}
	}
	
	if (f != NULL)
	{
		while (true)
		{
			char xor, axes, buttons;
			
			xor = fgetc(f);
			axes = fgetc(f);
			buttons = fgetc(f);
			
			if (feof(f))
				break;
			
			if (xor == joystick_xor && axes == joystick_axes && buttons == joystick_buttons) // found a match
				return f;
			
			fseek(f, -3 + entry_size, SEEK_CUR);
		}
	}
	
	if (!read_only)
	{
		if (f == NULL) // create new config
		{
			f = fopen(cfg_file, "wb+");
			if (f == NULL)
				return f;
			
			fputc(joystick_cfg_version, f);
		}
		fputc(joystick_xor, f);
		fputc(joystick_axes, f);
		fputc(joystick_buttons, f);
	}
	
	return f;
}

// captures joystick input for configuring assignments
// returns false if non-joystick input was detected
// TODO: input from joystick other than the one being configured should not be ignored
bool detect_joystick_assignment( int j, struct joystick_assignment_struct *assignment )
{
	const int axes = SDL_JoystickNumAxes(joystick[j].handle);
	Sint16 *axis = malloc(axes * sizeof(*axis));
	for (int i = 0; i < axes; i++)
	{
		axis[i] = SDL_JoystickGetAxis(joystick[j].handle, i);
	}
	
	const int buttons = SDL_JoystickNumButtons(joystick[j].handle);
	Uint8 *button = malloc(buttons * sizeof(*button));
	for (int i = 0; i < buttons; i++)
	{
		button[i] = SDL_JoystickGetButton(joystick[j].handle, i);
	}
	
	bool detected = false;
	
	do
	{
		setjasondelay(1);
		
		SDL_JoystickUpdate();
		
		for (int i = 0; i < axes; i++)
		{
			Sint16 temp = SDL_JoystickGetAxis(joystick[j].handle, i);
			if (abs(temp - axis[i]) > joystick_analog_max * 2 / 3)
			{
				assignment->is_axis = true;
				assignment->axis_negative = temp < axis[i];
				assignment->num = i;
				detected = true;
				break;
			}
		}
		
		for (int i = 0; i < buttons; i++)
		{
			Uint8 temp = SDL_JoystickGetButton(joystick[j].handle, i);
			if (temp == 1 && button[i] == 0)
			{
				assignment->is_axis = false;
				assignment->axis_negative = false;
				assignment->num = i;
				detected = true;
				break;
			}
			else if (temp == 0 && button[i] == 1)
			{
				button[i] = 0;
			}
		}
		
		service_SDL_events(true);
		JE_showVGA();
		
		wait_delay();
	}
	while (!detected && !newkey && !newmouse);
	
	free(axis);
	free(button);
	
	return detected;
}

// compares assignments for equality
bool joystick_assignment_cmp( struct joystick_assignment_struct *a, struct joystick_assignment_struct *b )
{
	return a->is_axis == b->is_axis
	       && (a->is_axis == false || a->axis_negative == b->axis_negative)
	       && a->num == b->num;
}

// kate: tab-width 4; vim: set noet:
