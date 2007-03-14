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
#include "opentyr.h"
#include "pcxload.h"

JE_ColorType Colors2;
JE_word width,
        depth;
JE_word Bytes;
JE_char c, c2;
JE_boolean overrideblack = FALSE;

void JE_UpdatePCXColorsSlow( JE_ColorType *ColorBuffer )
{
	int i;
    for(i = 0; i < 256; i++)
    {
        ColorBuffer[i]->r <<= 2;
        ColorBuffer[i]->g <<= 2;
        ColorBuffer[i]->b <<= 2;
    }
}

void JE_LoadPCX( JE_string Name, JE_boolean storepalette)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
printf("%s doesn't support big-endian processors yet. =[\n", __FILE__); exit(1);
#endif
    struct JE_pcxheader header;
    JE_char textbuf[1024]; /* [1..1024] */
    FILE *fi;
    FILE *PCXfile;
    int i;

    fi = fopen(Name, "rb");
    fread(&header.manufacturer, 1, 1, fi);
    fread(&header.version, 1, 1, fi);
    fread(&header.encoding, 1, 1, fi);
    fread(&header.bits_per_pixel, 1, 1, fi);
    fread(&header.xmin, 2, 1, fi);
    fread(&header.ymin, 2, 1, fi);
    fread(&header.xmax, 2, 1, fi);
    fread(&header.ymax, 2, 1, fi);
    fread(&header.hres, 2, 1, fi);
    fread(&header.vres, 2, 1, fi);
    for(i = 0; i < 48; i++)
        fread(&header.palette[i], 1, 1, fi);
    fread(&header.reserved, 1, 1, fi);
    fread(&header.colour_planes, 1, 1, fi);
    fread(&header.bytes_per_line, 2, 1, fi);
    fread(&header.palette_type, 2, 1, fi);

    if((header.manufacturer == 10) && (header.version == 5) && (header.bits_per_pixel == 8) && (header.colour_planes == 1))
    {
        /* TODO */
    }
}
