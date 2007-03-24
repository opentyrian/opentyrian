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
#define NO_EXTERNS
#include "helptext.h"
#undef NO_EXTERNS

#include "fonthand.h"
#include "newshape.h"
#include "error.h"
#include "config.h"
#include "episodes.h"

#include <string.h>

const JE_byte menuhelp[maxmenu][11] =     /* [1..maxmenu, 1..11] */
{
    {1, 34, 2, 3, 4, 5,                       0, 0, 0, 0, 0},
    {6, 7, 8, 9, 10, 11, 11, 12,                    0, 0, 0},
    {13, 14, 15, 15, 16, 17, 12,                 0, 0, 0, 0},
    {                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {4, 30, 30, 3, 5,                      0, 0, 0, 0, 0, 0},
    {                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {16, 17, 15, 15, 12,                   0, 0, 0, 0, 0, 0},
    {31, 31, 31, 31, 32, 12,                  0, 0, 0, 0, 0},
    {4, 34, 3, 5,                       0, 0, 0, 0, 0, 0, 0}
};

JE_byte verticalheight = 7;
JE_byte helpboxcolor = 12;
JE_byte helpboxbrightness = 1;
JE_byte helpboxshadetype = FullShade;

char helptxt[maxhelpmessage][231];
char pname[21][16];          /* [1..21] of string [15] */
char misctext[68][42];       /* [1..68] of string [41] */
char misctextB[5][11];       /* [1..5] of string [10] */
char keyname[8][18];         /* [1..8] of string [17] */
char menutext[7][21];        /* [1..7] of string [20] */
char outputs[9][31];         /* [1..9] of string [30] */
char topicname[6][21];       /* [1..6] of string [20] */
char mainmenuhelp[34][66];
char ingametext[6][21];      /* [1..6] of string [20] */
char detaillevel[6][13];     /* [1..6] of string [12] */
char gamespeedtext[5][13];   /* [1..5] of string [12] */
char episodename[6][31];     /* [0..5] of string [30] */
char difficultyname[7][21];  /* [0..6] of string [20] */
char playername[5][26];      /* [0..4] of string [25] */
char inputdevices[3][13];    /* [1..3] of string [12] */
char networktext[4][21];     /* [1..4] of string [20] */
char difficultynameB[10][21];/* [0..9] of string [20] */
char joybutton[5][21];       /* [1..5] of string [20] */
char superships[11][26];     /* [0..10] of string [25] */
char specialname[9][10];     /* [1..9] of string [9] */
char destructhelp[25][22];
char weaponnames[17][17];    /* [1..17] of string [16] */
char destructmodename[destructmodes][13]; /* [1..destructmodes] of string [12] */
char shipinfo[13][2][256];
char menuint[maxmenu+1][11][18]; /* [0..maxmenu, 1..11] of string [17] */


JE_byte temp, temp2;

void JE_HelpBox( JE_word x, JE_word y, JE_string message, JE_byte boxwidth )
{
    JE_byte startpos, endpos, pos;
    JE_boolean endstring;

    char *substring;

    if(message[0] == '\0') {
        return;
    }

    pos = 1;
    endpos = 0;
    endstring = FALSE;

    do {
        startpos = endpos + 1;

        do {
            endpos = pos;
            do {
                pos++;
                if(message[pos-1] == '\0') {
                    endstring = TRUE;
                    if(pos - startpos < boxwidth) {
                        endpos = pos + 1;
                    }
                }

            } while((message[pos-1] != ' ') && !endstring);

        } while(!(pos - startpos > boxwidth) && !endstring);

        substring = malloc(endpos - startpos + 1);
        memcpy(substring, message + startpos - 1, endpos - startpos);
        substring[endpos - startpos] = 0;

        JE_TextShade(x, y, substring, helpboxcolor, helpboxbrightness, helpboxshadetype);

        free(substring);

        y += verticalheight;

    } while(!endstring);

    if(endpos != pos) {
        JE_TextShade(x, y, message + endpos, helpboxcolor, helpboxbrightness, helpboxshadetype);
    }

    helpboxcolor = 12;
    helpboxshadetype = FullShade;
}

void JE_HBox(JE_word x, JE_word y, JE_byte messagenum, JE_byte boxwidth)
{
    JE_HelpBox(x, y, helptxt[messagenum], boxwidth);
}

void JE_loadhelptext( void )
{
    FILE *f; int i;

    JE_resetfileext(&f, "TYRIAN.HDT", FALSE);
    fread(&episode1DataLoc, sizeof(episode1DataLoc), 1, f);
    JE_skipcryptln(f);

    for (i = 0; i < maxhelpmessage; i++) {
        JE_readcryptln(f,helptxt[i]);           /*Online Help*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);

    for (i = 0; i < 21; i++) {
        JE_readcryptln(f,pname[i]);             /*Planet names*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);

    for (i = 0; i < 68; i++) {
        JE_readcryptln(f,misctext[i]);          /*Miscellaneous text*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);

    for (i = 0; i < 5; i++) {
        JE_readcryptln(f,misctextB[i]);         /*Little Miscellaneous text*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);

    for (i = 0; i < 11; i++) {
        JE_readcryptln(f,menuint[6][i]);        /*Key names*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);

    for (i = 0; i < 7; i++) {
        JE_readcryptln(f,menutext[i]);          /*Main Menu*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);

    for (i = 0; i < 9; i++) {
        JE_readcryptln(f,outputs[i]);           /*Event text*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);

    for (i = 0; i < 6; i++) {
        JE_readcryptln(f,topicname[i]);         /*Help topics*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);

    for (i = 0; i < 34; i++) {
        JE_readcryptln(f,mainmenuhelp[i]);      /*Main Menu Help*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);

    for (i = 0; i < 7; i++) {
        JE_readcryptln(f,menuint[1][i]);        /*Menu 1 - Main*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 9; i++) {
        JE_readcryptln(f,menuint[2][i]);        /*Menu 2 - Items*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 8; i++) {
        JE_readcryptln(f,menuint[3][i]);        /*Menu 3 - Options*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 6; i++) {
        JE_readcryptln(f,ingametext[i]);        /*InGame Menu*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 6; i++) {
        JE_readcryptln(f,detaillevel[i]);       /*Detail Level*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 5; i++) {
        JE_readcryptln(f,gamespeedtext[i]);     /*Game speed text*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 5; i++) {
        JE_readcryptln(f,episodename[i]);       /*Episode Names*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 6; i++) {
        JE_readcryptln(f,difficultyname[i]);    /*Difficulty Level*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 4; i++) {
        JE_readcryptln(f,playername[i]);        /*Player Names*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 6; i++) {
        JE_readcryptln(f,menuint[10][i]);       /*Menu 10 - 2Player Main*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 3; i++) {
        JE_readcryptln(f,inputdevices[i]);      /*Input Devices*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 4; i++) {
        JE_readcryptln(f,networktext[i]);       /*Network text*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 4; i++) {
        JE_readcryptln(f,menuint[11][i]);       /*Menu 11 - 2Player Network*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 10; i++) {
        JE_readcryptln(f,difficultynameB[i]);   /*HighScore Difficulty Names*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 6; i++) {
        JE_readcryptln(f,menuint[12][i]);       /*Menu 12 - Network Options*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 7; i++) {
        JE_readcryptln(f,menuint[13][i]);       /*Menu 13 - Joystick*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 5; i++) {
        JE_readcryptln(f,joybutton[i]);         /*Joystick Button Assignments*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);                                                                                                                                  
    
    for (i = 0; i < 10; i++) {
        JE_readcryptln(f,superships[i]);        /*SuperShips - For Super Arcade Mode*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 9; i++) {
        JE_readcryptln(f,specialname[i]);       /*SuperShips - For Super Arcade Mode*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 25; i++) {
        JE_readcryptln(f,destructhelp[i]);      /*Secret DESTRUCT game*/                                                                                       
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 17; i++) {
        JE_readcryptln(f,weaponnames[i]);       /*Secret DESTRUCT weapons*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < destructmodes; i++) {
        JE_readcryptln(f,destructmodename[i]);  /*Secret DESTRUCT modes*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 13; i++) {
        JE_readcryptln(f,shipinfo[i][0]);       /*NEW: Ship Info*/
        JE_readcryptln(f,shipinfo[i][1]);       /*NEW: Ship Info*/
    }
    JE_skipcryptln(f); JE_skipcryptln(f);
    
    for (i = 0; i < 5; i++) {
        JE_readcryptln(f,menuint[14][i]);       /*Menu 12 - Network Options*/
    }

    fclose(f);
}
