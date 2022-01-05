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
#include "mouse.h"

#include "keyboard.h"
#include "nortvars.h"
#include "sprite.h"
#include "video.h"
#include "vga256d.h"

#if defined(TARGET_GP2X) || defined(TARGET_DINGUX)
bool has_mouse = false;
#else
bool has_mouse = true;
#endif

bool mouseInactive = true;
JE_byte mouseCursor;
JE_word mouseX, mouseY, mouseButton;
JE_word mouseXB, mouseYB;

static JE_word mouseGrabX, mouseGrabY;
static JE_byte mouseGrabShape[24 * 28];

static void JE_drawShapeTypeOne( JE_word x, JE_word y, JE_byte *shape )
{
	JE_word xloop = 0, yloop = 0;
	JE_byte *p = shape; /* shape pointer */
	Uint8 *s;   /* screen pointer, 8-bit specific */
	Uint8 *s_limit; /* buffer boundary */

	s = (Uint8 *)VGAScreen->pixels;
	s += y * VGAScreen->pitch + x;

	s_limit = (Uint8 *)VGAScreen->pixels;
	s_limit += VGAScreen->h * VGAScreen->pitch;

	for (yloop = 0; yloop < 28; yloop++)
	{
		for (xloop = 0; xloop < 24; xloop++)
		{
			if (s >= s_limit) return;
			*s = *p;
			s++; p++;
		}
		s -= 24;
		s += VGAScreen->pitch;
	}
}

static void JE_grabShapeTypeOne( JE_word x, JE_word y, JE_byte *shape )
{
	JE_word xloop = 0, yloop = 0;
	JE_byte *p = shape; /* shape pointer */
	Uint8 *s;   /* screen pointer, 8-bit specific */
	Uint8 *s_limit; /* buffer boundary */

	s = (Uint8 *)VGAScreen->pixels;
	s += y * VGAScreen->pitch + x;

	s_limit = (Uint8 *)VGAScreen->pixels;
	s_limit += VGAScreen->h * VGAScreen->pitch;

	for (yloop = 0; yloop < 28; yloop++)
	{
		for (xloop = 0; xloop < 24; xloop++)
		{
			if (s >= s_limit) return;
			*p = *s;
			s++; p++;
		}
		s -= 24;
		s += VGAScreen->pitch;
	}
}

typedef struct
{
	Uint16 index;
	Uint8 x;
	Uint8 y;
	Uint8 w;
	Uint8 h;
	Uint8 fx;
	Uint8 fy;
} MousePointerSpriteInfo;

static const MousePointerSpriteInfo mousePointerSprites[] = // fka mouseCursorGr
{
	{ 273, 0, 0, 11, 16,  0,  0 },
	{ 275, 0, 0, 21, 16, 10,  8 },
	{ 277, 0, 0, 21, 16, 10,  7 },
	{ 279, 0, 0, 16, 21,  8, 10 },
	{ 281, 8, 0, 16, 21,  7, 10 },
};

void JE_mouseStart( void )
{
	if (has_mouse)
	{
		service_SDL_events(false);

		mouseButton = mousedown ? lastmouse_but : 0; /* incorrect, possibly unimportant */

		const MousePointerSpriteInfo *spriteInfo = &mousePointerSprites[mouseCursor];

		mouseGrabX = MIN(MAX(spriteInfo->fx, mouse_x), 320 - (spriteInfo->w - spriteInfo->fx)) - spriteInfo->fx;
		mouseGrabY = MIN(MAX(spriteInfo->fy, mouse_y), 200 - (spriteInfo->h - spriteInfo->fy)) - spriteInfo->fy;

		JE_grabShapeTypeOne(mouseGrabX, mouseGrabY, mouseGrabShape);

		if (!mouseInactive)
		{
			const Sint32 x = mouse_x - spriteInfo->x - spriteInfo->fx;
			const Sint32 y = mouse_y - spriteInfo->y - spriteInfo->fy;
			blit_sprite2x2_clip(VGAScreen, x, y, shopSpriteSheet, spriteInfo->index);
		}
	 }
}

void JE_mouseStartFilter( Uint8 filter )
{
	if (has_mouse)
	{
		mouseButton = mousedown ? lastmouse_but : 0; /* incorrect, possibly unimportant */

		const MousePointerSpriteInfo *spriteInfo = &mousePointerSprites[mouseCursor];

		mouseGrabX = MIN(MAX(spriteInfo->fx, mouse_x), 320 - (spriteInfo->w - spriteInfo->fx)) - spriteInfo->fx;
		mouseGrabY = MIN(MAX(spriteInfo->fy, mouse_y), 200 - (spriteInfo->h - spriteInfo->fy)) - spriteInfo->fy;

		JE_grabShapeTypeOne(mouseGrabX, mouseGrabY, mouseGrabShape);

		if (!mouseInactive)
		{
			const Sint32 x = mouse_x - spriteInfo->x - spriteInfo->fx;
			const Sint32 y = mouse_y - spriteInfo->y - spriteInfo->fy;
			blit_sprite2x2_filter_clip(VGAScreen, x, y, shopSpriteSheet, spriteInfo->index, filter);
		}
	}
}

void JE_mouseReplace( void )
{
	if (has_mouse)
		JE_drawShapeTypeOne(mouseGrabX, mouseGrabY, mouseGrabShape);
}

