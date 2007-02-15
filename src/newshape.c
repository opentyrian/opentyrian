#include "opentyr.h"
#include "newshape.h"

const JE_integer OldBlack = 0;
const JE_integer NewBlack = 253;
const JE_integer _PlanetShapes    = 0;
const JE_integer _FontShapes      = 1;
const JE_integer _SmallFontShapes = 2;
const JE_integer _FaceShapes      = 3;
const JE_integer _OptionShapes    = 4;   /*Also contains help shapes*/
const JE_integer _TinyFont        = 5;
const JE_integer _WeaponShapes    = 6;
const JE_integer _ExtraShapes     = 7;   /*Used for Ending pics*/

SDL_Surface *tempscreenseg = NULL;

void JE_NewLoadShapesB(JE_byte table, FILE *f)
{
	short tempw;
	short z;

	fread(&tempw, 2, 1, f);
	maxshape[table] = tempw;

	if(!LoadOverride)
	{
		min = 1;
		max = maxshape[table];
	}

	if(min > 1)
	{
		for(z = 0; z < min-1; z++)
		{
			fread(&shapexist[table][z], 1, 1, f);

			if(shapexist[table][z])
			{
				fread(&shapex   [table][z], 2, 1, f);
				fread(&shapey   [table][z], 2, 1, f);
				fread(&shapesize[table][z], 2, 1, f);

				(*shapearray)[table][z] = malloc(shapex[table][z]*shapey[table][z]);

				fread((*shapearray)[table][z], shapesize[table][z], 1, f);

				free((*shapearray)[table][z]);
			}
		}
	}

	for(z = min-1; z < max; z++)
	{
		tempw = z-min+1;
		fread(&shapexist[table][tempw], 1, 1, f);

		if(shapexist[table][tempw])
		{
			fread(&shapex   [table][tempw], 2, 1, f);
			fread(&shapey   [table][tempw], 2, 1, f);
			fread(&shapesize[table][tempw], 2, 1, f);

			(*shapearray)[table][tempw] = malloc(shapex[table][tempw]*shapey[table][tempw]);

			fread((*shapearray)[table][tempw], shapesize[table][tempw], 1, f);
		}
	}
}

void JE_NewDrawCShape(JE_byte *shape, JE_word xsize, JE_word ysize)
{
	JE_word x = 2, y = 2;
	JE_word xloop = 0, yloop = 0;
	JE_byte *p;	/* shape pointer */
	unsigned char *s;	/* screen pointer, 8-bit specific */

	SDL_LockSurface(tempscreenseg);
	s = (unsigned char *)tempscreenseg->pixels;
	s += y * tempscreenseg->w + x;

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
			*s = *p;
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

void JE_NewDrawCShapeNum(JE_byte table, JE_byte shape, JE_word x, JE_word y)
{
	JE_word xsize, ysize;
	JE_word xloop = 0, yloop = 0;
	JE_byte *p;	/* shape pointer */
	unsigned char *s;	/* screen pointer, 8-bit specific */

	if((shape > maxshape[table]) || (!shapexist[table][shape]) || (shape == 255))
		exit(99);

	xsize = shapex[table][shape]; ysize = shapey[table][shape];

	SDL_LockSurface(tempscreenseg);
	s = (unsigned char *)tempscreenseg->pixels;
	s += y * tempscreenseg->w + x;

	for(p = (*shapearray)[table][shape]; yloop < ysize; p++)
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
			*s = *p;
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

void newshape_init()
{
	LoadOverride = 0; /*false*/
	tempscreenseg = vgascreenseg;
	for(x = 0; x < maxtable; x++)
		maxshape[x] = 0;
	shapearray = malloc(sizeof(shapearraytype));
}
