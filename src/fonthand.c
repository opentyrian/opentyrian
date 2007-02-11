#include "opentyrian.h"
#include "newshape.h"
#include "fonthand.h"

const JE_byte fontmap[136] = 	/* [33..168] */
{
	27,34,61,62,63,0,33,65,66,64,85,30,84,29,81,80,71,72,73,74,75,76,77,78,
	79,32,31,0,0,0,28,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
	21,22,23,24,25,26,69,83,70,0,0,0,35,36,37,38,39,40,41,42,43,44,45,46,47,
	48,49,50,51,52,53,54,55,56,57,58,59,60,67,82,68,0,0,

	87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,
	108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,
	126,127
};

const JE_integer _Partshade  = 0;
const JE_integer _Fullshade  = 1;
const JE_integer _Darken     = 2;
const JE_integer _Trick      = 3;
const JE_integer _Noshade    = 255;

/*
const JE_integer _PlanetShapes    = 0;
const JE_integer _FontShapes      = 1;
const JE_integer _SmallFontShapes = 2;
const JE_integer _FaceShapes      = 3;
const JE_integer _OptionShapes    = 4;	/-*Also contains help shapes*-/
const JE_integer _TinyFont        = 5;
const JE_integer _WeaponShapes    = 6;
*/

JE_integer DefaultBrightness = -3;
JE_byte TextGlowBrightness = 6;

/*Intended for Font drawing - maximum X size is 255*/
void JE_NewDrawCShapeBright(JE_byte *shape, JE_word xsize, JE_word ysize, JE_word x, JE_word y, JE_byte filter, JE_shortint brightness)
{
	JE_word xloop = 0, yloop = 0;
	JE_byte *p;	/* shape pointer */
	unsigned char *s;	/* screen pointer, 8-bit specific */

	SDL_LockSurface(tempscreenseg);
	s = (unsigned char *)((int)tempscreenseg->pixels + y * tempscreenseg->pitch + x * tempscreenseg->format->BytesPerPixel);

	filter <<= 4;

	for(p = shape; yloop < ysize; p++)
	{
		switch(*p)
		{
		case 255:	/* p transparent pixels */
			p++;
			s += *p; xloop += *p;
			break;
		case 254:	/* next y */
			s -= xloop; xloop = 0;
			s += tempscreenseg->w; yloop++;
			break;
		case 253:	/* 1 transparent pixel */
			s++; xloop++;
			break;
		default:	/* set a pixel */
			*s = ((*p & 0x0f) | filter) + brightness;
			s++; xloop++;
		}
		if(xloop == xsize)
		{
			s -= xloop; xloop = 0;
			s += tempscreenseg->w; yloop++;
		}
	}

	SDL_UnlockSurface(tempscreenseg);
}
