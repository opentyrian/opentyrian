#ifndef NEWSHAPE_H
#define NEWSHAPE_H

#include "opentyr.h"

#define OldBlack         0
#define NewBlack         253
#define PlanetShapes    0
#define FontShapes      1
#define SmallFontShapes 2
#define FaceShapes      3
#define OptionShapes    4 /*Also contains help shapes*/
#define TinyFont        5
#define WeaponShapes    6
#define ExtraShapes     7 /*Used for Ending pics*/

#define maximumshape     151
#define maxtable         8

typedef JE_byte *JE_shapearraytype[maxtable][maximumshape];	/* [1..maxtable, 1..maximumshape] */

#define NEWSHAPE_EXTERNS \
extern JE_word min, max; \
extern SDL_Surface *tempscreenseg; \
extern JE_shapearraytype *shapearray; \
extern JE_word shapex[maxtable][maximumshape],	/* [1..maxtable,1..maximumshape] */ \
               shapey[maxtable][maximumshape];	/* [1..maxtable,1..maximumshape] */ \
extern JE_word shapesize[maxtable][maximumshape];	/* [1..maxtable,1..maximumshape] */ \
extern JE_boolean shapexist[maxtable][maximumshape];	/* [1..maxtable,1..maximumshape] */ \
extern JE_byte maxshape[maxtable];	/* [1..maxtable] */ \
extern JE_byte mousegrabshape[24*28];	/* [1..24*28] */ \
extern JE_byte x; \
extern JE_boolean LoadOverride; \
extern JE_byte *Shapes6Pointer;

void JE_NewLoadShapes( JE_byte table, JE_string shapefile );
void JE_NewLoadShapesB( JE_byte table, FILE *f );
void JE_NewCompressBlock( JE_byte **shape, JE_word xsize, JE_word ysize, JE_word *shapesize );
void JE_NewDrawShape( JE_byte *shape, JE_word xsize, JE_word ysize );
void JE_NewDrawCShape( JE_byte *shape, JE_word xsize, JE_word ysize );
void JE_NewDrawCShapeNum( JE_byte table, JE_byte shape, JE_word x, JE_word y );
void JE_NewPurgeShapes( JE_byte table );
/*void  JE_OverrideLoadShapes( JE_byte table, char *shapefile, JE_word minimum, JE_word maximum );*/

void JE_DrawShapeTypeOne( JE_word x, JE_word y, JE_byte *shape );
void JE_GrabShapeTypeOne( JE_word x, JE_word y, JE_byte *shape );

JE_boolean JE_WaitAction( JE_byte time, JE_boolean checkjoystick );
void JE_MouseStart( void );
void JE_MouseReplace( void );

void newshape_init( void );

#endif /* NEWSHAPE_H */
