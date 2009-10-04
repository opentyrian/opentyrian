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
#include "file.h"
#include "opentyr.h"
#include "sprite.h"
#include "video.h"

#include <assert.h>

Sprite_array sprite_table[SPRITE_TABLES_MAX];

JE_byte *eShapes1 = NULL,
        *eShapes2 = NULL,
        *eShapes3 = NULL,
        *eShapes4 = NULL,
        *eShapes5 = NULL,
        *eShapes6 = NULL;
JE_byte *shapesC1 = NULL,
        *shapes6  = NULL,
        *shapes9  = NULL,
        *shapesW2 = NULL;

JE_word eShapes1Size,
        eShapes2Size,
        eShapes3Size,
        eShapes4Size,
        eShapes5Size,
        eShapes6Size,
        shapesC1Size,
        shapes6Size,
        shapes9Size,
        shapesW2Size;

void load_sprites_file( unsigned int table, const char *filename )
{
	free_sprites(table);
	
	FILE *f = dir_fopen_die(data_dir(), filename, "rb");
	
	load_sprites(table, f);
	
	fclose(f);
}

void load_sprites( unsigned int table, FILE *f )
{
	free_sprites(table);
	
	Uint16 temp;
	efread(&temp, sizeof(Uint16), 1, f);
	
	sprite_table[table].count = temp;
	
	for (unsigned int i = 0; i < sprite_table[table].count; ++i)
	{
		Sprite * const cur_sprite = sprite(table, i);
		
		if (!getc(f)) // sprite is empty
			continue;
		
		efread(&cur_sprite->width,  sizeof(Uint16), 1, f);
		efread(&cur_sprite->height, sizeof(Uint16), 1, f);
		efread(&cur_sprite->size,   sizeof(Uint16), 1, f);
		
		cur_sprite->data = malloc(cur_sprite->size);
		
		efread(cur_sprite->data, sizeof(Uint8), cur_sprite->size, f);
	}
}

void free_sprites( unsigned int table )
{
	for (unsigned int i = 0; i < sprite_table[table].count; ++i)
	{
		Sprite * const cur_sprite = sprite(table, i);
		
		cur_sprite->width  = 0;
		cur_sprite->height = 0;
		cur_sprite->size   = 0;
		
		free(cur_sprite->data);
		cur_sprite->data = NULL;
	}
	
	sprite_table[table].count = 0;
}

// does not clip on left or right edges of surface
void blit_shape( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index )
{
	if (index >= sprite_table[table].count || !sprite_exists(table, index))
	{
		assert(false);
		return;
	}
	
	const Sprite * const cur_sprite = sprite(table, index);
	
	Uint8 *data = cur_sprite->data;
	const unsigned int width = cur_sprite->width,
	                   height = cur_sprite->height;
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (unsigned int x_offset = 0, y_offset = 0; y_offset < height; data++)
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

// does not clip on left or right edges of surface
void blit_shape_blend( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index )
{
	if (index >= sprite_table[table].count || !sprite_exists(table, index))
	{
		assert(false);
		return;
	}
	
	const Sprite * const cur_sprite = sprite(table, index);
	
	Uint8 *data = cur_sprite->data;
	const unsigned int width = cur_sprite->width,
	                   height = cur_sprite->height;
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (unsigned int x_offset = 0, y_offset = 0; y_offset < height; data++)
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

// does not clip on left or right edges of surface
// unsafe because it doesn't check that value won't overflow into hue
// we can replace it when we know that we don't rely on that 'feature'
void blit_shape_hv_unsafe( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value )
{
	if (index >= sprite_table[table].count || !sprite_exists(table, index))
	{
		assert(false);
		return;
	}
	
	const Sprite * const cur_sprite = sprite(table, index);
	
	Uint8 *data = cur_sprite->data;
	const unsigned int width = cur_sprite->width,
	                   height = cur_sprite->height;
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	hue <<= 4;
	
	for (unsigned int x_offset = 0, y_offset = 0; y_offset < height; data++)
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

// does not clip on left or right edges of surface
void blit_shape_hv( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value )
{
	if (index >= sprite_table[table].count || !sprite_exists(table, index))
	{
		assert(false);
		return;
	}
	
	const Sprite * const cur_sprite = sprite(table, index);
	
	Uint8 *data = cur_sprite->data;
	const unsigned int width = cur_sprite->width,
	                   height = cur_sprite->height;
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	hue <<= 4;
	
	for (unsigned int x_offset = 0, y_offset = 0; y_offset < height; data++)
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

// does not clip on left or right edges of surface
void blit_shape_hv_blend( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value )
{
	if (index >= sprite_table[table].count || !sprite_exists(table, index))
	{
		assert(false);
		return;
	}
	
	const Sprite * const cur_sprite = sprite(table, index);
	
	Uint8 *data = cur_sprite->data;
	const unsigned int width = cur_sprite->width,
	                   height = cur_sprite->height;
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	hue <<= 4;
	
	for (unsigned int x_offset = 0, y_offset = 0; y_offset < height; data++)
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

// does not clip on left or right edges of surface
void blit_shape_dark( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, bool black )
{
	if (index >= sprite_table[table].count || !sprite_exists(table, index))
	{
		assert(false);
		return;
	}
	
	const Sprite * const cur_sprite = sprite(table, index);
	
	Uint8 *data = cur_sprite->data;
	const unsigned int width = cur_sprite->width,
	                   height = cur_sprite->height;
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *pixels = (Uint8 *)surface->pixels + (y * surface->pitch) + x,
	      *pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	      *pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (unsigned int x_offset = 0, y_offset = 0; y_offset < height; data++)
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


void JE_loadCompShapesB( JE_byte **shapes, FILE *f, JE_word shapeSize )
{
	*shapes = malloc(shapeSize);
	efread(*shapes, sizeof(JE_byte), shapeSize, f);
}

void JE_loadMainShapeTables( const char *shpfile )
{
	const int SHP_NUM = 12;
	
	FILE *f = dir_fopen_die(data_dir(), shpfile, "rb");
	
	JE_word shpNumb;
	JE_longint shpPos[SHP_NUM + 1]; // +1 for storing file length
	
	efread(&shpNumb, sizeof(JE_word), 1, f);
	assert(shpNumb + 1 <= COUNTOF(shpPos));
	
	for (int i = 0; i < shpNumb; i++)
	{
		efread(&shpPos[i], sizeof(JE_longint), 1, f);
	}
	fseek(f, 0, SEEK_END);
	shpPos[shpNumb] = ftell(f);
	
	int i;
	// fonts, interface, option sprites
	for (i = 0; i < 7; i++)
	{
		fseek(f, shpPos[i], SEEK_SET);
		load_sprites(i, f);
	}
	
	// player shot sprites
	shapesC1Size = shpPos[i + 1] - shpPos[i];
	JE_loadCompShapesB(&shapesC1, f, shapesC1Size);
	i++;
	
	// player ship sprites
	shapes9Size = shpPos[i + 1] - shpPos[i];
	JE_loadCompShapesB(&shapes9 , f, shapes9Size);
	i++;
	
	// power-up sprites
	eShapes6Size = shpPos[i + 1] - shpPos[i];
	JE_loadCompShapesB(&eShapes6, f, eShapes6Size);
	i++;
	
	// coins, datacubes, etc sprites
	eShapes5Size = shpPos[i + 1] - shpPos[i];
	JE_loadCompShapesB(&eShapes5, f, eShapes5Size);
	i++;
	
	// more player shot sprites
	shapesW2Size = shpPos[i + 1] - shpPos[i];
	JE_loadCompShapesB(&shapesW2, f, shapesW2Size);
	
	fclose(f);
}

void free_main_shape_tables( void )
{
	for (int i = 0; i < COUNTOF(sprite_table); ++i)
		free_sprites(i);
	
	free(shapesC1);
	shapesC1 = NULL;
	
	free(shapes9);
	shapes9 = NULL;
	
	free(eShapes6);
	eShapes6 = NULL;
	
	free(eShapes5);
	eShapes5 = NULL;
	
	free(shapesW2);
	shapesW2 = NULL;
}

void JE_loadCompShapes( JE_byte **shapes, JE_word *shapeSize, JE_char s )
{
	char buffer[11];
	sprintf(buffer, "newsh%c.shp", tolower(s));

	if (*shapes != NULL)
	{
		free(*shapes);
	}

	FILE *f = dir_fopen_die(data_dir(), buffer, "rb");

	fseek(f, 0, SEEK_END);
	*shapeSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	*shapes = malloc(*shapeSize);

	efread(*shapes, sizeof(JE_byte), *shapeSize, f);

	fclose(f);
}

void JE_drawShape2( int x, int y, int s_, JE_byte *shape )
{
	JE_byte *p; /* shape pointer */
	Uint8 *s; /* screen pointer, 8-bit specific */
	Uint8 *s_limit; /* buffer boundary */

	int i;

	s = (Uint8 *)VGAScreen->pixels;
	s += y * VGAScreen->pitch + x;

	s_limit = (Uint8 *)VGAScreen->pixels;
	s_limit += VGAScreen->h * VGAScreen->pitch;

	p = shape;
	p += SDL_SwapLE16(((JE_word *)p)[s_ - 1]);

	while (*p != 0x0f)
	{
		s += *p & 0x0f;
		i = (*p & 0xf0) >> 4;
		if (i)
		{
			while (i--)
			{
				p++;
				if (s >= s_limit)
					return;
				if ((void *)s >= VGAScreen->pixels)
					*s = *p;
				s++;
			}
		} else {
			s -= 12;
			s += VGAScreen->pitch;
		}
		p++;
	}
}

void JE_superDrawShape2( int x, int y, int s_, JE_byte *shape )
{
	JE_byte *p; /* shape pointer */
	Uint8 *s; /* screen pointer, 8-bit specific */
	Uint8 *s_limit; /* buffer boundary */

	int i;

	s = (Uint8 *)VGAScreen->pixels;
	s += y * VGAScreen->pitch + x;

	s_limit = (Uint8 *)VGAScreen->pixels;
	s_limit += VGAScreen->h * VGAScreen->pitch;

	p = shape;
	p += SDL_SwapLE16(((JE_word *)p)[s_ - 1]);

	while (*p != 0x0f)
	{
		s += *p & 0x0f;
		i = (*p & 0xf0) >> 4;
		if (i)
		{
			while (i--)
			{
				p++;
				if (s >= s_limit)
					return;
				if ((void *)s >= VGAScreen->pixels)
					*s = (((*p & 0x0f) + (*s & 0x0f)) >> 1) | (*p & 0xf0);
				s++;
			}
		} else {
			s -= 12;
			s += VGAScreen->pitch;
		}
		p++;
	}
}

void JE_drawShape2Shadow( int x, int y, int s_, JE_byte *shape )
{
	JE_byte *p; /* shape pointer */
	Uint8 *s; /* screen pointer, 8-bit specific */
	Uint8 *s_limit; /* buffer boundary */

	int i;

	s = (Uint8 *)VGAScreen->pixels;
	s += y * VGAScreen->pitch + x;

	s_limit = (Uint8 *)VGAScreen->pixels;
	s_limit += VGAScreen->h * VGAScreen->pitch;

	p = shape;
	p += SDL_SwapLE16(((JE_word *)p)[s_ - 1]);

	while (*p != 0x0f)
	{
		s += *p & 0x0f;
		i = (*p & 0xf0) >> 4;
		if (i)
		{
			while (i--)
			{
				p++;
				if (s >= s_limit)
					return;
				if ((void *)s >= VGAScreen->pixels)
					*s = ((*s & 0x0f) >> 1) + (*s & 0xf0);
				s++;
			}
		} else {
			s -= 12;
			s += VGAScreen->pitch;
		}
		p++;
	}
}

void JE_drawShape2x2( int x, int y, int s, JE_byte *shape )
{
	JE_drawShape2(x,    y,    s,    shape);
	JE_drawShape2(x+12, y,    s+1,  shape);
	JE_drawShape2(x,    y+14, s+19, shape);
	JE_drawShape2(x+12, y+14, s+20, shape);
}

void JE_superDrawShape2x2( int x, int y, int s, JE_byte *shape )
{
	JE_superDrawShape2(x,    y,    s,    shape);
	JE_superDrawShape2(x+12, y,    s+1,  shape);
	JE_superDrawShape2(x,    y+14, s+19, shape);
	JE_superDrawShape2(x+12, y+14, s+20, shape);
}

void JE_drawShape2x2Shadow( int x, int y, int s, JE_byte *shape )
{
	JE_drawShape2Shadow(x,    y,    s,    shape);
	JE_drawShape2Shadow(x+12, y,    s+1,  shape);
	JE_drawShape2Shadow(x,    y+14, s+19, shape);
	JE_drawShape2Shadow(x+12, y+14, s+20, shape);
}


// kate: tab-width 4; vim: set noet:
