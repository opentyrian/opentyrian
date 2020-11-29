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
#include "musmast.h"

#include "opentyr.h"

JE_byte songBuy;

const char musicTitle[MUSIC_NUM][48] =
{
	"Asteroid Dance Part 2",
	"Asteroid Dance Part 1",
	"Buy/Sell Music",
	"CAMANIS",
	"CAMANISE",
	"Deli Shop Quartet",
	"Deli Shop Quartet No. 2",
	"Ending Number 1",
	"Ending Number 2",
	"End of Level",
	"Game Over Solo",
	"Gryphons of the West",
	"Somebody pick up the Gryphone",
	"Gyges, Will You Please Help Me?",
	"I speak Gygese",
	"Halloween Ramble",
	"Tunneling Trolls",
	"Tyrian, The Level",
	"The MusicMan",
	"The Navigator",
	"Come Back to Me, Savara",
	"Come Back again to Savara",
	"Space Journey 1",
	"Space Journey 2",
	"The final edge",
	"START5",
	"Parlance",
	"Torm - The Gathering",
	"TRANSON",
	"Tyrian: The Song",
	"ZANAC3",
	"ZANACS",
	"Return me to Savara",
	"High Score Table",
	"One Mustn't Fall",
	"Sarah's Song",
	"A Field for Mag",
	"Rock Garden",
	"Quest for Peace",
	"Composition in Q",
	"BEER"
};

JE_boolean musicFade;

