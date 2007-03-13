#ifndef NORTVARS_H
#define NORTVARS_H

#include "opentyr.h"
#include "vga256d.h"

typedef JE_char JE_textbuffer[4096]; /* [1..4096] */

struct JE_ColorRec {
    JE_byte r, g, b;
};

typedef struct JE_ColorRec JE_ColorType[256]; /* [0..255] */

/*typedef JE_byte JE_shapetypeone[168];*/ /* [0..168-1] */ /* defined in vga256d */
typedef JE_shapetypeone JE_shapetype[304]; /* [1..304] */

typedef JE_byte JE_newshapetypeone[182]; /* [0..168+14-1] */
typedef JE_newshapetypeone JE_newshapetype[304]; /* [1..304] */

#define NORTVARS_EXTERNS \
extern JE_byte NV_shapeactive; \
extern JE_byte NV_shapeinactive; \
extern JE_boolean ScanForJoystick; \
extern JE_word z, y; \
extern JE_boolean InputDetected; \
extern JE_word LastMouseX, LastMouseY; \
extern JE_byte MouseCursor; \
extern JE_boolean Mouse_ThreeButton; \
extern JE_word MouseX, MouseY, MouseButton;

void JE_LoadShapeFile( JE_shapetype *Shapes, JE_char s );
void JE_LoadNewShapeFile( JE_newshapetype *Shapes, JE_char s );

void JE_CompressShapeFileC( JE_char s );
void JE_LoadCompShapes( JE_byte **Shapes, JE_word *ShapeSize, JE_char s, JE_byte **Shape );
JE_word JE_BtoW( JE_byte a, JE_byte b );
JE_string JE_st( JE_longint num );

JE_word JE_MousePosition( JE_word *MouseX, JE_word *MouseY );
JE_boolean JE_Buttonpressed( void );
void JE_SetMousePosition( JE_word MouseX, JE_word MouseY );

JE_boolean JE_AnyButton( void );

/*void JE_ShowMouse( void );
void JE_HideMouse( void );*/

void JE_dbar( JE_word x, JE_word y, JE_word num, JE_word col );
void JE_dbar2( JE_word x, JE_word y, JE_word num, JE_word col );
void JE_dbar3( JE_integer x,  JE_integer y,  JE_integer num,  JE_integer col );
void JE_dbar4( JE_word x, JE_word y, JE_word num, JE_word col );
void JE_bardraw( JE_word x, JE_word y, JE_word res, JE_word col, JE_word amt, JE_word xsize, JE_word ysize );
void JE_bardrawshadow( JE_word x, JE_word y, JE_word res, JE_word col, JE_word amt, JE_word xsize, JE_word ysize );
void JE_bardrawdirect( JE_word x, JE_word y, JE_word res, JE_word col, JE_word amt, JE_word xsize, JE_word ysize );
void JE_wipekey( void );
void JE_drawansi( JE_string ansiscreen );
/*JE_boolean JE_waitaction( JE_byte time );*/

void JE_DrawShape2( JE_word x, JE_word y, JE_word s, JE_byte *Shape );

void JE_SuperDrawShape2( JE_word x, JE_word y, JE_word s, JE_byte *Shape );

void JE_DrawShape2Shadow( JE_word x, JE_word y, JE_word s, JE_byte *Shape );
void JE_DrawShape2x2( JE_word x, JE_word y, JE_word s, JE_byte *Shape );
void JE_SuperDrawShape2x2( JE_word x, JE_word y, JE_word s, JE_byte *Shape );
void JE_DrawShape2x2Shadow( JE_word x, JE_word y, JE_word s, JE_byte *Shape );

#endif /* NORTVARS_H */
