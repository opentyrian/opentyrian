#include "opentyr.h"
#include "helptext.h"
#include "fonthand.h"
#include "newshape.h"

JE_byte menuhelp[maxmenu][11] =     /* [1..maxmenu, 1..11] */
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
    endstring = 0;  /*false*/

    do {
        startpos = endpos + 1;

        do {
            endpos = pos;
            do {
                pos++;
                if(message[pos-1] == '\0') {
                    endstring = 1;  /*true*/
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
