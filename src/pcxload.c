/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) The OpenTyrian Development Team
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
#include "pcxload.h"

#include "file.h"
#include "logging.h"
#include "memreader.h"
#include "memwriter.h"
#include "palette.h"
#include "video.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void JE_loadPCX(const char *filename) // this is only meant to load tshp2.pcx
{
	File file = dataFileOpen(filename, "rb");
	if (file.error)
	{
		logFatal("Failed to open file '%s': %s", filename, fileGetError(&file));
		exit(EXIT_FAILURE);
	}

	long fileLength = fileGetLength(&file);
	if (fileLength < 128 + 3 * 256)
	{
		logError("Failed to load file '%s'.", filename);
		return;
	}

	fileSetPosition(&file, fileLength - 3 * 256);

	Uint8 paletteData[3 * 256];

	fileReadExactly(&file, paletteData, sizeof paletteData);

	Uint8 *rgb = paletteData;
	for (size_t i = 0; i < 256; ++i, rgb += 3)
	{
		colors[i].r = rgb[0];
		colors[i].g = rgb[1];
		colors[i].b = rgb[2];
	}

	fileSetPosition(&file, 128);

	size_t size = fileLength - 128 - 3 * 256;
	Uint8 *data = malloc(size);
	fileReadExactly(&file, data, size);

	if (file.error)
		logError("Failed to read from file '%s': %s", filename, fileGetError(&file));

	fileClose(&file);

	const size_t imageSize = 320 * 200;
	Uint8 *image = calloc(imageSize, 1);

	MemReader reader = { data, size, false };
	MemWriter writer = { image, imageSize, false };

	while (!reader.error && writer.size > 0)
	{
		Uint8 b = memReadU8(&reader);

		if ((b & 0xC0) == 0xC0)
		{
			Uint8 size = b & 0x3F;
			Uint8 value = memReadU8(&reader);
			memWriteFill(&writer, value, size);
		}
		else
		{
			memWriteU8(&writer, b);
		}
	}

	free(data);

	SDL_Surface *const screen = VGAScreen;

	assert(screen->w == 320 && screen->h == 200 && screen->format->BytesPerPixel == 1);
	for (size_t y = 0; y < 200; ++y)
		memcpy((Uint8 *)screen->pixels + y * screen->pitch, image + y * 320, 320);

	free(image);
}
