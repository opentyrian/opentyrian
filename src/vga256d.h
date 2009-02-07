/* 
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
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
#include "palette.h"

typedef JE_word JE_shape16B[1]; /* [0.. 0] */
typedef JE_shape16B *JE_shape16;
/*typedef JE_byte JE_shapetypeone[168];*/ /* [0..168-1] originally: JE_word JE_shapetypeone[84]; [1..84] */

extern JE_boolean mouseInstalled;
extern JE_char k;
extern JE_word speed; /* JE: holds timer speed for 70Hz */
extern JE_byte scancode;
extern JE_byte outcol;

extern palette_t vga_palette;

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

#endif /* VGA256D_H */

// kate: tab-width 4; vim: set noet:
