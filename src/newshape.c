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
#include "newshape.h"

#include "error.h"
#include "joystick.h"
#include "keyboard.h"
#include "network.h"
#include "nortsong.h"
#include "nortvars.h"
#include "params.h"
#include "varz.h"
#include "vga256d.h"
#include "video.h"

#include <assert.h>

SDL_Surface *tempScreenSeg = NULL;

JE_ShapeArrayType shapeArray = { { NULL } };

JE_word shapeX[MAX_TABLE][MAXIMUM_SHAPE],        /* [1..maxtable,1..maximumshape] */
        shapeY[MAX_TABLE][MAXIMUM_SHAPE];        /* [1..maxtable,1..maximumshape] */
JE_word shapeSize[MAX_TABLE][MAXIMUM_SHAPE];     /* [1..maxtable,1..maximumshape] */
JE_boolean shapeExist[MAX_TABLE][MAXIMUM_SHAPE]; /* [1..maxtable,1..maximumshape] */

JE_byte maxShape[MAX_TABLE] = { 0 };             /* [1..maxtable] */

JE_byte mouseGrabShape[24 * 28];                 /* [1..24*28] */

/*
  Colors:
  253 : Black
  254 : Jump to next line

   Skip X Pixels
   Draw X pixels of color Y
*/


void JE_newLoadShapesB( JE_byte table, FILE *f )
{
	JE_word temp;
	
	efread(&temp, sizeof(JE_word), 1, f);
	
	maxShape[table] = temp;
	
	for (int i = 0; i < maxShape[table]; i++)
	{
		shapeExist[table][i] = getc(f);
		
		if (shapeExist[table][i])
		{
			efread(&shapeX   [table][i], sizeof(JE_word), 1, f);
			efread(&shapeY   [table][i], sizeof(JE_word), 1, f);
			efread(&shapeSize[table][i], sizeof(JE_word), 1, f);
			
			shapeArray[table][i] = malloc(shapeX[table][i] * shapeY[table][i]);
			
			efread(shapeArray[table][i], sizeof(JE_byte), shapeSize[table][i], f);
		}
	}
}

void JE_newLoadShapes( JE_byte table, char *shapefile )
{
	FILE *f;
	
	JE_newPurgeShapes(table);
	
	JE_resetFile(&f, shapefile);
	
	JE_newLoadShapesB(table, f);
	
	fclose(f);
}

void JE_newPurgeShapes( JE_byte table )
{
	for (int i = 0; i < maxShape[table]; i++)
	{
		if (shapeExist[table][i])
		{
			free(shapeArray[table][i]);
			
			shapeExist[table][i] = false;
		}
	}
	
	maxShape[table] = 0;
}


void blit_shape( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index )
{
	if (index >= maxShape[table] || !shapeExist[table][index])
	{
		assert(false);
		return;
	}
	
	Uint8 *data = shapeArray[table][index];
	unsigned int width = shapeX[table][index], height = shapeY[table][index];
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (int x_offset = 0, y_offset = 0; y_offset < height; data++)
	{
		switch (*data)
		{
			case 255:  // transparent pixels
				data++;  // next byte tells how many
				pixels += *data;
				x_offset += *data;
				break;
				
			case 254:  // next pixel row
				break;
				
			case 253:  // 1 transparent pixel
				pixels++;
				x_offset++;
				break;
				
			default:  // set a pixel
				if (pixels >= pixels_ul)
					return;
				if (pixels >= pixels_ll)
					*pixels = *data;
				
				pixels++;
				x_offset++;
				break;
		}
		if (*data == 254 || x_offset >= width)
		{
			pixels += surface->pitch - x_offset;
			x_offset = 0;
			y_offset++;
		}
	}
}

void blit_shape_blend( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index )
{
	if (index >= maxShape[table] || !shapeExist[table][index])
	{
		assert(false);
		return;
	}
	
	Uint8 *data = shapeArray[table][index];
	unsigned int width = shapeX[table][index], height = shapeY[table][index];
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (int x_offset = 0, y_offset = 0; y_offset < height; data++)
	{
		switch (*data)
		{
			case 255:  // transparent pixels
				data++;  // next byte tells how many
				pixels += *data;
				x_offset += *data;
				break;
				
			case 254:  // next pixel row
				break;
				
			case 253:  // 1 transparent pixel
				pixels++;
				x_offset++;
				break;
				
			default:  // set a pixel
				if (pixels >= pixels_ul)
					return;
				if (pixels >= pixels_ll)
					*pixels = (*data & 0xf0) | (((*pixels & 0x0f) + (*data & 0x0f)) / 2);
				
				pixels++;
				x_offset++;
				break;
		}
		if (*data == 254 || x_offset >= width)
		{
			pixels += surface->pitch - x_offset;
			x_offset = 0;
			y_offset++;
		}
	}
}

// unsafe because it doesn't check that value won't overflow into hue
// we can replace it when we know that we don't rely on that 'feature'
void blit_shape_hv_unsafe( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value )
{
	if (index >= maxShape[table] || !shapeExist[table][index])
	{
		assert(false);
		return;
	}
	
	Uint8 *data = shapeArray[table][index];
	unsigned int width = shapeX[table][index], height = shapeY[table][index];
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	hue <<= 4;
	
	for (int x_offset = 0, y_offset = 0; y_offset < height; data++)
	{
		switch (*data)
		{
			case 255:  // transparent pixels
				data++;  // next byte tells how many
				pixels += *data;
				x_offset += *data;
				break;
				
			case 254:  // next pixel row
				break;
				
			case 253:  // 1 transparent pixel
				pixels++;
				x_offset++;
				break;
				
			default:  // set a pixel
				if (pixels >= pixels_ul)
					return;
				if (pixels >= pixels_ll)
					*pixels = hue | ((*data & 0x0f) + value);
				
				pixels++;
				x_offset++;
				break;
		}
		if (*data == 254 || x_offset >= width)
		{
			pixels += surface->pitch - x_offset;
			x_offset = 0;
			y_offset++;
		}
	}
}

void blit_shape_hv( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value )
{
	if (index >= maxShape[table] || !shapeExist[table][index])
	{
		assert(false);
		return;
	}
	
	Uint8 *data = shapeArray[table][index];
	unsigned int width = shapeX[table][index], height = shapeY[table][index];
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	hue <<= 4;
	
	for (int x_offset = 0, y_offset = 0; y_offset < height; data++)
	{
		switch (*data)
		{
			case 255:  // transparent pixels
				data++;  // next byte tells how many
				pixels += *data;
				x_offset += *data;
				break;
				
			case 254:  // next pixel row
				break;
				
			case 253:  // 1 transparent pixel
				pixels++;
				x_offset++;
				break;
				
			default:  // set a pixel
				if (pixels >= pixels_ul)
					return;
				if (pixels >= pixels_ll)
				{
					Uint8 temp_value = (*data & 0x0f) + value;
					if (temp_value > 0xf)
						temp_value = (temp_value >= 0x1f) ? 0x0 : 0xf;
					
					*pixels = hue | temp_value;
				}
				
				pixels++;
				x_offset++;
				break;
		}
		if (*data == 254 || x_offset >= width)
		{
			pixels += surface->pitch - x_offset;
			x_offset = 0;
			y_offset++;
		}
	}
}

void blit_shape_hv_blend( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value )
{
	if (index >= maxShape[table] || !shapeExist[table][index])
	{
		assert(false);
		return;
	}
	
	Uint8 *data = shapeArray[table][index];
	unsigned int width = shapeX[table][index], height = shapeY[table][index];
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	hue <<= 4;
	
	for (int x_offset = 0, y_offset = 0; y_offset < height; data++)
	{
		switch (*data)
		{
			case 255:  // transparent pixels
				data++;  // next byte tells how many
				pixels += *data;
				x_offset += *data;
				break;
				
			case 254:  // next pixel row
				break;
				
			case 253:  // 1 transparent pixel
				pixels++;
				x_offset++;
				break;
				
			default:  // set a pixel
				if (pixels >= pixels_ul)
					return;
				if (pixels >= pixels_ll)
				{
					Uint8 temp_value = (*data & 0x0f) + value;
					if (temp_value > 0xf)
						temp_value = (temp_value >= 0x1f) ? 0x0 : 0xf;
					
					*pixels = hue | (((*pixels & 0x0f) + temp_value) / 2);
				}
				
				pixels++;
				x_offset++;
				break;
		}
		if (*data == 254 || x_offset >= width)
		{
			pixels += surface->pitch - x_offset;
			x_offset = 0;
			y_offset++;
		}
	}
}

void blit_shape_dark( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, bool black )
{
	if (index >= maxShape[table] || !shapeExist[table][index])
	{
		assert(false);
		return;
	}
	
	Uint8 *data = shapeArray[table][index];
	unsigned int width = shapeX[table][index], height = shapeY[table][index];
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (int x_offset = 0, y_offset = 0; y_offset < height; data++)
	{
		switch (*data)
		{
			case 255:  // transparent pixels
				data++;  // next byte tells how many
				pixels += *data;
				x_offset += *data;
				break;
				
			case 254:  // next pixel row
				break;
				
			case 253:  // 1 transparent pixel
				pixels++;
				x_offset++;
				break;
				
			default:  // set a pixel
				if (pixels >= pixels_ul)
					return;
				if (pixels >= pixels_ll)
					*pixels = black ? 0x00 : ((*pixels & 0xf0) | ((*pixels & 0x0f) / 2));
				
				pixels++;
				x_offset++;
				break;
		}
		if (*data == 254 || x_offset >= width)
		{
			pixels += surface->pitch - x_offset;
			x_offset = 0;
			y_offset++;
		}
	}
}


void JE_drawShapeTypeOne( JE_word x, JE_word y, JE_byte *shape )
{
	JE_word xloop = 0, yloop = 0;
	JE_byte *p = shape; /* shape pointer */
	Uint8 *s;   /* screen pointer, 8-bit specific */
	Uint8 *s_limit; /* buffer boundary */

	s = (Uint8 *)VGAScreen->pixels;
	s += y * VGAScreen->pitch + x;

	s_limit = (Uint8 *)tempScreenSeg->pixels;
	s_limit += tempScreenSeg->h * tempScreenSeg->pitch;

	for (yloop = 0; yloop < 28; yloop++)
	{
		for (xloop = 0; xloop < 24; xloop++)
		{
			if (s >= s_limit) return;
			*s = *p;
			s++; p++;
		}
		s -= 24;
		s += VGAScreen->pitch;
	}
}

void JE_grabShapeTypeOne( JE_word x, JE_word y, JE_byte *shape )
{
	JE_word xloop = 0, yloop = 0;
	JE_byte *p = shape; /* shape pointer */
	Uint8 *s;   /* screen pointer, 8-bit specific */
	Uint8 *s_limit; /* buffer boundary */

	s = (Uint8 *)VGAScreen->pixels;
	s += y * VGAScreen->pitch + x;

	s_limit = (Uint8 *)tempScreenSeg->pixels;
	s_limit += tempScreenSeg->h * tempScreenSeg->pitch;

	for (yloop = 0; yloop < 28; yloop++)
	{
		for (xloop = 0; xloop < 24; xloop++)
		{
			if (s >= s_limit) return;
			*p = *s;
			s++; p++;
		}
		s -= 24;
		s += VGAScreen->pitch;
	}
}

void JE_mouseStart( void )
{
	const JE_word mouseCursorGr[3] /* [1..3] */ = {273, 275, 277};

	JE_word tempW;

	if (mouseInstalled)
	{
		tempW = mouseCursorGr[mouseCursor];

		service_SDL_events(false);
		mouseButton = mousedown ? lastmouse_but : 0; /* incorrect, possibly unimportant */
		lastMouseX = mouse_x;
		lastMouseY = mouse_y;

		if (lastMouseX > 320 - 13)
		{
			lastMouseX = 320 - 13;
		}
		if (lastMouseY > 200 - 16)
		{
			lastMouseY = 200 - 16;
		}

		JE_grabShapeTypeOne(lastMouseX, lastMouseY, mouseGrabShape);

		/*JE_drawShape2x2shadow(lastmousex+2,lastmousey+2,tempW,shapes6);*/
		JE_drawShape2x2(lastMouseX, lastMouseY, tempW, shapes6);
	 }
}

void JE_mouseReplace( void )
{
	if (mouseInstalled)
	{
		JE_drawShapeTypeOne(lastMouseX, lastMouseY, mouseGrabShape);
	}
}

// kate: tab-width 4; vim: set noet:
