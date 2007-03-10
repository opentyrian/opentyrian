#ifndef PCXMAST_H
#define PCXMAST_H

#include "opentyr.h"

#define PCXnum 13

JE_string pcxfile[PCXnum];	/* [1..PCXnum] */

JE_byte pcxpal[PCXnum];	/* [1..PCXnum] */

/*FACEMAX*/
JE_byte facepal[12];	/* [1..12] */

typedef JE_longint JE_pcxpostype[PCXnum + 1];	/* [1..PCXnum + 1] */

JE_pcxpostype pcxpos;

#endif /* PCXMAST_H */
