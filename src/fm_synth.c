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

#include "adlibemu.h"
#include "loudness.h"

#define NO_EXTERNS
#include "fm_synth.h"
#undef NO_EXTERNS


const unsigned short note_table[12] = {363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647, 686};
const unsigned char op_table[9] = {0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12};


void opl_update( short* buf, long samples)
{
	samples *= BYTES_PER_SAMPLE;
	/*if(stereo) samples *= 2;*/
	adlibgetsample((unsigned char*)buf, samples);
}


void opl_init( void )
{
	/* adlibinit(rate, usestereo ? 2 : 1, bit16 ? 2 : 1); */
	adlibinit((11025 * OUTPUT_QUALITY), 1, BYTES_PER_SAMPLE);
	/*adlibinit(44100, 1, 2);*/
}

void opl_write(int reg, int val)
{
	/*if(currChip == 0)*/
	adlib0(reg, val);
}


/*
class CKemuopl: public Copl
{
public:
  CKemuopl(int rate, bool bit16, bool usestereo)
    : use16bit(bit16), stereo(usestereo)
    {
      adlibinit(rate, usestereo ? 2 : 1, bit16 ? 2 : 1);
      currType = TYPE_OPL2;
    };

  void update(short *buf, int samples)
    {
      if(use16bit) samples *= 2;
      if(stereo) samples *= 2;
      adlibgetsample(buf, samples);
    }

  // template methods
  void write(int reg, int val)
    {
      if(currChip == 0)
	adlib0(reg, val);
    };

  void init() {};

private:
  bool	use16bit,stereo;
};
*/
