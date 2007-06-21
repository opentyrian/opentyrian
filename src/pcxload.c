/* vim: set noet:
 *
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

#include "starfade.h"

#define NO_EXTERNS
#include "pcxload.h"
#undef NO_EXTERNS

#include "SDL.h"


JE_ColorType colors2;
JE_word width, depth;
JE_word bytes;
JE_char c, c2;
JE_boolean overrideBlack = FALSE;

void JE_updatePCXColorsSlow( JE_ColorType *colorBuffer )
{
	int i;
	for (i = 0; i < 256; i++)
	{
		colorBuffer[i]->r >>= 2;
		colorBuffer[i]->g >>= 2;
		colorBuffer[i]->b >>= 2;
	}
}

void JE_loadPCX( char *name, JE_boolean storePalette)
{
	struct JE_PCXHeader_rec header;
	/* JE_char textbuf[1024]; [1..1024] Unused */
	FILE *fi;
	/* FILE *PCXfile; Unused */
	int i;

	JE_resetFile(&fi, name);
	efread(&header.manufacturer, 1, 1, fi);
	efread(&header.version, 1, 1, fi);
	efread(&header.encoding, 1, 1, fi);
	efread(&header.bits_per_pixel, 1, 1, fi);
	efread(&header.xmin, 2, 1, fi);
	efread(&header.ymin, 2, 1, fi);
	efread(&header.xmax, 2, 1, fi);
	efread(&header.ymax, 2, 1, fi);
	efread(&header.hres, 2, 1, fi);
	efread(&header.vres, 2, 1, fi);
	for (i = 0; i < 48; i++)
	{
		efread(&header.palette[i], 1, 1, fi);
	}
	efread(&header.reserved, 1, 1, fi);
	efread(&header.colour_planes, 1, 1, fi);
	efread(&header.bytes_per_line, 2, 1, fi);
	efread(&header.palette_type, 2, 1, fi);

	if ((header.manufacturer == 10) && (header.version == 5) && (header.bits_per_pixel == 8) && (header.colour_planes == 1))
	{
		fseek(fi, -768, SEEK_END);

		for (i = 0; i < 256; i++)
		{
			colors2[i].r = getc(fi);
			colors2[i].g = getc(fi);
			colors2[i].b = getc(fi);
		}
		JE_updatePCXColorsSlow(&colors2);
		if (storePalette)
		{
			JE_updateColorsFast(&colors2);
		} else {
			if (!overrideBlack)
			{
				JE_updateColorsFast(&black);
			} else {
				overrideBlack = FALSE;
			}
		}

		memcpy(colors, colors2, sizeof(colors));

		fclose(fi);


		/* TODO */
	}

	fclose(fi);
}

/*void JE_unpackPCX( void )
{
	STUB(JE_unpackPCX);
}*/
