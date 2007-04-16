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
#ifndef PCXLOAD_H
#define PCXLOAD_H

#include "opentyr.h"
#include "nortvars.h"
#include "error.h"

struct JE_PCXHeader_rec {
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
	JE_word bytes_per_line;
	JE_word palette_type;
};

#ifndef NO_EXTERNS
extern JE_ColorType colors2;
extern JE_word width, depth;
extern JE_word bytes;
extern JE_char /*c,*/ c2;
extern JE_boolean overrideBlack;
#endif

void JE_loadPCX( JE_string name, JE_boolean storePalette );
void JE_updatePCXColorsSlow( JE_ColorType *colorBuffer );

#endif /* PCXLOAD_H */
