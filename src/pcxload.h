#include "opentyr.h"
#include "nortvars.h"

struct JE_pcxheader {
    JE_byte manufacturer;
    JE_byte version;
    JE_byte encoding;
    JE_byte bits_per_pixel;
    JE_word xmin, ymin;
    JE_word xmax, ymax;
    JE_word hres, vres;
    JE_byte palette[48];  /* [0..47] */
    JE_byte reserved;
    JE_byte colour_planes;
    JE_word Bytes_per_line;
    JE_word palette_Type;
};

#define PCXLOAD_EXTERNS \
extern JE_ColorType Colors2; \
extern JE_word width, \
               depth; \
extern JE_word Bytes; \
extern JE_char c, c2; \
extern JE_boolean overrideblack;

void JE_LoadPCX( JE_string Name, JE_boolean storepalette );
void JE_UpdatePCXColorsSlow( JE_ColorType *ColorBuffer );
