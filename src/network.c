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
#include "network.h"


JE_longint startTime;
JE_longint frames;

JE_integer netPlayers,     /* Number of net players: 0, 1 or 2 */
           thisPlayerNum = 0,  /* Player number on this PC (1 or 2) */
           otherPlayerNum; /* Player number on remote PC (1 or 2) */

/* TODO UseOutPacket : SMALL_PACKET;*/   /* This is the original player's packet - time lag */

JE_boolean haltGame;
JE_boolean netQuit;
JE_boolean done;

JE_boolean moveOk;
JE_boolean firstTime;
JE_boolean netResult;

JE_byte gameQuitDelay;  /* Make sure the game doesn't quit within the first few frames of starting */

JE_byte outputData[10]; /* [1..10] */
JE_byte inputData[10]; /* [1..10] */

/* Special Requests */
JE_boolean pauseRequest, skipLevelRequest, helpRequest, nortShipRequest;
JE_boolean yourInGameMenuRequest, inGameMenuRequest;

JE_boolean portConfigChange, portConfigDone;

JE_byte exchangeCount;

/* Network Stuff */
JE_boolean netSuccess;

void JE_updateStream( void )
{
//	STUB();
}

void JE_setNetByte( JE_byte send)
{
//	STUB();
}

JE_boolean JE_scanNetByte( JE_byte scan )
{
//	STUB();
	return true;
}

void JE_clearSpecialRequests( void )
{
	pauseRequest = false;
	inGameMenuRequest = false;
	skipLevelRequest = false;
	helpRequest = false;
	nortShipRequest = false;
}

/* TODO */
