/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
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
#include "pcxload.h"

#include "file.h"
#include "opentyr.h"
#include "palette.h"
#include "video.h"

#include <string.h>

void JE_loadPCX( const char *file ) // this is only meant to load tshp2.pcx
{
	Uint8 *s = VGAScreen->pixels; /* 8-bit specific */
	
	FILE *f = dir_fopen_die(data_dir(), file, "rb");
	
	fseek(f, -769, SEEK_END);

	Uint8 temp;
	fread_u8_die(&temp, 1, f);
	if (temp == 12)
	{
		for (int i = 0; i < 256; i++)
		{
			Uint8 rgb[3];
			fread_u8_die(rgb, 3, f);
			colors[i].r = rgb[0];
			colors[i].g = rgb[1];
			colors[i].b = rgb[2];
		}
	}
	
	fseek(f, 128, SEEK_SET);
	
	for (int i = 0; i < 320 * 200; )
	{
		Uint8 p;
		fread_u8_die(&p, 1, f);
		if ((p & 0xc0) == 0xc0)
		{
			i += (p & 0x3f);
			fread_u8_die(&temp, 1, f);
			memset(s, temp, (p & 0x3f));
			s += (p & 0x3f);
		}
		else
		{
			i++;
			*s = p;
			s++;
		}
		if (i && (i % 320 == 0))
		{
			s += VGAScreen->pitch - 320;
		}
	}
	
	fclose(f);
}

