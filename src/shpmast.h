#ifndef _SHPMAST_H
#define _SHPMAST_H

#include "opentyr.h"

#define SHPnum 12

const JE_string shpfile[SHPnum];

typedef JE_longint JE_shppostype[SHPnum + 1]; /* [1..SHPnum + 1] */

JE_shppostype shppos;

#endif /* _NEWSHAPE_H */
