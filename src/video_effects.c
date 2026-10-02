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
#include "video_effects.h"

#include "logging.h"
#include "opentyr.h"
#include "palette.h"
#include "video.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// The effects are drawn by the renderer as overlays on top of the scaled game screen, so they
// work with every scaler and scaling mode.  The CPU work is done at (or below) the game's native
// 320x200 resolution, and nothing at all is done for effects that are off.
//
//   phosphor   Adds the part of the previous frames' afterglow that is brighter than the current
//              frame, so the result is max(current, decayed previous).  The afterglow decays with
//              time rather than per frame, and the last frame is redrawn while it fades out.
//   scanlines  Modulates the output rows by a brightness profile that is darkest between lines.
//   pixel grid Does the same for the output columns.  Both keep their line spacing close to
//              LINE_MASK_PITCH output pixels by drawing more lines per game pixel as the output
//              gets bigger, so they look like a CRT's lines rather than stripes when full screen.
//   bloom      Adds a blurred, half-resolution copy of the bright parts of the frame.

const char *const effectLevelNames[EffectLevel_MAX] = {
	"Off",
	"Low",
	"Medium",
	"High",
};

EffectLevel scanlinesLevel = EFFECT_OFF;
EffectLevel pixelGridLevel = EFFECT_OFF;
EffectLevel bloomLevel = EFFECT_OFF;
EffectLevel phosphorLevel = EFFECT_OFF;

// Darkness between lines, out of 256.
static const unsigned int scanlinesStrengths[EffectLevel_MAX] = { 0, 64, 112, 160 };
static const unsigned int pixelGridStrengths[EffectLevel_MAX] = { 0, 48, 88, 128 };
// Brightness of the glow, out of 255.
static const Uint8 bloomIntensities[EffectLevel_MAX] = { 0, 96, 160, 224 };
// Brightness that the afterglow retains per PHOSPHOR_DECAY_PERIOD, out of 256.
static const unsigned int phosphorRetentions[EffectLevel_MAX] = { 0, 64, 112, 160 };

enum
{
	GLOW_WIDTH = vga_width / 2,
	GLOW_HEIGHT = vga_height / 2,
	GLOW_THRESHOLD = 192,  // brightest channel value above which pixels start to glow
	GLOW_BLUR_RADIUS = 3,
	GLOW_BLUR_PASSES = 2,
	PHOSPHOR_DECAY_PERIOD = 28,  // ms; about one frame of gameplay
	PHOSPHOR_REFRESH_PERIOD = 33,  // ms; redraw interval while the afterglow fades on a still screen
	LINE_MASK_PITCH_X2 = 5,  // desired output pixels between scanlines or grid lines, times two
};

typedef enum
{
	LINE_MASK_ROWS,
	LINE_MASK_COLUMNS,
} LineMaskOrientation;

/** A one-pixel-thick texture that darkens the gaps between lines when stretched over the output. */
typedef struct
{
	SDL_Texture *texture;
	int size;
	EffectLevel level;
} LineMask;

static SDL_Renderer *renderer = NULL;

static SDL_Texture *phosphorTexture = NULL;
static SDL_Texture *bloomTexture = NULL;

static LineMask scanlinesMask = { NULL };
static LineMask pixelGridMask = { NULL };

// The last source frame, which is reused when redrawing.
static Uint8 srcPixels[vga_height][vga_width];
static Uint8 srcPaletteRgb[256][3];

// The frame as seen on screen (including afterglow), in RGB.
static Uint8 frameRgb[vga_height][vga_width][3];
static Uint32 frameTicks = 0;
static bool afterglowVisible = false;

static Uint8 glowRgb[GLOW_HEIGHT][GLOW_WIDTH][3];
static Uint8 glowRgbTemp[GLOW_HEIGHT][GLOW_WIDTH][3];

bool setEffectLevelByName(EffectLevel *const level, const char *const name)
{
	for (int i = 0; i < EffectLevel_MAX; ++i)
	{
		if (strcmp(name, effectLevelNames[i]) == 0)
		{
			*level = i;
			return true;
		}
	}
	return false;
}

void initVideoEffects(SDL_Renderer *const newRenderer)
{
	renderer = newRenderer;
}

static void destroyTexture(SDL_Texture **const texture)
{
	if (*texture != NULL)
	{
		SDL_DestroyTexture(*texture);
		*texture = NULL;
	}
}

void deinitVideoEffects(void)
{
	destroyTexture(&phosphorTexture);
	destroyTexture(&bloomTexture);
	destroyTexture(&scanlinesMask.texture);
	destroyTexture(&pixelGridMask.texture);

	renderer = NULL;
}

static SDL_Texture *createTexture(int access, int w, int h, SDL_BlendMode blendMode)
{
	SDL_Texture *const texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, access, w, h);
	if (texture == NULL)
	{
		logError("Failed to create effect texture (%dx%d): %s", w, h, SDL_GetError());
		return NULL;
	}

	SDL_SetTextureBlendMode(texture, blendMode);
	return texture;
}

static inline Uint32 packRgb(Uint8 r, Uint8 g, Uint8 b)
{
	return (Uint32)r << 16 | (Uint32)g << 8 | b;
}

static void storeSrcFrame(SDL_Surface *const srcSurface)
{
	for (int y = 0; y < vga_height; ++y)
		memcpy(srcPixels[y], (Uint8 *)srcSurface->pixels + y * srcSurface->pitch, vga_width);

	for (int i = 0; i < 256; ++i)
		SDL_GetRGB(rgb_palette[i], main_window_tex_format, &srcPaletteRgb[i][0], &srcPaletteRgb[i][1], &srcPaletteRgb[i][2]);
}

/** Converts the source frame to RGB, blending in the afterglow and outputting the part of it that is brighter than the source frame. */
static void updateFrameRgb(Uint32 *const phosphorPixels, const int phosphorPitch)
{
	const Uint32 ticks = SDL_GetTicks();
	const Uint32 elapsed = ticks - frameTicks;
	frameTicks = ticks;

	unsigned int retention = 0;
	if (phosphorLevel != EFFECT_OFF)
		retention = (unsigned int)(256 * pow(phosphorRetentions[phosphorLevel] / 256.0, (double)elapsed / PHOSPHOR_DECAY_PERIOD));

	unsigned int afterglowBits = 0;

	for (int y = 0; y < vga_height; ++y)
	{
		Uint32 *const dst = phosphorPixels != NULL ? (Uint32 *)((Uint8 *)phosphorPixels + y * phosphorPitch) : NULL;

		for (int x = 0; x < vga_width; ++x)
		{
			const Uint8 *const color = srcPaletteRgb[srcPixels[y][x]];
			Uint8 *const frame = frameRgb[y][x];
			Uint8 afterglow[3] = { 0, 0, 0 };

			for (int c = 0; c < 3; ++c)
			{
				const Uint8 decayed = frame[c] * retention >> 8;
				frame[c] = MAX(decayed, color[c]);
				afterglow[c] = frame[c] - color[c];
			}

			afterglowBits |= afterglow[0] | afterglow[1] | afterglow[2];

			if (dst != NULL)
				dst[x] = packRgb(afterglow[0], afterglow[1], afterglow[2]);
		}
	}

	afterglowVisible = afterglowBits != 0;
}

/** Blurs each line of RGB samples with a box filter, clamping at the edges. */
static void boxBlur(const Uint8 *const src, Uint8 *const dst, const int length, const int step, const int lineCount, const int lineStep)
{
	assert(length > 2 * GLOW_BLUR_RADIUS + 1);

	const int radius = GLOW_BLUR_RADIUS;
	const int last = length - 1;
	const Uint32 reciprocal = ((1 << 16) + radius) / (2 * radius + 1);  // division by the window size, in 16.16 fixed point

	for (int line = 0; line < lineCount; ++line)
	{
		for (int c = 0; c < 3; ++c)
		{
			const Uint8 *const s = src + line * lineStep + c;
			Uint8 *const d = dst + line * lineStep + c;

			Uint32 sum = (radius + 1) * s[0];
			for (int i = 1; i <= radius; ++i)
				sum += s[i * step];

			int i = 0;
			for (; i < radius; ++i)  // leading edge
			{
				d[i * step] = (sum * reciprocal + 0x8000) >> 16;
				sum += s[(i + radius + 1) * step] - s[0];
			}
			for (; i < last - radius; ++i)  // interior
			{
				d[i * step] = (sum * reciprocal + 0x8000) >> 16;
				sum += s[(i + radius + 1) * step] - s[(i - radius) * step];
			}
			for (; i < length; ++i)  // trailing edge
			{
				d[i * step] = (sum * reciprocal + 0x8000) >> 16;
				sum += s[last * step] - s[(i - radius) * step];
			}
		}
	}
}

static void updateGlow(void)
{
	// Downsample the bright parts of the frame, keeping their hue.
	for (int y = 0; y < GLOW_HEIGHT; ++y)
	{
		for (int x = 0; x < GLOW_WIDTH; ++x)
		{
			const Uint8 *const p0 = frameRgb[2 * y][2 * x], *const p1 = frameRgb[2 * y][2 * x + 1];
			const Uint8 *const p2 = frameRgb[2 * y + 1][2 * x], *const p3 = frameRgb[2 * y + 1][2 * x + 1];

			Uint8 *const glow = glowRgb[y][x];

			int brightest = 0;
			for (int c = 0; c < 3; ++c)
			{
				glow[c] = (p0[c] + p1[c] + p2[c] + p3[c] + 2) / 4;
				brightest = MAX(brightest, glow[c]);
			}

			// Fade the glow in smoothly above the threshold.
			const int excess = MAX(brightest - GLOW_THRESHOLD, 0);
			const int weight = (excess * excess << 8) / ((255 - GLOW_THRESHOLD) * (255 - GLOW_THRESHOLD));

			for (int c = 0; c < 3; ++c)
				glow[c] = glow[c] * weight >> 8;
		}
	}

	for (int pass = 0; pass < GLOW_BLUR_PASSES; ++pass)
	{
		boxBlur(glowRgb[0][0], glowRgbTemp[0][0], GLOW_WIDTH, 3, GLOW_HEIGHT, 3 * GLOW_WIDTH);
		boxBlur(glowRgbTemp[0][0], glowRgb[0][0], GLOW_HEIGHT, 3 * GLOW_WIDTH, GLOW_WIDTH, 3);
	}

	void *pixels;
	int pitch;
	if (SDL_LockTexture(bloomTexture, NULL, &pixels, &pitch) != 0)
		return;

	for (int y = 0; y < GLOW_HEIGHT; ++y)
	{
		Uint32 *const dst = (Uint32 *)((Uint8 *)pixels + y * pitch);

		for (int x = 0; x < GLOW_WIDTH; ++x)
			dst[x] = packRgb(glowRgb[y][x][0], glowRgb[y][x][1], glowRgb[y][x][2]);
	}

	SDL_UnlockTexture(bloomTexture);
}

/**
 * Returns whether the line mask can be drawn over `size` output pixels that show `gameSize` game
 * pixels, (re)building its texture if needed.
 */
static bool updateLineMask(LineMask *const mask, const LineMaskOrientation orientation, const EffectLevel level, const unsigned int strength, const int size, const int gameSize)
{
	// Lines need at least two output pixels each to be visible.
	if (level == EFFECT_OFF || size < 2 * gameSize)
		return false;

	if (mask->texture != NULL && mask->size == size && mask->level == level)
		return true;

	destroyTexture(&mask->texture);

	Uint32 *const pixels = malloc(sizeof(*pixels) * size);
	if (pixels == NULL)
		return false;

	const bool rows = orientation == LINE_MASK_ROWS;
	mask->texture = createTexture(SDL_TEXTUREACCESS_STATIC, rows ? 1 : size, rows ? size : 1, SDL_BLENDMODE_MOD);
	if (mask->texture == NULL)
	{
		free(pixels);
		return false;
	}

	// Draw a whole number of lines per game pixel so that the lines stay aligned with the game
	// pixels, choosing the number that gets closest to the desired line spacing.
	const int linesPerGamePixel = MAX(1, (2 * size + LINE_MASK_PITCH_X2 * gameSize / 2) / (LINE_MASK_PITCH_X2 * gameSize));
	const int lineCount = linesPerGamePixel * gameSize;

	// Each line is bright for the first half of its period and dark for the second half.  Output
	// pixels are shaded by how much of them is covered by dark halves, which keeps the lines even
	// when the line spacing is not a whole number of pixels.  Positions are measured in units of
	// 1/(2 * size) of a line so that a whole period is 2 * size units and half of one is size units.
	const Sint64 period = 2 * (Sint64)size;
	const Sint64 pixelWidth = 2 * (Sint64)lineCount;

	Sint64 darkBefore = 0;  // dark units from the start of the mask up to the current pixel
	for (int i = 0; i < size; ++i)
	{
		const Sint64 end = (i + 1) * pixelWidth;
		const Sint64 darkBeforeEnd = end / period * size + MAX(end % period - size, 0);

		const unsigned int darkness = (unsigned int)(strength * (darkBeforeEnd - darkBefore) / pixelWidth);
		darkBefore = darkBeforeEnd;

		const Uint8 brightness = 255 - MIN(darkness, 255);
		pixels[i] = packRgb(brightness, brightness, brightness);
	}

	SDL_UpdateTexture(mask->texture, NULL, pixels, rows ? sizeof(*pixels) : sizeof(*pixels) * size);
	free(pixels);

	mask->size = size;
	mask->level = level;
	return true;
}

bool videoEffectsNeedRedraw(void)
{
	return phosphorLevel != EFFECT_OFF && afterglowVisible &&
	       SDL_GetTicks() - frameTicks >= PHOSPHOR_REFRESH_PERIOD;
}

void renderVideoEffects(SDL_Surface *const srcSurface, const SDL_Rect *const dstRect)
{
	assert(renderer != NULL);
	assert(srcSurface == NULL || (srcSurface->w == vga_width && srcSurface->h == vga_height));

	// Textures are created on first use.  If that fails, the effect is turned off.
	if (phosphorLevel != EFFECT_OFF && phosphorTexture == NULL)
	{
		phosphorTexture = createTexture(SDL_TEXTUREACCESS_STREAMING, vga_width, vga_height, SDL_BLENDMODE_ADD);
		if (phosphorTexture == NULL)
			phosphorLevel = EFFECT_OFF;
	}

	if (bloomLevel != EFFECT_OFF && bloomTexture == NULL)
	{
		bloomTexture = createTexture(SDL_TEXTUREACCESS_STREAMING, GLOW_WIDTH, GLOW_HEIGHT, SDL_BLENDMODE_ADD);
		if (bloomTexture == NULL)
			bloomLevel = EFFECT_OFF;
#if SDL_VERSION_ATLEAST(2, 0, 12)
		else
			SDL_SetTextureScaleMode(bloomTexture, SDL_ScaleModeLinear);
#endif
	}

	const bool phosphor = phosphorLevel != EFFECT_OFF;
	const bool bloom = bloomLevel != EFFECT_OFF;

	if ((phosphor || bloom) && srcSurface != NULL)
		storeSrcFrame(srcSurface);

	if (phosphor)
	{
		void *pixels;
		int pitch;
		if (SDL_LockTexture(phosphorTexture, NULL, &pixels, &pitch) == 0)
		{
			updateFrameRgb(pixels, pitch);
			SDL_UnlockTexture(phosphorTexture);

			SDL_RenderCopy(renderer, phosphorTexture, NULL, dstRect);
		}
	}
	else if (bloom)
	{
		updateFrameRgb(NULL, 0);
	}

	if (updateLineMask(&scanlinesMask, LINE_MASK_ROWS, scanlinesLevel, scanlinesStrengths[scanlinesLevel], dstRect->h, vga_height))
		SDL_RenderCopy(renderer, scanlinesMask.texture, NULL, dstRect);

	if (updateLineMask(&pixelGridMask, LINE_MASK_COLUMNS, pixelGridLevel, pixelGridStrengths[pixelGridLevel], dstRect->w, vga_width))
		SDL_RenderCopy(renderer, pixelGridMask.texture, NULL, dstRect);

	if (bloom)
	{
		updateGlow();

		const Uint8 intensity = bloomIntensities[bloomLevel];
		SDL_SetTextureColorMod(bloomTexture, intensity, intensity, intensity);
		SDL_RenderCopy(renderer, bloomTexture, NULL, dstRect);
	}
}
