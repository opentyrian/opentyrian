/* vim: set noet:
 *
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Development Team
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
#ifndef VGA256D_H
#define VGA256D_H

#include "opentyr.h"

#include "nortvars.h"

#include "SDL.h"


#define surface_width 320
#ifdef TARGET_GP2X
#	define surface_height 240
#else
#	define surface_height 200
#endif // TARGET_GP2X

typedef JE_word JE_shape16B[1]; /* [0.. 0] */
typedef JE_shape16B *JE_shape16;
/*typedef JE_byte JE_shapetypeone[168];*/ /* [0..168-1] originally: JE_word JE_shapetypeone[84]; [1..84] */
typedef JE_byte JE_screentype[65535]; /* [0..65534] */
typedef JE_screentype *JE_screenptr;

extern JE_boolean mouseInstalled;
extern JE_char k;
extern SDL_Surface *display_surface;
extern SDL_Surface *VGAScreen, *VGAScreenSeg;
extern SDL_Surface *game_screen;
extern SDL_Surface *VGAScreen2;
extern JE_word speed; /* JE: holds timer speed for 70Hz */
extern JE_byte scancode;
extern JE_byte outcol;

extern bool fullscreen_enabled;

extern SDL_Color vga_palette[];

void JE_initVGA256( void );
void set_fullscreen( bool full );
void JE_closeVGA256( void );
void JE_clr256( void );
void JE_showVGA( void );
void JE_showVGARetrace( void );
void JE_getVGA( void );
void JE_onScreen( void );
void JE_offScreen( void );
void JE_disableRefresh( void );
void JE_enableRefresh( void );
void JE_waitRetrace( void );
void JE_waitPartialRetrace( void );
void JE_waitNotRetrace( void );
void JE_pix( JE_word x, JE_word y, JE_byte c );
void JE_pix2( JE_word x, JE_word y, JE_byte c );
void JE_pixCool( JE_word x, JE_word y, JE_byte c );
void JE_pix3( JE_word x, JE_word y, JE_byte c );
void JE_pixAbs( JE_word x, JE_byte c );
void JE_getPix( JE_word x, JE_word y, JE_byte *c );
JE_byte JE_getPixel( JE_word x, JE_word y );
void JE_rectangle( JE_word a, JE_word b, JE_word c, JE_word d, JE_word e );
void JE_bar( JE_word a, JE_word b, JE_word c, JE_word d, JE_byte e );
void JE_c_bar( JE_word a, JE_word b, JE_word c, JE_word d, JE_byte e );
void JE_barShade( JE_word a, JE_word b, JE_word c, JE_word d );
void JE_barShade2( JE_word a, JE_word b, JE_word c, JE_word d );
void JE_barBright( JE_word a, JE_word b, JE_word c, JE_word d );
void JE_circle( JE_word x, JE_byte y, JE_word z, JE_byte c );
void JE_line( JE_word a, JE_byte b, JE_longint c, JE_byte d, JE_byte e );
void JE_getImage16( JE_word a, JE_byte b, JE_shape16B *p );
void JE_putImage16( JE_word a, JE_byte b, JE_shape16B *p );
void JE_drawGraphic( JE_word x, JE_word y, JE_ShapeTypeOne s );
void JE_absDrawGraphic( JE_ShapeTypeOne s );
void JE_drawGraphicOver( JE_word x, JE_word y, JE_ShapeTypeOne s );
void JE_absDrawGraphicOver( JE_ShapeTypeOne s );
void JE_readGraphic( JE_integer x, JE_integer y, JE_ShapeTypeOne s );
void JE_getK( JE_char *y );
void JE_getUpK( JE_char *k );
JE_boolean JE_keyPressed( JE_char *kp );
JE_boolean JE_kp( void );
void JE_wait( JE_byte min, JE_byte sec, JE_byte hun );
void JE_getPalette( JE_byte col, JE_byte *red, JE_byte *green, JE_byte *blue );
void JE_setPalette( JE_byte col, JE_byte red, JE_byte green, JE_byte blue );
void JE_darkenScreen( void );

#endif /* VGA256D_H */
