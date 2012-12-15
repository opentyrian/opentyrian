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
#include "fmopl.h"
#include "fm_synth.h"
#include "loudness.h"
#include "opentyr.h"

const unsigned char op_table[9] = {0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12};

#define opl 0

void opl_update( OPLSAMPLE *buf, int samples )
{
	YM3812UpdateOne(opl, buf, samples);
}

void opl_init( void )
{
	YM3812Init(1, 3579545, 11025 * OUTPUT_QUALITY);
}

void opl_deinit( void )
{
	YM3812Shutdown();
}

void opl_reset( void )
{
	YM3812ResetChip(opl);
}

void opl_write(int reg, int val)
{
  YM3812Write(opl, 0, reg);
  YM3812Write(opl, 1, val);
}

