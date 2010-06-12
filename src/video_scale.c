/* 
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2010  The OpenTyrian Development Team
 * 
 * Scale2x, Scale3x
 * Copyright (C) 2001, 2002, 2003, 2004 Andrea Mazzoleni
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

#include "video_scale.h"

#include "palette.h"
#include "video.h"

#include <assert.h>

void nn_32( SDL_Surface *src_surface, SDL_Surface *dst_surface );
void nn_16( SDL_Surface *src_surface, SDL_Surface *dst_surface );

void scale2x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface );
void scale2x_16( SDL_Surface *src_surface, SDL_Surface *dst_surface );
void scale3x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface );
void scale3x_16( SDL_Surface *src_surface, SDL_Surface *dst_surface );

void hq2x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface );
void hq3x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface );
void hq4x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface );

int scaler;

const struct scaler_struct scalers[] =
{
	{ 1 * vga_width, 1 * vga_height,  8, nn_16,      nn_32,      "None" },
	{ 2 * vga_width, 2 * vga_height, 16, nn_16,      nn_32,      "2x" },
	{ 2 * vga_width, 2 * vga_height, 16, scale2x_16, scale2x_32, "Scale2x" },
	{ 2 * vga_width, 2 * vga_height, 32, NULL,       hq2x_32,    "hq2x" },
	{ 3 * vga_width, 3 * vga_height, 16, nn_16,      nn_32,      "3x" },
	{ 3 * vga_width, 3 * vga_height, 16, scale3x_16, scale3x_32, "Scale3x" },
	{ 3 * vga_width, 3 * vga_height, 32, NULL,       hq3x_32,    "hq3x" },
	{ 4 * vga_width, 4 * vga_height, 16, nn_16,      nn_32,      "4x" },
	{ 4 * vga_width, 4 * vga_height, 32, NULL,       hq4x_32,    "hq4x" },
};

void set_scaler_by_name( const char *name )
{
	for (uint i = 0; i < COUNTOF(scalers); ++i)
	{
		if (strcmp(name, scalers[i].name) == 0)
		{
			scaler = i;
			break;
		}
	}
}

void nn_32( SDL_Surface *src_surface, SDL_Surface *dst_surface )
{
	Uint8 *src = src_surface->pixels, *src_temp,
	      *dst = dst_surface->pixels, *dst_temp;
	int src_pitch = src_surface->pitch,
	    dst_pitch = dst_surface->pitch;
	const int dst_Bpp = 4;         // dst_surface->format->BytesPerPixel
	
	const int height = vga_height, // src_surface->h
	          width = vga_width,   // src_surface->w
	          scale = dst_surface->w / width;
	assert(scale == dst_surface->h / height);
	
	for (int y = height; y > 0; y--)
	{
		src_temp = src;
		dst_temp = dst;
		
		for (int x = width; x > 0; x--)
		{
			for (int z = scale; z > 0; z--)
			{
				*(Uint32 *)dst = rgb_palette[*src];
				dst += dst_Bpp;
			}
			src++;
		}
		
		src = src_temp + src_pitch;
		dst = dst_temp + dst_pitch;
		
		for (int z = scale; z > 1; z--)
		{
			memcpy(dst, dst_temp, dst_pitch);
			dst += dst_pitch;
		}
	}
}

void nn_16( SDL_Surface *src_surface, SDL_Surface *dst_surface )
{
	Uint8 *src = src_surface->pixels, *src_temp,
	      *dst = dst_surface->pixels, *dst_temp;
	int src_pitch = src_surface->pitch,
	    dst_pitch = dst_surface->pitch;
	const int dst_Bpp = 2;         // dst_surface->format->BytesPerPixel
	
	const int height = vga_height, // src_surface->h
	          width = vga_width,   // src_surface->w
	          scale = dst_surface->w / width;
	assert(scale == dst_surface->h / height);
	
	for (int y = height; y > 0; y--)
	{
		src_temp = src;
		dst_temp = dst;
		
		for (int x = width; x > 0; x--)
		{
			for (int z = scale; z > 0; z--)
			{
				*(Uint16 *)dst = rgb_palette[*src];
				dst += dst_Bpp;
			}
			src++;
		}
		
		src = src_temp + src_pitch;
		dst = dst_temp + dst_pitch;
		
		for (int z = scale; z > 1; z--)
		{
			memcpy(dst, dst_temp, dst_pitch);
			dst += dst_pitch;
		}
	}
}


void scale2x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface )
{
	Uint8 *src = src_surface->pixels, *src_temp,
	      *dst = dst_surface->pixels, *dst_temp;
	int src_pitch = src_surface->pitch,
	    dst_pitch = dst_surface->pitch;
	const int dst_Bpp = 4;         // dst_surface->format->BytesPerPixel
	
	const int height = vga_height, // src_surface->h
	          width = vga_width;   // src_surface->w
	
	int prevline, nextline;
	
	Uint32 E0, E1, E2, E3, B, D, E, F, H;
	for (int y = 0; y < height; y++)
	{
		src_temp = src;
		dst_temp = dst;
		
		prevline = (y > 0) ? -src_pitch : 0;
		nextline = (y < height - 1) ? src_pitch : 0;
		
		for (int x = 0; x < width; x++)
		{
			B = rgb_palette[*(src + prevline)];
			D = rgb_palette[*(x > 0 ? src - 1 : src)];
			E = rgb_palette[*src];
			F = rgb_palette[*(x < width - 1 ? src + 1 : src)];
			H = rgb_palette[*(src + nextline)];
			
			if (B != H && D != F) {
				E0 = D == B ? D : E;
				E1 = B == F ? F : E;
				E2 = D == H ? D : E;
				E3 = H == F ? F : E;
			} else {
				E0 = E1 = E2 = E3 = E;
			}
			
			*(Uint32 *)dst = E0;
			*(Uint32 *)(dst + dst_Bpp) = E1;
			*(Uint32 *)(dst + dst_pitch) = E2;
			*(Uint32 *)(dst + dst_pitch + dst_Bpp) = E3;
			
			src++;
			dst += 2 * dst_Bpp;
		}
		
		src = src_temp + src_pitch;
		dst = dst_temp + 2 * dst_pitch;
	}
}

void scale2x_16( SDL_Surface *src_surface, SDL_Surface *dst_surface )
{
	Uint8 *src = src_surface->pixels, *src_temp,
	      *dst = dst_surface->pixels, *dst_temp;
	int src_pitch = src_surface->pitch,
	    dst_pitch = dst_surface->pitch;
	const int dst_Bpp = 2;         // dst_surface->format->BytesPerPixel
	
	const int height = vga_height, // src_surface->h
	          width = vga_width;   // src_surface->w
	
	int prevline, nextline;
	
	Uint16 E0, E1, E2, E3, B, D, E, F, H;
	for (int y = 0; y < height; y++)
	{
		src_temp = src;
		dst_temp = dst;
		
		prevline = (y > 0) ? -src_pitch : 0;
		nextline = (y < height - 1) ? src_pitch : 0;
		
		for (int x = 0; x < width; x++)
		{
			B = rgb_palette[*(src + prevline)];
			D = rgb_palette[*(x > 0 ? src - 1 : src)];
			E = rgb_palette[*src];
			F = rgb_palette[*(x < width - 1 ? src + 1 : src)];
			H = rgb_palette[*(src + nextline)];
			
			if (B != H && D != F) {
				E0 = D == B ? D : E;
				E1 = B == F ? F : E;
				E2 = D == H ? D : E;
				E3 = H == F ? F : E;
			} else {
				E0 = E1 = E2 = E3 = E;
			}
			
			*(Uint16 *)dst = E0;
			*(Uint16 *)(dst + dst_Bpp) = E1;
			*(Uint16 *)(dst + dst_pitch) = E2;
			*(Uint16 *)(dst + dst_pitch + dst_Bpp) = E3;
			
			src++;
			dst += 2 * dst_Bpp;
		}
		
		src = src_temp + src_pitch;
		dst = dst_temp + 2 * dst_pitch;
	}
}


void scale3x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface )
{
	Uint8 *src = src_surface->pixels, *src_temp,
	      *dst = dst_surface->pixels, *dst_temp;
	int src_pitch = src_surface->pitch,
	    dst_pitch = dst_surface->pitch;
	const int dst_Bpp = 4;         // dst_surface->format->BytesPerPixel
	
	const int height = vga_height, // src_surface->h
	          width = vga_width;   // src_surface->w
	
	int prevline, nextline;
	
	Uint32 E0, E1, E2, E3, E4, E5, E6, E7, E8, A, B, C, D, E, F, G, H, I;
	for (int y = 0; y < height; y++)
	{
		src_temp = src;
		dst_temp = dst;
		
		prevline = (y > 0) ? -src_pitch : 0;
		nextline = (y < height - 1) ? src_pitch : 0;
		
		for (int x = 0; x < width; x++)
		{
			A = rgb_palette[*(src + prevline - (x > 0 ? 1 : 0))];
			B = rgb_palette[*(src + prevline)];
			C = rgb_palette[*(src + prevline + (x < width - 1 ? 1 : 0))];
			D = rgb_palette[*(src - (x > 0 ? 1 : 0))];
			E = rgb_palette[*src];
			F = rgb_palette[*(src + (x < width - 1 ? 1 : 0))];
			G = rgb_palette[*(src + nextline - (x > 0 ? 1 : 0))];
			H = rgb_palette[*(src + nextline)];
			I = rgb_palette[*(src + nextline + (x < width - 1 ? 1 : 0))];
			
			if (B != H && D != F) {
				E0 = D == B ? D : E;
				E1 = (D == B && E != C) || (B == F && E != A) ? B : E;
				E2 = B == F ? F : E;
				E3 = (D == B && E != G) || (D == H && E != A) ? D : E;
				E4 = E;
				E5 = (B == F && E != I) || (H == F && E != C) ? F : E;
				E6 = D == H ? D : E;
				E7 = (D == H && E != I) || (H == F && E != G) ? H : E;
				E8 = H == F ? F : E;
			} else {
				E0 = E1 = E2 = E3 = E4 = E5 = E6 = E7 = E8 = E;
			}
			
			*(Uint32 *)dst = E0;
			*(Uint32 *)(dst + dst_Bpp) = E1;
			*(Uint32 *)(dst + 2 * dst_Bpp) = E2;
			*(Uint32 *)(dst + dst_pitch) = E3;
			*(Uint32 *)(dst + dst_pitch + dst_Bpp) = E4;
			*(Uint32 *)(dst + dst_pitch + 2 * dst_Bpp) = E5;
			*(Uint32 *)(dst + 2 * dst_pitch) = E6;
			*(Uint32 *)(dst + 2 * dst_pitch + dst_Bpp) = E7;
			*(Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp) = E8;
			
			src++;
			dst += 3 * dst_Bpp;
		}
		
		src = src_temp + src_pitch;
		dst = dst_temp + 3 * dst_pitch;
	}
}

void scale3x_16( SDL_Surface *src_surface, SDL_Surface *dst_surface )
{
	Uint8 *src = src_surface->pixels, *src_temp,
	      *dst = dst_surface->pixels, *dst_temp;
	int src_pitch = src_surface->pitch,
	    dst_pitch = dst_surface->pitch;
	const int dst_Bpp = 2;         // dst_surface->format->BytesPerPixel
	
	const int height = vga_height, // src_surface->h
	          width = vga_width;   // src_surface->w
	
	int prevline, nextline;
	
	Uint16 E0, E1, E2, E3, E4, E5, E6, E7, E8, A, B, C, D, E, F, G, H, I;
	for (int y = 0; y < height; y++)
	{
		src_temp = src;
		dst_temp = dst;
		
		prevline = (y > 0) ? -src_pitch : 0;
		nextline = (y < height - 1) ? src_pitch : 0;
		
		for (int x = 0; x < width; x++)
		{
			A = rgb_palette[*(src + prevline - (x > 0 ? 1 : 0))];
			B = rgb_palette[*(src + prevline)];
			C = rgb_palette[*(src + prevline + (x < width - 1 ? 1 : 0))];
			D = rgb_palette[*(src - (x > 0 ? 1 : 0))];
			E = rgb_palette[*src];
			F = rgb_palette[*(src + (x < width - 1 ? 1 : 0))];
			G = rgb_palette[*(src + nextline - (x > 0 ? 1 : 0))];
			H = rgb_palette[*(src + nextline)];
			I = rgb_palette[*(src + nextline + (x < width - 1 ? 1 : 0))];
			
			if (B != H && D != F) {
				E0 = D == B ? D : E;
				E1 = (D == B && E != C) || (B == F && E != A) ? B : E;
				E2 = B == F ? F : E;
				E3 = (D == B && E != G) || (D == H && E != A) ? D : E;
				E4 = E;
				E5 = (B == F && E != I) || (H == F && E != C) ? F : E;
				E6 = D == H ? D : E;
				E7 = (D == H && E != I) || (H == F && E != G) ? H : E;
				E8 = H == F ? F : E;
			} else {
				E0 = E1 = E2 = E3 = E4 = E5 = E6 = E7 = E8 = E;
			}
			
			*(Uint16 *)dst = E0;
			*(Uint16 *)(dst + dst_Bpp) = E1;
			*(Uint16 *)(dst + 2 * dst_Bpp) = E2;
			*(Uint16 *)(dst + dst_pitch) = E3;
			*(Uint16 *)(dst + dst_pitch + dst_Bpp) = E4;
			*(Uint16 *)(dst + dst_pitch + 2 * dst_Bpp) = E5;
			*(Uint16 *)(dst + 2 * dst_pitch) = E6;
			*(Uint16 *)(dst + 2 * dst_pitch + dst_Bpp) = E7;
			*(Uint16 *)(dst + 2 * dst_pitch + 2 * dst_Bpp) = E8;
			
			src++;
			dst += 3 * dst_Bpp;
		}
		
		src = src_temp + src_pitch;
		dst = dst_temp + 3 * dst_pitch;
	}
}

// kate: tab-width 4; vim: set noet:
