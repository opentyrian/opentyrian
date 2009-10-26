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
#include "file.h"
#include "joystick.h"
#include "keyboard.h"
#include "nortsong.h"
#include "opentyr.h"
#include "params.h"
#include "varz.h"
#include "video.h"

#include <assert.h>

int joystick_axis_threshold( int j, int value );
int check_assigned( SDL_Joystick *joystick_handle, const Joystick_assignment assignment[2] );

int joystick_repeat_delay = 300; // milliseconds, repeat delay for buttons
bool joydown = false;            // any joystick buttons down, updated by poll_joysticks()
bool ignore_joystick = false;

int joysticks = 0;
Joystick *joystick = NULL;

static const char joystick_cfg_version = 1;
static const int joystick_analog_max = 32767;

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

// converts joystick axis to sane Tyrian-usable value (based on sensitivity)
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

/* gives back value 0..joystick_analog_max indicating that one of the assigned
 * buttons has been pressed or that one of the assigned axes/hats has been moved
 * in the assigned direction
 */
int check_assigned( SDL_Joystick *joystick_handle, const Joystick_assignment assignment[2] )
{
	int result = 0;
	
	for (int i = 0; i < 2; i++)
	{
		int temp;
		
		switch (assignment[i].type)
		{
		case NONE:
			continue;
			
		case AXIS:
			temp = SDL_JoystickGetAxis(joystick_handle, assignment[i].num);
			
			if (assignment[i].negative_axis)
				temp = -temp;
			break;
		
		case BUTTON:
			temp = SDL_JoystickGetButton(joystick_handle, assignment[i].num) == 1 ? joystick_analog_max : 0;
			break;
		
		case HAT:
			temp = SDL_JoystickGetHat(joystick_handle, assignment[i].num);
			
			if (assignment[i].x_axis)
				temp &= SDL_HAT_LEFT | SDL_HAT_RIGHT;
			else
				temp &= SDL_HAT_UP | SDL_HAT_DOWN;
			
			if (assignment[i].negative_axis)
				temp &= SDL_HAT_LEFT | SDL_HAT_UP;
			else
				temp &= SDL_HAT_RIGHT | SDL_HAT_DOWN;
			
			temp = temp ? joystick_analog_max : 0;
			break;
		}
		
		if (temp > result)
			result = temp;
	}
	
	return result;
}

// updates joystick state
void poll_joystick( int j )
{
	assert(j < joysticks);
	
	if (joystick[j].handle == NULL)
		return;
	
	SDL_JoystickUpdate();
	
	// indicates that a direction/action was pressed since last poll
	joystick[j].input_pressed = false;
	
	// indicates that an direction/action has been held long enough to fake a repeat press
	bool repeat = joystick[j].joystick_delay < SDL_GetTicks();
	
	// update direction state
	for (int d = 0; d < COUNTOF(joystick[j].direction); d++)
	{
		bool old = joystick[j].direction[d];
		
		joystick[j].analog_direction[d] = check_assigned(joystick[j].handle, joystick[j].assignment[d]);
		joystick[j].direction[d] = joystick[j].analog_direction[d] > (joystick_analog_max / 2);
		joydown |= joystick[j].direction[d];
		
		joystick[j].direction_pressed[d] = joystick[j].direction[d] && (!old || repeat);
		joystick[j].input_pressed |= joystick[j].direction_pressed[d];
	}
	
	joystick[j].x = -joystick[j].analog_direction[3] + joystick[j].analog_direction[1];
	joystick[j].y = -joystick[j].analog_direction[0] + joystick[j].analog_direction[2];
	
	// update action state
	for (int d = 0; d < COUNTOF(joystick[j].action); d++)
	{
		bool old = joystick[j].action[d];
		
		joystick[j].action[d] = check_assigned(joystick[j].handle, joystick[j].assignment[d + COUNTOF(joystick[j].direction)]);
		joydown |= joystick[j].action[d];
		
		joystick[j].action_pressed[d] = joystick[j].action[d] && (!old || repeat);
		joystick[j].input_pressed |= joystick[j].action_pressed[d];
	}
	
	joystick[j].confirm = joystick[j].action[0] || joystick[j].action[4];
	joystick[j].cancel = joystick[j].action[1] || joystick[j].action[5];
	
	// if new input, reset press-repeat delay
	if (joystick[j].input_pressed)
		joystick[j].joystick_delay = SDL_GetTicks() + joystick_repeat_delay;
}

// updates all joystick states
void poll_joysticks( void )
{
	joydown = false;
	
	for (int j = 0; j < joysticks; j++)
		poll_joystick(j);
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

// helps us be lazy by pretending joysticks are a keyboard (useful for menus)
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
		fprintf(stderr, "warning: failed to initialize joystick system: %s\n", SDL_GetError());
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
			printf("(%d axes, %d buttons, %d hats)\n", 
			       SDL_JoystickNumAxes(joystick[j].handle),
			       SDL_JoystickNumButtons(joystick[j].handle),
			       SDL_JoystickNumHats(joystick[j].handle));
			
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
	
	// defaults: first 2 axes, first hat, first 6 buttons
	for (int a = 0; a < COUNTOF(joystick[j].assignment); a++)
	{
		// clear assignments
		for (int i = 0; i < COUNTOF(joystick[j].assignment[a]); i++)
			joystick[j].assignment[a][i].type = NONE;
		
		if (a < 4)
		{
			if (SDL_JoystickNumAxes(joystick[j].handle) >= 2)
			{
				joystick[j].assignment[a][0].type = AXIS;
				joystick[j].assignment[a][0].num = (a + 1) % 2;
				joystick[j].assignment[a][0].negative_axis = (a == 0 || a == 3);
			}
			
			if (SDL_JoystickNumHats(joystick[j].handle) >= 1)
			{
				joystick[j].assignment[a][1].type = HAT;
				joystick[j].assignment[a][1].num = 0;
				joystick[j].assignment[a][0].x_axis = (a == 1 || a == 3);
				joystick[j].assignment[a][0].negative_axis = (a == 0 || a == 3);
			}
		}
		else
		{
			if (a - 4 < SDL_JoystickNumButtons(joystick[j].handle))
			{
				joystick[j].assignment[a][0].type = BUTTON;
				joystick[j].assignment[a][0].num = a - 4;
			}
		}
	}
	
	joystick[j].analog = false;
	joystick[j].sensitivity = 5;
	joystick[j].threshold = 5;
}

bool load_joystick_assignments( int j )
{
	FILE *f = seek_joystick_assignments(j, true);
	
	if (f == NULL || feof(f))
		return false;
	
	joystick[j].analog = fgetc(f);
	joystick[j].sensitivity = fgetc(f);
	joystick[j].threshold = fgetc(f);
	
	for (int a = 0; a < COUNTOF(joystick[j].assignment); a++)
	{
		for (int i = 0; i < COUNTOF(*joystick[j].assignment); i++)
		{
			joystick[j].assignment[a][i].type = fgetc(f);
			joystick[j].assignment[a][i].num = (signed char)fgetc(f);
			joystick[j].assignment[a][i].x_axis = fgetc(f);
			joystick[j].assignment[a][i].negative_axis = fgetc(f);
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
			fputc(joystick[j].assignment[a][i].type, f);
			fputc(joystick[j].assignment[a][i].num, f);
			fputc(joystick[j].assignment[a][i].x_axis, f);
			fputc(joystick[j].assignment[a][i].negative_axis, f);
		}
	}
	
	fclose(f);
	
	return true;
}

// seeks to a particular joystick's entry in the config file
// TODO: rewrite this to use a text config instead
FILE *seek_joystick_assignments( int j, bool read_only )
{
	assert(j < joysticks);
	
	const int joystick_axes = SDL_JoystickNumAxes(joystick[j].handle), joystick_buttons = SDL_JoystickNumButtons(joystick[j].handle);
	char joystick_xor = 0;
	
	const char *joystick_name = SDL_JoystickName(j);
	for (int i = 0; joystick_name[i] != '\0'; i++)
		joystick_xor ^= joystick_name[i];
	
	const int entry_size = 3 + 3 + COUNTOF(joystick->assignment) * COUNTOF(*joystick->assignment) * 4;
	
	FILE *f = dir_fopen_warn(get_user_directory(), "joystick.cfg", read_only ? "rb" : "rb+");
	
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
			f = dir_fopen_warn(get_user_directory(), "joystick.cfg", "wb+");
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

/* gives the short (6 or less characters) identifier for a joystick assignment
 * 
 * two of these per direction/action is all that can fit on the joystick config screen,
 * assuming two digits for the axis/button/hat number
 */
const char *joystick_assignment_name( const Joystick_assignment *assignment )
{
	static char name[7];
	
	switch (assignment->type)
	{
	case NONE:
		strcpy(name, "");
		break;
		
	case AXIS:
		snprintf(name, sizeof(name), "AX %d%c",
		         assignment->num + 1,
		         assignment->negative_axis ? '-' : '+');
		break;
		
	case BUTTON:
		snprintf(name, sizeof(name), "BTN %d",
		         assignment->num + 1);
		break;
		
	case HAT:
		snprintf(name, sizeof(name), "H %d%c%c",
		         assignment->num + 1,
		         assignment->x_axis ? 'X' : 'Y',
		         assignment->negative_axis ? '-' : '+');
		break;
	}
	
	return name;
}

// captures joystick input for configuring assignments
// returns false if non-joystick input was detected
// TODO: input from joystick other than the one being configured probably should not be ignored
bool detect_joystick_assignment( int j, Joystick_assignment *assignment )
{
	// get initial joystick state to compare against to see if anything was pressed
	
	const int axes = SDL_JoystickNumAxes(joystick[j].handle);
	Sint16 *axis = malloc(axes * sizeof(*axis));
	for (int i = 0; i < axes; i++)
		axis[i] = SDL_JoystickGetAxis(joystick[j].handle, i);
	
	const int buttons = SDL_JoystickNumButtons(joystick[j].handle);
	Uint8 *button = malloc(buttons * sizeof(*button));
	for (int i = 0; i < buttons; i++)
		button[i] = SDL_JoystickGetButton(joystick[j].handle, i);
	
	const int hats = SDL_JoystickNumHats(joystick[j].handle);
	Uint8 *hat = malloc(hats * sizeof(*hat));
	for (int i = 0; i < hats; i++)
		hat[i] = SDL_JoystickGetHat(joystick[j].handle, i);
	
	bool detected = false;
	
	do
	{
		setjasondelay(1);
		
		SDL_JoystickUpdate();
		
		for (int i = 0; i < axes; ++i)
		{
			Sint16 temp = SDL_JoystickGetAxis(joystick[j].handle, i);
			
			if (abs(temp - axis[i]) > joystick_analog_max * 2 / 3)
			{
				assignment->type = AXIS;
				assignment->num = i;
				assignment->negative_axis = temp < axis[i];
				detected = true;
				break;
			}
		}
		
		for (int i = 0; i < buttons; ++i)
		{
			Uint8 new_button = SDL_JoystickGetButton(joystick[j].handle, i),
			      changed = button[i] ^ new_button;
			
			if (!changed)
				continue;
			
			if (new_button == 0) // button was released
			{
				button[i] = new_button;
			}
			else                 // button was pressed
			{
				assignment->type = BUTTON;
				assignment->num = i;
				detected = true;
				break;
			}
		}
		
		for (int i = 0; i < hats; ++i)
		{
			Uint8 new_hat = SDL_JoystickGetHat(joystick[j].handle, i),
			      changed = hat[i] ^ new_hat;
			
			if (!changed)
				continue;
			
			if ((new_hat & changed) == SDL_HAT_CENTERED) // hat was centered
			{
				hat[i] = new_hat;
			}
			else
			{
				assignment->type = HAT;
				assignment->num = i;
				assignment->x_axis = changed & (SDL_HAT_LEFT | SDL_HAT_RIGHT);
				assignment->negative_axis = changed & (SDL_HAT_LEFT | SDL_HAT_UP);
				detected = true;
			}
		}
		
		service_SDL_events(true);
		JE_showVGA();
		
		wait_delay();
	}
	while (!detected && !newkey && !newmouse);
	
	free(axis);
	free(button);
	free(hat);
	
	return detected;
}

// compares relevant parts of joystick assignments for equality
bool joystick_assignment_cmp( const Joystick_assignment *a, const Joystick_assignment *b )
{
	if (a->type == b->type)
	{
		switch (a->type)
		{
		case NONE:
			return true;
		case AXIS:
			return (a->num == b->num) &&
			       (a->negative_axis == b->negative_axis);
		case BUTTON:
			return (a->num == b->num);
		case HAT:
			return (a->num == b->num) &&
			       (a->x_axis == b->x_axis) &&
			       (a->negative_axis == b->negative_axis);
		}
	}
	
	return false;
}

// kate: tab-width 4; vim: set noet:
