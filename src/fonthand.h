#ifndef _FONTHAND_H
#define _FONTHAND_H

#include "opentyr.h"
#include "newshape.h"

const JE_byte fontmap[136]; 	/* [33..168] */

#define _Partshade  0
#define _Fullshade  1
#define _Darken     2
#define _Trick      3
#define _Noshade    255

JE_integer DefaultBrightness;
JE_byte TextGlowFont,
        TextGlowBrightness;

/*Warning stuff*/
JE_boolean levelwarningdisplay;
JE_byte levelwarninglines;
char levelwarningtext[10][61];	/* [1..10] of string [60] */
JE_boolean warningred;

JE_byte warningsounddelay;
JE_word armorshipdelay;
JE_byte warningcol;
JE_shortint warningcolchange;

void JE_Dstring(JE_word x, JE_word y, JE_string s, JE_byte font);
void JE_NewDrawCShapeBright(JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_shortint brightness);
void JE_NewDrawCShapeShadow(JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y);
void JE_NewDrawCShapeDarken(JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y);
void JE_NewDrawCShapeDarkenNum(JE_byte table, JE_byte shape, JE_word x, JE_word y);
void JE_NewDrawCShapeTrick(JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y);
void JE_NewDrawCShapeTrickNum(JE_byte table, JE_byte shape, JE_word x, JE_word y);
void JE_NewDrawCShapeModify(JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_byte brightness);
void JE_NewDrawCShapeModifyNum(JE_byte table, JE_byte shape, JE_word x, JE_word y, JE_byte filter, JE_byte brightness);
void JE_NewDrawCShapeAdjust(JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_byte brightness);
void JE_NewDrawCShapeAdjustNum(JE_byte table, JE_byte shape, JE_word x, JE_word y, JE_byte filter, JE_byte brightness);
void JE_NewDrawCShapeBrightAndDarken(JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_byte brightness);
/*void JE_NewDrawCShapeZoom(JE_byte table, JE_byte shape, JE_word x, JE_word y, JE_real scale);*/
JE_word JE_FontCenter(JE_string s, JE_byte font);
JE_word JE_TextWidth(JE_string s, JE_byte font);
void JE_TextShade(JE_word x, JE_word y, JE_string s, JE_byte colorbank, JE_shortint brightness, JE_byte shadetype);
void JE_Outtext(JE_word x, JE_word y, JE_string s, JE_byte colorbank, JE_shortint brightness);
void JE_OuttextModify(JE_word x, JE_word y, JE_string s, JE_byte filter, JE_byte brightness, JE_byte font);
void JE_OuttextShade(JE_word x, JE_word y, JE_string s, JE_byte font);
void JE_OuttextAdjust(JE_word x, JE_word y, JE_string s, JE_byte filter, JE_shortint brightness, JE_byte font, JE_boolean shadow);
void JE_OuttextAndDarken(JE_word x, JE_word y, JE_string s, JE_byte colorbank, JE_byte brightness, JE_byte font);
JE_char JE_Bright(JE_boolean makebright);

void JE_UpdateWarning();
void JE_OuttextGlow(JE_word x, JE_word y, JE_string s);

#endif /* _FONTHAND_H */
