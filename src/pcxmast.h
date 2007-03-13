#ifndef PCXMAST_H
#define PCXMAST_H

#include "opentyr.h"

#define PCXnum 13

typedef JE_longint JE_pcxpostype[PCXnum + 1];	/* [1..PCXnum + 1] */

#define PCXMAST_EXTERNS \
extern const JE_string pcxfile[PCXnum];	/* [1..PCXnum] */ \
extern const JE_byte pcxpal[PCXnum];	/* [1..PCXnum] */ \
extern const JE_byte facepal[12];	/* [1..12] */ \
extern JE_pcxpostype pcxpos;

#endif /* PCXMAST_H */
