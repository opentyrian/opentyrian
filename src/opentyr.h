#ifndef OPENTYRIAN_H
#define OPENTYRIAN_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h> /* For the ROUND() macro */

#if __STDC_VERSION__ >= 199901L
#define INLINE inline
#elif defined __GNUC__
#define INLINE __inline__
#else
#define INLINE
#endif

#define ROUND(x) (floor((x)+0.5))

/* Gets number of elements in an array.
 * !!! USE WITH ARRAYS ONLY !!! */
#define COUNTOF(x) (sizeof(x) / sizeof *(x))

#define TRUE 1
#define FALSE 0

typedef signed long    JE_longint;
typedef signed short   JE_integer;
typedef signed char    JE_shortint;
typedef unsigned short JE_word;
typedef unsigned char  JE_byte;
typedef int            JE_boolean;

typedef char          *JE_string;
typedef char           JE_char;

typedef double         JE_real;

#endif /* OPENTYRIAN_H */
