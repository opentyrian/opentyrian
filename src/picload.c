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
#include "picload.h"

#include "file.h"
#include "logging.h"
#include "memreader.h"
#include "memwriter.h"
#include "opentyr.h"
#include "palette.h"
#include "pcxmast.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

void JE_loadPic(SDL_Surface *screen, JE_byte id, JE_boolean storepal)
{
	const char *filename = "tyrian.pic";

	File file = dataFileOpen(filename, "rb");
	if (file.error)
	{
		logFatal("Failed to open file '%s': %s", filename, fileGetError(&file));
		exit(EXIT_FAILURE);
	}

	static bool first = true;
	if (first)
	{
		first = false;

		Uint16 count = fileReadU16(&file);
		assert(count == PCX_NUM);

		for (size_t i = 0; i < count && i < COUNTOF(pcxpos); ++i)
			pcxpos[i] = fileReadU32(&file);

		for (size_t i = count; i < COUNTOF(pcxpos); ++i)
			pcxpos[i] = fileGetLength(&file);

		if (file.error)
			logError("Failed to read from file '%s': %s", filename, fileGetError(&file));
	}

	if (id < 1 || id > PCX_NUM)
	{
		logError("Attempted to load picture %d, which does not exist.", id);
		return;
	}

	long position = pcxpos[id - 1];
	long endPosition = pcxpos[id];
	size_t size = endPosition > position ? endPosition - position : 0;

	fileSetPosition(&file, position);

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

	assert(screen->w == 320 && screen->h == 200 && screen->format->BytesPerPixel == 1);
	for (size_t y = 0; y < 200; ++y)
		memcpy((Uint8 *)screen->pixels + y * screen->pitch, image + y * 320, 320);

	free(image);

	memcpy(colors, palettes[pcxpal[id - 1]], sizeof(colors));

	if (storepal)
		set_palette(colors, 0, 255);
}
