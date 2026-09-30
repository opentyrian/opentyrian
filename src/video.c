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
#include "video.h"

#include "keyboard.h"
#include "logging.h"
#include "palette.h"
#include "video_scale.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

bool fullscreen_enabled = false;
static SDL_Rect last_output_rect = { 0, 0, vga_width, vga_height };

SDL_Surface *VGAScreen, *VGAScreenSeg;
SDL_Surface *VGAScreen2;
SDL_Surface *game_screen;

static ScalerFunction scaler_function;

static void scale_and_flip(SDL_Surface *);

void init_video(void)
{
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
	{
		logFatal("Failed to initialize SDL video: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_WM_SetCaption("OpenTyrian", NULL);

	// Create the software surfaces that the game renders to. These are all 320x200x8 regardless
	// of the window size or monitor resolution.
	VGAScreen = VGAScreenSeg = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	VGAScreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);
	game_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, vga_width, vga_height, 8, 0, 0, 0, 0);

	// The game code writes to surface->pixels directly without locking, so make sure that we
	// indeed created software surfaces that support this.
	assert(!SDL_MUSTLOCK(VGAScreen));
	assert(!SDL_MUSTLOCK(VGAScreen2));
	assert(!SDL_MUSTLOCK(game_screen));

	JE_clr256(VGAScreen);

	if (!init_scaler(scaler, fullscreen_enabled) &&  // try desired scaler and desired fullscreen state
	    !init_any_scaler(fullscreen_enabled) &&      // try any scaler in desired fullscreen state
	    !init_any_scaler(!fullscreen_enabled))       // try any scaler in other fullscreen state
	{
		logFatal("Failed to initialize any supported video mode.");
		exit(EXIT_FAILURE);
	}
}

void deinit_video(void)
{
	SDL_FreeSurface(VGAScreenSeg);
	SDL_FreeSurface(VGAScreen2);
	SDL_FreeSurface(game_screen);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void toggle_fullscreen(void)
{
	if (!init_scaler(scaler, !fullscreen_enabled) && // try new fullscreen state
	    !init_any_scaler(!fullscreen_enabled) &&     // try any scaler in new fullscreen state
	    !init_scaler(scaler, fullscreen_enabled))    // revert on fail
	{
		logFatal("Failed to initialize any supported video mode.");
		exit(EXIT_FAILURE);
	}
}

int can_init_scaler(unsigned int new_scaler, bool fullscreen)
{
	if (new_scaler >= scalers_count)
		return false;

	int w = scalers[new_scaler].width;
	int h = scalers[new_scaler].height;
	int flags = SDL_SWSURFACE | SDL_HWPALETTE | (fullscreen ? SDL_FULLSCREEN : 0);

	// test each bitdepth
	for (uint bpp = 32; bpp > 0; bpp -= 8)
	{
		uint temp_bpp = SDL_VideoModeOK(w, h, bpp, flags);

		if ((temp_bpp == 32 && scalers[new_scaler].scaler32) ||
		    (temp_bpp == 16 && scalers[new_scaler].scaler16) ||
		    (temp_bpp == 8  && scalers[new_scaler].scaler8 ))
		{
			return temp_bpp;
		}
		else if (temp_bpp == 24 && scalers[new_scaler].scaler32)
		{
			// scalers don't support 24 bpp because it's a pain
			// so let SDL handle the conversion
			return 32;
		}
	}

	return 0;
}

bool init_scaler(unsigned int new_scaler, bool fullscreen)
{
	int w = scalers[new_scaler].width;
	int h = scalers[new_scaler].height;
	int bpp = can_init_scaler(new_scaler, fullscreen);
	int flags = SDL_SWSURFACE | SDL_HWPALETTE | (fullscreen ? SDL_FULLSCREEN : 0);

	if (bpp == 0)
		return false;

	SDL_Surface *const surface = SDL_SetVideoMode(w, h, bpp, flags);

	if (surface == NULL)
	{
		logError("Failed to initialize %s video mode %dx%dx%d: %s", fullscreen ? "fullscreen" : "windowed", w, h, bpp, SDL_GetError());
		return false;
	}

	w = surface->w;
	h = surface->h;
	bpp = surface->format->BitsPerPixel;

	logInfo("Initialized %s video mode %dx%dx%d.", fullscreen ? "fullscreen" : "windowed", w, h, bpp);

	scaler = new_scaler;
	fullscreen_enabled = fullscreen;

	last_output_rect.w = w;
	last_output_rect.h = h;

	switch (bpp)
	{
	case 32:
		scaler_function = scalers[scaler].scaler32;
		break;
	case 16:
		scaler_function = scalers[scaler].scaler16;
		break;
	case 8:
		scaler_function = scalers[scaler].scaler8;
		break;
	default:
		scaler_function = NULL;
		break;
	}

	if (scaler_function == NULL)
	{
		assert(false);
		return false;
	}

	if (bpp == 8)
		SDL_SetColors(surface, palette, 0, 256);

	JE_showVGA();

	return true;
}

bool can_init_any_scaler(bool fullscreen)
{
	for (int i = scalers_count - 1; i >= 0; --i)
		if (can_init_scaler(i, fullscreen) != 0)
			return true;

	return false;
}

bool init_any_scaler(bool fullscreen)
{
	// attempts all scalers from last to first
	for (int i = scalers_count - 1; i >= 0; --i)
		if (init_scaler(i, fullscreen))
			return true;

	return false;
}

void JE_clr256(SDL_Surface *screen)
{
	SDL_FillRect(screen, NULL, 0);
}

void JE_showVGA(void)
{
	scale_and_flip(VGAScreen);
}

static void scale_and_flip(SDL_Surface *src_surface)
{
	assert(src_surface->format->BitsPerPixel == 8);

	SDL_Surface *dst_surface = SDL_GetVideoSurface();

	// Do software scaling
	assert(scaler_function != NULL);
	scaler_function(src_surface, dst_surface);

	SDL_Flip(dst_surface);
}

/** Maps a specified point in game screen coordinates to window coordinates. */
void mapScreenPointToWindow(Sint32 *const inout_x, Sint32 *const inout_y)
{
	*inout_x = (2 * *inout_x + 1) * last_output_rect.w / (2 * VGAScreen->w) + last_output_rect.x;
	*inout_y = (2 * *inout_y + 1) * last_output_rect.h / (2 * VGAScreen->h) + last_output_rect.y;
}

/** Maps a specified point in window coordinates to game screen coordinates. */
void mapWindowPointToScreen(Sint32 *const inout_x, Sint32 *const inout_y)
{
	*inout_x = (2 * (*inout_x - last_output_rect.x) + 1) * VGAScreen->w / (2 * last_output_rect.w);
	*inout_y = (2 * (*inout_y - last_output_rect.y) + 1) * VGAScreen->h / (2 * last_output_rect.h);
}

/** Scales a distance in window coordinates to game screen coordinates. */
void scaleWindowDistanceToScreen(Sint32 *const inout_x, Sint32 *const inout_y)
{
	*inout_x = (2 * *inout_x + 1) * VGAScreen->w / (2 * last_output_rect.w);
	*inout_y = (2 * *inout_y + 1) * VGAScreen->h / (2 * last_output_rect.h);
}
