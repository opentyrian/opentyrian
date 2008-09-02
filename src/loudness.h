/* 
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Development Team
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
#ifndef LOUDNESS_H
#define LOUDNESS_H

#include "opentyr.h"

#include "fmopl.h"

#include "SDL.h"


#define SFX_CHANNELS 8

#ifndef TARGET_GP2X
#define OUTPUT_QUALITY 4
#else  /* TARGET_GP2X */
#define OUTPUT_QUALITY 2
#endif /* TARGET_GP2X */

#define SAMPLE_SCALING OUTPUT_QUALITY
#define SAMPLE_TYPE OPLSAMPLE
#define BYTES_PER_SAMPLE (OPL_SAMPLE_BITS / 8)

typedef JE_byte JE_MusicType[20000];

extern JE_MusicType musicData;
extern JE_boolean repeated;
extern JE_boolean playing;

extern float sample_volume;
extern float music_volume;

void JE_initialize( void );
void JE_deinitialize( void );

void JE_play( void );

/* SYN: selectSong is called with 0 to disable the current song. Calling it with 1 will start the current song if not playing,
   or restart it if it is. */
void JE_selectSong( JE_word value );

/* TODO: Some of these procs take segment and offset or other weird arguments, the signature of many of these may change
   as I make stuff more "sensible" */

void JE_samplePlay(JE_word addlo, JE_word addhi, JE_word size, JE_word freq);
void JE_bigSamplePlay(JE_word addlo, JE_word addhi, JE_word size, JE_word freq);
JE_word JE_sampleStatus(JE_byte chan);
	
void JE_multiSampleInit(JE_word addlo, JE_word addhi, JE_word dmalo, JE_word dmahi);
void JE_multiSampleMix( void );
/* void JE_multiSamplePlay(JE_word addlo, JE_word addhi, JE_word size, JE_byte chan, JE_byte vol); */
void JE_multiSamplePlay(JE_byte *buffer, JE_word size, JE_byte chan, JE_byte vol);

void JE_setVol(JE_word volume, JE_word sample);

JE_word JE_getVol( void );
JE_word JE_getSampleVol( void );

#endif /* LOUDNESS_H */

// kate: tab-width 4; vim: set noet:
