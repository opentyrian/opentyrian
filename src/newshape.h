#ifndef _NEWSHAPE_H
#define _NEWSHAPE_H

const JE_integer OldBlack = 0;
const JE_integer NewBlack = 253;
#define maximumshape 151
#define maxtable 8
const JE_integer _PlanetShapes    = 0;
const JE_integer _FontShapes      = 1;
const JE_integer _SmallFontShapes = 2;
const JE_integer _FaceShapes      = 3;
const JE_integer _OptionShapes    = 4;   /*Also contains help shapes*/
const JE_integer _TinyFont        = 5;
const JE_integer _WeaponShapes    = 6;
const JE_integer _ExtraShapes     = 7;   /*Used for Ending pics*/

typedef JE_byte *shapearraytype[maxtable][maximumshape];

JE_boolean LoadOverride;
JE_word min, max;

SDL_Surface *tempscreenseg = NULL;

shapearraytype *shapearray;

JE_word shapex[maxtable][maximumshape],	/* [1..maxtable,1..maximumshape] */
        shapey[maxtable][maximumshape];	/* [1..maxtable,1..maximumshape] */
JE_word shapesize[maxtable][maximumshape];	/* [1..maxtable,1..maximumshape] */
JE_boolean shapexist[maxtable][maximumshape];	/* [1..maxtable,1..maximumshape] */

JE_byte maxshape[maxtable];	/* [1..maxtable] */

JE_byte mousegrabshape[24*28];	/* [1..24*28] */

JE_byte x;

/*
  Colors:
  253 : Black
  254 : Jump to next line

   Skip X Pixels
   Draw X pixels of color Y
*/

void JE_NewLoadShapes(JE_byte table, char *shapefile);
void JE_NewLoadShapesB(JE_byte table, FILE *f);
void JE_NewCompressBlock(JE_byte **shape, JE_word xsize, JE_word ysize, JE_word *shapesize);
void JE_NewDrawShape(JE_byte *shape, JE_word xsize, JE_word ysize);
void JE_NewDrawCShape(JE_byte *shape, JE_word xsize, JE_word ysize);
void JE_NewDrawCShapeNum(JE_byte table, JE_byte shape, JE_word x, JE_word y);
void JE_NewPurgeShapes(JE_byte table);
/*void  JE_OverrideLoadShapes(JE_byte table, char *shapefile, JE_word minimum, JE_word maximum);*/

void JE_DrawShapeTypeOne(JE_word x, JE_word y, JE_byte *shape);
void JE_GrabShapeTypeOne(JE_word x, JE_word y, JE_byte *shape);

JE_boolean JE_WaitAction(JE_byte time, JE_boolean checkjoystick);
void JE_MouseStart();
void JE_MouseReplace();

JE_byte *Shapes6Pointer;

void newshape_init();

#endif /* _NEWSHAPE_H */
