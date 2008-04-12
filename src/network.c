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
#include "opentyr.h"
#include "network.h"

#include "episodes.h"
#include "fonthand.h"
#include "helptext.h"
#include "joystick.h"
#include "keyboard.h"
#include "mainint.h"
#include "newshape.h"
#include "starfade.h"
#include "picload.h"
#include "varz.h"
#include "vga256d.h"

#include "SDL.h"
#include "SDL_net.h"

#include <assert.h>


#define NETWORK_VERSION       1            // increment whenever networking changes might create incompatability
#define NETWORK_PORT          1333         // UDP

#define NETWORK_PACKET_SIZE   256
#define NETWORK_PACKET_QUEUE  16

#define NETWORK_RETRY         640          // ticks to wait for packet acknowledgement before resending
#define NETWORK_RESEND        320          // ticks to wait before requesting unreceived game packet
#define NETWORK_KEEP_ALIVE    1600         // ticks to wait between keep-alive packets
#define NETWORK_TIME_OUT      16000        // ticks to wait before considering connection dead

int network_delay = 1 + 1;  // minimum is 1

char *network_opponent_host = 0;

Uint16 network_player_port = NETWORK_PORT,
       network_opponent_port = NETWORK_PORT;

char *network_player_name = "",
     *network_opponent_name = "";

UDPsocket socket;
IPaddress ip;

UDPpacket *packet_in, *packet_out, *packet_ack, *packet_temp;

Uint16 last_in_sync = 0, last_out_sync = 0, last_ack_sync = 0;
Uint32 last_in_tick = 0, last_out_tick = 0, last_ack_tick = 0;

UDPpacket *packet_state_in[NETWORK_PACKET_QUEUE] = {0}, *packet_state_out[NETWORK_PACKET_QUEUE] = {0};

Uint16 last_state_in_sync = 0, last_state_out_sync = 0;
Uint32 last_state_in_tick = 0, last_state_out_tick = 0;

bool quit = false;


JE_integer thisPlayerNum = 0;  /* Player number on this PC (1 or 2) */

JE_boolean haltGame = false;
JE_boolean netQuit = false;

JE_boolean moveOk;

/* Special Requests */
JE_boolean pauseRequest, skipLevelRequest, helpRequest, nortShipRequest;
JE_boolean yourInGameMenuRequest, inGameMenuRequest;

JE_boolean portConfigChange, portConfigDone;

/* networking deals in two types of packets

acknowledged packets:
receiving: the client expects to receive no packets with a higher sync id
           until it has acknowledged the current packet's sync id, thus
           these packets are always expected to be in order and reliable (there
           are currently no checks to make sure a packet matches the expected
           sync id, these duplicate packets are not expected to cause adverse
           effects)
sending:   the client sends a packet only after the previous packet's sync id
           has been aknowledged; it does, however, resend the current packet
           (using the same sync id) after a specified timeout since it may have
           been lost

state packets:
receiving: the client places the packet in a queue and retrieves it by its sync
           id when needed, if a packet is not available when needed it is
           requested via an acknowledged packet
sending:   the client sends a packet once (although it could send an xor of
           previous packets to avoid the effects of packet loss), it retains
           only the previously sent packets it expects might be requested for
           resending
*/

int network_check( void )
{
	switch (SDLNet_UDP_Recv(socket, packet_temp))
	{
		case -1:
			printf("SDLNet_UDP_Recv: %s\n", SDL_GetError());
			return -1;
			break;
		case 0:
			break;
		default:
			if (packet_temp->channel == 0 && packet_temp->len >= 4)
			{
				switch (SDLNet_Read16(&packet_temp->data[0]))
				{
					case PACKET_ACKNOWLEDGE:
						// process acknowledgement only for sent packets
						if (SDLNet_Read16(&packet_temp->data[2]) <= last_out_sync)
							last_ack_sync = SDLNet_Read16(&packet_temp->data[2]);
						
						last_ack_tick = SDL_GetTicks();
						break;
						
					case PACKET_GAME_RESEND:
						// resend state packet if still available
						if (last_state_out_sync - (SDLNet_Read16(&packet_temp->data[4]) - 1) > 0 && last_state_out_sync - (SDLNet_Read16(&packet_temp->data[4]) - 1) < NETWORK_PACKET_QUEUE)
						{
							if (packet_state_out[last_state_out_sync - (SDLNet_Read16(&packet_temp->data[4]) - 1)])
							{
								if (!SDLNet_UDP_Send(socket, 0, packet_state_out[last_state_out_sync - (SDLNet_Read16(&packet_temp->data[4]) - 1)]))
								{
									printf("SDLNet_UDP_Send: %s\n", SDL_GetError());
									return -1;
								}
							}
						}
						
						network_acknowledge(SDLNet_Read16(&packet_temp->data[2]));
						break;
						
					case PACKET_GAME_STATE:
						if (SDLNet_Read16(&packet_temp->data[2]) >= last_state_in_sync)
						{
							int i, j;
							// find empty packet in queue
							for (i = 1; i < NETWORK_PACKET_QUEUE && packet_state_in[i]; i++);
							// check for duplicates
							for (j = 1; j < NETWORK_PACKET_QUEUE && (!packet_state_in[j] || SDLNet_Read16(&packet_state_in[j]->data[2]) != SDLNet_Read16(&packet_temp->data[2])); j++);
							if (j == NETWORK_PACKET_QUEUE)
							{
								// if no empty packets, overwrite last in queue
								if (i == NETWORK_PACKET_QUEUE)
									i--;
								else
									packet_state_in[i] = SDLNet_AllocPacket(NETWORK_PACKET_SIZE);
								
								network_packet_copy(packet_state_in[i], packet_temp);
							}
						}
						break;
						
					case PACKET_GAME_QUIT:
					case PACKET_DETAILS:
					case PACKET_WAITING:
					case PACKET_BUSY:
						network_acknowledge(SDLNet_Read16(&packet_temp->data[2]));
						
					case PACKET_CONNECT:
						last_in_sync = SDLNet_Read16(&packet_temp->data[2]);
						last_in_tick = SDL_GetTicks();
						
						network_packet_copy(packet_in, packet_temp);
						break;
						
					case PACKET_QUIT:
						if (!quit)
						{
							network_prepare(PACKET_QUIT);
							network_send(4);  // PACKET_QUIT
						}
						
						network_acknowledge(SDLNet_Read16(&packet_temp->data[2]));
						
						if (!quit)
							network_tyrian_halt(1, true);
						break;
						
					default:
						printf("warning: bad packet %d received\n", SDLNet_Read16(&packet_temp->data[0]));
						return 0;
						break;
				}
				
				return 1;
			}
			break;
	}
	
	// timeout
	if (!network_is_alive())
	{
		if (!quit)
			network_tyrian_halt(2, false);
	}
	
	// retry
	if (!network_is_sync() && SDL_GetTicks() - last_out_tick > NETWORK_RETRY)
	{
		network_send(packet_out->len);
	}
	
	return 0;
}

int network_acknowledge( Uint16 sync )
{
	SDLNet_Write16(PACKET_ACKNOWLEDGE, &packet_ack->data[0]);
	SDLNet_Write16(sync,               &packet_ack->data[2]);
	packet_ack->len = 4;
	
	if (!SDLNet_UDP_Send(socket, 0, packet_ack))
	{
		printf("SDLNet_UDP_Send: %s\n", SDL_GetError());
		return -1;
	}
	
	return 0;
}

void network_prepare( Uint16 type )
{
	if (!network_is_sync())
		printf("warning: network is not syncronized (previous packet remains unsent)\n");
	
	SDLNet_Write16(type, &packet_out->data[0]);
	SDLNet_Write16(++last_out_sync, &packet_out->data[2]);
}

int network_send( int len )
{
	packet_out->len = len;
	
	if (!SDLNet_UDP_Send(socket, 0, packet_out))
	{
		printf("SDLNet_UDP_Send: %s\n", SDL_GetError());
		return -1;
	}
	
	last_out_tick = SDL_GetTicks();
	
	return 0;
}

bool network_is_sync( void )
{
	return (last_out_sync == last_ack_sync);
}

bool network_is_alive( void )
{
	return (SDL_GetTicks() - last_in_tick < NETWORK_TIME_OUT || SDL_GetTicks() - last_ack_tick < NETWORK_TIME_OUT || SDL_GetTicks() - last_state_in_tick < NETWORK_TIME_OUT);
}

void network_reset_keep_alive( void )
{
	last_out_tick = 0;
}

bool network_keep_alive( void )
{
	return (SDL_GetTicks() - last_out_tick > NETWORK_KEEP_ALIVE);
}


void network_state_prepare( void )
{
	if (packet_state_out[0])
	{
		printf("warning: state packet overwritten (previous packet remains unsent)\n");
	} else {
		packet_state_out[0] = SDLNet_AllocPacket(NETWORK_PACKET_SIZE);
		packet_state_out[0]->len = 28;
	}
	
	SDLNet_Write16(PACKET_GAME_STATE, &packet_state_out[0]->data[0]);
	SDLNet_Write16(last_state_out_sync + 1, &packet_state_out[0]->data[2]);
	memset(&packet_state_out[0]->data[4], 0, 28 - 4);
}

int network_state_send( void )
{
	if (!SDLNet_UDP_Send(socket, 0, packet_state_out[0]))
	{
		printf("SDLNet_UDP_Send: %s\n", SDL_GetError());
		return -1;
	}
	
	if (packet_state_out[NETWORK_PACKET_QUEUE - 1])
	{
		SDLNet_FreePacket(packet_state_out[NETWORK_PACKET_QUEUE - 1]);
	}
	for (int i = NETWORK_PACKET_QUEUE - 1; i > 0; i--)
	{
		packet_state_out[i] = packet_state_out[i - 1];
	}
	packet_state_out[0] = NULL;
	
	last_state_out_sync++;
	last_state_out_tick = SDL_GetTicks();
	
	return 0;
}

bool network_state_update( void )
{
	if (network_state_is_reset())
	{
		return 0;
	} else {
		last_state_in_sync++;
		
		if (packet_state_in[0])
		{
			SDLNet_FreePacket(packet_state_out[0]);
			packet_state_in[0] = NULL;
		}
		
		while (!packet_state_in[0])
		{
			int i;
			for (i = 1; i < NETWORK_PACKET_QUEUE; i++)
			{
				if (packet_state_in[i] && SDLNet_Read16(&packet_state_in[i]->data[2]) == last_state_in_sync)
				{
					packet_state_in[0] = packet_state_in[i];
					packet_state_in[i] = NULL;
					break;
				}
			}
			if (i == NETWORK_PACKET_QUEUE)
			{
				if (network_is_sync() && SDL_GetTicks() - last_state_in_tick > NETWORK_RESEND)
				{
					network_prepare(PACKET_GAME_RESEND);
					SDLNet_Write16(last_state_in_sync, &packet_out->data[4]);
					network_send(6);  // PACKET_RESEND
				}
				
				if (network_check() == 0)
					SDL_Delay(1);
			}
		}
		
		last_state_in_tick = SDL_GetTicks();
	}
	
	return 1;
}

bool network_state_is_latest( void )
{
	return (last_state_in_tick > last_in_tick);
}

bool network_state_is_reset( void )
{
	return (last_state_out_sync < network_delay);
}

void network_state_reset( void )
{
	last_state_in_sync = 0;
	last_state_out_sync = 0;
	
	for (int i = 0; i < NETWORK_PACKET_QUEUE; i++)
	{
		if (packet_state_in[i])
		{
			SDLNet_FreePacket(packet_state_in[i]);
			packet_state_in[i] = NULL;
		}
	}
	for (int i = 0; i < NETWORK_PACKET_QUEUE; i++)
	{
		if (packet_state_out[i])
		{
			SDLNet_FreePacket(packet_state_out[i]);
			packet_state_out[i] = NULL;
		}
	}
}


int network_connect( void ) {
	SDLNet_ResolveHost(&ip, network_opponent_host, network_opponent_port);
	
	SDLNet_UDP_Bind(socket, 0, &ip);
	
	Uint16 episodes = 0;
	assert(EPISODE_MAX <= 16);
	for (int i = EPISODE_MAX - 1; i >= 0; i--) {
		episodes <<= 1;
		episodes |= (episodeAvail[i] != 0);
	}
	
	assert(NETWORK_PACKET_SIZE - 12 >= 20 + 1);
	if (strlen(network_player_name) > 20)
		network_player_name[20] = '\0';
	
	network_prepare(PACKET_CONNECT);
	
	SDLNet_Write16(NETWORK_VERSION, &packet_out->data[4]);
	SDLNet_Write16(network_delay,  &packet_out->data[6]);
	SDLNet_Write16(episodes,       &packet_out->data[8]);
	SDLNet_Write16(thisPlayerNum,  &packet_out->data[10]);
	strcpy((char *)&packet_out->data[12], network_player_name);
	
	network_send(12 + strlen(network_player_name) + 1); // PACKET_CONNECT
	
	// until opponent sends connect packet
	while (SDLNet_Read16(&packet_in->data[0]) != PACKET_CONNECT)
	{
		service_SDL_events(false);
		JE_joystickTranslate();
		
		last_in_tick = SDL_GetTicks();
		
		network_check();
		SDL_Delay(16);
		
		if (newkey && lastkey_sym == SDLK_ESCAPE)
			network_tyrian_halt(0, false);
	}
	
	network_acknowledge(SDLNet_Read16(&packet_temp->data[2]));
	
	if (SDLNet_Read16(&packet_in->data[4]) != NETWORK_VERSION)
	{
		printf("error: network version did not match opponent's\n");
		network_tyrian_halt(4, true);
	}
	if (SDLNet_Read16(&packet_in->data[6]) != network_delay)
	{
		printf("error: network delay did not match opponent's\n");
		network_tyrian_halt(5, true);
	}
	if (SDLNet_Read16(&packet_in->data[10]) == thisPlayerNum)
	{
		printf("error: player number conflicts with opponent's\n");
		network_tyrian_halt(6, true);
	}
	
	episodes = SDLNet_Read16(&packet_in->data[8]);
	for (int i = 0; i < EPISODE_MAX; i++) {
		episodeAvail[i] &= (episodes & 1);
		episodes >>= 1;
	}
	
	network_opponent_name = malloc(packet_in->len - 12 + 1);
	strcpy(network_opponent_name, (char *)&packet_in->data[12]);
	
	// until opponent has acknowledged
	while (!network_is_sync())
	{
		service_SDL_events(false);
		
		network_check();
		SDL_Delay(16);
	}
	
	return 0;
}

void network_tyrian_halt( int err, bool attempt_sync )
{
	const char *err_msg[] = {
		"Quitting...",
		"Other player quit the game.",
		"Network connection was lost.",
		"Network connection failed.",
		"Network version mismatch.",
		"Network delay mismatch.",
		"Network player number conflict.",
	};
	
	quit = true;
	
	if (err >= COUNTOF(err_msg))
		err = 0;
	
	JE_fadeBlack(10);
	
	tempScreenSeg = VGAScreen = VGAScreenSeg;
	
	JE_loadPic(2, false);
	JE_dString(JE_fontCenter(err_msg[err], SMALL_FONT_SHAPES), 140, err_msg[err], SMALL_FONT_SHAPES);
	
	JE_showVGA();
	JE_fadeColor(10);
	
	if (attempt_sync)
	{
		while (!network_is_sync() && network_is_alive())
		{
			service_SDL_events(false);
			
			network_check();
			SDL_Delay(16);
		}
	}
	
	if (err)
	{
		while (!JE_anyButton())
			SDL_Delay(16);
	}
	
	JE_fadeBlack(10);
	
	SDLNet_Quit();
	
	JE_tyrianHalt(5);
}

int network_init( void )
{
	printf("Initializing network...\n");
	
	if (network_delay * 2 > NETWORK_PACKET_QUEUE - 2)
	{
		printf("error: network delay would overflow packet queue\n");
		return -4;
	}
	
	if (SDLNet_Init() == -1)
	{
		printf("SDLNet_Init: %s\n", SDLNet_GetError());
		return -1;
	}
	
	socket = SDLNet_UDP_Open(network_player_port);
	if (!socket)
	{
		printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		return -2;
	}
	
	packet_in = SDLNet_AllocPacket(NETWORK_PACKET_SIZE);
	packet_out = SDLNet_AllocPacket(NETWORK_PACKET_SIZE);
	packet_temp = SDLNet_AllocPacket(NETWORK_PACKET_SIZE);
	packet_ack = SDLNet_AllocPacket(NETWORK_PACKET_SIZE);
	
	if (!packet_in || !packet_out || !packet_temp || !packet_ack)
	{
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		return -3;
	}
	
	memset(&packet_in->data[0], 0, 256);
	memset(&packet_out->data[0], 0, 256);
	
	return 0;
}

void network_packet_copy( UDPpacket *dst, UDPpacket *src )
{
	void *temp = dst->data;
	memcpy(dst, src, sizeof(*dst));
	dst->data = temp;
	memcpy(dst->data, src->data, src->len);
}


void JE_clearSpecialRequests( void )
{
	pauseRequest = false;
	inGameMenuRequest = false;
	skipLevelRequest = false;
	helpRequest = false;
	nortShipRequest = false;
}

