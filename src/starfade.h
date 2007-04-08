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
#ifndef STARFADE_H
#define STARFADE_H

#include "opentyr.h"
#include "nortvars.h"

#ifndef NO_EXTERNS
extern JE_colortype black, colors, colors2;
#endif

/*void UpdateColorsSlow( JE_colortype *ColorBuffer );*/
void JE_updateColorsFast( JE_colortype *ColorBuffer );
void JE_fadeColors( JE_colortype *FromColors, JE_colortype *ToColors, JE_byte StartCol, JE_byte NoColors, JE_byte NoSteps );
void JE_fadeBlack( JE_byte steps );
void JE_fadeColor( JE_byte steps );
void JE_fadeWhite( JE_byte steps );

#endif /* STARFADE_H */
