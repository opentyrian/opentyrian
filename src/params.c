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
#include "params.h"

#include "joystick.h"
#include "loudness.h"
#include "network.h"
#include "varz.h"
#include "xmas.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>

JE_boolean richMode, robertWeird, constantPlay, constantDie, joyMax, forceAveraging, stupidWindows;

/* YKS: Note: LOOT cheat had non letters removed. */
const char pars[][9] = {
	"LOOT", "RECORD", "NOJOY", "NOROBERT", "CONSTANT", "DEATH", "NOSOUND", "JOYMAX", "WEAKJOY", "NOXMAS", "YESXMAS"
};

void JE_paramCheck( int argc, char *argv[] )
{
	char *tempStr;
	JE_word y;

	robertWeird     = true;
	richMode        = false;
	constantPlay    = false;
	forceAveraging  = false;

	const struct
	{
		char short_opt;
		char *long_opt;
	}
	options[] =
	{
		{ 'h', "help" },
		
		{ 's', "no-sound" },
		{ 'j', "no-joystick" },
		{ 'x', "no-xmas" },
		
		{ 'n', "net" },
		{ 'p', "net-port" },
		{ 'd', "net-delay" },
		
		{ 'X', "xmas" },
		{ 'c', "constant" },
		{ 'k', "death" },
		{ 'r', "record" },
		{ 'l', "loot" },
	};
	
	for (int i = 1; i < argc; i++)
	{
		char match = '\0';
		
		if (argv[i][0] == '-')
		{
			if (strncmp(argv[i], "--", 2) == 0)
			{
				for (int j = 0; j < COUNTOF(options); j++)
				{
					if (strcmp(&argv[i][2], options[j].long_opt) == 0)
					{
						match = options[j].short_opt;
						break;
					}
				}
			}
			else
			{
				// could have support for multiple options following a -
				match = argv[i][1];
			}
			
			switch (match)
			{
				case 'h':
					printf("Usage: tyrian [options]\n\n"
					       "Options:\n"
					       "  --help                                   Show help about options\n\n"
					       "  --no-sound                               Disable audio\n"
					       "  --no-joystick                            Disable joystick/gamepad input\n"
					       "  --no-xmas                                Disable Christmas mode\n\n"
					       "  --net <host>[:<port>] <number> <name>    Start a networked game\n"
					       "  --net-port <port>                        Local port to bind\n"
					       "  --net-delay <frames>                     Set lag-compensation delay\n");
					exit(0);
					break;
					
				case 's':
					// Disables sound/music usage
					audio_disabled = true;
					break;
					
				case 'j':
					// Disables joystick detection
					ignore_joystick = true;
					break;
					
				case 'x':
					xmas = false;
					break;
					
				case 'n':
					if (argc > i + 3)
					{
						isNetworkGame = true;
						
						{
							intptr_t temp = (intptr_t)strchr(argv[++i], ':');
							if (temp)
							{
								temp -= (intptr_t)argv[i];
								
								int temp_port = atoi(&argv[i][temp + 1]);
								if (temp_port > 0 && temp_port < 49152)
									network_opponent_port = temp_port;
								
								network_opponent_host = malloc(temp + 1);
								strnztcpy(network_opponent_host, argv[i], temp);
							}
							else
							{
								network_opponent_host = malloc(strlen(argv[i]) + 1);
								strcpy(network_opponent_host, argv[i]);
							}
						}
						
						int temp = atoi(argv[++i]);
						if (temp >= 1 && temp <= 2)
							thisPlayerNum = temp;
						
						network_player_name = malloc(strlen(argv[++i]) + 1);
						strcpy(network_player_name, argv[i]);
					}
					else
					{
						printf("Argument missing for '%s'.\nUse --help to get a list of available command line options.\n", argv[i]);
						exit(-1);
					}
					break;
					
				case 'p':
					if (argc > i + 1)
					{
						int temp = atoi(argv[++i]);
						if (temp > 0 && temp < 49152)
							network_player_port = temp;
					}
					else
					{
						printf("Argument missing for '%s'.\nUse --help to get a list of available command line options.\n", argv[i]);
						exit(-1);
					}
					break;
					
				case 'd':
					if (argc > i + 1)
					{
						errno = 0;
						int temp = strtol(argv[++i], (char **)NULL, 10);
						if (errno == 0)
							network_delay = 1 + temp;
					}
					else
					{
						printf("Argument missing for '%s'.\nUse --help to get a list of available command line options.\n", argv[i]);
						exit(-1);
					}
					break;
				
				case 'X':
					xmas = true;
					break;
					
				case 'c':
					/* Constant play for testing purposes (C key activates invincibility)
					   This might be useful for publishers to see everything - especially
					   those who can't play it */
					constantPlay = true;
					break;
					
				case 'k':
					constantDie = true;
					break;
					
				case 'r':
					record_demo = true;
					break;
					
				case 'l':
					// Gives you mucho bucks
					richMode = true;
					break;
					
				default:
					printf("Unknown option '%s'.\nUse --help to get a list of available command line options.\n", argv[i]);
					exit(-1);
					break;
			}
		}
		else
		{
			// legacy parameter support
			
			tempStr = argv[i];
			for (y = 0; y < strlen(tempStr); y++)
			{
				tempStr[y] = toupper(tempStr[y]);
			}
			
			for (y = 0; y < COUNTOF(pars); y++)
			{
				if (strcmp(tempStr, pars[y]) == 0)
				{
					switch (y)
					{
						case 0:
							richMode = true;
							break;
						case 1:
							record_demo = true;
							break;
						case 2:
							ignore_joystick = true;
							break;
						case 3:
							robertWeird = false;
							break;
						case 4:
							constantPlay = true;
							break;
						case 5:
							constantDie = true;
							break;
						case 6:
							audio_disabled = true;
							break;
						case 7:
							joyMax = true;
							break;
						case 8:
							forceAveraging = true;
							break;
						case 9:
							xmas = false;
							break;
						case 10:
							xmas = true;
							break;
						default:
							assert(false);
							break;
					}
				}
			}
		}
	}
}

// kate: tab-width 4; vim: set noet:
