/*
 * Jumpers Editor++: A cross-platform and extendable editor and player for the Jumper series of games;
 * Copyright (C) 2007  Yuri K. Schlesner
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
#ifndef 3DSTAR_H
#define 3DSTAR_H

#include "opentyr.h"

#define MAXSTARS 1000
#define MAXRAND 3999

struct Star {
    JE_integer spx, spy, spz; /* stars x,y & z positions */
    JE_integer lastx, lasty;
};

JE_boolean run;

struct Star star[MAXSTARS+1];

JE_integer tempx, tempy;

JE_char k;
JE_byte scancode;

JE_byte setup;
JE_byte setupcounter;

JE_real nsp; /* new sprite pointer */
JE_real nspvarinc;
JE_real nspvarvarinc;

JE_word changetime;
JE_boolean dochange;

JE_boolean grayb;

JE_integer x;

JE_word speed;
JE_shortint speedchange;

JE_byte pcolor;

/******************************************/

JE_boolean keypressed(JE_char kp);
void initgraph( void );
void inittext( void );
void wait_vbi( void );
void makegray( void );
void init( void );
void resetvalues( void );
void changesetup( JE_byte setuptype );

void main( void ); /* <- ??? */
    void newstar( void );


/* TODO... */

#endif /* 3DSTAR_H */
