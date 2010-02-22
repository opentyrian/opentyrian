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
#include "keyboard.h"
#include "opentyr.h"
#include "palette.h"
#include "video.h"
#include "video_scale.h"

#include <assert.h>
#include <stdbool.h>

bool fullscreen_enabled = false;

SDL_Surface *display_surface;

SDL_Surface *VGAScreen, *VGAScreenSeg;
SDL_Surface *VGAScreen2;
SDL_Surface *game_screen;

void init_video( void )
{
	if (SDL_WasInit(SDL_INIT_VIDEO))
		return;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
	{
		fprintf(stderr, "error: failed to initialize SDL video: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_WM_SetCaption("OpenTyrian", NULL);

	VGAScreen = VGAScreenSeg = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	VGAScreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	game_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);

	SDL_FillRect(VGAScreen, NULL, 0);

	if (!init_scaler(scaler, fullscreen_enabled) &&  // try desired scaler and desired fullscreen state
	    !init_any_scaler(fullscreen_enabled) &&      // try any scaler in desired fullscreen state
	    !init_any_scaler(!fullscreen_enabled))       // try any scaler in other fullscreen state
	{
		exit(EXIT_FAILURE);
	}
}

bool init_scaler( int new_scaler, bool fullscreen )
{
	if (new_scaler < 0 || new_scaler >= COUNTOF(scalers))
		return false;

	int w = scalers[new_scaler].width,
	    h = scalers[new_scaler].height;
	int bpp = 32;
	int flags = SDL_SWSURFACE | SDL_HWPALETTE | (fullscreen ? SDL_FULLSCREEN : 0);

	bpp = SDL_VideoModeOK(w, h, bpp, flags);

	if (bpp < scalers[new_scaler].min_bpp)
	{
		// we can't get exactly what we want, but SDL will try to find something close
		bpp = scalers[new_scaler].min_bpp;
	}
	else if (bpp == 24)
	{
		// scalers don't support 24 bpp because it's a pain
		bpp = 32;
	}

	display_surface = SDL_SetVideoMode(w, h, bpp, flags);

	if (display_surface == NULL)
	{
		fprintf(stderr, "error: failed to initialize video mode %dx%dx%d: %s\n", w, h, bpp, SDL_GetError());
		return false;
	}

	w = display_surface->w;
	h = display_surface->h;
	bpp = display_surface->format->BitsPerPixel;

	printf("initialized video: %dx%dx%d\n", w, h, bpp);

	scaler = new_scaler;
	fullscreen_enabled = fullscreen;

	input_grab();

	JE_showVGA();

	return true;
}

bool init_any_scaler( bool fullscreen )
{
	// attempts all scalers from last to first
	for (int i = COUNTOF(scalers) - 1; i >= 0; --i)
		if (init_scaler(i, fullscreen))
			return true;

	return false;
}

void deinit_video( void )
{
	SDL_FreeSurface(VGAScreenSeg);
	SDL_FreeSurface(VGAScreen2);
	SDL_FreeSurface(game_screen);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void JE_clr256( SDL_Surface * screen)
{
	memset(screen->pixels, 0, screen->pitch * screen->h);
}
void JE_showVGA( void ) { JE_showScreen(VGAScreen); }
void JE_showScreen( SDL_Surface * screen)
{
	switch (display_surface->format->BitsPerPixel)
	{
		case 32:
			if (scalers[scaler].scaler32 == NULL)
				scaler = 0;
			scalers[scaler].scaler32(screen, display_surface);
			break;
		case 16:
			if (scalers[scaler].scaler16 == NULL)
				scaler = 0;
			scalers[scaler].scaler16(screen, display_surface);
			break;
		case 8:
			// only 8-bit scaler is None
			memcpy(display_surface->pixels, screen->pixels, display_surface->pitch * display_surface->h);
			break;
		default:
			assert(0);
			break;
	}

	SDL_Flip(display_surface);
}

// kate: tab-width 4; vim: set noet:
