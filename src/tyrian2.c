/*
 * OpenTyrian Classic: A moden cross-platform port of Tyrian
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
#include "tyrian2.h"

#include "newshape.h"
#include "fonthand.h"
#include "vga256d.h"
#include "picload.h"
#include "starfade.h"
#include "helptext.h"

#include <string.h>

JE_boolean MoveTyrianLogoUp = TRUE; /* TODO TODO REMOVE!!!!!!!!!!! */
JE_integer temp, tempx, tempy;

void TitleScreen( JE_boolean animate )
{
    /* TODO TODO TODO: Major stuff TODO. */
    const int menunum = 7;
    JE_byte namego[7+2] = {0}; /* [1..SA+2] */
    JE_word waitfordemo;
    JE_byte menu;
    JE_boolean redraw, fadein, first;
    JE_char flash;
    JE_word z;

    tempscreenseg = VGAScreen;

    JE_LoadPIC(4, TRUE);

    first = TRUE;
    redraw = TRUE;
    fadein = FALSE;

    /* If IsNetworkActive { TODO } else { */

    DefaultBrightness = -3;

    /* Animate instead of quickly fading in */
    if (redraw)
    {
        /* if currentsong<>Song_Title then playsong(Song_Title); */
        menu = 1;
        redraw = FALSE;
        if (animate)
        {
            if (fadein)
            {
                JE_FadeBlack(10);
            }
            JE_LoadPIC(4,FALSE);

            memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));

            if (MoveTyrianLogoUp)
            {
                temp = 62;
            } else {
                temp = 4;
            }

            JE_NewDrawCShapeNum(PlanetShapes,146,11,temp);


            memcpy(colors2, colors, sizeof(colors));
            for (temp = 256-16; temp < 256; temp++)
            {
                colors[temp].r = 0;
                colors[temp].g = 0;
                colors[temp].b = 0;
            }
            colors2[temp].r = 0;

            JE_ShowVGA();
            fadein = FALSE;

            if (MoveTyrianLogoUp)
            {
                for (temp = 61; temp >= 4; temp -= 2)
                {
                    int i;
                    /*framecount = 2;*/
                    memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));

                    JE_NewDrawCShapeNum(PlanetShapes,146,11,temp);

                    SDL_Delay(16*2);
                    JE_ShowVGA();
                    /*waitframecount;*/
                }
            }
            MoveTyrianLogoUp = FALSE;

            /* Draw Menu Text on Screen */
            for (temp = 0; temp < menunum; temp++)
            {
                tempx = 104+(temp)*13;
                /*tempy = JE_FontCenter(menutext[temp],SmallFontShapes);*/
                tempy = JE_FontCenter("FOOBAR",SmallFontShapes);

                /*JE_OuttextAdjust(tempy-1,tempx-1,menutext[temp],15,-10,SmallFontShapes,FALSE);
                JE_OuttextAdjust(tempy+1,tempx+1,menutext[temp],15,-10,SmallFontShapes,FALSE);
                JE_OuttextAdjust(tempy+1,tempx-1,menutext[temp],15,-10,SmallFontShapes,FALSE);
                JE_OuttextAdjust(tempy-1,tempx+1,menutext[temp],15,-10,SmallFontShapes,FALSE);
                JE_OuttextAdjust(tempy,tempx,menutext[temp],15,-3,SmallFontShapes,FALSE);*/
                JE_OuttextAdjust(tempy-1,tempx-1,"FOOBAR",15,-10,SmallFontShapes,FALSE);
                JE_OuttextAdjust(tempy+1,tempx+1,"FOOBAR",15,-10,SmallFontShapes,FALSE);
                JE_OuttextAdjust(tempy+1,tempx-1,"FOOBAR",15,-10,SmallFontShapes,FALSE);
                JE_OuttextAdjust(tempy-1,tempx+1,"FOOBAR",15,-10,SmallFontShapes,FALSE);
                JE_OuttextAdjust(tempy,tempx,"FOOBAR",15,-3,SmallFontShapes,FALSE);
            }
            JE_ShowVGA();

            JE_FadeColors(&colors, &colors2, 0, 255, 20);

            memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
        }
    }
}
