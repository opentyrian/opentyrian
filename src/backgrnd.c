/* vim: set noet:
 *
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
#include "config.h"
#include "vga256d.h"
#include "varz.h"

#define NO_EXTERNS
#include "backgrnd.h"
#undef NO_EXTERNS

/*Special Background 2 and Background 3*/

/*Back Pos 3*/
JE_word backPos, backPos2, backPos3;
JE_word backMove, backMove2, backMove3;

/*Main Maps*/
JE_word megaDataSeg, megaDataOfs, megaData2Seg, megaData2Ofs, megaData3Seg, megaData3Ofs;
JE_word mapx, mapy, mapx2, mapx3, mapy2, mapy3;
JE_byte **mapyPos, **mapy2Pos, **mapy3Pos;
JE_word mapxPos, oldmapxOfs, mapxOfs, mapx2Ofs, mapx2Pos, mapx3Pos, oldMapx3Ofs, mapx3Ofs, tempMapxOfs,
        mapxbpPos, mapx2bpPos, mapx3bpPos;
JE_byte map1yDelay, map1yDelayMax, map2yDelay, map2yDelayMax;


void      *smoothiesScreen;
JE_word    smoothiesSeg;
JE_boolean anySmoothies;
JE_word    TSS;       /*Temp smoothies screen*/
JE_byte    SDAT[9]; /* [1..9] */

JE_byte temp, temp2;

void JE_darkenBackground( JE_word neat )
{
	STUB(JE_darkenBackground);
}

void JE_drawBackground2( void )
{
	JE_boolean useBackground1ofs;

	JE_byte **bp, *src;
	unsigned char *s = NULL; /* screen pointer, 8-bit specific */

	int i, j;
	int x, y;

	if (map2yDelayMax > 1)
	{
		if (backMove2 < 2)
		{
			if (map2yDelay == 1)
			{
				backMove2 = 1;
			} else {
				backMove2 = 0;
			}
		}
	}

	useBackground1ofs = smoothies[1];

	/*Draw background*/

	/* BP is used by all backgrounds */

	if (background2 != 0)
	{

		/*Offset for top*/
		s = VGAScreen->pixels;
		s += 11 * 24;

		if (useBackground1ofs != 0)
		{
			s += mapxPos;
			/* Map location number in BP */
			bp = mapy2Pos + mapxbpPos;
		} else {
			s += mapx2Pos;
			/* Map location number in BP */
			bp = mapy2Pos + mapx2bpPos;
		}

		/* Use DS for MegaDataSeg */
		src = megaData2->mainmap[0][0];

		/*============BACKGROUND 2 TOP=============*/
		if (backPos2 != 0)
		{
			for (i = 12; i; i--)
			{
				/* move to previous map X location */
				bp--;

				src = *bp;
				if (src != NULL)
				{
					src += (28 - backPos2) * 24;

					for (y = backPos2; y; y--)
					{
						for(x = 0; x < 24; x++)
						{
							if (src[x])
							{
								s[x] = src[x];
							}
						}

						s += VGAScreen->w;
						src += 24;
					}

					s -= backPos2 * VGAScreen->w;
				}

				s -= 24;
			}

			s += backPos2 * VGAScreen->w;
			s += 24 * 12;

			/* Increment Map Location for next line */
			bp += 14 - 2;   /* 44+44 +4 (Map Width) */
		}

		bp += 14;

		/*============BACKGROUND 2 CENTER=============*/

		/* Screen 6 lines high */
		for (i = 6; i; i--)
		{
			for (j = 12; j; j--)
			{
				/* move to previous map X location */
				bp--;

				src = *bp;
				if (src != NULL)
				{
					for (y = 28; y; y--)
					{
						for(x = 0; x < 24; x++)
						{
							if (src[x])
							{
								s[x] = src[x];
							}
						}

						s += VGAScreen->w;
						src += 24;
					}

					/* AX=320*13+12 for subtracting from DI when done drawing a shape */
					s -= VGAScreen->w * 28;
				}

				s -= 24;
			}

			/* Increment Map Location for next line */
			bp += 14 + 14 - 2;  /* 44+44 +6 (Map Width) */
			s += VGAScreen->w * 28 + 24 * 12;
		}

		if (backPos2 <= 15)
		{
			/*============BACKGROUND 2 BOTTOM=============*/
			for (i = 12; i; i--)
			{
				/* move to previous map X location */
				bp--;

				src = *bp;
				if (src != NULL)
				{

					for (y = 15 - backPos2 + 1; y; y--)
					{
						for(x = 0; x < 24; x++)
						{
							if (src[x])
							{
								s[x] = src[x];
							}
						}

						s += VGAScreen->w;
						src += 24;
					}

					s -= (15 - backPos2 + 1) * VGAScreen->w;
				}

				s -= 24;
			}
		}

	}

	/*Set Movement of background*/
	if (--map2yDelay == 0)
	{
		map2yDelay = map2yDelayMax;

		backPos2 += backMove2;

		if (backPos2 >  27)
		{
			backPos2 -= 28;
			mapy2--;
			mapy2Pos -= 14;  /*Map Width*/
		}
	}
}

void JE_superBackground2( void )
{
	/*=======================BACKGROUNDS========================*/
	/*=======================BACKGROUND 2========================*/
	JE_byte **bp, *src;
	unsigned char *s = NULL; /* screen pointer, 8-bit specific */

	int i, j;
	int x, y;

	if (map2yDelayMax > 1)
	{
		if (backMove2 < 2)
		{
			if (map2yDelay == 1)
			{
				backMove2 = 1;
			} else {
				backMove2 = 0;
			}
		}
	}

	/*Draw background*/

	/* BP is used by all backgrounds */

	/*Offset for top*/
	s = VGAScreen->pixels;
	s += 11 * 24;

	s += mapx2Pos;
	/* Map location number in BP */
	bp = mapy2Pos + mapx2bpPos;

	/* Use DS for MegaDataSeg */
	src = megaData2->mainmap[0][0];

	/*============BACKGROUND 2 TOP=============*/
	if (backPos2 != 0)
	{
		for (i = 12; i; i--)
		{
			/* move to previous map X location */
			bp--;

			src = *bp;
			if (src != NULL)
			{
				src += (28 - backPos2) * 24;

				for (y = backPos2; y; y--)
				{
					for(x = 0; x < 24; x++)
					{
						if (*src != 0)
						{
							*s = (((*s & 0x0f) + (*src & 0x0f)) / 2) | (*src & 0xf0);
						}

						s++;
						src++;
					}

					s += VGAScreen->w - 24;
				}

				s -= backPos2 * VGAScreen->w;
			}

			s -= 24;
		}

		s += backPos2 * VGAScreen->w;
		s += 24 * 12;

		/* Increment Map Location for next line */
		bp += 14 - 2;   /* 44+44 +4 (Map Width) */
	}

	bp += 14;

	/*============BACKGROUND 2 CENTER=============*/

	/* Screen 6 lines high */
	for (i = 6; i; i--)
	{
		for (j = 12; j; j--)
		{
			/* move to previous map X location */
			bp--;

			src = *bp;
			if (src != NULL)
			{
				for (y = 28; y; y--)
				{
					for(x = 0; x < 24; x++)
					{
						if (*src != 0)
						{
							*s = (((*s & 0x0f) + (*src & 0x0f)) / 2) | (*src & 0xf0);
						}

						s++;
						src++;
					}

					s += VGAScreen->w - 24;
				}

				/* AX=320*13+12 for subtracting from DI when done drawing a shape */
				s -= VGAScreen->w * 28;
			}

			s -= 24;
		}

		/* Increment Map Location for next line */
		bp += 14 + 14 - 2;  /* 44+44 +6 (Map Width) */
		s += VGAScreen->w * 28 + 24 * 12;
	}

	if (backPos2 <= 15)
	{
		/*============BACKGROUND 2 BOTTOM=============*/
		for (i = 12; i; i--)
		{
			/* move to previous map X location */
			bp--;

			src = *bp;
			if (src != NULL)
			{

				for (y = 15 - backPos2 + 1; y; y--)
				{
					for(x = 0; x < 24; x++)
					{
						if (*src != 0)
						{
							*s = (((*s & 0x0f) + (*src & 0x0f)) / 2) | (*src & 0xf0);
						}

						s++;
						src++;
					}

					s += VGAScreen->w - 24;
				}

				s -= (15 - backPos2 + 1) * VGAScreen->w;
			}

			s -= 24;
		}
	}

	/*Set Movement of background*/
	if (--map2yDelay == 0)
	{
		map2yDelay = map2yDelayMax;

		backPos2 += backMove2;

		if (backPos2 >  27)
		{
			backPos2 -= 28;
			mapy2--;
			mapy2Pos -= 14;  /*Map Width*/
		}
	}
}

void JE_drawBackground3( void )
{
	JE_byte **bp, *src;
	unsigned char *s = NULL; /* screen pointer, 8-bit specific */

	int i, j;
	int x, y;

	/* Movement of background */
   backPos3 += backMove3;

	if (backPos3 > 27)
	{
		backPos3 -= 28;
		mapy3--;
		mapy3Pos -= 15;   /*Map Width*/
	}

	/* Offset for top*/
	s = VGAScreen->pixels;
	s += 11 * 24;

	s += mapx3Pos;

	/* Map location number in BP */
	bp = mapy3Pos + mapx3bpPos;

	/* Use DS for MegaDataSeg */
	src = megaData3->mainmap[0][0];

	/*============BACKGROUND 3 TOP=============*/
	if (backPos3 != 0)
	{
		for (i = 12; i; i--)
		{
			/* move to previous map X location */
			bp--;

			src = *bp;
			if (src != NULL)
			{
				src += (28 - backPos3) * 24;

				for (y = backPos3; y; y--)
				{
					for(x = 0; x < 24; x++)
					{
						if (src[x])
						{
							s[x] = src[x];
						}
					}

					s += VGAScreen->w;
					src += 24;
				}

				s -= backPos3 * VGAScreen->w;
			}

			s -= 24;
		}

		s += backPos3 * VGAScreen->w;
		s += 24 * 12;

		/* Increment Map Location for next line */
		bp += 15 - 3;
	}

	bp += 15;

	/*============BACKGROUND 3 CENTER=============*/

	/* Screen 14 lines high */
	for (i = 6; i; i--)
	{
		for (j = 12; j; j--)
		{
			/* move to previous map X location */
			bp--;

			src = *bp;
			if (src != NULL)
			{
				for (y = 28; y; y--)
				{
					for(x = 0; x < 24; x++)
					{
						if (src[x])
						{
							s[x] = src[x];
						}
					}

					s += VGAScreen->w;
					src += 24;
				}

				/* AX=320*13+12 for subtracting from DI when done drawing a shape */
				s -= VGAScreen->w * 28;
			}

			s -= 24;
		}

		/* Increment Map Location for next line */
		bp += 15 + 15 - 3;  /* 44+44 +6 (Map Width) */
		s += VGAScreen->w * 28 + 24 * 12;
	}

	if (backPos3 <= 15)
	{
		/*============BACKGROUND 3 BOTTOM=============*/
		for (i = 12; i; i--)
		{
			/* move to previous map X location */
			bp--;

			src = *bp;
			if (src != NULL)
			{

				for (y = 15 - backPos3 + 1; y; y--)
				{
					for(x = 0; x < 24; x++)
					{
						if (src[x])
						{
							s[x] = src[x];
						}
					}

					s += VGAScreen->w;
					src += 24;
				}

				s -= (15 - backPos3 + 1) * VGAScreen->w;
			}

			s -= 24;
		}
	}

}

void JE_checkSmoothies( void )
{
	STUB(JE_checkSmoothies);
}

void JE_smoothies1( void )
{
	STUB(JE_smoothies1);
}

void JE_smoothies2( void )
{
	STUB(JE_smoothies2);
}

void JE_smoothies3( void )
{
	STUB(JE_smoothies3);
}

void JE_smoothies4( void )
{
	STUB(JE_smoothies4);
}

void JE_smoothies6( void )
{
	STUB(JE_smoothies6);
}
