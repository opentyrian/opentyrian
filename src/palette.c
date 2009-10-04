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
#include "file.h"
#include "nortsong.h"
#include "opentyr.h"
#include "palette.h"
#include "video.h"

#include <assert.h>

Uint32 rgb_to_yuv( int r, int g, int b );

palette_t palettes[23];
int palette_count;

palette_t palette;
Uint32 rgb_palette[256], yuv_palette[256];

palette_t black = {{0,0,0}}; /* Rest is filled with 0's too */
palette_t colors;

void JE_loadPals( void )
{
	FILE *f = dir_fopen_die(data_dir(), "palette.dat", "rb");
	
	palette_count = ftell_eof(f) / (256 * 3);
	assert(palette_count == 23); // game assumes 23 palettes
	
	for (int p = 0; p < palette_count; ++p)
	{
		for (int i = 0; i < 256; ++i)
		{
			palettes[p][i].r = getc(f) << 2;
			palettes[p][i].g = getc(f) << 2;
			palettes[p][i].b = getc(f) << 2;
		}
	}
	
	fclose(f);
}

void JE_updateColorsFast( palette_t colorBuffer )
{
	for (int i = 0; i < 256; i++)
	{
		palette[i] = colorBuffer[i];
		
#ifndef TARGET_GP2X
		rgb_palette[i] = SDL_MapRGB(display_surface->format, palette[i].r, palette[i].g, palette[i].b);
		yuv_palette[i] = rgb_to_yuv(palette[i].r, palette[i].g, palette[i].b);
#endif // TARGET_GP2X
	}
	
#ifdef TARGET_GP2X
	SDL_SetColors(display_surface, palette, 0, 256);
#endif /* TARGET_GP2X */
}


void set_palette( SDL_Color color, unsigned int first_color, unsigned int last_color )
{
	for (int i = first_color; i <= last_color; ++i )
	{
		palette[i] = color;
		rgb_palette[i] = SDL_MapRGB(display_surface->format, palette[i].r, palette[i].g, palette[i].b);
		yuv_palette[i] = rgb_to_yuv(palette[i].r, palette[i].g, palette[i].b);
	}
}
void init_step_fade_palette( int diff[256][3], palette_t colors, unsigned int first_color, unsigned int last_color )
{
	for (unsigned int i = first_color; i <= last_color; i++)
	{
		diff[i][0] = (int)colors[i].r - palette[i].r;
		diff[i][1] = (int)colors[i].g - palette[i].g;
		diff[i][2] = (int)colors[i].b - palette[i].b;
	}
}

void init_step_fade_solid( int diff[256][3], SDL_Color color, unsigned int first_color, unsigned int last_color )
{
	for (unsigned int i = first_color; i <= last_color; i++)
	{
		diff[i][0] = (int)color.r - palette[i].r;
		diff[i][1] = (int)color.g - palette[i].g;
		diff[i][2] = (int)color.b - palette[i].b;
	}
}

void step_fade_palette( int diff[256][3], int steps, unsigned int first_color, unsigned int last_color )
{
	assert(steps > 0);
	
	for (unsigned int i = first_color; i <= last_color; i++)
	{
		int delta[3] = { diff[i][0] / steps, diff[i][1] / steps, diff[i][2] / steps };
		
		diff[i][0] -= delta[0];
		diff[i][1] -= delta[1];
		diff[i][2] -= delta[2];
		
		palette[i].r += delta[0];
		palette[i].g += delta[1];
		palette[i].b += delta[2];
		
#ifndef TARGET_GP2X
		rgb_palette[i] = SDL_MapRGB(display_surface->format, palette[i].r, palette[i].g, palette[i].b);
		yuv_palette[i] = rgb_to_yuv(palette[i].r, palette[i].g, palette[i].b);
#endif // TARGET_GP2X
	}
	
#ifndef TARGET_GP2X
	JE_showVGA();
#else // TARGET_GP2X
	SDL_SetColors(display_surface, palette, 0, 256);
#endif // TARGET_GP2X
}


void fade_palette( palette_t colors, int steps, unsigned int first_color, unsigned int last_color )
{
	assert(steps > 0);
	
	static int diff[256][3];
	init_step_fade_palette(diff, colors, first_color, last_color);
	
	for (; steps > 0; steps--)
	{
		setdelay(1);
		
		step_fade_palette(diff, steps, first_color, last_color);
		
		wait_delay();
	}
}

void fade_solid( SDL_Color color, int steps, unsigned int first_color, unsigned int last_color )
{
	assert(steps > 0);
	
	static int diff[256][3];
	init_step_fade_solid(diff, color, first_color, last_color);
	
	for (; steps > 0; steps--)
	{
		setdelay(1);
		
		step_fade_palette(diff, steps, first_color, last_color);
		
		wait_delay();
	}
}

void fade_black( int steps )
{
	SDL_Color black = { 0, 0, 0 };
	fade_solid(black, steps, 0, 255);
}

void fade_white( int steps )
{
	SDL_Color white = { 255, 255, 255 };
	fade_solid(white, steps, 0, 255);
}

void JE_fadeColors( palette_t fromColors, palette_t toColors, JE_byte startCol, JE_byte noColors, JE_byte noSteps )
{
	int s, i;

	for (s = 0; s <= noSteps; s++)
	{
		setdelay(1);
		
		for (i = startCol; i <= startCol + noColors; i++)
		{
			palette[i].r = fromColors[i].r + (((toColors[i].r - fromColors[i].r) * s) / noSteps);
			palette[i].g = fromColors[i].g + (((toColors[i].g - fromColors[i].g) * s) / noSteps);
			palette[i].b = fromColors[i].b + (((toColors[i].b - fromColors[i].b) * s) / noSteps);
			rgb_palette[i] = SDL_MapRGB(display_surface->format, palette[i].r, palette[i].g, palette[i].b);
			yuv_palette[i] = rgb_to_yuv(palette[i].r, palette[i].g, palette[i].b);
		}
		
#ifndef TARGET_GP2X
		JE_showVGA();
#else /* TARGET_GP2X */
		SDL_SetColors(display_surface, palette, 0, 256);
#endif /* TARGET_GP2X */
		
		wait_delay();
	}
}

void JE_setPalette( JE_byte col, JE_byte red, JE_byte green, JE_byte blue )
{
	palette[col].r = red;
	palette[col].g = green;
	palette[col].b = blue;
	rgb_palette[col] = SDL_MapRGB(display_surface->format, palette[col].r, palette[col].g, palette[col].b);
	yuv_palette[col] = rgb_to_yuv(palette[col].r, palette[col].g, palette[col].b);
	
#ifdef TARGET_GP2X
	SDL_SetColors(display_surface, palette, 0, 256);
#endif /* TARGET_GP2X */
}

Uint32 rgb_to_yuv( int r, int g, int b )
{
	int y = (r + g + b) >> 2,
	    u = 128 + ((r - b) >> 2),
	    v = 128 + ((-r + 2 * g - b) >> 3);
	return (y << 16) + (u << 8) + v;
}

// kate: tab-width 4; vim: set noet:
