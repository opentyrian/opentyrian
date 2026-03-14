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
#ifndef FONT_H
#define FONT_H

#include "SDL.h"

#include <stdbool.h>

typedef enum
{
	FONT_LARGE = 0,
	FONT_NORMAL = 1,
	FONT_SMALL = 2,
}
Font;

typedef enum
{
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
}
FontAlignment;

void drawFontHvShadow(SDL_Surface *surface, int x, int y, const char *text, Font font, Uint8 hue, Sint8 value, bool black, int shadowDist);
void drawFontHvFullShadow(SDL_Surface *surface, int x, int y, const char *text, Font font, Uint8 hue, Sint8 value, bool black, int shadowDist);

void drawFontHv(SDL_Surface *surface, int x, int y, const char *text, Font font, Uint8 hue, Sint8 value);
void drawFontHvBlend(SDL_Surface *surface, int x, int y, const char *text, Font font, Uint8 hue, Sint8 value);
void drawFontDark(SDL_Surface *surface, int x, int y, const char *text, Font font, bool black);

void drawFontHvShadowAligned(SDL_Surface *surface, int x, int y, const char *text, Font, FontAlignment, Uint8 hue, Sint8 value, bool black, int shadowDist);
void drawFontHvFullShadowAligned(SDL_Surface *surface, int x, int y, const char *text, Font, FontAlignment, Uint8 hue, Sint8 value, bool black, int shadowDist);

void drawFontHvAligned(SDL_Surface *surface, int x, int y, const char *text, Font, FontAlignment, Uint8 hue, Sint8 value);
void drawFontHvBlendAligned(SDL_Surface *surface, int x, int y, const char *text, Font, FontAlignment, Uint8 hue, Sint8 value);
void drawFontDarkAligned(SDL_Surface *surface, int x, int y, const char *text, Font, FontAlignment, bool black);

#endif // FONT_H
