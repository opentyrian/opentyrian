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
#include "keyboard.h"
#include "palette.h"
#include "video.h"
#include "video_scale.h"

#include <assert.h>
#include <stdbool.h>

bool fullscreen_enabled;

SDL_Surface *display_surface;

SDL_Surface *VGAScreen, *VGAScreenSeg;
SDL_Surface *game_screen;
SDL_Surface *VGAScreen2;

void init_video( void )
{
	if (SDL_WasInit(SDL_INIT_VIDEO))
		return;
	
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
	{
video_error:
		printf("error: failed to initialize SDL video: %s\n", SDL_GetError());
		exit(1);
	}
	
	SDL_WM_SetCaption("OpenTyrian (ctrl-backspace to kill)", NULL);
	
#ifdef TARGET_GP2X
	VGAScreen = VGAScreenSeg = display_surface;
#else
	VGAScreen = VGAScreenSeg = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
#endif /* TARGET_GP2X */
	VGAScreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	game_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	
#ifdef TARGET_GP2X
	SDL_FillRect(VGAScreenSeg, NULL, 0x0);
#endif /* TARGET_GP2X */
	SDL_FillRect(game_screen, NULL, 0x0);
	
	reinit_video();
	
	SDL_FillRect(display_surface, NULL, 0x0);
}

void reinit_video( void )
{
	scale = scalers[scaler].scale;
	
	int w = vga_width * scale,
	    h = vga_height * scale;
	int bpp = 32;
	int flags = SDL_SWSURFACE | SDL_HWPALETTE | (fullscreen_enabled ? SDL_FULLSCREEN : 0);
	
	bpp = SDL_VideoModeOK(w, h, bpp, flags);
	if (bpp == 24)
		bpp = 32;
	
	display_surface = SDL_SetVideoMode(w, h, bpp, flags);
	
	if (display_surface == NULL)
	{
		printf("error: failed to initialize SDL video: %s\n", SDL_GetError());
		exit(1);
	} else {
		printf("initialized SDL video: %dx%dx%d\n", w, h, bpp);
	}
	
	input_grab();
	
	JE_showVGA();
}

void deinit_video( void )
{
#ifndef TARGET_GP2X
	SDL_FreeSurface(VGAScreenSeg);
#endif /* TARGET_GP2X */
	SDL_FreeSurface(VGAScreen2);
	SDL_FreeSurface(game_screen);
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void JE_clr256( void )
{
	memset(VGAScreen->pixels, 0, VGAScreen->pitch * VGAScreen->h);
}

void JE_showVGA( void )
{
#ifndef TARGET_GP2X
	switch (display_surface->format->BitsPerPixel)
	{
		case 32:
			if (scalers[scaler].scaler32 == NULL)
				scaler = 0;
			scalers[scaler].scaler32(VGAScreen, display_surface, scale);
			break;
		case 16:
			if (scalers[scaler].scaler16 == NULL)
				scaler = 0;
			scalers[scaler].scaler16(VGAScreen, display_surface, scale);
			break;
		default:
			assert(0);
			break;
	}
#endif /* TARGET_GP2X */
	
	SDL_Flip(display_surface);
}
