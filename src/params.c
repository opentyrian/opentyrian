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

#define NO_EXTERNS
#include "params.h"
#undef NO_EXTERNS

#include <time.h>
#include <ctype.h>
#include <string.h>

JE_boolean timMode, richMode, recordDemo, robertWeird, constantPlay, constantDie, scanForJoystick, useBios, noSound, quikJuke, noRetrace, joyMax, forceAveraging, forceMaxVolume, soundInfoReport, stupidWindows;

/* JE: Special Note:
 * The two booleans to detect network play for Tim's stuff.
 * It's in here because of the dumb port 60 bug.
 */
JE_boolean isNetworkGame, isNetworkActive;

JE_boolean tyrianXmas;

/* YKS: Note: LOOT and SOUND cheats had non letters removed. */
const char pars[18][8] = {
	"LOOT", "BORDER", "RECORD", "NOJOY", "NOROBERT", "CONSTANT", "DEATH", "NOKEY", "NOSOUND", "JUKEBOX", "MAXVOL",
	"SOUND", "FLICKER", "JOYMAX", "WEAKJOY", "NOWINCHK", "NOXMAS", "YESXMAS"
};

void JE_paramCheck( int argc, char *argv[] )
{
	char *tempStr;
	JE_word x,y;
	struct tm *broken_time;
	time_t now;

	robertWeird     = TRUE;
	richMode        = FALSE;
	timMode        = FALSE;
	recordDemo      = FALSE;
	scanForJoystick = TRUE;
	constantPlay    = FALSE;
	useBios         = FALSE;
	noSound         = FALSE;
	quikJuke        = FALSE;
	forceMaxVolume  = FALSE;
	soundInfoReport = FALSE;
	noRetrace       = FALSE;
	joyMax          = FALSE;
	forceAveraging  = FALSE;

	now = time(NULL);
	broken_time = localtime(&now);
	tyrianXmas = broken_time->tm_mon == 12;

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
						richMode = TRUE;
						break;
					case 1:
						timMode = TRUE;
						break;
					case 2:
						recordDemo = TRUE;
						printf("Use a keyboard to record a demo.\n");
						break;
					case 3:
						scanForJoystick = FALSE;
						break;
					case 4:
						robertWeird = FALSE;
						break;
					case 5:
						constantPlay = TRUE;
						break;
					case 6:
						constantDie = TRUE;
						break;
					case 7:
						useBios = TRUE;
						printf("Keyboard redirection enabled.\n");
						break;
					case 8:
						noSound = TRUE;
						break;
					case 9:
						quikJuke = TRUE;
						break;
					case 10:
						forceMaxVolume = TRUE;
						break;
					case 11:
						soundInfoReport = TRUE;
						break;
					case 12:
						noRetrace = TRUE;
						break;
					case 13:
						joyMax = TRUE;
						break;
					case 14:
						forceAveraging = TRUE;
						break;
					case 15:
						stupidWindows = FALSE;
						break;
					case 16:
						tyrianXmas = FALSE;
						break;
					case 17:
						tyrianXmas = TRUE;
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
