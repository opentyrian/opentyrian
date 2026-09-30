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
#ifndef VIDEO_H
#define VIDEO_H

#include "SDL.h"

#include <stdbool.h>

#define vga_width 320
#define vga_height 200

extern bool fullscreen_enabled;

extern SDL_Surface *VGAScreen, *VGAScreenSeg;
extern SDL_Surface *game_screen;
extern SDL_Surface *VGAScreen2;

void init_video(void);

void toggle_fullscreen(void);
int can_init_scaler(unsigned int new_scaler, bool fullscreen);
bool init_scaler(unsigned int new_scaler, bool fullscreen);
bool can_init_any_scaler(bool fullscreen);
bool init_any_scaler(bool fullscreen);

void deinit_video(void);

void JE_clr256(SDL_Surface *);
void JE_showVGA(void);

void mapScreenPointToWindow(Sint32 *inout_x, Sint32 *inout_y);
void mapWindowPointToScreen(Sint32 *inout_x, Sint32 *inout_y);
void scaleWindowDistanceToScreen(Sint32 *inout_x, Sint32 *inout_y);

#endif /* VIDEO_H */
