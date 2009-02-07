/* 
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
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
#include "opentyr.h"
#include "picload.h"

#include "error.h"
#include "nortvars.h"
#include "palette.h"
#include "pcxmast.h"
#include "video.h"

#include <string.h>


JE_boolean notyetloadedpcx;
JE_boolean notYetLoadedPCX = true;

void JE_loadPic( JE_byte PCXnumber, JE_boolean storepal )
{
	typedef JE_byte JE_buftype[63000]; /* [1..63000] */

	JE_word x;
	JE_buftype buf;
	FILE *PCXfile;

	int i;
	JE_byte *p;
	Uint8 *s; /* screen pointer, 8-bit specific */

	s = (Uint8 *)VGAScreen->pixels;

	PCXnumber--;

	JE_resetFile(&PCXfile, "tyrian.pic");

	/*Same as old AnalyzePic*/
	if (notYetLoadedPCX)
	{
		notYetLoadedPCX = false;
		efread(&x, sizeof(JE_word), 1, PCXfile);
		for (x = 0; x < PCX_NUM; x++)
		{
			efread(&pcxpos[x], sizeof(JE_longint), 1, PCXfile);
		}
		fseek(PCXfile, 0, SEEK_END);
		pcxpos[PCX_NUM] = ftell(PCXfile);
	}

	fseek(PCXfile, pcxpos[PCXnumber], SEEK_SET);
	efread(buf, sizeof(JE_byte), pcxpos[PCXnumber + 1] - pcxpos[PCXnumber], PCXfile);
	fclose(PCXfile);

	p = (JE_byte *)buf;
	for (i = 0; i < 320 * 200; )
	{
		if ((*p & 0xc0) == 0xc0)
		{
			i += (*p & 0x3f);
			memset(s, *(p + 1), (*p & 0x3f));
			s += (*p & 0x3f); p += 2;
		} else {
			i++;
			*s = *p;
			s++; p++;
		}
		if (i && (i % 320 == 0))
		{
			s += VGAScreen->pitch - 320;
		}
	}

	memcpy(colors, palettes[pcxpal[PCXnumber]], sizeof(colors));
	if (storepal)
	{
		JE_updateColorsFast(colors);
	}
}

// kate: tab-width 4; vim: set noet:
