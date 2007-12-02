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
#include "params.h"

#include <ctype.h>
#include <string.h>
#include <time.h>


JE_boolean timMode, richMode, recordDemo, robertWeird, constantPlay, constantDie, scanForJoystick, useBios, noSound, quikJuke, noRetrace, joyMax, forceAveraging, forceMaxVolume, soundInfoReport, stupidWindows;

/* JE: Special Note:
 * The two booleans to detect network play for Tim's stuff.
 * It's in here because of the dumb port 60 bug.
 */
JE_boolean isNetworkGame, isNetworkActive;

JE_boolean tyrianXmas;

/* YKS: Note: LOOT cheat had non letters removed. */
const char pars[18][9] = {
	"LOOT", "BORDER", "RECORD", "NOJOY", "NOROBERT", "CONSTANT", "DEATH", "NOKEY", "NOSOUND", "JUKEBOX", "MAXVOL",
	"SOUND?", "FLICKER", "JOYMAX", "WEAKJOY", "NOWINCHK", "NOXMAS", "YESXMAS"
};

void JE_paramCheck( int argc, char *argv[] )
{
	char *tempStr;
	JE_word x,y;
	time_t now;

	robertWeird     = true;
	richMode        = false;
	timMode        = false;
	recordDemo      = false;
	scanForJoystick = true;
	constantPlay    = false;
	useBios         = false;
	noSound         = false;
	quikJuke        = false;
	forceMaxVolume  = false;
	soundInfoReport = false;
	noRetrace       = false;
	joyMax          = false;
	forceAveraging  = false;

	now = time(NULL);
	tyrianXmas = localtime(&now)->tm_mon == 11;

/* JE: Note:
Parameters are no longer case-sensitive.

LOOTþ    - Gives you mucho bucks.
BORDER   - Network border mode (useless)
NOROBERT - Alternate selection of items in upgrade menu.
RECORD   - Records all level and stores the last one in DEMOREC.num file
           (When a file is taken, it increments "num" until no file is found)

        Note: Mouse and Joystick are disabled when recording.

NOJOY    - Disables joystick detection
NOKEY    - Forces Tyrian to send keys to the BIOS just in case some other
           device driver needs input.  It still uses the keyboard input
           driver, though.

CONSTANT - Constant play for testing purposes (C key activates invincibility)
           This might be useful for publishers to see everything - especially
           those who can't play it.

DEATH    - Constant death mode.  Only useful for testing.

NOSOUND  - Disables Sound/Music usage even if FX or Music are in the config
           file.  It tells the initialization routine to NEVER check for
           initialization and will not load the Sound Effects, saving memory
           in SETUP.
           FM sound will still be automatically detected since I have no
           control over that.
           You can, however, select sound and music selections that would
           otherwise not work and it won't return an error since it does
           not try to detect them.  This might help if someone has a problem
           only in SETUP and not in Tyrian.

JUKEBOX  - Only works in SETUP.  Takes you directly to the jukebox.

MAXVOL   - Force Tyrian or SETUP to set the card's main volume to maximum
           if it is able to instead of reading the current value.

SOUND?   - Display IRQ/DMA junk so Andreas has more info.  Only works in SETUP.
           This will save a file called SOUND.RPT to your HD detailing your
           sound selections and any initialization errors for convenient
           uploading on CServe/AOL/Internet.

FLICKER  - This will force Tyrian and SETUP to NEVER WAIT FOR A RETRACE.
           Might be helpful for unknown reasons.  Perhaps a sound card is
           cutting out for some related reason?    [V1.2]

JOYMAX   - Sets your joystick to maximum sensitivity.   [V1.2]
*/

	for (x = 0; x < argc; x++)
	{
		tempStr = argv[x];
		for (y = 0; y < 8; y++)
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
						timMode = true;
						break;
					case 2:
						recordDemo = true;
						printf("Use a keyboard to record a demo.\n");
						break;
					case 3:
						scanForJoystick = false;
						break;
					case 4:
						robertWeird = false;
						break;
					case 5:
						constantPlay = true;
						break;
					case 6:
						constantDie = true;
						break;
					case 7:
						useBios = true;
						printf("Keyboard redirection enabled.\n");
						break;
					case 8:
						noSound = true;
						break;
					case 9:
						quikJuke = true;
						break;
					case 10:
						forceMaxVolume = true;
						break;
					case 11:
						soundInfoReport = true;
						break;
					case 12:
						noRetrace = true;
						break;
					case 13:
						joyMax = true;
						break;
					case 14:
						forceAveraging = true;
						break;
					case 15:
						stupidWindows = false;
						break;
					case 16:
						tyrianXmas = false;
						break;
					case 17:
						tyrianXmas = true;
						break;
					default:
						/* YKS: This shouldn't ever be reached. */
						printf("!!! WARNING: Something's very wrong on %s:%d!\n", __FILE__, __LINE__);
						break;
				}
			}
		}
	}
}
