#ifndef HELPTEXT_H
#define HELPTEXT_H

#include "opentyr.h"

#define maxhelpmessage 39
#define maxmenu 14

#define destructmodes 5

typedef char JE_helptxttype[maxhelpmessage][231];	/* [1..maxhelpmessage] of string [230] */
typedef char JE_helptexttype[34][66];	/* [1..34] of string [65] */
typedef char JE_destructhelptype[25][22];	/* [1..25] of string [21] */
typedef JE_string JE_shipinfotype[13][2];	/* [1..13, 1..2] */

#define HELPTEXT_EXTERNS \
extern JE_byte verticalheight; \
extern JE_byte helpboxcolor, helpboxbrightness, helpboxshadetype; \
extern JE_helptxttype *helptxt; \
extern char pname[21][16];		/* [1..21] of string [15] */ \
extern char misctext[68][42];		/* [1..68] of string [41] */ \
extern char misctextB[5][11];		/* [1..5] of string [10] */ \
extern char keyname[8][18];		/* [1..8] of string [17] */ \
extern char menutext[7][21];		/* [1..7] of string [20] */ \
extern char outputs[9][31];		/* [1..9] of string [30] */ \
extern char topicname[6][21];		/* [1..6] of string [20] */ \
extern JE_helptexttype *mainmenuhelp; \
extern char ingametext[6][21];		/* [1..6] of string [20] */ \
extern char detaillevel[6][13];	/* [1..6] of string [12] */ \
extern char gamespeedtext[5][13];	/* [1..5] of string [12] */ \
extern char episodename[6][31];	/* [0..5] of string [30] */ \
extern char difficultyname[7][21];	/* [0..6] of string [20] */ \
extern char playername[5][26];		/* [0..4] of string [25] */ \
extern char inputdevices[3][13];	/* [1..3] of string [12] */ \
extern char networktext[4][21];	/* [1..4] of string [20] */ \
extern char difficultynameB[10][21];	/* [0..9] of string [20] */ \
extern char joybutton[5][21];		/* [1..5] of string [20] */ \
extern char superships[11][26];	/* [0..10] of string [25] */ \
extern char SpecialName[9][10];	/* [1..9] of string [9] */ \
extern JE_destructhelptype *Destructhelp; \
extern char weaponnames[17][17];	/* [1..17] of string [16] */ \
extern char destructmodename[destructmodes][13];	/* [1..destructmodes] of string [12] */ \
extern JE_shipinfotype *shipinfo; \
extern char menuint[maxmenu+1][11][18];	/* [0..maxmenu, 1..11] of string [17] */ \
extern JE_byte menuhelp[maxmenu][11];	/* [1..maxmenu, 1..11] */

/*JE_byte temp, temp2;*/

void JE_HelpBox( JE_word x, JE_word y, JE_string message, JE_byte boxwidth );
void JE_HBox( JE_word x, JE_word y, JE_byte messagenum, JE_byte boxwidth );
void JE_loadhelptext( void );

#endif /* HELPTEXT_H */
