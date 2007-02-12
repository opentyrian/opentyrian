#ifndef _OPENTYRIAN_H
#define _OPENTYRIAN_H

#include <stdlib.h>
#include <stdio.h>

#include <SDL/SDL.h>

typedef signed long JE_longint;
typedef signed short JE_integer;
typedef signed char JE_shortint;
typedef unsigned short JE_word;
typedef unsigned char JE_byte;
typedef int JE_boolean;

typedef char *JE_string;
typedef char JE_char;

typedef double JE_real; /* TODO: check if type is right */

SDL_Surface *vgascreenseg;

#endif /* _OPENTYRIAN_H */
