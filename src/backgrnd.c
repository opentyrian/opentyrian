/* vim: set noet:
 *
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Development Team
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
#include "backgrnd.h"

#include "config.h"
#include "varz.h"
#include "vga256d.h"


/*Special Background 2 and Background 3*/

/*Back Pos 3*/
JE_word backPos, backPos2, backPos3;
JE_word backMove, backMove2, backMove3;

/*Main Maps*/
JE_word megaDataSeg, megaDataOfs, megaData2Seg, megaData2Ofs, megaData3Seg, megaData3Ofs;
JE_word mapX, mapY, mapX2, mapX3, mapY2, mapY3;
JE_byte **mapYPos, **mapY2Pos, **mapY3Pos;
JE_word mapXPos, oldMapXOfs, mapXOfs, mapX2Ofs, mapX2Pos, mapX3Pos, oldMapX3Ofs, mapX3Ofs, tempMapXOfs;
intptr_t mapXbpPos, mapX2bpPos, mapX3bpPos;
JE_byte map1YDelay, map1YDelayMax, map2YDelay, map2YDelayMax;


SDL_Surface *smoothiesScreen;
JE_boolean  anySmoothies;
JE_byte     SDAT[9]; /* [1..9] */

JE_byte temp, temp2;

void JE_darkenBackground( JE_word neat )
{
	Uint8 *s = VGAScreen->pixels; /* screen pointer, 8-bit specific */
	int x, y;
	
	s += 24;
	
	for (y = 184; y; y--)
	{
		for (x = 264; x; x--)
		{
			*s = ((((*s & 0x0f) << 4) - (*s & 0x0f) + ((((x - neat - y) >> 2) + *(s-2) + (y == 184 ? 0 : *(s-(VGAScreen->w-1)))) & 0x0f)) >> 4) | (*s & 0xf0);
			s++;
		}
		s += VGAScreen->w - 264;
	}
}

void JE_drawBackground2( void )
{
	JE_boolean useBackground1ofs;

	JE_byte **bp, *src;
	Uint8 *s = NULL; /* screen pointer, 8-bit specific */

	int i, j;
	int x, y;

	if (map2YDelayMax > 1)
	{
		if (backMove2 < 2)
		{
			if (map2YDelay == 1)
			{
				backMove2 = 1;
			} else {
				backMove2 = 0;
			}
		}
	}

	useBackground1ofs = smoothies[2-1];

	/*Draw background*/

	/* BP is used by all backgrounds */

	if (background2 != 0)
	{

		/*Offset for top*/
		s = VGAScreen->pixels;
		s += 11 * 24;

		if (useBackground1ofs != 0)
		{
			s += mapXPos;
			/* Map location number in BP */
			bp = mapY2Pos + mapXbpPos;
		} else {
			s += mapX2Pos;
			/* Map location number in BP */
			bp = mapY2Pos + mapX2bpPos;
		}

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
	if (--map2YDelay == 0)
	{
		map2YDelay = map2YDelayMax;

		backPos2 += backMove2;

		if (backPos2 >  27)
		{
			backPos2 -= 28;
			mapY2--;
			mapY2Pos -= 14;  /*Map Width*/
		}
	}
}

void JE_superBackground2( void )
{
	/*=======================BACKGROUNDS========================*/
	/*=======================BACKGROUND 2========================*/
	JE_byte **bp, *src;
	Uint8 *s = NULL; /* screen pointer, 8-bit specific */

	int i, j;
	int x, y;

	if (map2YDelayMax > 1)
	{
		if (backMove2 < 2)
		{
			if (map2YDelay == 1)
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

	s += mapX2Pos;
	/* Map location number in BP */
	bp = mapY2Pos + mapX2bpPos;

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
	if (--map2YDelay == 0)
	{
		map2YDelay = map2YDelayMax;

		backPos2 += backMove2;

		if (backPos2 >  27)
		{
			backPos2 -= 28;
			mapY2--;
			mapY2Pos -= 14;  /*Map Width*/
		}
	}
}

void JE_drawBackground3( void )
{
	JE_byte **bp, *src;
	Uint8 *s = NULL; /* screen pointer, 8-bit specific */

	int i, j;
	int x, y;

	/* Movement of background */
   backPos3 += backMove3;

	if (backPos3 > 27)
	{
		backPos3 -= 28;
		mapY3--;
		mapY3Pos -= 15;   /*Map Width*/
	}

	/* Offset for top*/
	s = VGAScreen->pixels;
	s += 11 * 24;

	s += mapX3Pos;

	/* Map location number in BP */
	bp = mapY3Pos + mapX3bpPos;

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

void JE_filterScreen( JE_shortint col, JE_shortint int_)
{
	Uint8 *s = NULL; /* screen pointer, 8-bit specific */
	int x, y;
	unsigned int temp;
	
	if (filterFade)
	{
		levelBrightness += levelBrightnessChg;
		if (filterFadeStart &&
		    (levelBrightness < -14 || levelBrightness > 14))
		{
			levelBrightnessChg = -levelBrightnessChg;
			filterFadeStart = false;
			levelFilter = levelFilterNew;
		}
		if (!filterFadeStart && levelBrightness == 0)
		{
			filterFade = false;
			levelBrightness = -99;
		}
	}
	
	if (col != -99 && filtrationAvail)
	{
		s = VGAScreen->pixels;
		s += 24;
		
		col <<= 4;
		
		for (y = 184; y; y--)
		{
			for (x = 264; x; x--)
			{
				*s = col | (*s & 0x0f);
				s++;
			}
			s += VGAScreen->w - 264;
		}
	}
	
	if (int_ != -99 && explosionTransparent)
	{
		s = VGAScreen->pixels;
		s += 24;
		
		for (y = 184; y; y--)
		{
			for (x = 264; x; x--)
			{
				temp = (*s & 0x0f) + int_;
				*s = (*s & 0xf0) | (temp > 0x1f ? 0 : (temp > 0x0f ? 0x0f : temp));
				s++;
			}
			s += VGAScreen->w - 264;
		}
	}
}

void JE_checkSmoothies( void )
{
	anySmoothies = false;
	if ((processorType > 2 && (smoothies[1-1] || smoothies[2-1])) || (processorType > 1 && (smoothies[3-1] || smoothies[4-1] || smoothies[5-1])))
	{
		anySmoothies = true;
		JE_initSmoothies();
	}
}

void JE_initSmoothies( void )
{
	smoothiesScreen = VGAScreen2;
}

void JE_smoothies1( void ) /*Lava Effect*/
{
	Uint8 *s = game_screen->pixels; /* screen pointer, 8-bit specific */
	Uint8 *src = VGAScreen->pixels; /* screen pointer, 8-bit specific */
	int i, j, temp;
	
	s += game_screen->w * 185;
	src += game_screen->w * 185;
	
	for (i = 185 * game_screen->w; i; i -= 8)
	{
		temp = (((i - 1) >> 9) & 15) - 8;
		temp = (temp < 0 ? -temp : temp) - 1;
		
		for (j = 8; j; j--)
		{
			*s = (((*(src + temp) & 0x0f) + (*(src + temp + game_screen->w) & 0x0f) + (i + temp < game_screen->w ? 0 : *(src + temp - game_screen->w) & 0x0f)) >> 2) | 0x70;
			s--;
			src--;
		}
	}
	VGAScreen = game_screen;
}

void JE_smoothies2( void ) /*Water effect*/
{
	Uint8 *s = game_screen->pixels; /* screen pointer, 8-bit specific */
	Uint8 *src = VGAScreen->pixels; /* screen pointer, 8-bit specific */
	int i, j, temp;

	s += game_screen->w * 185;
	src += game_screen->w * 185;

	for (i = 185 * game_screen->w; i; i -= 8)
	{
		temp = (((i - 1) >> 10) & 7) - 4;
		temp = (temp < 0 ? -temp : temp) - 1;
		
		for (j = 8; j; j--)
		{
			if (*src & 0x30)
				*s = (((*src & 0x0f) + (*(s + temp + game_screen->w) & 0x0f)) >> 1) | (SDAT[2-1] << 4);
			else
				*s = *src;
			s--;
			src--;
		}
	}
	VGAScreen = game_screen;
}

void JE_smoothies3( void ) /* iced motion blur */
{
	Uint8 *s = game_screen->pixels; /* screen pointer, 8-bit specific */
	Uint8 *src = VGAScreen->pixels; /* screen pointer, 8-bit specific */
	int i;

	for (i = 184 * game_screen->w; i; i--)
	{
			*s = ((((*src & 0x0f) + (*s & 0x0f)) >> 1) & 0x0f) | 0x80;
			s++;
			src++;
	}
	VGAScreen = game_screen;
}

void JE_smoothies4( void ) /* motion blur */
{
	Uint8 *s = game_screen->pixels; /* screen pointer, 8-bit specific */
	Uint8 *src = VGAScreen->pixels; /* screen pointer, 8-bit specific */
	int i;

	for (i = 184 * game_screen->w; i; i--)
	{
			*s = ((((*src & 0x0f) + (*s & 0x0f)) >> 1) & 0x0f) | (*src & 0xf0);
			s++;
			src++;
	}
	VGAScreen = game_screen;
}
