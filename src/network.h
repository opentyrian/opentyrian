/* vim: set noet:
 *
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Development Team
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
#ifndef NETWORK_H
#define NETWORK_H

#include "opentyr.h"

#include "SDL.h"
#include "SDL_net.h"


#define PACKET_ACKNOWLEDGE  0x00    // 

#define PACKET_CONNECT      0x10    // version, delay, episodes, player_number, name
#define PACKET_DETAILS      0x11    // episode, difficulty

#define PACKET_QUIT         0x20    // 
#define PACKET_WAITING      0x21    // 
#define PACKET_BUSY         0x22    // 

#define PACKET_GAME_QUIT    0x30    // 
#define PACKET_GAME_STATE   0x31    // <state>  ... these packets are not acknowledged
#define PACKET_GAME_RESEND  0x32    // state_id

extern int network_delay;

extern char *network_opponent_host;
extern Uint16 network_player_port, network_opponent_port;
extern char *network_player_name, *network_opponent_name;

extern UDPpacket *packet_in, *packet_out,
                 *packet_state_in[], *packet_state_out[];

extern JE_integer thisPlayerNum;
extern JE_boolean haltGame;
extern JE_boolean netQuit;
extern JE_boolean moveOk;
extern JE_boolean pauseRequest, skipLevelRequest, helpRequest, nortShipRequest;
extern JE_boolean yourInGameMenuRequest, inGameMenuRequest;
extern JE_boolean portConfigChange, portConfigDone;


int network_check( void );
int network_acknowledge( Uint16 sync );
void network_prepare( Uint16 type );
int network_send( int len );
bool network_is_sync( void );
bool network_is_alive( void );
void network_reset_keep_alive( void );
bool network_keep_alive( void );

void network_state_prepare( void );
int network_state_send( void );
bool network_state_update( void );
bool network_state_is_latest( void );
bool network_state_is_reset( void );
void network_state_reset( void );

int network_connect( void );
void network_tyrian_halt( int err, bool attempt_sync );

int network_init( void );

void network_packet_copy( UDPpacket *dst, UDPpacket *src );

void JE_clearSpecialRequests( void );

#define NETWORK_BUSY_KEEP_ALIVE() \
		if (isNetworkGame) \
		{ \
			network_check(); \
			if (network_keep_alive() && network_is_sync()) \
			{ \
				network_prepare(PACKET_BUSY); \
				network_send(4); \
			} \
		}


#endif /* NETWORK_H */
