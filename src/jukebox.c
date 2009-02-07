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
#include "jukebox.h"

#include "loudness.h"
#include "mtrand.h"
#include "nortsong.h"


JE_boolean continuousPlay = false;
JE_word currentJukeboxSong = 0; /* SYN: used to be currentsong, but that name conflicted with elsewhere */

void JE_playNewSelection( void )
{
	currentSong = ( mt_rand() % MUSIC_NUM );
	JE_playSong(currentSong);
}

void JE_jukebox_selectSong( JE_word song )
{
	JE_selectSong(song);
	repeated = false;
	playing = true;
}

void JE_checkEndOfSong( void )
{
	if (!continuousPlay && (repeated || !playing) )
	{
		JE_playNewSelection();
		repeated = false;
	}
	if (continuousPlay && !playing)
	{
		JE_jukebox_selectSong(1);
	}
}

// kate: tab-width 4; vim: set noet:
