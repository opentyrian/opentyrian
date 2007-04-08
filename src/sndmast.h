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
#ifndef SNDMAST_H
#define SNDMAST_H

#define SoundNum 29

#define Click 24
#define Wrong 23
#define ESC   16
#define Item  18
#define Select 8
#define CursorMove 28
#define PowerUp 29

#define V_GoodLuck (soundnum + 4)
#define V_LevelEnd (soundnum + 5)
#define V_DataCube (soundnum + 8)

#ifndef NO_EXTERNS
extern const char soundTitle[SoundNum + 9][9];
extern const JE_byte WindowTextSamples[9];
#endif

#endif /* SNDMAST_H */
