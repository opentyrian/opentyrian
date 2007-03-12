#include "opentyr.h"
#include "nortvars.h"

struct pcxheader_rec {
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

JE_ColorType Colors2;
JE_word width,
        depth;
JE_word Bytes;
JE_char c, c2;
JE_boolean overrideblack;

void LoadPCX( JE_string Name, JE_boolean storepalette );
void UpdatePCXColorsSlow( JE_ColorType *ColorBuffer );
