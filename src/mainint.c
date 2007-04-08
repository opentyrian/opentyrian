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
#include "episodes.h"
#include "setup.h"
#include "helptext.h"
#include "sndmast.h"
#include "shpmast.h"
#include "error.h"
#include "params.h"

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
    JE_integer page, lastPage = 0;
    JE_byte menu;
    JE_char flash;

    page = topicStart[startTopic-1];
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
        service_SDL_events();

        memcpy(VGAScreen->pixels, VGAScreen2Seg, sizeof(VGAScreen2Seg));

        temp2 = 0;

        for (temp = 0; temp < TOPICS; temp++)
        {
            if (topicStart[temp] <= page)
            {
                temp2 = temp;
            }
        }

        if (page > 0)
        {
            JE_char buf[128];

            snprintf(buf, sizeof(buf), "%s %d", miscText[24], page-topicStart[temp2]+1);
            JE_outText(10, 192, buf, 13, 5);

            snprintf(buf, sizeof(buf), "%s %d of %d", miscText[25], page, MAXPAGE);
            JE_outText(220, 192, buf, 13, 5);

            JE_dString(JE_fontCenter(topicName[temp2], SmallFontShapes), 1, topicName[temp2], SmallFontShapes);
        }

        menu = 0;

        helpBoxBrightness = 3;
        verticalHeight = 8;

        switch (page)
        {
            case 0:
                menu = 0;
                if (lastPage == MAXPAGE)
                {
                    menu = TOPICS-2;
                }
                /* joystickwaitmax = 120; joystickwait = 0; */
                JE_dString(JE_fontCenter(topicName[0], FontShapes), 30, topicName[0], FontShapes);

                do
                {
                    for (temp = 1; temp <= TOPICS; temp++)
                    {
                        char buf[21+1];

                        if (temp == menu+1)
                        {
                            strcpy(buf+1, topicName[temp]);
                            buf[0] = '~';
                        } else {
                            strcpy(buf, topicName[temp]);
                        }

                        JE_dString(JE_fontCenter(topicName[temp], SmallFontShapes), temp * 20 + 40, buf, SmallFontShapes);
                    }

                    JE_ShowVGA();

                    tempw = 0;
                    JE_textMenuWait(&tempw, FALSE);
                    switch (lastkey_sym)
                    {
                        case SDLK_UP:
                            if (menu == 0)
                            {
                                menu = TOPICS-2; /* -2 since TOPICS apparently also include the title >_> */
                            } else {
                                menu--;
                            }
                            JE_playSampleNum(CursorMove);
                            break;
                        case SDLK_DOWN:
                            if (menu == TOPICS-2)
                            {
                                menu = 0;
                            } else {
                                menu++;
                            }
                            JE_playSampleNum(CursorMove);
                            break;
                        default: break;
                    }
                } while (lastkey_sym != SDLK_ESCAPE && lastkey_sym != SDLK_RETURN);

                if (lastkey_sym == SDLK_RETURN)
                {
                    page = topicStart[menu+1];
                    JE_playSampleNum(Click);
                }

                /* joystickwaitmax = 120; joystickwait = 80; */
                break;
            case 1: /* One-Player Menu */
                JE_HBox(10,  20,  2, 60);
                JE_HBox(10,  50,  5, 60);
                JE_HBox(10,  80, 21, 60);
                JE_HBox(10, 110,  1, 60);
                JE_HBox(10, 140, 28, 60);
                break;
            case 2: /* Two-Player Menu */
                JE_HBox(10,  20,  1, 60);
                JE_HBox(10,  60,  2, 60);
                JE_HBox(10, 100, 21, 60);
                JE_HBox(10, 140, 28, 60);
                break;
            case 3: /* Upgrade Ship */
                JE_HBox(10,  20,  5, 60);
                JE_HBox(10,  70,  6, 60);
                JE_HBox(10, 110,  7, 60);
                break;
            case 4:
                JE_HBox(10,  20,  8, 60);
                JE_HBox(10,  55,  9, 60);
                JE_HBox(10,  87, 10, 60);
                JE_HBox(10, 120, 11, 60);
                JE_HBox(10, 170, 13, 60);
                break;
            case 5:
                JE_HBox(10,  20, 14, 60);
                JE_HBox(10,  80, 15, 60);
                JE_HBox(10, 120, 16, 60);
                break;
            case 6:
                JE_HBox(10,  20, 17, 60);
                JE_HBox(10,  40, 18, 60);
                JE_HBox(10, 130, 20, 60);
                break;
            case 7: /* Options */
                JE_HBox(10,  20, 21, 60);
                JE_HBox(10,  70, 22, 60);
                JE_HBox(10, 110, 23, 60);
                JE_HBox(10, 140, 24, 60);
                break;
            case 8:
                JE_HBox(10,  20, 25, 60);
                JE_HBox(10,  60, 26, 60);
                JE_HBox(10, 100, 27, 60);
                JE_HBox(10, 140, 28, 60);
                JE_HBox(10, 170, 29, 60);
                break;
        }

        helpBoxBrightness = 1;
        verticalHeight = 7;

        lastPage = page;

        if (menu == 0)
        {
            JE_ShowVGA();
            wait_nomouse();
            JE_waitAction(1, TRUE);

            if (newmouse /*TODO: not sure if this is correct*/)
            {
                switch (lastmouse_but)
                {
                    case SDL_BUTTON_LEFT:
                        lastkey_sym = SDLK_RIGHT;
                        break;
                    case SDL_BUTTON_RIGHT:
                        lastkey_sym = SDLK_LEFT;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        lastkey_sym = SDLK_ESCAPE;
                        break;
                }
                wait_nomouse();
                newkey = TRUE;
            }

            if (newkey)
            {
                switch (lastkey_sym)
                {
                    case SDLK_LEFT:
                    case SDLK_UP:
                    case SDLK_PAGEUP:
                        page--;
                        JE_playSampleNum(CursorMove);
                        break;
                    case SDLK_RIGHT:
                    case SDLK_DOWN:
                    case SDLK_PAGEDOWN:
                    case SDLK_RETURN:
                    case SDLK_SPACE:
                        if (page == MAXPAGE)
                        {
                            page = 0;
                        } else {
                            page++;
                        }
                        JE_playSampleNum(CursorMove);
                        break;
                    case SDLK_F1:
                        page = 0;
                        JE_playSampleNum(CursorMove);
                        break;
                    default:
                        break;
                }
            }
        }

        if (page == 255)
        {
            lastkey_sym = SDLK_ESCAPE;
        }
    } while (lastkey_sym != SDLK_ESCAPE);
}

JE_boolean JE_playerSelect( void )
{
    JE_byte maxSel;
    JE_byte sel;
    JE_boolean quit;

    JE_LoadPIC(2, FALSE);
    memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
    JE_ShowVGA();
    JE_FadeColor(20);
    quit = FALSE;

    sel = 1;
    maxSel = 4;

    do {

        JE_dString(JE_fontCenter(playerName[0], FontShapes), 20, playerName[0], FontShapes);

        for(temp = 1; temp <= maxSel; temp++)
            JE_outTextAdjust(JE_fontCenter(playerName[temp], SmallFontShapes), temp * 24 + 30, playerName[temp], 15, - 4 + ((sel == temp) << 1), SmallFontShapes, TRUE);

        /*BETA TEST VERSION*/
        /*  JE_Dstring(JE_FontCenter(misctext[34], FontShapes), 170, misctext[34], FontShapes);*/

        JE_ShowVGA();
        tempw = 0;
        JE_textMenuWait(&tempw, FALSE);

        switch(lastkey_sym)
        {
            case SDLK_UP:
                sel--;
                if(sel < 1)
                    sel = maxSel;
                JE_playSampleNum(CursorMove);
                break;
            case SDLK_DOWN:
                sel++;
                if(sel > maxSel)
                    sel = 1;
                JE_playSampleNum(CursorMove);
                break;
            case SDLK_RETURN:
                quit = TRUE;
                twoPlayerMode = (sel == 3);
                onePlayerAction = (sel == 2);
                JE_playSampleNum(Select);
                if(sel == 4)
                    netQuit = TRUE;
                break;
            case SDLK_ESCAPE:
               quit = TRUE;
               JE_playSampleNum(ESC);
               return(FALSE);
               break;
            default:
                break;
        }

    } while(!quit);

    return(TRUE); /*MXD assumes this default return value here*/
}

JE_boolean JE_episodeSelect( void )
{
    JE_byte sel;
    JE_boolean quit;
    JE_byte max;

    max = EpisodeMax;

    if(!(episodeAvail[0] && episodeAvail[1] && episodeAvail[2]))
        episodeAvail[3] = FALSE;

    if(episodeAvail[4] == FALSE)
        max = 4;

    /*if(!episodeavail[3]) max = 3;*/

    startepisodeselect:
    JE_LoadPIC(2, FALSE);
    memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
    JE_ShowVGA();
    JE_FadeColor(10);
    quit = FALSE;

    sel = 1;

    do {

        JE_dString(JE_fontCenter(episodeName[0], FontShapes), 20, episodeName[0], FontShapes);

        for(temp = 1; temp <= max; temp++)
            JE_outTextAdjust(20, temp * 30 + 20, episodeName[temp], 15, - 4 - (!episodeAvail[temp-1] << 2) + ((sel == temp) << 1), SmallFontShapes, TRUE);

        /*JE_Dstring(JE_fontCenter(misctext[34], FontShapes), 170, misctext[34], FontShapes);*/

        JE_ShowVGA();
        tempw = 0;
        JE_textMenuWait(&tempw, FALSE);

        switch(lastkey_sym)
        {
            case SDLK_UP:
                sel--;
                if(sel < 1)
                    sel = max;
                JE_playSampleNum(CursorMove);
                break;
            case SDLK_DOWN:
                sel++;
                if(sel > max)
                    sel = 1;
                JE_playSampleNum(CursorMove);
                break;
            case SDLK_RETURN:
                if(episodeAvail[sel-1])
                {
                    JE_playSampleNum(Select);

                    quit = TRUE;
                    JE_initEpisode(sel);
                    return(TRUE);
                } else {
                    if(sel > 1)
                    {
                        JE_playSampleNum(ESC);
                        JE_FadeBlack (10);
                        /*TODO: loadpcx ('EPISODE' + st (sel) + '.PCX', FALSE);*/
                        verticalHeight = 9;
                        helpBoxColor = 15;
                        helpBoxBrightness = 4;
                        helpBoxShadeType = FullShade;
                        JE_helpBox(10, 10, helpTxt[29], 50);
                        JE_ShowVGA();
                        JE_FadeColor(10);
                        while(!JE_anyButton())
                            ;
                        lastkey_sym = 0;
                        JE_FadeBlack(10);
                        goto startepisodeselect;
                    }
                }
                break;
            case SDLK_ESCAPE:
                quit = TRUE;
                JE_playSampleNum(ESC);
                return(FALSE);
                break;
            default:
                break;
        }

    } while(!(quit || haltGame /*|| netQuit*/));
    pItems[8] = episodeNum;

    return(FALSE); /*MXD assumes this default return value here*/
}

JE_boolean JE_difficultySelect( void )
{
    JE_byte maxSel;
    JE_byte sel;
    JE_boolean quit;

    JE_LoadPIC(2, FALSE);
    memcpy(VGAScreen2Seg, VGAScreen->pixels, sizeof(VGAScreen2Seg));
    JE_ShowVGA();
    JE_FadeColor(20);
    quit = FALSE;

    sel = 2;
    maxSel = 3;

    do {

        JE_dString(JE_fontCenter(difficultyName[0], FontShapes), 20, difficultyName[0], FontShapes);

        for(temp = 1; temp <= maxSel; temp++)
            JE_outTextAdjust(JE_fontCenter(difficultyName[temp], SmallFontShapes), temp * 24 + 30, difficultyName[temp], 15, - 4 + ((sel == temp) << 1), SmallFontShapes, TRUE);

        /*BETA TEST VERSION*/
        /*  JE_Dstring(JE_FontCenter(misctext[34], FontShapes), 170, misctext[34], FontShapes);*/

        JE_ShowVGA();
        tempw = 0;
        JE_textMenuWait(&tempw, FALSE);

        if(keysactive[SDLK_l] && keysactive[SDLK_o] && keysactive[SDLK_r] && keysactive[SDLK_d])
        {
            if(maxSel == 5)
                maxSel = 6;
        }

        switch(lastkey_sym)
        {
            case SDLK_UP:
                sel--;
                if(sel < 1)
                    sel = maxSel;
                JE_playSampleNum(CursorMove);
                break;
            case SDLK_DOWN:
                sel++;
                if(sel > maxSel)
                    sel = 1;
                JE_playSampleNum(CursorMove);
                break;
            case SDLK_RETURN:
                quit = TRUE;

                if(sel == 5)
                    sel++;
                if(sel == 6)
                    sel = 8;
                difficultyLevel = sel;
                JE_playSampleNum(Select);
                break;
            case SDLK_ESCAPE:
                quit = TRUE;
                JE_playSampleNum(ESC);
                return(FALSE);
                break;
            case SDLK_g:
                if(SDL_GetModState() & KMOD_SHIFT)
                    if(maxSel < 4)
                        maxSel = 4;
                break;
            case SDLK_RIGHTBRACKET:
                if(SDL_GetModState() & KMOD_SHIFT)
                    if(maxSel == 4)
                        maxSel = 5;
                break;
            default:
                break;
        }

    } while(!(quit || haltGame /*|| netQuit*/));

    return(TRUE); /*MXD assumes this default return value here*/
}

void JE_loadCompShapesB( JE_byte **shapes, FILE *f, JE_word shapeSize )
{
    *shapes = malloc(shapeSize);
    fread(*shapes, 1, shapeSize, f);
}

void JE_loadMainShapeTables( void )
{
    const JE_byte shapeReorderList[7] /* [1..7] */ = {1, 2, 5, 0, 3, 4, 6};

    FILE *f;

    typedef JE_longint JE_ShpPosType[SHPnum + 1]; /* [1..shpnum + 1] */

    JE_ShpPosType shpPos;
    JE_word shpNumb;

    if(tyrianXmas)
        JE_resetFileExt(&f, "TYRIANC.SHP", FALSE);
    else
        JE_resetFileExt(&f, "TYRIAN.SHP", FALSE);

    fread(&shpNumb, 2, 1, f);
    for(x = 0; x < shpNumb; x++)
        fread(&shpPos[x], sizeof(shpPos[x]), 1, f);
    fseek(f, 0, SEEK_END);
    shpPos[shpNumb] = ftell(f);

    /*fclose(f);*/

    for(temp = 0; temp < 7; temp++)
    { /*Load EST shapes*/
        fseek(f, shpPos[temp], SEEK_SET);
        JE_newLoadShapesB(shapeReorderList[temp], f);
    }

    shapesC1Size = shpPos[temp + 1] - shpPos[temp];
    JE_loadCompShapesB(&shapesC1, f, shapesC1Size);
    temp++;

    shapes9Size = shpPos[temp + 1] - shpPos[temp];
    JE_loadCompShapesB(&shapes9 , f, shapes9Size);
    temp++;

    eShapes6Size = shpPos[temp + 1] - shpPos[temp];
    JE_loadCompShapesB(&eShapes6, f, eShapes6Size);
    temp++;

    eShapes5Size = shpPos[temp + 1] - shpPos[temp];
    JE_loadCompShapesB(&eShapes5, f, eShapes5Size);
    temp++;

    shapesW2Size = shpPos[temp + 1] - shpPos[temp];
    JE_loadCompShapesB(&shapesW2, f, shapesW2Size);

    fclose(f);
}
