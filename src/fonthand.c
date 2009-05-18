/* 
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
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
#include "fonthand.h"

#include "newshape.h"
#include "network.h"
#include "nortsong.h"
#include "params.h"
#include "vga256d.h"
#include "video.h"


const JE_byte fontMap[136] = /* [33..168] */
{
	26,33,60,61,62,255,32,64,65,63,84,29,83,28,80,79,70,71,72,73,74,75,76,77,
	78,31,30,255,255,255,27,255,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
	20,21,22,23,24,25,68,82,69,255,255,255,34,35,36,37,38,39,40,41,42,43,44,45,46,
	47,48,49,50,51,52,53,54,55,56,57,58,59,66,81,67,255,255,

	86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,
	107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,
	125,126
};

/* shape constants included in newshape.h */

JE_integer defaultBrightness = -3;
JE_byte textGlowFont, textGlowBrightness = 6;

JE_boolean levelWarningDisplay;
JE_byte levelWarningLines;
char levelWarningText[10][61]; /* [1..10] of string [60] */
JE_boolean warningRed;

JE_byte warningSoundDelay;
JE_word armorShipDelay;
JE_byte warningCol;
JE_shortint warningColChange;

void JE_dString( JE_word x, JE_word y, const char *s, JE_byte font )
{
	int bright = 0;
	
	for (int a = 0; s[a] != 0; a++)
	{
		char b = s[a];
		
		if ((b > 32) && (b < 126))
		{
			if (fontMap[b-33] != 255)
			{
				blit_shape_dark(tempScreenSeg, x + 2, y + 2, font, fontMap[b-33], false);
				blit_shape_hv_unsafe(tempScreenSeg, x, y, font, fontMap[b-33], 0xf, defaultBrightness + bright);
				
				x += shapeX[font][fontMap[b-33]] + 1;
			}
		}
		else if (b == 32)
		{
			x += 6;
		}
		else if (b == 126)
		{
			bright = (bright == 0) ? 2 : 0;
		}
	}
}


JE_word JE_fontCenter( const char *s, JE_byte font )
{
	return 160 - (JE_textWidth(s, font) / 2);
}

JE_word JE_textWidth( const char *s, JE_byte font )
{
	JE_byte a, b;
	JE_word x = 0;
	
	for (a = 0; s[a] != 0; a++)
	{
		b = s[a];
		
		if ((b > 32) && (b < 126))
		{
			if (fontMap[b-33] != 255)
			{
				x += shapeX[font][fontMap[b-33]] + 1;
			}
		}
		else if (b == 32)
		{
			x += 6;
		}
	}
	return x;
}

void JE_textShade( JE_word x, JE_word y, const char *s, JE_byte colorbank, JE_shortint brightness, JE_byte shadetype )
{
	switch (shadetype)
	{
		case PART_SHADE:
			JE_outText(x+1, y+1, s, 0, -1);
			JE_outText(x, y, s, colorbank, brightness);
			break;
		case FULL_SHADE:
			JE_outText(x-1, y, s, 0, -1);
			JE_outText(x+1, y, s, 0, -1);
			JE_outText(x, y-1, s, 0, -1);
			JE_outText(x, y+1, s, 0, -1);
			JE_outText(x, y, s, colorbank, brightness);
			break;
		case DARKEN:
			JE_outTextAndDarken(x+1, y+1, s, colorbank, brightness, TINY_FONT);
			break;
		case TRICK:
			JE_outTextModify(x, y, s, colorbank, brightness, TINY_FONT);
			break;
	}
}

void JE_outText( JE_word x, JE_word y, const char *s, JE_byte colorbank, JE_shortint brightness )
{
	JE_byte a, b;
	JE_byte bright = 0;
	
	for (a = 0; s[a] != 0; a++)
	{
		b = s[a];
		
		if ((b > 32) && (b < 169) && (fontMap[b-33] != 255) && (shapeArray[TINY_FONT][fontMap[b-33]] != NULL))
		{
			if (brightness >= 0)
				blit_shape_hv_unsafe(tempScreenSeg, x, y, TINY_FONT, fontMap[b - 33], colorbank, brightness + bright);
			else
				blit_shape_dark(tempScreenSeg, x, y, TINY_FONT, fontMap[b-33], true);
			
			x += shapeX[TINY_FONT][fontMap[b-33]] + 1;
		}
		else if (b == 32)
		{
			x += 6;
		}
		else if (b == 126)
		{
			bright = (bright == 0) ? 4 : 0;
		}
	}
	if (brightness >= 0)
	{
		tempScreenSeg = VGAScreen;
	}
}

void JE_outTextModify( JE_word x, JE_word y, const char *s, JE_byte filter, JE_byte brightness, JE_byte font )
{
	JE_byte a, b;
	
	for (a = 0; s[a] != 0; a++)
	{
		b = s[a];
		
		if ((b > 32) && (b < 169) && (fontMap[b-33] != 255))
		{
			blit_shape_hv_blend(tempScreenSeg, x, y, font, fontMap[b-33], filter, brightness);
			
			x += shapeX[font][fontMap[b-33]] + 1;
		}
		else if (b == 32)
		{
			x += 6;
		}
	}
}

void JE_outTextShade( JE_word x, JE_word y, const char *s, JE_byte font )
{
	JE_byte a, b;
	
	for (a = 0; s[a] != 0; a++)
	{
		b = s[a];
		
		if ((b > 32) && (b < 169) && (fontMap[b-33] != 255))
		{
			blit_shape_dark(tempScreenSeg, x, y, font, fontMap[b-33], false);
			
			x += shapeX[font][fontMap[b-33]] + 1;
		}
		else if (b == 32)
		{
			x += 6;
		}
	}
}

void JE_outTextAdjust( JE_word x, JE_word y, const char *s, JE_byte filter, JE_shortint brightness, JE_byte font, JE_boolean shadow )
{
	int bright = 0;
	
	for (int a = 0; s[a] != 0; a++)
	{
		char b = s[a];
		
		if ((b > 32) && (b < 169) && (fontMap[b-33] != 255))
		{
			if (shadow)
				blit_shape_dark(tempScreenSeg, x + 2, y + 2, font, fontMap[b - 33], false);
			blit_shape_hv(tempScreenSeg, x, y, font, fontMap[b - 33], filter, brightness + bright);
			
			x += shapeX[font][fontMap[b-33]] + 1;
		}
		else if (b == 126)
		{
			bright = (bright == 0) ? 4 : 0;
		}
		else if (b == 32)
		{
			x += 6;
		}
	}
}

void JE_outTextAndDarken( JE_word x, JE_word y, const char *s, JE_byte colorbank, JE_byte brightness, JE_byte font )
{
	int bright = 0;
	
	for (int a = 0; s[a] != 0; a++)
	{
		char b = s[a];
		
		if ((b > 32) && (b < 169) && (fontMap[b-33] != 255))
		{
			blit_shape_dark(tempScreenSeg, x + 1, y + 1, font, fontMap[b-33], false);
			blit_shape_hv_unsafe(tempScreenSeg, x, y, font, fontMap[b-33], colorbank, brightness + bright);
			
			x += shapeX[font][fontMap[b-33]] + 1;
		}
		else if (b == 32)
		{
			x += 6;
		}
		else if (b == 126)
		{
			bright = (bright == 0) ? 4 : 0;
		}
	}
}

void JE_updateWarning( void )
{
	if (delaycount2() == 0)
	{ /*Update Color Bars*/
		
		warningCol += warningColChange;
		if (warningCol > 14 * 16 + 10 || warningCol < 14 * 16 + 4)
		{
			warningColChange = -warningColChange;
		}
		JE_bar(0, 0, 319, 5, warningCol);
		JE_bar(0, 194, 319, 199, warningCol);
		JE_showVGA();
		
		setjasondelay2(6);
		
		if (warningSoundDelay > 0)
		{
			warningSoundDelay--;
		}
		else
		{
			warningSoundDelay = 14;
			JE_playSampleNum(17);
		}
	}
}

void JE_outTextGlow( JE_word x, JE_word y, const char *s )
{
	JE_integer z;
	JE_byte c = 15;
	
	if (warningRed)
	{
		c = 7;
	}
	
	tempScreenSeg = VGAScreenSeg; /* sega000 */
	JE_outTextAdjust(x - 1, y,     s, 0, -12, textGlowFont, false);
	JE_outTextAdjust(x,     y - 1, s, 0, -12, textGlowFont, false);
	JE_outTextAdjust(x + 1, y,     s, 0, -12, textGlowFont, false);
	JE_outTextAdjust(x,     y + 1, s, 0, -12, textGlowFont, false);
	if (frameCountMax > 0)
		for (z = 1; z <= 12; z++)
		{
			setjasondelay(frameCountMax);
			tempScreenSeg = VGAScreenSeg; /* sega000 */
			JE_outTextAdjust(x, y, s, c, z - 10, textGlowFont, false);
			if (JE_anyButton())
			{
				frameCountMax = 0;
			}
			
			NETWORK_KEEP_ALIVE();
			
			JE_showVGA();
			
			wait_delay();
		}
	for (z = (frameCountMax == 0) ? 6 : 12; z >= textGlowBrightness; z--)
	{
		setjasondelay(frameCountMax);
		tempScreenSeg = VGAScreenSeg; /* sega000 */
		JE_outTextAdjust(x, y, s, c, z - 10, textGlowFont, false);
		if (JE_anyButton())
		{
			frameCountMax = 0;
		}
		
		NETWORK_KEEP_ALIVE();
		
		JE_showVGA();
		
		wait_delay();
	}
	textGlowBrightness = 6;
}

// kate: tab-width 4; vim: set noet:
