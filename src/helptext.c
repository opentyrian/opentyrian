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
#include <string.h>

#include "opentyr.h"
#include "helptext.h"
#include "fonthand.h"
#include "newshape.h"

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

JE_helptxttype *helptxt;
char pname[21][16];     /* [1..21] of string [15] */
char misctext[68][42];      /* [1..68] of string [41] */
char misctextB[5][11];      /* [1..5] of string [10] */
char keyname[8][18];        /* [1..8] of string [17] */
char menutext[7][21];       /* [1..7] of string [20] */
char outputs[9][31];        /* [1..9] of string [30] */
char topicname[6][21];      /* [1..6] of string [20] */
JE_helptexttype *mainmenuhelp;
char ingametext[6][21];     /* [1..6] of string [20] */
char detaillevel[6][13];    /* [1..6] of string [12] */
char gamespeedtext[5][13];  /* [1..5] of string [12] */
char episodename[6][31];    /* [0..5] of string [30] */
char difficultyname[7][21]; /* [0..6] of string [20] */
char playername[5][26];     /* [0..4] of string [25] */
char inputdevices[3][13];   /* [1..3] of string [12] */
char networktext[4][21];    /* [1..4] of string [20] */
char difficultynameB[10][21];   /* [0..9] of string [20] */
char joybutton[5][21];      /* [1..5] of string [20] */
char superships[11][26];    /* [0..10] of string [25] */
char SpecialName[9][10];    /* [1..9] of string [9] */
JE_destructhelptype *Destructhelp;
char weaponnames[17][17];   /* [1..17] of string [16] */
char destructmodename[destructmodes][13];   /* [1..destructmodes] of string [12] */
JE_shipinfotype *shipinfo;

char menuint[maxmenu+1][11][18];    /* [0..maxmenu, 1..11] of string [17] */

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
    JE_HelpBox(x, y, (*helptxt)[messagenum], boxwidth);
}


void helptext_init(void)
{
  helptxt = malloc(sizeof(JE_helptxttype));
  mainmenuhelp = malloc(sizeof(JE_helptxttype));
  Destructhelp = malloc(sizeof(JE_destructhelptype));
  shipinfo = malloc(sizeof(JE_shipinfotype));
}
