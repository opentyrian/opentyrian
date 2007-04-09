/* vim: set noet:
 *
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Team
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

typedef JE_longint JE_SongPosType [MUSIC_NUM + 1]; /* [1..Musicnum + 1] */
typedef JE_byte JE_DigiMixType [0x4ff];
typedef JE_byte JE_AweType [35000];

#ifndef NO_EXTERNS
extern Uint32 target;

extern JE_char hexa[17];
extern JE_boolean mixEnable;

extern JE_boolean notYetLoadedSound, notYetLoadedMusic;
extern JE_SongPosType songPos;
extern JE_byte soundEffects;

extern JE_byte currentSong;
extern JE_byte soundActive, musicActive;

extern JE_byte *digiFx[SOUND_NUM + 9];
extern JE_word fxSize[SOUND_NUM + 9];

extern JE_word fxVolume, fxPlayVol;
#endif

void setdelay( JE_byte delay );
int delaycount( void );
void wait_delay( void );
void wait_delayorinput( JE_boolean keyboard, JE_boolean mouse, JE_boolean joystick );

void JE_resetTimerInt( void );
void JE_playSong ( JE_word songnum );
void JE_loadSong( JE_word songnum );
void JE_endMusic ( JE_byte soundeffects);
void JE_stopSong( void );
void JE_restartSong( void );
void JE_reinit ( JE_boolean redo );
void JE_aweStuff( void );
void JE_setTimerInt( void );
void JE_calcFXVol( void );
void JE_changeVolume( JE_word temp, JE_integer change, JE_word fxvol, JE_integer fxchange );
void JE_frameDelay( JE_byte delay );

void JE_loadSmpFile ( JE_string name, JE_byte samplenum);
void JE_loadSndFile( void );
void JE_playSampleNum ( JE_byte samplenum );

void JE_fxDestruct ( JE_byte samplenum );

void JE_setvol (JE_byte musicvolume, JE_byte sample );

void JE_waitframecount( void );

JE_string JE_hexa2 (JE_byte data );
JE_string JE_hexa4 (JE_word data );


/* SYN: This stuff is probably unneeded, as it deals with sound hardware issues abstracted 
   away by SDL. Pascal code is left here as reference, just in case we want this stuff
   later.

midiportlist : ARRAY [1..10] OF
  WORD =
  ($0000, $0000, $1000, $0666, $0330, $0320, $0332, $0334, $0336, $0300);

fxportlist : ARRAY [1..4] OF
  WORD =
  ($FFFF, $0000, $0666, $1000);

dmalist : ARRAY [1..3] OF
  WORD =
  ($0000, $0003, $0001);

*/

#endif /* NORTSONG_H */
