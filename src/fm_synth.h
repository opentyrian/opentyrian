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
#ifndef FM_SYNTH_H
#define FM_SYNTH_H

#include "fmopl.h"
#include "opentyr.h"

extern const unsigned char op_table[9]; /* the 9 operators as expected by the OPL2 */

void opl_update( OPLSAMPLE *buf, int samples );
void opl_init( void );
void opl_deinit( void );
void opl_reset( void );
void opl_write( int, int );

#endif /* FM_SYNTH_H */

