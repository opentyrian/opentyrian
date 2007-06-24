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

#include "fmopl.h"
#include "loudness.h"

#define NO_EXTERNS
#include "fm_synth.h"
#undef NO_EXTERNS


const unsigned short note_table[12] = {363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647, 686};
const unsigned char op_table[9] = {0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12};

int use16bit, stereo;
FM_OPL	*opl; /* holds emulator data */

void opl_update( short* buf, long samples)
{
	int i;
	
	if (use16bit) {
		YM3812UpdateOne(opl, buf, samples);
		
		if (stereo)
			for(i = samples - 1; i >= 0; i--) {
				buf[i*2] = buf[i];
				buf[i*2+1] = buf[i];
			}
  	} else {
		short *tempbuf = malloc(sizeof(short) * (stereo ? samples * 2 : samples));
		int i;
	
		YM3812UpdateOne(opl, tempbuf, samples);
	
		if (stereo)
			for (i = samples - 1; i >= 0; i--) {
				tempbuf[i*2] = tempbuf[i];
				tempbuf[i*2+1] = tempbuf[i];
			}
	
		for (i = 0; i < (stereo ? samples * 2 : samples); i++)
			((char *)buf)[i] = (tempbuf[i] >> 8) ^ 0x80;
	
		free(tempbuf);
	}
}

void opl_init( void )
{
	use16bit = (BYTES_PER_SAMPLE == 2);
	stereo = 0;

	opl = OPLCreate(OPL_TYPE_YM3812, 3579545, (11025 * OUTPUT_QUALITY));
}

void opl_deinit( void )
{
	OPLDestroy(opl);
}

void opl_reset( void )
{
	OPLResetChip(opl);
}

void opl_write(int reg, int val)
{
  OPLWrite(opl, 0, reg);
  OPLWrite(opl, 1, val);
}
