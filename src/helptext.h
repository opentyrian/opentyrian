#ifndef HELPTEXT_H
#define HELPTEXT_H

#include "opentyr.h"
#include "fonthand.h"
#include "newshape.h"

#define maxhelpmessage 39
#define maxmenu 14

JE_byte menuhelp[maxmenu][11];	/* [1..maxmenu, 1..11] */

#define destructmodes 5

typedef char JE_helptxttype[maxhelpmessage][231];	/* [1..maxhelpmessage] of string [230] */
typedef char JE_helptexttype[34][66];	/* [1..34] of string [65] */
typedef char JE_destructhelptype[25][22];	/* [1..25] of string [21] */
typedef JE_string JE_shipinfotype[13][2];	/* [1..13, 1..2] */

JE_byte verticalheight;
JE_byte helpboxcolor, helpboxbrightness, helpboxshadetype;

JE_helptxttype *helptxt;
char pname[21][16];		/* [1..21] of string [15] */
char misctext[68][42];		/* [1..68] of string [41] */
char misctextB[5][11];		/* [1..5] of string [10] */
char keyname[8][18];		/* [1..8] of string [17] */
char menutext[7][21];		/* [1..7] of string [20] */
char outputs[9][31];		/* [1..9] of string [30] */
char topicname[6][21];		/* [1..6] of string [20] */
JE_helptexttype *mainmenuhelp;
char ingametext[6][21];		/* [1..6] of string [20] */
char detaillevel[6][13];	/* [1..6] of string [12] */
char gamespeedtext[5][13];	/* [1..5] of string [12] */
char episodename[6][31];	/* [0..5] of string [30] */
char difficultyname[7][21];	/* [0..6] of string [20] */
char playername[5][26];		/* [0..4] of string [25] */
char inputdevices[3][13];	/* [1..3] of string [12] */
char networktext[4][21];	/* [1..4] of string [20] */
char difficultynameB[10][21];	/* [0..9] of string [20] */
char joybutton[5][21];		/* [1..5] of string [20] */
char superships[11][26];	/* [0..10] of string [25] */
char SpecialName[9][10];	/* [1..9] of string [9] */
JE_destructhelptype *Destructhelp;
char weaponnames[17][17];	/* [1..17] of string [16] */
char destructmodename[destructmodes][13];	/* [1..destructmodes] of string [12] */
JE_shipinfotype *shipinfo;

char menuint[maxmenu+1][11][18];	/* [0..maxmenu, 1..11] of string [17] */

JE_byte temp, temp2;

void JE_HelpBox(JE_word x, JE_word y, JE_string message, JE_byte boxwidth);
void JE_HBox(JE_word x, JE_word y, JE_byte messagenum, JE_byte boxwidth);
void JE_loadhelptext(void);

#endif /* HELPTEXT_H */
