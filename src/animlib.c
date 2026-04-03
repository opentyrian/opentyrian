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
#include "animlib.h"

#include "file.h"
#include "keyboard.h"
#include "memreader.h"
#include "memwriter.h"
#include "nortsong.h"
#include "palette.h"
#include "video.h"

#include <assert.h>
#include <string.h>

typedef struct FileHeader
{
	Uint16 pageCount;
	Uint32 recordCount;
} FileHeader;

typedef struct PageDescriptor
{
	Uint16 firstRecord;
	Uint16 recordCount;
	Uint16 recordsSize;
} PageDescriptor;

static bool readFileHeader(FileHeader *fileHeader, FILE *f)
{
	Uint8 data[256];
	size_t size = fread(data, 1, sizeof(data), f);

	MemReader reader = { data, size, false };

	memReaderSkip(&reader, 6);
	fileHeader->pageCount = memReadU16LE(&reader);
	fileHeader->recordCount = memReadU32LE(&reader);
	memReaderSkip(&reader, 244);

	assert(reader.size == 0 || reader.error);
	return !reader.error;
}

static bool readPalette(SDL_Color *palette, FILE *f)
{
	Uint8 data[4 * 256];
	size_t size = fread(data, 1, sizeof(data), f);

	MemReader reader = { data, size, false };

	for (size_t i = 0; i < 256; ++i)
	{
		palette[i].b = memReadU8(&reader);
		palette[i].g = memReadU8(&reader);
		palette[i].r = memReadU8(&reader);
		(void)memReadU8(&reader);
	}

	assert(reader.size == 0 || reader.error);
	return !reader.error;
}

static bool readPageDescriptors(PageDescriptor *pageDescriptors, FILE *f)
{
	Uint8 data[6 * 256];
	size_t size = fread(data, 1, sizeof(data), f);

	MemReader reader = { data, size, false };

	for (size_t i = 0; i < 256; ++i)
	{
		pageDescriptors[i].firstRecord = memReadU16LE(&reader);
		pageDescriptors[i].recordCount = memReadU16LE(&reader);
		pageDescriptors[i].recordsSize = memReadU16LE(&reader);
	}

	assert(reader.size == 0 || reader.error);
	return !reader.error;
}

static void decodeRunSkipDump(MemWriter *writer, MemReader *reader)
{
	while (!reader->error)
	{
		Uint8 opCode = memReadU8(reader);

		if (opCode == 0)  // 00: Short run
		{
			Uint8 size = memReadU8(reader);
			Uint8 value = memReadU8(reader);
			memWriteFill(writer, value, size);
		}
		else if (opCode > 0x80)  // 81..FF: Short skip
		{
			Uint8 size = opCode - 0x80;
			memWriterSkip(writer, size);
		}
		else if (opCode < 0x80)  // 01..7F: Short dump
		{
			Uint8 size = opCode;
			memWriteRead(writer, reader, size);
		}
		else  // 80: Long op
		{
			Uint16 opCode = memReadU16LE(reader);

			if (opCode == 0)  // 0000: Stop
			{
				return;
			}
			else if (opCode >= 0xC000)  // C000..FFFF: Long run
			{
				Uint16 size = opCode - 0xC000;
				Uint8 value = memReadU8(reader);
				memWriteFill(writer, value, size);
			}
			else if (opCode < 0x8000)  // 0001..7FFF: Long skip
			{
				Uint16 size = opCode;
				memWriterSkip(writer, size);
			}
			else  // 8000..BFFF: Long dump
			{
				Uint16 size = opCode - 0x8000;
				memWriteRead(writer, reader, size);
			}
		}
	}
}

void playAnim(const char *filename, Uint8 startingFrame, Uint8 speed)
{
	JE_clr256(VGAScreen);
	JE_showVGA();

	FILE *f = dir_fopen(data_dir(), filename, "rb");
	if (f == NULL)
		return;

	FileHeader fileHeader;
	bool success = readFileHeader(&fileHeader, f);

	SDL_Color palette[256];
	success = success && readPalette(palette, f);

	// The file contains a bunch of fixed-size pages.  Each page contains a sequence of records
	// (each corresponding to a frame of the animation).  The records within a page are in order.
	// The pages, however, are not in order.  The page descriptors specify the range of records that
	// each page contains.  (The page descriptors are also duplicated in the page header but are not
	// to be relied upon.)

	PageDescriptor pageDescriptors[256];
	success = success && readPageDescriptors(pageDescriptors, f);

	if (!success)
		goto fail;

	palette[0].r = 0;
	palette[0].g = 0;
	palette[0].b = 0;
	set_palette(palette, 0, 255);

	Uint16 firstRecord = 0;
	Uint16 recordCount = 0;

	const size_t dataSize = 1 << 16;
	Uint8 *data = malloc(dataSize);

	MemReader recordSizesReader = { 0 };
	MemReader recordsReader = { 0 };

	const size_t imageSize = 320 * 200;
	Uint8 *image = calloc(imageSize, 1);

	for (Uint16 record = startingFrame; record < fileHeader.recordCount - 1; ++record)
	{
		setFrameCount(speed);

		if (record < firstRecord ||
		    record - firstRecord >= recordCount)
		{
			// The record is not in the page that is loaded.
			// Find the page that contains the record and load it.

			for (size_t i = 0; i < fileHeader.pageCount; ++i)
			{
				PageDescriptor *pageDescriptor = &pageDescriptors[i];
				firstRecord = pageDescriptor->firstRecord;
				recordCount = pageDescriptor->recordCount;
				Uint16 recordsSize = pageDescriptor->recordsSize;

				if (record >= firstRecord &&
				    record - firstRecord < recordCount)
				{
					fseek(f, 0xB00 + (i << 16), SEEK_SET);

					size_t pageSize = 8 + 2 * (size_t)recordCount + recordsSize;
					size_t size = fread(data, 1, MIN(pageSize, dataSize), f);

					MemReader pageReader = { data, size, size != pageSize };

					// Skip the page header.
					memReaderSkip(&pageReader, 8);

					// The page header is followed by the record sizes and the records.
					recordSizesReader = memReaderSplit(&pageReader, 2 * (size_t)recordCount);
					recordsReader = pageReader;
					break;
				}
			}
		}

		while (record >= firstRecord && recordCount > 0)
		{
			firstRecord += 1;
			recordCount -= 1;

			Uint16 recordSize = memReadU16LE(&recordSizesReader);

			if (recordSizesReader.error)
				continue;

			MemReader recordReader = memReaderSplit(&recordsReader, recordSize);

			// Skip record if unneeded.
			if (record > firstRecord)
				continue;

			// Read record header.
			(void)memReadU8(&recordReader);     // Bitmap ID (assumed 'B')
			(void)memReadU8(&recordReader);     // Flags (assumed 0)
			(void)memReadU16LE(&recordReader);  // Body Type (assumed 1)

			// Decode record body to image.
			MemWriter imageWriter = { image, imageSize, false };
			decodeRunSkipDump(&imageWriter, &recordReader);

			assert(VGAScreen->w == 320 && VGAScreen->h == 200);
			for (size_t y = 0; y < 200; ++y)
				memcpy((Uint8 *)VGAScreen->pixels + y * VGAScreen->pitch, image + y * 320, 320);

			JE_showVGA();
		}

		if (waitUntilGetInputOrElapsed())
			break;
	}

	free(image);
	free(data);

fail:
	fclose(f);
}
