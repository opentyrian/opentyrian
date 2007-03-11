#ifndef VGA256D_H
#define VGA256D_H

#include "opentyr.h"

#include "SDL.h"

typedef JE_word JE_shape16B[1]; /* [0.. 0] */
typedef JE_shape16B *JE_shape16;
typedef JE_byte JE_shapetypeone[168]; /* [0..168-1] */  /* originally: JE_word JE_shapetypeone[84]; [1..84] */
typedef JE_byte JE_screentype[65535]; /* [0..65534] */
typedef JE_screentype *JE_screenptr;

#define CrtAddress 0x3D4
#define StatusReg 0x3DA

JE_word TestAX, TestBX, TestCX, TestDX;
JE_boolean Mouse_Installed;
/* screentype *VGAScreen, *VGAScreen2; */
/* JE_THandle VGAScreenHandler, VGAScreen2Handler; */
JE_char k;
JE_word ABSLoc;
/* JE_word VGAScreenSeg, VGAScreen2Seg; */

SDL_Surface *VGAScreenSeg;

/* JE: From Nortsong */
JE_word speed; /* JE: holds timer speed for 70Hz */

JE_byte scancode;
JE_byte outcol;
JE_byte Screen_Attribute;

void JE_initvga256( void );
void JE_InitVGA256X( void ); /* dead code */
void JE_closevga256( void );
void JE_clr256( void );
void JE_ShowVGA( void );
void JE_ShowVGARetrace( void );
void JE_GetVGA( void );
void JE_OnScreen( void ); /* TODO */
void JE_OffScreen( void ); /* TODO */
void JE_disable_refresh( void ); /* dead code */
void JE_enable_refresh( void ); /* dead code */
void JE_WaitRetrace( void );
void JE_WaitPartialRetrace( void );
void JE_WaitNotRetrace( void );

INLINE void JE_pix( JE_word x, JE_word y, JE_byte c );
INLINE void JE_pix2( JE_word x, JE_word y, JE_byte c );
INLINE void JE_pixcool( JE_word x, JE_word y, JE_byte c );
void JE_pix3( JE_word x, JE_word y, JE_byte c );
INLINE void JE_pixabs( JE_word x, JE_byte c );
INLINE void JE_getpix( JE_word x, JE_word y, JE_byte *c );
INLINE JE_byte JE_getpixel( JE_word x, JE_word y );
void JE_rectangle( JE_word a, JE_word b, JE_word c, JE_word d, JE_word e );
void JE_bar( JE_word a, JE_word b, JE_word c, JE_word d, JE_byte e );
void JE_barshade( JE_word a, JE_word b, JE_word c, JE_word d );
void JE_barshade2( JE_word a, JE_word b, JE_word c, JE_word d );
void JE_barbright( JE_word a, JE_word b, JE_word c, JE_word d );
void JE_circle( JE_word x, JE_byte y, JE_word z, JE_byte c );
void JE_line( JE_word a, JE_byte b, JE_longint c, JE_byte d, JE_byte e );
void JE_getimage16( JE_word a, JE_byte b, JE_shape16B *p );
void JE_putimage16( JE_word a, JE_byte b, JE_shape16B *p );
void JE_drawgraphic( JE_word x, JE_word y, JE_shapetypeone s ); /* TODO */
void JE_ABSdrawgraphic( JE_shapetypeone s );
void JE_drawgraphicover( JE_word x, JE_word y, JE_shapetypeone s );
void JE_ABSdrawgraphicover( JE_shapetypeone s );
void JE_readgraphic( JE_integer x, JE_integer y, JE_shapetypeone s );
void JE_getk( JE_char *y ); /* TODO */
void JE_getupk( JE_char *k ); /* TODO */
JE_boolean JE_keypressed( JE_char *kp ); /* TODO */
JE_boolean JE_kp( void ); /* TODO */
void JE_wait( JE_byte min, JE_byte sec, JE_byte hun );
void JE_GetPalette( JE_byte col, JE_byte *red, JE_byte *green, JE_byte *blue ); /* TODO */
void JE_SetPalette( JE_byte col, JE_byte red, JE_byte green, JE_byte blue );
void JE_darkenscreen( void );

#endif /* VGA256D_H */
