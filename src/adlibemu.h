/*
 * ADLIBEMU.H
 * Copyright (C) 1998-2001 Ken Silverman
 * Ken Silverman's official web site: "http://www.advsys.net/ken"
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

typedef struct
{
    float val, t, tinc, vol, sustain, amp, mfb;
    float a0, a1, a2, a3, decaymul, releasemul;
    short *waveform;
    long wavemask;
    void (*cellfunc)(void *, float);
    unsigned char flags, dum0, dum1, dum2;
} celltype;

void adlibinit(long dasamplerate,long danumspeakers,long dabytespersample);
void adlib0(long i,long v);
/* void adlibgetsample(void *sndptr,long numbytes); */
void adlibgetsample (unsigned char *sndptr, long numbytes);
void adlibsetvolume(int i);
void randoinsts( void );

#ifndef NO_EXTERNS
extern float lvol[9],rvol[9]; /*,lplc[9],rplc[9];*/
extern long lplc[9], rplc[9];
#endif

void docell4 (void *c, float modulator);
void docell3 (void *c, float modulator);
void docell2 (void *c, float modulator);
void docell1 (void *c, float modulator);
void docell0 (void *c, float modulator);
void cellon (long i, long j, celltype *c, unsigned char iscarrier);
void cellfreq (signed long i, signed long j, celltype *c);
