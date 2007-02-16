#include "pcxmast.h"

JE_string pcxfile[PCXnum] = 	/* [1..PCXnum] */
{
	"INTSHPB.PCX",
	"SETUP2.PCX",
	"TYRPLAY.PCX",
	"TYRLOG2.PCX",
	"P1.PCX",
	"TYRPLAY2.PCX",
	"BUC4.PCX",
	"GMOVR4a.PCX",
	"GMOVR4b.PCX",
	"EPICSKY.PCX",
	"DESTRUCT.PCX",
	"ECLIPSE.PCX",
	"FIREPICA.PCX"
};

JE_byte pcxpal[PCXnum] = 	/* [1..PCXnum] */
{ 1, 8, 6, 9, 11, 6, 19, 20, 20, 21, 22, 23, 6};

/*FACEMAX*/
JE_byte facepal[12] = 	/* [1..12] */
{ 2, 3, 4, 5, 7, 10, 12, 13, 17, 14, 15, 16};
