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
#include "opentyr.h"
#include "backgrnd.h"

#include "config.h"
#include "varz.h"
#include "video.h"

#include <assert.h>

/*Special Background 2 and Background 3*/

/*Back Pos 3*/
JE_word backPos, backPos2, backPos3;
JE_word backMove, backMove2, backMove3;

/*Main Maps*/
JE_word mapX, mapY, mapX2, mapX3, mapY2, mapY3;
JE_byte **mapYPos, **mapY2Pos, **mapY3Pos;
JE_word mapXPos, oldMapXOfs, mapXOfs, mapX2Ofs, mapX2Pos, mapX3Pos, oldMapX3Ofs, mapX3Ofs, tempMapXOfs;
intptr_t mapXbpPos, mapX2bpPos, mapX3bpPos;
JE_byte map1YDelay, map1YDelayMax, map2YDelay, map2YDelayMax;


SDL_Surface *smoothiesScreen;
JE_boolean  anySmoothies;
JE_byte     SDAT[9]; /* [1..9] */

void JE_darkenBackground( JE_word neat )  /* wild detail level */
{
	Uint8 *s = VGAScreen->pixels; /* screen pointer, 8-bit specific */
	int x, y;
	
	s += 24;
	
	for (y = 184; y; y--)
	{
		for (x = 264; x; x--)
		{
			*s = ((((*s & 0x0f) << 4) - (*s & 0x0f) + ((((x - neat - y) >> 2) + *(s-2) + (y == 184 ? 0 : *(s-(VGAScreen->pitch-1)))) & 0x0f)) >> 4) | (*s & 0xf0);
			s++;
		}
		s += VGAScreen->pitch - 264;
	}
}

void blit_background_row( SDL_Surface *surface, int x, int y, Uint8 **map )
{
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (int y = 0; y < 28; y++)
	{
		// not drawing on screen yet; skip y
		if ((pixels + (12 * 24)) < pixels_ll)
		{
			pixels += surface->pitch;
			continue;
		}
		
		for (int tile = 0; tile < 12; tile++)
		{
			Uint8 *data = *(map + tile);
			
			// no tile; skip tile
			if (data == NULL)
			{
				pixels += 24;
				continue;
			}
			
			data += y * 24;
			
			for (int x = 24; x; x--)
			{
				if (pixels >= pixels_ul)
					return;
				if (pixels >= pixels_ll && *data != 0)
					*pixels = *data;
				
				pixels++;
				data++;
			}
		}
		
		pixels += surface->pitch - 12 * 24;
	}
}

void blit_background_row_blend( SDL_Surface *surface, int x, int y, Uint8 **map )
{
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (int y = 0; y < 28; y++)
	{
		// not drawing on screen yet; skip y
		if ((pixels + (12 * 24)) < pixels_ll)
		{
			pixels += surface->pitch;
			continue;
		}
		
		for (int tile = 0; tile < 12; tile++)
		{
			Uint8 *data = *(map + tile);
			
			// no tile; skip tile
			if (data == NULL)
			{
				pixels += 24;
				continue;
			}
			
			data += y * 24;
			
			for (int x = 24; x; x--)
			{
				if (pixels >= pixels_ul)
					return;
				if (pixels >= pixels_ll && *data != 0)
					*pixels = (*data & 0xf0) | (((*pixels & 0x0f) + (*data & 0x0f)) / 2);
				
				pixels++;
				data++;
			}
		}
		
		pixels += surface->pitch - 12 * 24;
	}
}

void draw_background_1( SDL_Surface *surface )
{
	SDL_FillRect(surface, NULL, 0);
	
	Uint8 **map = (Uint8 **)mapYPos + mapXbpPos - 12;
	
	for (int i = -1; i < 7; i++)
	{
		blit_background_row(surface, mapXPos, (i * 28) + backPos, map);
		
		map += 14;
	}
}

void draw_background_2( SDL_Surface *surface )
{
	if (map2YDelayMax > 1 && backMove2 < 2)
		backMove2 = (map2YDelay == 1) ? 1 : 0;
	
	if (background2 != 0)
	{
		// water effect combines background 1 and 2 by syncronizing the x coordinate
		int x = smoothies[1] ? mapXPos : mapX2Pos;
		
		Uint8 **map = (Uint8 **)mapY2Pos + (smoothies[1] ? mapXbpPos : mapX2bpPos) - 12;
		
		for (int i = -1; i < 7; i++)
		{
			blit_background_row(surface, x, (i * 28) + backPos2, map);
			
			map += 14;
		}
	}
	
	/*Set Movement of background*/
	if (--map2YDelay == 0)
	{
		map2YDelay = map2YDelayMax;
		
		backPos2 += backMove2;
		
		if (backPos2 >  27)
		{
			backPos2 -= 28;
			mapY2--;
			mapY2Pos -= 14;  /*Map Width*/
		}
	}
}

void draw_background_2_blend( SDL_Surface *surface )
{
	if (map2YDelayMax > 1 && backMove2 < 2)
		backMove2 = (map2YDelay == 1) ? 1 : 0;
	
	Uint8 **map = (Uint8 **)mapY2Pos + mapX2bpPos - 12;
	
	for (int i = -1; i < 7; i++)
	{
		blit_background_row_blend(surface, mapX2Pos, (i * 28) + backPos2, map);
		
		map += 14;
	}
	
	/*Set Movement of background*/
	if (--map2YDelay == 0)
	{
		map2YDelay = map2YDelayMax;
		
		backPos2 += backMove2;
		
		if (backPos2 >  27)
		{
			backPos2 -= 28;
			mapY2--;
			mapY2Pos -= 14;  /*Map Width*/
		}
	}
}

void draw_background_3( SDL_Surface *surface )
{
	/* Movement of background */
	backPos3 += backMove3;
	
	if (backPos3 > 27)
	{
		backPos3 -= 28;
		mapY3--;
		mapY3Pos -= 15;   /*Map Width*/
	}
	
	Uint8 **map = (Uint8 **)mapY3Pos + mapX3bpPos - 12;
	
	for (int i = -1; i < 7; i++)
	{
		blit_background_row(surface, mapX3Pos, (i * 28) + backPos3, map);
		
		map += 15;
	}
}

void JE_filterScreen( JE_shortint col, JE_shortint int_)
{
	Uint8 *s = NULL; /* screen pointer, 8-bit specific */
	int x, y;
	unsigned int temp;
	
	if (filterFade)
	{
		levelBrightness += levelBrightnessChg;
		if ((filterFadeStart && levelBrightness < -14) || levelBrightness > 14)
		{
			levelBrightnessChg = -levelBrightnessChg;
			filterFadeStart = false;
			levelFilter = levelFilterNew;
		}
		if (!filterFadeStart && levelBrightness == 0)
		{
			filterFade = false;
			levelBrightness = -99;
		}
	}
	
	if (col != -99 && filtrationAvail)
	{
		s = VGAScreen->pixels;
		s += 24;
		
		col <<= 4;
		
		for (y = 184; y; y--)
		{
			for (x = 264; x; x--)
			{
				*s = col | (*s & 0x0f);
				s++;
			}
			s += VGAScreen->pitch - 264;
		}
	}
	
	if (int_ != -99 && explosionTransparent)
	{
		s = VGAScreen->pixels;
		s += 24;
		
		for (y = 184; y; y--)
		{
			for (x = 264; x; x--)
			{
				temp = (*s & 0x0f) + int_;
				*s = (*s & 0xf0) | (temp >= 0x1f ? 0 : (temp >= 0x0f ? 0x0f : temp));
				s++;
			}
			s += VGAScreen->pitch - 264;
		}
	}
}

void JE_checkSmoothies( void )
{
	anySmoothies = false;
	if ((processorType > 2 && (smoothies[1-1] || smoothies[2-1])) || (processorType > 1 && (smoothies[3-1] || smoothies[4-1] || smoothies[5-1])))
	{
		anySmoothies = true;
		JE_initSmoothies();
	}
}

void JE_initSmoothies( void )
{
	smoothiesScreen = VGAScreen2;
}

void JE_smoothies1( void ) /*Lava Effect*/
{
	Uint8 *s = game_screen->pixels; /* screen pointer, 8-bit specific */
	Uint8 *src = VGAScreen->pixels; /* screen pointer, 8-bit specific */
	int i, j, temp;
	
	s += game_screen->pitch * 185 - 1;
	src += game_screen->pitch * 185 - 1;
	
	for (i = 185 * game_screen->pitch; i; i -= 8)
	{
		temp = (((i - 1) >> 9) & 15) - 8;
		temp = (temp < 0 ? -temp : temp) - 1;
		
		for (j = 8; j; j--)
		{
			Uint8 temp_s = (*(src + temp) & 0x0f) * 2;
			temp_s += *(s + temp + game_screen->pitch) & 0x0f;
			temp_s += (i + temp < game_screen->pitch) ? 0 : *(s + temp - game_screen->pitch) & 0x0f;
			*s = (temp_s >> 2) | 0x70;
			s--;
			src--;
		}
	}
	VGAScreen = game_screen;
}

void JE_smoothies2( void ) /*Water effect*/
{
	Uint8 *s = game_screen->pixels; /* screen pointer, 8-bit specific */
	Uint8 *src = VGAScreen->pixels; /* screen pointer, 8-bit specific */
	int i, j, temp;

	s += game_screen->pitch * 185 - 1;
	src += game_screen->pitch * 185 - 1;

	for (i = 185 * game_screen->pitch; i; i -= 8)
	{
		temp = (((i - 1) >> 10) & 7) - 4;
		temp = (temp < 0 ? -temp : temp) - 1;
		
		for (j = 8; j; j--)
		{
			if (*src & 0x30)
			{
				Uint8 temp_s = *src & 0x0f;
				temp_s += *(s + temp + game_screen->pitch) & 0x0f;
				*s = (temp_s >> 1) | (SDAT[2-1] << 4);
			} else
				*s = *src;
			s--;
			src--;
		}
	}
	VGAScreen = game_screen;
}

void JE_smoothies3( void ) /* iced motion blur */
{
	Uint8 *s = game_screen->pixels; /* screen pointer, 8-bit specific */
	Uint8 *src = VGAScreen->pixels; /* screen pointer, 8-bit specific */
	int i;
	
	for (i = 184 * game_screen->pitch; i; i--)
	{
		*s = ((((*src & 0x0f) + (*s & 0x0f)) >> 1) & 0x0f) | 0x80;
		s++;
		src++;
	}
	VGAScreen = game_screen;
}

void JE_smoothies4( void ) /* motion blur */
{
	Uint8 *s = game_screen->pixels; /* screen pointer, 8-bit specific */
	Uint8 *src = VGAScreen->pixels; /* screen pointer, 8-bit specific */
	int i;
	
	for (i = 184 * game_screen->pitch; i; i--)
	{
		*s = ((((*src & 0x0f) + (*s & 0x0f)) >> 1) & 0x0f) | (*src & 0xf0);
		s++;
		src++;
	}
	VGAScreen = game_screen;
}

// kate: tab-width 4; vim: set noet:
