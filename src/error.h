#ifndef ERROR_H
#define ERROR_H

#include "opentyr.h"

#include <stdio.h>

#define ERROR_EXTERNS \
extern JE_boolean ErrorActive; \
extern JE_boolean ErrorOccurred;

JE_boolean JE_find( const JE_string s );
void JE_resetfile( FILE **f, const JE_string filename );
void JE_resettext( FILE **f, const JE_string filename );
JE_string JE_locatefile( const JE_string filename );
void JE_findtyrian( const JE_string filename );
JE_boolean JE_IsCFGThere( void );
/* void JE_OutputString( JE_string s ); Obscure DOS trick (?) */
void JE_DetectCFG( void );
JE_longint JE_getfilesize( const JE_string filename );

#endif /* ERROR_H */
