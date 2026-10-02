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
#ifndef VIDEO_EFFECTS_H
#define VIDEO_EFFECTS_H

#include "SDL.h"

#include <stdbool.h>

typedef enum
{
	EFFECT_OFF,
	EFFECT_LOW,
	EFFECT_MEDIUM,
	EFFECT_HIGH,
	EffectLevel_MAX
} EffectLevel;

extern const char *const effectLevelNames[EffectLevel_MAX];

extern EffectLevel scanlinesLevel;
extern EffectLevel pixelGridLevel;
extern EffectLevel bloomLevel;
extern EffectLevel phosphorLevel;

bool setEffectLevelByName(EffectLevel *level, const char *name);

void initVideoEffects(SDL_Renderer *renderer);
void deinitVideoEffects(void);

/** Returns whether the last frame should be redrawn because an effect is changing over time. */
bool videoEffectsNeedRedraw(void);
/** Draws the effects over the scaled game screen.  Pass NULL to redraw the effects for the last frame. */
void renderVideoEffects(SDL_Surface *srcSurface, const SDL_Rect *dstRect);

#endif /* VIDEO_EFFECTS_H */
