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
#include "backgrnd.h"

#include "config.h"
#include "mtrand.h"
#include "opentyr.h"
#include "swar.h"
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

JE_boolean  anySmoothies;
JE_byte     smoothie_data[9]; /* [1..9] */

void JE_darkenBackground(JE_word neat)  /* wild detail level */
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

// Draw row of 12 tiles from map.
// Each tile is 24x28 and is opaque.
// Visible area is located at (24, 0) and is 264x184.  Tiles are drawn beyond
// this area to avoid the need to clip the left and right edges and to support
// wavey smoothies.
static void blitBackground1Row(SDL_Surface *dst, int_fast16_t x, int_fast16_t y, Uint8 **map)
{
	assert(x >= 0 && x < 24);
	assert(y >= -28 && y < vga_height);

	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;

	int_fast16_t ty = 0;
	int_fast16_t th = 28;

	// Skip pixel rows that are above top of screen.
	if (y < 0)
	{
		ty = -y;
		y = 0;
	}
	// Stop when pixel row would be below bottom of screen.
	else if (y > 184 - th)
	{
		th = 184 - y;
	}

	size_t di = y * dst_pitch + x;

	for (; ty < th; ++ty)
	{
		for (int ti = 0; ti < 12; ++ti)
		{
			Uint8 *data = map[ti];
			assert(data != NULL);

			data += ty * 24;

			memcpy(&dst_pixels[di], data, 24);

			di += 24;
		}

		di += dst_pitch - 12 * 24;
	}
}

// Draw row of 12 tiles from map.
// Each tile is 24x28 and can have transparent pixels.
// Visible area is located at (24, 0) and is 264x184.  Tiles are drawn beyond
// this area to avoid the need to clip the left and right edges and to support
// wavey smoothies.
static void blitBackgroundRow(SDL_Surface *dst, int_fast16_t x, int_fast16_t y, Uint8 **map)
{
	assert(x >= 0 && x < 24);
	assert(y >= -28 && y < vga_height);

	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;

	int_fast16_t ty = 0;
	int_fast16_t th = 28;

	// Skip pixel rows that are above top of screen.
	if (y < 0)
	{
		ty = -y;
		y = 0;
	}
	// Stop when pixel row would be below bottom of screen.
	else if (y > 184 - th)
	{
		th = 184 - y;
	}

	size_t di = y * dst_pitch + x;

	for (; ty < th; ++ty)
	{
		for (int ti = 0; ti < 12; ++ti)
		{
			Uint8 *data = map[ti];

			// Skip transparent tile.
			if (data == NULL)
			{
				di += 24;
				continue;
			}

			data += ty * 24;

#if SWAR8_LANES > 2
			for (int tx = 24; tx > 0; tx -= sizeof(Swar8))
			{
				Swar8 dstPixels = swar8Read(&dst_pixels[di]);
				Swar8 srcPixels = swar8Read(data);

				// Mask is 0xFF if pixel is not 0x00, otherwise 0x00.
				Swar8 bit7 = (srcPixels & SWAR8(0x7F)) + SWAR8(0x7F);
				Swar8 bit = (srcPixels | bit7) & SWAR8(0x80);
				Swar8 mask = bit | (bit - (bit >> 7));

				// Conditionally select each lane by mask.
				dstPixels ^= (dstPixels ^ srcPixels) & mask;

				swar8Write(&dst_pixels[di], dstPixels);

				data += sizeof(Swar8);
				di += sizeof(Swar8);
			}
#else
			for (int tx = 24; tx > 0; --tx)
			{
				if (*data != 0)
					dst_pixels[di] = *data;

				data += 1;
				di += 1;
			}
#endif
		}

		di += dst_pitch - 12 * 24;
	}
}

// Draw row of 12 tiles from map with blending.
// Each tile is 24x28 and can have transparent pixels.
// Visible area is located at (24, 0) and is 264x184.  Tiles are drawn beyond
// this area to avoid the need to clip the left and right edges and to support
// wavey smoothies.
static void blitBackgroundRowBlend(SDL_Surface *dst, int_fast16_t x, int_fast16_t y, Uint8 **map)
{
	assert(x >= 0 && x < 24);
	assert(y >= -28 && y < vga_height);

	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;

	int_fast16_t ty = 0;
	int_fast16_t th = 28;

	// Skip pixel rows that are above top of screen.
	if (y < 0)
	{
		ty = -y;
		y = 0;
	}
	// Stop when pixel row would be below bottom of screen.
	else if (y > 184 - th)
	{
		th = 184 - y;
	}

	size_t di = y * dst_pitch + x;

	for (; ty < th; ++ty)
	{
		for (int ti = 0; ti < 12; ++ti)
		{
			Uint8 *data = map[ti];

			// Skip transparent tile.
			if (data == NULL)
			{
				di += 24;
				continue;
			}

			data += ty * 24;

#if SWAR8_LANES > 2
			for (int tx = 24; tx > 0; tx -= sizeof(Swar8))
			{
				Swar8 dstPixels = swar8Read(&dst_pixels[di]);
				Swar8 srcPixels = swar8Read(data);

				// Mask is 0xFF if pixel is not 0x00, otherwise 0x00.
				Swar8 bit7 = (srcPixels & SWAR8(0x7F)) + SWAR8(0x7F);
				Swar8 bit = (srcPixels | bit7) & SWAR8(0x80);
				Swar8 mask = bit | (bit - (bit >> 7));

				const Swar8 huesMask = SWAR8(0xF0);
				Swar8 hues = srcPixels & huesMask;
				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = srcPixels & valuesMask;
				values += dstPixels & valuesMask;
				values = (values >> 1) & valuesMask;
				srcPixels = hues | values;

				// Conditionally select each lane by mask.
				dstPixels ^= (dstPixels ^ srcPixels) & mask;

				swar8Write(&dst_pixels[di], dstPixels);

				data += sizeof(Swar8);
				di += sizeof(Swar8);
			}
#else
			for (int tx = 24; tx > 0; --tx)
			{
				if (*data != 0)
				{
					Uint8 hue = *data & 0xF0;
					Uint8 value = *data & 0x0F;
					value += dst_pixels[di] & 0x0F;
					dst_pixels[di] = hue | (value >> 1);
				}

				data += 1;
				di += 1;
			}
#endif
		}

		di += dst_pitch - 12 * 24;
	}
}

void draw_background_1(SDL_Surface *surface)
{
	assert(surface->w == vga_width &&
	       surface->h >= vga_height &&
	       surface->format->BitsPerPixel == 8);

	Uint8 **map = (Uint8 **)mapYPos + mapXbpPos - 12;
	
	for (int i = -1; i < 7; i++)
	{
		blitBackground1Row(surface, mapXPos, (i * 28) + backPos, map);
		
		map += 14;
	}
}

void draw_background_2(SDL_Surface *surface)
{
	assert(surface->w == vga_width &&
	       surface->h >= vga_height &&
	       surface->format->BitsPerPixel == 8);

	if (map2YDelayMax > 1 && backMove2 < 2)
		backMove2 = (map2YDelay == 1) ? 1 : 0;
	
	if (background2 != 0)
	{
		// water effect combines background 1 and 2 by synchronizing the x coordinate
		int x = smoothies[1] ? mapXPos : mapX2Pos;
		
		Uint8 **map = (Uint8 **)mapY2Pos + (smoothies[1] ? mapXbpPos : mapX2bpPos) - 12;
		
		for (int i = -1; i < 7; i++)
		{
			blitBackgroundRow(surface, x, (i * 28) + backPos2, map);
			
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

void draw_background_2_blend(SDL_Surface *surface)
{
	assert(surface->w == vga_width &&
	       surface->h >= vga_height &&
	       surface->format->BitsPerPixel == 8);

	if (map2YDelayMax > 1 && backMove2 < 2)
		backMove2 = (map2YDelay == 1) ? 1 : 0;
	
	Uint8 **map = (Uint8 **)mapY2Pos + mapX2bpPos - 12;
	
	for (int i = -1; i < 7; i++)
	{
		blitBackgroundRowBlend(surface, mapX2Pos, (i * 28) + backPos2, map);
		
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

void draw_background_3(SDL_Surface *surface)
{
	assert(surface->w == vga_width &&
	       surface->h >= vga_height &&
	       surface->format->BitsPerPixel == 8);

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
		blitBackgroundRow(surface, mapX3Pos, (i * 28) + backPos3, map);
		
		map += 15;
	}
}

static void filterScreenHue(SDL_Surface *screen, Uint8 hue)
{
	Uint8 *screen_pixels = screen->pixels;
	int screen_pitch = screen->pitch;

	// Hue is configurable.  Value is unmodified.

	hue = hue << 4;
	Swar8 hues = SWAR8(hue);

	size_t i = 24;

	for (int y = 184; y > 0; --y)
	{
		for (int x = 264; x > 0; x -= sizeof(Swar8))
		{
			const Swar8 valuesMask = SWAR8(0x0F);
			Swar8 values = swar8Read(&screen_pixels[i]) & valuesMask;
			swar8Write(&screen_pixels[i], hues | values);

			i += sizeof(Swar8);
		}

		i += screen_pitch - 264;
	}
}

static void filterScreenValue(SDL_Surface *screen, Sint8 brightness)
{
	Uint8 *screen_pixels = screen->pixels;
	int screen_pitch = screen->pitch;

	// Hue is unmodified.  Value is sum of value and brightness if sum is
	// 0x0..0xF, 0xF if sum is 0x10..0x1E, and 0x0 otherwise.

	if (brightness == 0)
		return;

	size_t i = 24;

	if (brightness <= -0xF || brightness > 0x1E)
	{
		// In this case, value will always be 0x0.

		for (int y = 184; y > 0; --y)
		{
			for (int x = 264; x > 0; x -= sizeof(Swar8))
			{
				Swar8 pixels = swar8Read(&screen_pixels[i]);
				const Swar8 huesMask = SWAR8(0xF0);
				Swar8 hues = pixels & huesMask;

				swar8Write(&screen_pixels[i], hues);

				i += sizeof(Swar8);
			}

			i += screen_pitch - 264;
		}
	}
	else if (brightness <= 0)  // -0xE..0x0
	{
		// In this case, value will be the sum or 0x0 on overflow (borrow).

		// Addend is 0x02..0x10.  Only lower nibble of sum needs to be
		// correct, so adding 0x10 does not affect the sum.
		Swar8 addend = SWAR8((Uint8)(brightness + 0x10));

		for (int y = 184; y > 0; --y)
		{
			for (int x = 264; x > 0; x -= sizeof(Swar8))
			{
				Swar8 pixels = swar8Read(&screen_pixels[i]);
				const Swar8 huesMask = SWAR8(0xF0);
				Swar8 hues = pixels & huesMask;
				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = pixels & valuesMask;

				Swar8 sum = values + addend;

				// Mask is 0x0F if value + brightness >= 0x00, otherwise 0x00.
				Swar8 bit = sum & SWAR8(0x10);
				Swar8 mask = bit - (bit >> 4);
				values = sum & mask;

				swar8Write(&screen_pixels[i], hues | values);

				i += sizeof(Swar8);
			}

			i += screen_pitch - 264;
		}
	}
	else if (brightness <= 0xF)  // 0x1..0xF
	{
		// In this case, value will be the sum or 0xF on overflow (carry).

		// Addend is 0x01..0x0F.
		Swar8 addend = SWAR8((Uint8)brightness);

		for (int y = 184; y > 0; --y)
		{
			for (int x = 264; x > 0; x -= sizeof(Swar8))
			{
				Swar8 pixels = swar8Read(&screen_pixels[i]);
				const Swar8 huesMask = SWAR8(0xF0);
				Swar8 hues = pixels & huesMask;
				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = pixels & valuesMask;

				Swar8 sum = values + addend;

				// Mask is 0x0F if value + brightness >= 0x10, otherwise 0x00.
				Swar8 bit = sum & SWAR8(0x10);
				Swar8 mask = bit - (bit >> 4);
				values = (sum & SWAR8(0x0F)) | mask;

				swar8Write(&screen_pixels[i], hues | values);

				i += sizeof(Swar8);
			}

			i += screen_pitch - 264;
		}
	}
	else  // 0x10..0x1E
	{
		// In this case, value will be either 0xF or 0x0.

		// Addend is 0x01..0x0F.
		Swar8 addend = SWAR8((Uint8)(brightness - 0xF));

		for (int y = 184; y > 0; --y)
		{
			for (int x = 264; x > 0; x -= sizeof(Swar8))
			{
				Swar8 pixels = swar8Read(&screen_pixels[i]);
				const Swar8 huesMask = SWAR8(0xF0);
				Swar8 hues = pixels & huesMask;
				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = pixels & valuesMask;

				Swar8 sum = values + addend;

				// Mask is 0x0F if value + brightness >= 0x1F, otherwise 0x00.
				Swar8 bit = sum & SWAR8(0x10);
				Swar8 mask = bit - (bit >> 4);
				values = SWAR8(0x0F) & ~mask;

				swar8Write(&screen_pixels[i], hues | values);

				i += sizeof(Swar8);
			}

			i += screen_pitch - 264;
		}
	}
}

void JE_filterScreen(JE_shortint col, JE_shortint brightness)
{
	assert(VGAScreen->w == vga_width &&
	       VGAScreen->h >= vga_height &&
	       VGAScreen->format->BitsPerPixel == 8);

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
		filterScreenHue(VGAScreen, col);
	
	if (brightness != -99 && explosionTransparent)
		filterScreenValue(VGAScreen, brightness);
}

void JE_checkSmoothies(void)
{
	anySmoothies = (processorType > 2 && (smoothies[1-1] || smoothies[2-1])) || (processorType > 1 && (smoothies[3-1] || smoothies[4-1] || smoothies[5-1]));
}

void lava_filter(SDL_Surface *dst, SDL_Surface *src)
{
	assert(src->w == vga_width &&
	       src->h >= vga_height &&
	       src->format->BitsPerPixel == 8);
	assert(dst->w == vga_width &&
	       dst->h >= vga_height &&
	       dst->format->BitsPerPixel == 8);

	Uint8 *src_pixels = src->pixels;
	int src_pitch = src->pitch;
	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;

	// The original implementation also worked on 8-pixel chunks, but the chunks
	// were misaligned, so this implementation is slightly different, but it's
	// faster and essentially indistinguishable.

	unsigned int diw = vga_width * 185;
	size_t di = dst_pitch * 185;
	size_t si = src_pitch * 185;

	for (int y = 185; y > 1; --y)
	{
		di -= dst_pitch - vga_width;
		si -= src_pitch - vga_width;

		for (int x = vga_width; x > 0; x -= 8, diw -= 8)
		{
			int wavey = abs((int)((diw >> 9) & 0xF) - 8) - 1;  // -1..7

			// Value is average of values of wavey source pixel (2x), wavey
			// destination pixel below, wavey and destination pixel above.
			// Hue is red.
			for (int i = 8; i > 0; i -= sizeof(Swar8))
			{
				di -= sizeof(Swar8);
				si -= sizeof(Swar8);

				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = (swar8Read(&src_pixels[si + wavey]) & valuesMask) << 1;
				values += swar8Read(&dst_pixels[di + wavey + dst_pitch]) & valuesMask;
				values += swar8Read(&dst_pixels[di + wavey - dst_pitch]) & valuesMask;
				values = (values >> 2) & valuesMask;
				const Swar8 redHues = SWAR8(0x70);
				swar8Write(&dst_pixels[di], redHues | values);
			}
		}
	}

	di -= dst_pitch - vga_width;
	si -= src_pitch - vga_width;

	for (int x = vga_width; x > 0; x -= 8, diw -= 8)
	{
		int wavey = abs((int)((diw >> 9) & 0xF) - 8) - 1;
		assert(wavey > 0);

		for (int i = 8; i > 0; i -= sizeof(Swar8))
		{
			di -= sizeof(Swar8);
			si -= sizeof(Swar8);

			const Swar8 valuesMask = SWAR8(0x0F);
			Swar8 values = (swar8Read(&src_pixels[si + wavey]) & valuesMask) << 1;
			values += swar8Read(&dst_pixels[di + wavey + dst_pitch]) & valuesMask;
			// Assume values above top row of pixels are zero.
			values = (values >> 2) & valuesMask;
			const Swar8 hues = SWAR8(0x70);
			swar8Write(&dst_pixels[di], hues | values);
		}
	}

	assert(diw == 0);
	assert(di == 0);
	assert(si == 0);
}

void water_filter(SDL_Surface *dst, SDL_Surface *src)
{
	assert(src->w == vga_width &&
	       src->h >= vga_height &&
	       src->format->BitsPerPixel == 8);
	assert(dst->w == vga_width &&
	       dst->h >= vga_height &&
	       dst->format->BitsPerPixel == 8);

	Uint8 *src_pixels = src->pixels;
	int src_pitch = src->pitch;
	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;

	// The original implementation also worked on 8-pixel chunks, but the chunks
	// were misaligned, so this implementation is slightly different, but it's
	// faster and essentially indistinguishable.

	Uint8 hue = smoothie_data[2-1] << 4;
#if SWAR8_LANES > 2
	Swar8 hues = SWAR8(hue);
#endif

	unsigned int diw = vga_width * 185;
	size_t di = dst_pitch * 185;
	size_t si = src_pitch * 185;

	for (int y = 185; y > 0; --y)
	{
		di -= dst_pitch - vga_width;
		si -= src_pitch - vga_width;

		for (int x = vga_width; x > 0; x -= 8, diw -= 8)
		{
			int wavey = abs((int)((diw >> 10) & 0x7) - 4) - 1;  // -1..3

			// Pixel is copied from source if hue is gray or cloud-blue.  Otherwise:
			// Value is average of values of source pixel and wavey destination
			// pixel below.  Hue is configurable.
#if SWAR8_LANES > 2
			for (int i = 8; i > 0; i -= sizeof(Swar8))
			{
				di -= sizeof(Swar8);
				si -= sizeof(Swar8);

				Swar8 srcPixels = swar8Read(&src_pixels[si]);

				Swar8 waterMask = srcPixels & SWAR8(0x30);
				// Put 0x01 in lanes where either of the bits is set, and 0x00 in others.
				waterMask = ((waterMask >> 5) | (waterMask >> 4)) & SWAR8(0x01);
				// Put 0xFF in lanes that were 0x01.
				waterMask = (waterMask << 8) - waterMask;

				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = srcPixels & valuesMask;
				values += swar8Read(&dst_pixels[di + wavey + dst_pitch]) & valuesMask;
				values = (values >> 1) & valuesMask;
				Swar8 waterPixels = hues | values;

				// Conditionally select each lane by mask.
				Swar8 dstPixels = srcPixels ^ ((srcPixels ^ waterPixels) & waterMask);

				swar8Write(&dst_pixels[di], dstPixels);
		}
#else
		for (int i = 8; i > 0; --i)
		{
				di -= 1;
				si -= 1;

				Uint8 srcPixel = src_pixels[si];
				if ((srcPixel & 0x30) == 0)
				{
					dst_pixels[di] = srcPixel;
				}
				else
				{
					Uint8 value = srcPixel & 0x0F;
					value += dst_pixels[di + wavey + dst_pitch] & 0x0F;
					dst_pixels[di] = hue | (value >> 1);
				}
			}
#endif
		}
	}

	assert(diw == 0);
	assert(di == 0);
	assert(si == 0);
}

void iced_blur_filter(SDL_Surface *dst, SDL_Surface *src)
{
	assert(src->w == vga_width &&
	       src->h >= vga_height &&
	       src->format->BitsPerPixel == 8);
	assert(dst->w == vga_width &&
	       dst->h >= vga_height &&
	       dst->format->BitsPerPixel == 8);

	Uint8 *src_pixels = src->pixels;
	int src_pitch = src->pitch;
	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;

	size_t di = dst_pitch * 184;
	size_t si = src_pitch * 184;

	for (int y = 184; y > 0; --y)
	{
		di -= dst_pitch - vga_width;
		si -= src_pitch - vga_width;

		for (int x = vga_width; x > 0; x -= 8)
		{
			// Value is average of values of source pixel and destination pixel.
			// Hue is ice-blue.
			for (int i = 8; i > 0; i -= sizeof(Swar8))
			{
				di -= sizeof(Swar8);
				si -= sizeof(Swar8);

				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = swar8Read(&src_pixels[si]) & valuesMask;
				values += swar8Read(&dst_pixels[di]) & valuesMask;
				values = (values >> 1) & valuesMask;
				const Swar8 hues = SWAR8(0x80);
				swar8Write(&dst_pixels[di], hues | values);
			}
		}
	}

	assert(di == 0);
	assert(si == 0);
}

void blur_filter(SDL_Surface *dst, SDL_Surface *src)
{
	assert(src->w == vga_width &&
	       src->h >= vga_height &&
	       src->format->BitsPerPixel == 8);
	assert(dst->w == vga_width &&
	       dst->h >= vga_height &&
	       dst->format->BitsPerPixel == 8);

	Uint8 *src_pixels = src->pixels;
	int src_pitch = src->pitch;
	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;

	size_t di = dst_pitch * 184;
	size_t si = src_pitch * 184;

	for (int y = 184; y > 0; --y)
	{
		di -= dst_pitch - vga_width;
		si -= src_pitch - vga_width;

		for (int x = vga_width; x > 0; x -= 8)
		{
			// Value is average of values of source pixel and destination pixel.
			// Hue is hue of source pixel.
			for (int i = 8; i > 0; i -= sizeof(Swar8))
			{
				di -= sizeof(Swar8);
				si -= sizeof(Swar8);

				Swar8 srcPixels = swar8Read(&src_pixels[si]);
				const Swar8 huesMask = SWAR8(0xF0);
				Swar8 hues = srcPixels & huesMask;
				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = srcPixels & valuesMask;
				values += swar8Read(&dst_pixels[di]) & valuesMask;
				values = (values >> 1) & valuesMask;
				swar8Write(&dst_pixels[di], hues | values);
			}
		}
	}

	assert(di == 0);
	assert(si == 0);
}

// Smoothie #6
void showHeadlight(SDL_Surface *dst, SDL_Surface *src, Sint16 playerX, Sint16 playerY)
{
	assert(src->w == vga_width &&
	       src->h >= vga_height &&
	       src->format->BitsPerPixel == 8);
	assert(dst->w == vga_width &&
	       dst->h >= vga_height &&
	       dst->format->BitsPerPixel == 8);

	Uint8 *src_pixels = src->pixels;
	int src_pitch = src->pitch;
	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;

	int lx = (playerX - 17) - (playerY + 12) + 1;  // light starting at X
	int rx = (playerX - 17) + (playerY + 12);      // light until X

	size_t di = 0;
	size_t si = 24;

	for (int y = 184; y > 0; --y)
	{
		int x = 264;

		if (lx <= rx + 1)
		{
			const int x1 = 264 - MAX(0, lx - 5);
			const int x2 = 264 - MAX(0, lx);
			const int x3 = 264 - MIN(rx, 264);
			const int x4 = 264 - MIN(rx + 5, 264);
			assert(x1 >= x2);
			assert(x2 >= 0 && x2 >= x3 - 1);
			assert(x3 >= x4);
			assert(x4 >= 0);

			lx += 1;
			rx -= 1;

			// Dark left side.
			const int x1r = (x1 + sizeof(Swar8) - 1) - (x1 + sizeof(Swar8) - 1) % sizeof(Swar8);  // round up to multiple
			for (; x > x1r; x -= sizeof(Swar8))
			{
				Swar8 srcPixels = swar8Read(&src_pixels[si]);
				const Swar8 huesMask = SWAR8(0xF0);
				Swar8 hues = srcPixels & huesMask;
				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = srcPixels & valuesMask;
				values = (values >> 2) & valuesMask;
				swar8Write(&dst_pixels[di], hues | values);

				di += sizeof(Swar8);
				si += sizeof(Swar8);
			}
			for (; x > x1; --x)
			{
				Uint8 pixel = src_pixels[si];
				dst_pixels[di] = (pixel & 0xF0) | ((pixel & 0x0F) >> 2);

				di += 1;
				si += 1;
			}

			// Gradient left side.
			for (; x > x2; --x)
			{
				int light = 3 * (6 - (x - x2));

				Uint8 pixel = src_pixels[si];
				Uint8 value = ((pixel & 0xF) + light) >> 2;
				dst_pixels[di] = (pixel & 0xF0) | value;

				di += 1;
				si += 1;
			}

			// Light.
			if (x2 > x3)
			{
				int w = x2 - x3;

				memcpy(&dst_pixels[di], &src_pixels[si], w);

				di += w;
				si += w;

				x -= w;
			}

			// Gradient right side.
			for (; x > x4; --x)
			{
				int light = 3 * (x - x4);

				Uint8 pixel = src_pixels[si];
				Uint8 value = ((pixel & 0xF) + light) >> 2;
				dst_pixels[di] = (pixel & 0xF0) | value;

				di += 1;
				si += 1;
			}

			// Dark right side.
			const int x4r = x4 - x4 % sizeof(Swar8);  // round down to multiple
			for (; x > x4r; --x)
			{
				Uint8 pixel = src_pixels[si];
				dst_pixels[di] = (pixel & 0xF0) | ((pixel & 0x0F) >> 2);

				di += 1;
				si += 1;
			}
		}
		for (; x > 0; x -= sizeof(Swar8))
		{
			Swar8 srcPixels = swar8Read(&src_pixels[si]);
			const Swar8 huesMask = SWAR8(0xF0);
			Swar8 hues = srcPixels & huesMask;
			const Swar8 valuesMask = SWAR8(0x0F);
			Swar8 values = srcPixels & valuesMask;
			values = (values >> 2) & valuesMask;
			swar8Write(&dst_pixels[di], hues | values);

			di += sizeof(Swar8);
			si += sizeof(Swar8);
		}

		assert(di % dst_pitch == 264);
		assert(si % src_pitch == 264 + 24);

		di += dst_pitch - 264;
		si += src_pitch - 264;
	}
}

// Smoothie #9
void showFlipped(SDL_Surface *dst, SDL_Surface *src)
{
	assert(src->w == vga_width &&
	       src->h >= vga_height &&
	       src->format->BitsPerPixel == 8);
	assert(dst->w == vga_width &&
	       dst->h >= vga_height &&
	       dst->format->BitsPerPixel == 8);

	Uint8 *src_pixels = src->pixels;
	int src_pitch = src->pitch;
	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;

	size_t di = dst_pitch * 184;
	size_t si = 24;

	for (int y = 184; y > 0; --y)
	{
		di -= dst_pitch;

		memcpy(&dst_pixels[di], &src_pixels[si], 264);

		si += src_pitch;
	}
}

// Not smoothie #6 xor #9
void showNormal(SDL_Surface *dst, SDL_Surface *src)
{
	assert(src->w == vga_width &&
	       src->h >= vga_height &&
	       src->format->BitsPerPixel == 8);
	assert(dst->w == vga_width &&
	       dst->h >= vga_height &&
	       dst->format->BitsPerPixel == 8);

	Uint8 *src_pixels = src->pixels;
	int src_pitch = src->pitch;
	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;

	size_t di = 0;
	size_t si = 24;

	for (int y = 184; y > 0; --y)
	{
		memcpy(&dst_pixels[di], &src_pixels[si], 264);

		di += dst_pitch;
		si += src_pitch;
	}
}

/* Background Starfield */
typedef struct
{
	Uint8 color;
	JE_word position; // relies on overflow wrap-around
	int speed;
} StarfieldStar;

#define MAX_STARS 100
#define STARFIELD_HUE 0x90
static StarfieldStar starfield_stars[MAX_STARS];
int starfield_speed;

void initialize_starfield(void)
{
	for (int i = MAX_STARS-1; i >= 0; --i)
	{
		starfield_stars[i].position = mt_rand() % 320 + mt_rand() % 200 * VGAScreen->pitch;
		starfield_stars[i].speed = mt_rand() % 3 + 2;
		starfield_stars[i].color = mt_rand() % 16 + STARFIELD_HUE;
	}
}

void update_and_draw_starfield(SDL_Surface* surface, int move_speed)
{
	Uint8* p = (Uint8*)surface->pixels;

	for (int i = MAX_STARS-1; i >= 0; --i)
	{
		StarfieldStar* star = &starfield_stars[i];

		star->position += (star->speed + move_speed) * surface->pitch;

		if (star->position < 177 * surface->pitch)
		{
			if (p[star->position] == 0)
			{
				p[star->position] = star->color;
			}

			// If star is bright enough, draw surrounding pixels
			if (star->color - 4 >= STARFIELD_HUE)
			{
				if (p[star->position + 1] == 0)
					p[star->position + 1] = star->color - 4;

				if (star->position > 0 && p[star->position - 1] == 0)
					p[star->position - 1] = star->color - 4;

				if (p[star->position + surface->pitch] == 0)
					p[star->position + surface->pitch] = star->color - 4;

				if (star->position >= surface->pitch && p[star->position - surface->pitch] == 0)
					p[star->position - surface->pitch] = star->color - 4;
			}
		}
	}
}
