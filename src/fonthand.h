/*
 * OpenTyrian Classic: A moden cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Team
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
#ifndef FONTHAND_H
#define FONTHAND_H

#include "opentyr.h"

#define PartShade  0
#define FullShade  1
#define Darken     2
#define Trick      3
#define NoShade    255

#ifndef NO_EXTERNS
extern JE_integer DefaultBrightness;
extern JE_byte TextGlowFont, TextGlowBrightness;
extern JE_boolean levelwarningdisplay;
extern JE_byte levelwarninglines;
extern JE_char levelwarningtext[10][61]; /* [1..10] of string [60] */
extern JE_boolean warningred;
extern JE_byte warningsounddelay;
extern JE_word armorshipdelay;
extern JE_byte warningcol;
extern JE_shortint warningcolchange;
extern const JE_byte fontmap[136]; /* [33..168] */
#endif

void JE_Dstring( JE_word x, JE_word y, JE_string s, JE_byte font );
void JE_NewDrawCShapeBright( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_shortint brightness );
void JE_NewDrawCShapeShadow( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y );
void JE_NewDrawCShapeDarken( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y );
void JE_NewDrawCShapeDarkenNum( JE_byte table, JE_byte shape, JE_word x, JE_word y );
void JE_NewDrawCShapeTrick( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y );
void JE_NewDrawCShapeTrickNum( JE_byte table, JE_byte shape, JE_word x, JE_word y );
void JE_NewDrawCShapeModify( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_byte brightness );
void JE_NewDrawCShapeModifyNum( JE_byte table, JE_byte shape, JE_word x, JE_word y, JE_byte filter, JE_byte brightness );
void JE_NewDrawCShapeAdjust( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_byte brightness );
void JE_NewDrawCShapeAdjustNum( JE_byte table, JE_byte shape, JE_word x, JE_word y, JE_byte filter, JE_byte brightness );
void JE_NewDrawCShapeBrightAndDarken( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_byte brightness );
/*void JE_NewDrawCShapeZoom( JE_byte table, JE_byte shape, JE_word x, JE_word y, JE_real scale );*/
JE_word JE_FontCenter( JE_string s, JE_byte font );
JE_word JE_TextWidth( JE_string s, JE_byte font );
void JE_TextShade( JE_word x, JE_word y, JE_string s, JE_byte colorbank, JE_shortint brightness, JE_byte shadetype );
void JE_Outtext( JE_word x, JE_word y, JE_string s, JE_byte colorbank, JE_shortint brightness );
void JE_OuttextModify( JE_word x, JE_word y, JE_string s, JE_byte filter, JE_byte brightness, JE_byte font );
void JE_OuttextShade( JE_word x, JE_word y, JE_string s, JE_byte font );
void JE_OuttextAdjust( JE_word x, JE_word y, JE_string s, JE_byte filter, JE_shortint brightness, JE_byte font, JE_boolean shadow );
void JE_OuttextAndDarken( JE_word x, JE_word y, JE_string s, JE_byte colorbank, JE_byte brightness, JE_byte font );
JE_char JE_Bright( JE_boolean makebright );

void JE_UpdateWarning( void );
void JE_OuttextGlow( JE_word x, JE_word y, JE_string s );

#endif /* FONTHAND_H */
