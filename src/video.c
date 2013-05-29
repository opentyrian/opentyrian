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
#include "keyboard.h"
#include "opentyr.h"
#include "palette.h"
#include "video.h"
#include "video_scale.h"

#include <assert.h>
#include <stdbool.h>

bool fullscreen_enabled = false;

SDL_Surface *VGAScreen, *VGAScreenSeg;
SDL_Surface *VGAScreen2;
SDL_Surface *game_screen;

SDL_Window* main_window;
static SDL_Renderer* main_window_renderer;
static SDL_Texture* main_window_texture;
SDL_PixelFormat* main_window_tex_format;

static ScalerFunction scaler_function;

void init_video( void )
{
	if (SDL_WasInit(SDL_INIT_VIDEO))
		return;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
	{
		fprintf(stderr, "error: failed to initialize SDL video: %s\n", SDL_GetError());
		exit(1);
	}

	VGAScreen = VGAScreenSeg = SDL_CreateRGBSurface(0, vga_width, vga_height, 8, 0, 0, 0, 0);
	VGAScreen2 = SDL_CreateRGBSurface(0, vga_width, vga_height, 8, 0, 0, 0, 0);
	game_screen = SDL_CreateRGBSurface(0, vga_width, vga_height, 8, 0, 0, 0, 0);

	assert(!SDL_MUSTLOCK(VGAScreen));
	assert(!SDL_MUSTLOCK(VGAScreen2));
	assert(!SDL_MUSTLOCK(game_screen));

	SDL_FillRect(VGAScreen, NULL, 0);

	if (SDL_CreateWindowAndRenderer(scalers[scaler].width, scalers[scaler].height,
			SDL_WINDOW_RESIZABLE | (fullscreen_enabled ? SDL_WINDOW_FULLSCREEN : 0),
			&main_window, &main_window_renderer) == -1)
	{
		fprintf(stderr, "error: failed to open window: %s\n", SDL_GetError());
	}

	SDL_SetWindowTitle(main_window, "OpenTyrian");

	if (!init_scaler(scaler)) {
		fprintf(stderr, "error: failed to initialize any supported video mode\n");
		exit(EXIT_FAILURE);
	}
	SDL_ShowWindow(main_window);
}

bool init_scaler( unsigned int new_scaler )
{
	int w = scalers[new_scaler].width,
	    h = scalers[new_scaler].height;
	int bpp = 32; // TODOSDL2
	Uint32 format = bpp == 32 ? SDL_PIXELFORMAT_RGB888 : SDL_PIXELFORMAT_RGB565;
	
	if (bpp == 0)
		return false;

	SDL_FreeFormat(main_window_tex_format);
	main_window_tex_format = NULL;
	SDL_DestroyTexture(main_window_texture);
	
	main_window_texture = SDL_CreateTexture(main_window_renderer,
			format, SDL_TEXTUREACCESS_STREAMING, w, h);
	
	if (main_window_texture == NULL)
	{
		fprintf(stderr, "error: failed to create scaler texture %dx%dx%s: %s\n",
				w, h, SDL_GetPixelFormatName(format), SDL_GetError());
		return false;
	}

	main_window_tex_format = SDL_AllocFormat(format);
	
	// TODOSDL2 printf("initialized video: %dx%dx%d %s\n", w, h, bpp, fullscreen ? "fullscreen" : "windowed");
	
	scaler = new_scaler;
	
	switch (bpp)
	{
	case 32:
		scaler_function = scalers[scaler].scaler32;
		break;
	case 16:
		scaler_function = scalers[scaler].scaler16;
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
	
	input_grab();
	
	JE_showVGA();
	
	return true;
}

void deinit_video( void )
{
	SDL_FreeFormat(main_window_tex_format);
	SDL_DestroyTexture(main_window_texture);

	SDL_DestroyRenderer(main_window_renderer);
	SDL_DestroyWindow(main_window);

	SDL_FreeSurface(VGAScreenSeg);
	SDL_FreeSurface(VGAScreen2);
	SDL_FreeSurface(game_screen);
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void JE_clr256( SDL_Surface * screen)
{
	SDL_FillRect(screen, NULL, 0);
}
void JE_showVGA( void ) { scale_and_flip(VGAScreen); }

void scale_and_flip( SDL_Surface *src_surface )
{
	assert(src_surface->format->BitsPerPixel == 8);
	
	assert(scaler_function != NULL);
	scaler_function(src_surface, main_window_texture);

	SDL_SetRenderDrawColor(main_window_renderer, 255, 0, 0, 255);
	SDL_RenderClear(main_window_renderer);

	// TODO More centering modes
	SDL_RenderCopy(main_window_renderer, main_window_texture, NULL, NULL);
	
	SDL_RenderPresent(main_window_renderer);
}

