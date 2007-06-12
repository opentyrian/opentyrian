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

#include "fonthand.h"
#include "jukebox.h"
#include "keyboard.h"
#include "newshape.h"
#include "nortsong.h"
#include "nortvars.h"
#include "picload.h"
#include "varz.h"
#include "vga256d.h"

#include "scroller.h"

#include "SDL.h"


#define min(a,b) ((a) < (b) ? (a) : (b))

const char *about_text[] =
{
"","","","","","", /* Filler */
"","","","","","",
"------------------",
"--- OpenTyrian ---",
"------------------",
"",
"Hello boys and girls!",
"This is a test text for", "OpenTyrian's scroller. by yuriks",
"Which means: TODO, probably", "copy it off the readme! :D",
"",
"Thanks for RealmRPGer for", "helping with the math. :D",
"",
"No automatic line breaks yet, so we'll", "have to see how this will work later.",
"",
"",
"Bye!",
NULL
};

#define BEER_SHAPE 241

/* Text is an array of strings terminated by a NULL */
void scroller3d( const char *text[] )
{
#	define TEXT_HEIGHT 15
#	define HORIZON 30
#	define HORIZON_W 200
#	define MAX_LINES ((200-HORIZON)/TEXT_HEIGHT)
	int txt_y = 0, cur_line = 0, line_off = 0, text_len = 0, wait = 0;
	JE_boolean quit = FALSE;
	Uint8 *surf;

	while (text[text_len])
	{
		text_len++;
	}

	JE_loadPic(4, FALSE); /* Steal palette for now. */
	memset(VGAScreen->pixels, 0, sizeof(VGAScreen2Seg));

	surf = malloc(sizeof(VGAScreen2Seg));

	wait_noinput(TRUE,TRUE,TRUE);
	currentJukeboxSong = 41; /* BEER! =D */
	JE_playSong(currentJukeboxSong);

	do
	{
		int start_line = cur_line, i, max = min(text_len, cur_line+MAX_LINES+1);

		service_SDL_events(TRUE);
		if (keydown)
		{
			quit = TRUE;
		}

		if (wait++ >= 60)
		{
			wait = 0;

			line_off++;
			if (line_off == 15)
			{
				line_off = 0;
				cur_line++;
			}

			if (cur_line == text_len)
			{
				cur_line = 0;
			}
		} else {
			JE_showVGA();
			continue;
		}

		memset(VGAScreen->pixels, 0, sizeof(VGAScreen2Seg));

		for (i = cur_line; i < max; i++)
		{
			int txt_x = JE_fontCenter(text[i], SMALL_FONT_SHAPES);

			JE_outTextAdjust(txt_x, (i-cur_line)*TEXT_HEIGHT-line_off+TEXT_HEIGHT, text[i], 15, -3, SMALL_FONT_SHAPES, FALSE);
		}

		memset(surf, 0, sizeof(VGAScreen2Seg));
		memcpy(surf+(HORIZON*320), ((Uint8 *)VGAScreen->pixels)+(TEXT_HEIGHT*320), (200-HORIZON)*320);
		memset(VGAScreen->pixels, 0, sizeof(VGAScreen2Seg));

		for (i = HORIZON; i < 200; i++)
		{
			int total_w = HORIZON_W+((i-HORIZON)/(199.-HORIZON))*(320-HORIZON_W);
			double step = 320./(double)total_w;
			int scr_x = 320/2-total_w/2;
			Uint8 *s = VGAScreen->pixels;
			double cur_x = 0.;
			double ratio = (i-HORIZON)/(total_w/320.);

			while (total_w--)
			{
				int j;
				double color = surf[(int)(ratio)*320+(int)cur_x];

/*				for (j = cur_x; j < cur_x+step; j++)
				{
					color = (color + surf[i*320+(int)round(j)]) / 2;
				}*/

				s[i*320+scr_x] = color;
				scr_x++; cur_x += step;
			}
		}

		JE_drawShape2x2(0,0, BEER_SHAPE, eShapes5);

		JE_showVGA();
	} while (!(quit));
}
