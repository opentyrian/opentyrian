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
#ifndef NORTSONG_H
#define NORTSONG_H

#include "opentyr.h"

#include "musmast.h"
#include "sndmast.h"

#include "SDL.h"

extern JE_word frameCountMax;

extern Sint16 *soundSamples[SOUND_COUNT];
extern size_t soundSampleCount[SOUND_COUNT];

extern JE_word tyrMusicVolume, fxVolume;
extern const JE_word fxPlayVol;
extern JE_word tempVolume;

void setFrameSpeed(Uint16 speed);
void setFrameCount(JE_word frameCount);
void setFrameCount2(JE_word frameCount2);
Uint32 getFrameCountTicks(void);
Uint32 getFrameCount2Ticks(void);
void delayUntilElapsed(void);

void JE_changeVolume(JE_word *music, int music_delta, JE_word *sample, int sample_delta);

void loadSndFile(bool xmas);
void JE_playSampleNum(JE_byte samplenum);

#endif /* NORTSONG_H */
