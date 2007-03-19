/*
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
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
#include "opentyr.h"
#include "fonthand.h"

#include "newshape.h"
#include "vga256d.h"

const JE_byte fontmap[136] =    /* [33..168] */
{
    26,33,60,61,62,255,32,64,65,63,84,29,83,28,80,79,70,71,72,73,74,75,76,77,
    78,31,30,255,255,255,27,255,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
    20,21,22,23,24,25,68,82,69,255,255,255,34,35,36,37,38,39,40,41,42,43,44,45,46,
    47,48,49,50,51,52,53,54,55,56,57,58,59,66,81,67,255,255,

    86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,
    107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,
    125,126
};

/* shape constants included in newshape.h */

JE_integer DefaultBrightness = -3;
JE_byte TextGlowBrightness = 6;

JE_boolean levelwarningdisplay;
JE_byte levelwarninglines;
JE_char levelwarningtext[10][61];  /* [1..10] of string [60] */
JE_boolean warningred;

JE_byte warningsounddelay;
JE_word armorshipdelay;
JE_byte warningcol;
JE_shortint warningcolchange;

void JE_Dstring( JE_word x, JE_word y, JE_string s, JE_byte font )
{
    JE_byte a, b;
    JE_boolean bright = FALSE;

    for(a = 0; s[a] != 0; a++) {
        b = s[a];

        if((b > 32) && (b < 126)) {
            if(fontmap[b-33] != 255) {
                JE_NewDrawCShapeDarken((*shapearray)[font][fontmap[b-33]], shapex[font][fontmap[b-33]], shapey[font][fontmap[b-33]], x + 2, y + 2);
                JE_NewDrawCShapeBright((*shapearray)[font][fontmap[b-33]], shapex[font][fontmap[b-33]], shapey[font][fontmap[b-33]], x, y, 15, DefaultBrightness + (bright << 1));

                x += shapex[font][fontmap[b-33]] + 1;
            }
        } else {
            if(b == 32) {
                x += 6;
            } else {
                if(b == 126) {
                    bright = !bright;
                }
            }
        }
    }
}

/*Intended for Font drawing - maximum X size is 255*/
void JE_NewDrawCShapeBright( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_shortint brightness )
{
    JE_word xloop = 0, yloop = 0;
    JE_byte *p; /* shape pointer */
    unsigned char *s;   /* screen pointer, 8-bit specific */

    s = (unsigned char *)tempscreenseg->pixels;
    s += y * tempscreenseg->w + x;

    filter <<= 4;

    for(p = shape; yloop < ysize; p++) {
        switch(*p) {
            case 255:   /* p transparent pixels */
                p++;
                s += *p; xloop += *p;
                break;
            case 254:   /* next y */
                s -= xloop; xloop = 0;
                s += tempscreenseg->w; yloop++;
                break;
            case 253:   /* 1 transparent pixel */
                s++; xloop++;
                break;
            default:    /* set a pixel */
                *s = ((*p & 0x0f) | filter) + brightness;
                s++; xloop++;
                break;
        }
        if(xloop == xsize) {
            s -= xloop; xloop = 0;
            s += tempscreenseg->w; yloop++;
        }
    }
}

/*Intended for Font drawing - maximum X size is 255*/
void JE_NewDrawCShapeShadow( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y )
{
    JE_word xloop = 0, yloop = 0;
    JE_byte *p; /* shape pointer */
    unsigned char *s;   /* screen pointer, 8-bit specific */

    s = (unsigned char *)tempscreenseg->pixels;
    s += y * tempscreenseg->w + x;

    for(p = shape; yloop < ysize; p++) {
        switch(*p) {
            case 255:   /* p transparent pixels */
                p++;
                s += *p; xloop += *p;
                break;
            case 254:   /* next y */
                s -= xloop; xloop = 0;
                s += tempscreenseg->w; yloop++;
                break;
            case 253:   /* 1 transparent pixel */
                s++; xloop++;
                break;
            default:    /* set a pixel */
                *s = 0;
                s++; xloop++;
                break;
        }
        if(xloop == xsize) {
            s -= xloop; xloop = 0;
            s += tempscreenseg->w; yloop++;
        }
    }
}

/*Intended for Font drawing - maximum X size is 255*/
void JE_NewDrawCShapeDarken( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y )
{
    JE_word xloop = 0, yloop = 0;
    JE_byte *p; /* shape pointer */
    unsigned char *s;   /* screen pointer, 8-bit specific */

    s = (unsigned char *)tempscreenseg->pixels;
    s += y * tempscreenseg->w + x;

    for(p = shape; yloop < ysize; p++) {
        /* (unported) compare the screen offset to 65535, if equal do case 253 */
        switch(*p) {
            case 255:   /* p transparent pixels */
                p++;
                s += *p; xloop += *p;
                break;
            case 254:   /* next y */
                s -= xloop; xloop = 0;
                s += tempscreenseg->w; yloop++;
                break;
            case 253:   /* 1 transparent pixel */
                s++; xloop++;
                break;
            default:    /* set a pixel */
                *s = ((*s & 0x0f) >> 1) + (*s & 0xf0);
                s++; xloop++;
                break;
        }
        if(xloop == xsize) {
            s -= xloop; xloop = 0;
            s += tempscreenseg->w; yloop++;
        }
    }
}

void JE_NewDrawCShapeDarkenNum( JE_byte table, JE_byte shape, JE_word x, JE_word y )
{
    JE_NewDrawCShapeDarken((*shapearray)[table][shape], shapex[table][shape], shapey[table][shape], x, y);
}

/*Intended for Font drawing - maximum X size is 255*/
void JE_NewDrawCShapeTrick( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y )
{
    JE_word xloop = 0, yloop = 0;
    JE_byte *p; /* shape pointer */
    unsigned char *s;   /* screen pointer, 8-bit specific */

    s = (unsigned char *)tempscreenseg->pixels;
    s += y * tempscreenseg->w + x;

    for(p = shape; yloop < ysize; p++) {
        switch(*p) {
            case 255:   /* p transparent pixels */
                p++;
                s += *p; xloop += *p;
                break;
            case 254:   /* next y */
                s -= xloop; xloop = 0;
                s += tempscreenseg->w; yloop++;
                break;
            case 253:   /* 1 transparent pixel */
                s++; xloop++;
                break;
            default:    /* set a pixel */
                *s = (((*s & 0x0f) >> 1) | (*p & 0xf0)) + ((*p & 0x0f) >> 1);
                s++; xloop++;
                break;
        }
        if(xloop == xsize) {
            s -= xloop; xloop = 0;
            s += tempscreenseg->w; yloop++;
        }
    }
}

void JE_NewDrawCShapeTrickNum( JE_byte table, JE_byte shape, JE_word x, JE_word y )
{
    JE_NewDrawCShapeTrick((*shapearray)[table][shape], shapex[table][shape], shapey[table][shape], x, y);
}

/*Intended for Font drawing - maximum X size is 255*/
void JE_NewDrawCShapeModify( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_byte brightness )
{
    JE_word xloop = 0, yloop = 0;
    JE_byte *p; /* shape pointer */
    unsigned char *s;   /* screen pointer, 8-bit specific */
    JE_byte temp;

    s = (unsigned char *)tempscreenseg->pixels;
    s += y * tempscreenseg->w + x;

    filter <<= 4;

    for(p = shape; yloop < ysize; p++) {
        switch(*p) {
            case 255:   /* p transparent pixels */
                p++;
                s += *p; xloop += *p;
                break;
            case 254:   /* next y */
                s -= xloop; xloop = 0;
                s += tempscreenseg->w; yloop++;
                break;
            case 253:   /* 1 transparent pixel */
                s++; xloop++;
                break;
            default:    /* set a pixel */
                temp = (*p & 0x0f) + brightness;
                if(temp >= 0x1f)
                    temp = 0;
                if(temp >= 0x0f)
                    temp = 0x0f;
                temp >>= 1;
                *s = (((*s & 0x0f) >> 1) | filter) + temp;
                s++; xloop++;
                break;
        }
        if(xloop == xsize) {
            s -= xloop; xloop = 0;
            s += tempscreenseg->w; yloop++;
        }
    }
}

void JE_NewDrawCShapeModifyNum( JE_byte table, JE_byte shape, JE_word x, JE_word y, JE_byte filter, JE_byte brightness )
{
    JE_NewDrawCShapeModify((*shapearray)[table][shape], shapex[table][shape], shapey[table][shape], x, y, filter, brightness);
}

/*Intended for Font drawing - maximum X size is 255*/
void JE_NewDrawCShapeAdjust( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_byte brightness )
{
    JE_word xloop = 0, yloop = 0;
    JE_byte *p; /* shape pointer */
    unsigned char *s;   /* screen pointer, 8-bit specific */
    JE_byte temp;

    s = (unsigned char *)tempscreenseg->pixels;
    s += y * tempscreenseg->w + x;

    filter <<= 4;

    for(p = shape; yloop < ysize; p++) {
        switch(*p) {
            case 255:   /* p transparent pixels */
                p++;
                s += *p; xloop += *p;
                break;
            case 254:   /* next y */
                s -= xloop; xloop = 0;
                s += tempscreenseg->w; yloop++;
                break;
            case 253:   /* 1 transparent pixel */
                s++; xloop++;
                break;
            default:    /* set a pixel */
                temp = (*p & 0x0f) + brightness;
                if(temp >= 0x1f) {
                    temp = 0;
                }
                if(temp >= 0x0f) {
                    temp = 0x0f;
                }
                *s = temp | filter;
                s++; xloop++;
                break;
        }
        if(xloop == xsize) {
            s -= xloop; xloop = 0;
            s += tempscreenseg->w; yloop++;
        }
    }
}

void JE_NewDrawCShapeAdjustNum( JE_byte table, JE_byte shape, JE_word x, JE_word y, JE_byte filter, JE_byte brightness )
{
    JE_NewDrawCShapeAdjust((*shapearray)[table][shape], shapex[table][shape], shapey[table][shape], x, y, filter, brightness);
}

/*Intended for Font drawing - maximum X size is 255*/
void JE_NewDrawCShapeBrightAndDarken( JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_byte brightness )
{
    JE_word xloop = 0, yloop = 0;
    JE_byte *p; /* shape pointer */
    unsigned char *s;   /* screen pointer, 8-bit specific */

    s = (unsigned char *)tempscreenseg->pixels;
    s += y * tempscreenseg->w + x;

    filter <<= 4;

    for(p = shape; yloop < ysize; p++) {
        switch(*p) {
            case 255:   /* p transparent pixels */
                p++;
                s += *p; xloop += *p;
                break;
            case 254:   /* next y */
                s -= xloop; xloop = 0;
                s += tempscreenseg->w; yloop++;
                break;
            case 253:   /* 1 transparent pixel */
                s++; xloop++;
                break;
            default:    /* set a pixel */
                *s = ((*p & 0x0f) | filter) + brightness;
                s++; xloop++;
                s += tempscreenseg->w;  /* jump a pixel down (after incrementing x) */
                *s = ((*s & 0x0f) >> 1) + (*s & 0xf0);
                s -= tempscreenseg->w;  /* jump back up */
                break;
        }
        if(xloop == xsize) {
            s -= xloop; xloop = 0;
            s += tempscreenseg->w; yloop++;
        }
    }
}

/*void JE_NewDrawCShapeZoom( JE_byte table, JE_byte, JE_word x, JE_word y, JE_real scale );
{
    JE_byte lookuphoriz[320];
    JE_byte lookupvert[200];

    for(x = 0; x < shapex[table][shape]; x++)
        ;
}*/

INLINE JE_word JE_FontCenter( JE_string s, JE_byte font )
{
    return(160 - (JE_TextWidth(s, font) / 2));
}

JE_word JE_TextWidth( JE_string s, JE_byte font )
{
    JE_byte a, b;
    JE_word x = 0;

    for(a = 0; s[a] != 0; a++) {
        b = s[a];

        if ((b > 32) && (b < 126)) {
            if(fontmap[b-33] != 255)
                x += shapex[font][fontmap[b-33]] + 1;
        } else {
            if(b == 32) {
                x += 6;
            }
        }
   }
   return(x);
}

void JE_TextShade( JE_word x, JE_word y, JE_string s, JE_byte colorbank, JE_shortint brightness, JE_byte shadetype )
{
    switch(shadetype) {
        case PartShade:
            JE_Outtext(x+1, y+1, s, 0, -1);
            JE_Outtext(x, y, s, colorbank, brightness);
            break;
        case FullShade:
            JE_Outtext(x-1, y, s, 0, -1);
            JE_Outtext(x+1, y, s, 0, -1);
            JE_Outtext(x, y-1, s, 0, -1);
            JE_Outtext(x, y+1, s, 0, -1);
            JE_Outtext(x, y, s, colorbank, brightness);
            break;
        case Darken:
            JE_OuttextAndDarken(x+1, y+1, s, colorbank, brightness, TinyFont);
            break;
        case Trick:
            JE_OuttextModify(x, y, s, colorbank, brightness, TinyFont);
            break;
   }
}

void JE_Outtext( JE_word x, JE_word y, JE_string s, JE_byte colorbank, JE_shortint brightness )
{
    JE_byte a, b;
    JE_byte bright = 0;

    for(a = 0; s[a] != 0; a++) {
        b = s[a];

        if((b > 32) && (b < 169) && (fontmap[b-33] != 255) && ((*shapearray)[TinyFont][fontmap[b-33]] != NULL)) {
            if (brightness >= 0) {
                JE_NewDrawCShapeBright((*shapearray)[TinyFont][fontmap[b-33]], shapex[TinyFont][fontmap[b-33]], shapey[TinyFont][fontmap[b-33]], x, y, colorbank, brightness + bright);
            } else {
                JE_NewDrawCShapeShadow((*shapearray)[TinyFont][fontmap[b-33]], shapex[TinyFont][fontmap[b-33]], shapey[TinyFont][fontmap[b-33]], x, y);
            }

            x += shapex[TinyFont][fontmap[b-33]] + 1;
        } else {
            if(b == 32) {
                x += 6;
            } else {
                if(b == 126) {
                    if(bright > 0) {
                        bright = 0;
                    } else {
                        bright = 4;
                    }
                }
            }
        }
    }
    if(brightness >= 0) {
        tempscreenseg = VGAScreen;
    }
}

void JE_OuttextModify( JE_word x, JE_word y, JE_string s, JE_byte filter, JE_byte brightness, JE_byte font )
{
    JE_byte a, b;

    for(a = 0; s[a] != 0; a++) {
        b = s[a];

        if((b > 32) && (b < 169) && (fontmap[b-33] != 255)) {
            JE_NewDrawCShapeModify((*shapearray)[font][fontmap[b-33]], shapex[font][fontmap[b-33]], shapey[font][fontmap[b-33]], x, y, filter, brightness);

            x += shapex[font][fontmap[b-33]] + 1;
        } else {
            if(b == 32) {
                x += 6;
            }
        }
    }
}

void JE_OuttextShade( JE_word x, JE_word y, JE_string s, JE_byte font )
{
    JE_byte a, b;

    for(a = 0; s[a] != 0; a++) {
        b = s[a];

        if((b > 32) && (b < 169) && (fontmap[b-33] != 255)) {
            JE_NewDrawCShapeDarken((*shapearray)[font][fontmap[b-33]], shapex[font][fontmap[b-33]], shapey[font][fontmap[b-33]], x, y);

            x += shapex[font][fontmap[b-33]] + 1;
        } else {
            if(b == 32) {
                x += 6;
            }
        }
    }
}

void JE_OuttextAdjust( JE_word x, JE_word y, JE_string s, JE_byte filter, JE_shortint brightness, JE_byte font, JE_boolean shadow )
{
    JE_byte a, b;
    JE_boolean bright = FALSE;

    for(a = 0; s[a] != 0; a++) {
        b = s[a];

        if((b > 32) && (b < 169) && (fontmap[b-33] != 255)) {
            if(shadow) {
                JE_NewDrawCShapeDarken((*shapearray)[font][fontmap[b-33]], shapex[font][fontmap[b-33]], shapey[font][fontmap[b-33]], x + 2, y + 2);
            }

            JE_NewDrawCShapeAdjust((*shapearray)[font][fontmap[b-33]], shapex[font][fontmap[b-33]], shapey[font][fontmap[b-33]], x, y, filter, brightness + (bright << 2));

            x += shapex[font][fontmap[b-33]] + 1;
        } else {
            if(b == 126) {
                bright = !bright;
            } else {
                if(b == 32) {
                    x += 6;
                }
            }
        }
    }
}

void JE_OuttextAndDarken( JE_word x, JE_word y, JE_string s, JE_byte colorbank, JE_byte brightness, JE_byte font )
{
    JE_byte a, b;
    JE_byte bright = 0;

    for(a = 0; s[a] != 0; a++) {
        b = s[a];

        if((b > 32) && (b < 169) && (fontmap[b-33] != 255)) {
            JE_NewDrawCShapeBrightAndDarken((*shapearray)[font][fontmap[b-33]], shapex[font][fontmap[b-33]], shapey[font][fontmap[b-33]], x, y, colorbank, brightness + bright);

            x += shapex[font][fontmap[b-33]] + 1;
        } else {
            if(b == 32) {
                x += 6;
            } else {
                if(b == 126) {
                    if(bright > 0) {
                        bright = 0;
                    } else {
                        bright = 4;
                    }
                }
            }
        }
    }
}

JE_char JE_Bright( JE_boolean makebright )
{
    if(makebright) {
        return('~');
    } else {
        return('\0');
    }
}
