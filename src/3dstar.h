#ifndef _3DSTAR_H
#define _3DSTAR_H

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

#endif /*_3DSTAR_H */
