/*
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

#include "varz.h"
#include "fonthand.h"
#include "vga256d.h"
#include "helptext.h"
#include "picload.h"
#include "starfade.h"
#include "newshape.h"
#include "nortsong.h"
#include "keyboard.h"
#include "config.h"

#define NO_EXTERNS
#include "mainint.h"
#undef NO_EXTERNS

#define MAXPAGE 8
#define TOPICS 6
const JE_byte topicStart[TOPICS] = { 0, 1, 2, 3, 7, 255 };

JE_shortint constantLastX;
JE_word textErase;
JE_word upgradeCost;
JE_word downgradeCost;
JE_boolean performSave;
JE_boolean jumpSection;
JE_boolean useLastBank; /* See if I want to use the last 16 colors for DisplayText */

extern int haltGame, netQuit; /* placeholders */

void JE_helpSystem( JE_byte startTopic )
{
    JE_integer page, lastPage;
    JE_byte menu;
    JE_char flash;

    page = topicStart[startTopic];
    k = '\0';
    
    JE_FadeBlack(10);
    JE_LoadPIC(2, FALSE);
    /* playsong(Song_MapView); */
    JE_ShowVGA();
    JE_FadeColor(10);

    memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));

    /* joystickwaitmax = 120; joystickwait = 0; */

    do
    {
        memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));

        temp2 = 0;

        for (temp = 0; temp < TOPICS; temp++)
        {
            if (topicStart[temp] < page)
            {
                temp2 = page;
            }
        }

        if (page > 0)
        {
            JE_char buf[128];

            snprintf(buf, sizeof(buf), "%s %d", misctext[24], page-topicStart[temp2]+1);
            JE_Outtext(10, 192, buf, 13, 5);

            snprintf(buf, sizeof(buf), "%s %d of %d", misctext[25], page, MAXPAGE);
            JE_Outtext(220, 192, buf, 13, 5);

            JE_Dstring(JE_FontCenter(topicname[temp2], SmallFontShapes), 1, topicname[temp2], SmallFontShapes);
        }

        menu = 0;

        helpboxbrightness = 3;
        verticalheight = 8;

        switch (page)
        {
            case 0:
                menu = 2;
                if (lastPage == MAXPAGE)
                {
                    menu = TOPICS;
                    /* joystickwaitmax = 120; joystickwait = 0; */
                    JE_Dstring(JE_FontCenter(topicname[0], FontShapes), 30, topicname[0], FontShapes);

                    do
                    {
                        for (temp = 1; temp < TOPICS; temp++)
                        {
                            if (temp == menu)
                            {
                                flash = '~';
                            } else {
                                flash = '\0';
                            }
                        }

                        setdelay(1);
                        JE_ShowVGA();
                        waitdelay();

                        tempw = 0;
                        /* textmenuwait(&tempw, FALSE); */
                        
                        /* TODO */
                } while(1 /*TODO*/);
            }
        }
    } while (1);
}

JE_boolean JE_playerSelect( void )
{
    JE_byte maxSel;
    JE_byte sel;
    JE_boolean quit;

    JE_LoadPIC(2, FALSE);
    memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
    JE_ShowVGA;
    JE_FadeColor(20);
    quit = FALSE;

    sel = 1;
    maxSel = 4;

    do {

        JE_Dstring(JE_FontCenter(playername[0], FontShapes), 20, playername[0], FontShapes);

        for(temp = 1; temp <= maxSel; temp++)
            JE_OuttextAdjust(JE_FontCenter(playername[temp], SmallFontShapes), temp * 24 + 30, playername[temp], 15, - 4 + ((sel == temp) << 1), SmallFontShapes, TRUE);

        /*BETA TEST VERSION*/
        /*  Dstring(fontcenter(misctext[35],_FontShapes),170,misctext[35],_FontShapes);*/

        JE_ShowVGA();
        tempw = 0;
        JE_textMenuWait(&tempw, FALSE);

        switch(lastkey_sym)
        {
            case SDLK_UP:
                sel--;
                if(sel < 1)
                    sel = maxSel;
                /*playsamplenum(_Cursormove);*/
                break;
            case SDLK_DOWN:
                sel++;
                if(sel > maxSel)
                    sel = 1;
                /*playsamplenum(_Cursormove);*/
                break;
            case SDLK_RETURN:
                quit = TRUE;
                twoPlayerMode = (sel == 3);
                onePlayerAction = (sel == 2);
                /*playsamplenum(_Select);*/
                if(sel == 4)
                    netQuit = TRUE;
                break;
            case SDLK_ESCAPE:
               quit = TRUE;
               /*playsamplenum(_ESC);*/
               return(FALSE);
               break;
            default:
                break;
        }

    } while(!quit);

    return(TRUE);
}

