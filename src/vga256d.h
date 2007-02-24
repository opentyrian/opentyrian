#ifndef _VGA256D_H
#define _VGA256D_H

#include "opentyr.h"

typedef JE_word shape16B[1]; /* [0.. 0] */
typedef shape16B *shape16;
typedef JE_word shapetypeone[84]; /* [1..84] */
typedef JE_byte screentype[65535]; /* [0..65534] */
typedef screentype *screenptr;

void initvga256( void );
void InitVGA256X( void );
void closevga256( void );
void clr256( void );
void ShowVGA( void );
void ShowVGARetrace( void );
void GetVGA( void );
void OnScreen( void );
void OffScreen( void );
void disable_refresh( void );
void enable_refresh( void );
void WaitRetrace( void );
void WaitPartialRetrace( void );
void WaitNotRetrace( void );

void pix( JE_word x, JE_word y, JE_byte c );
void pixcool( JE_word x, JE_word y, JE_byte c );
void pixabs( JE_word x, JE_byte c );
void getpix( JE_word x, JE_word y, JE_byte *c );
JE_byte getpixel( JE_word x, JE_word y );
void rectangle( JE_word a, JE_word b, JE_word c, JE_word d, JE_word e );
void bar( JE_word a, JE_word b, JE_word c, JE_word d, JE_byte e );
void barshade( JE_word a, JE_word b, JE_word c, JE_word d );
void barshade2( JE_word a, JE_word b, JE_word c, JE_word d );
void barbright( JE_word a, JE_word b, JE_word c, JE_word d );
void circle( JE_word x, JE_byte y, JE_word z, JE_byte c );
void line( JE_word a, JE_byte b, JE_longint c, JE_byte d, JE_byte e );
void getimage16( JE_word a, JE_byte b, shape16B *p );
void putimage16( JE_word a, JE_byte b, shape16B *p );
void drawgraphic( JE_word x, JE_word y, shapetypeone s );
void ABSdrawgraphic( shapetypeone s );
void drawgraphicover( JE_word x, JE_word y, shapetypeone s );
void ABSdrawgraphicover( shapetypeone s );
void readgraphic( JE_integer x, JE_integer y, shapetypeone s );
void getk( JE_char *y );
void getupk( JE_char *k );
JE_boolean keypressed( JE_char *kp );
JE_boolean kp( void );
void wait( JE_byte min, JE_byte sec, JE_byte hun );
void GetPalette( JE_byte col, JE_byte *Red, JE_byte *Green, JE_byte *Blue );
void SetPalette( JE_byte Col, JE_byte Red, JE_byte Green, JE_byte Blue );
void pix2( JE_word x, JE_word y, JE_byte c );
void pix3( JE_word x, JE_word y, JE_byte c );
void darkenscreen( void );

#endif /* _VGA256D_H */
