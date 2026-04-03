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
#include "memreader.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

MemReader memReaderSplit(MemReader *reader, size_t size)
{
	reader->error |= reader->size < size;

	if (reader->error)
		return (MemReader) { NULL, 0, true };

	const uint8_t *data = reader->data;

	reader->data += size;
	reader->size -= size;

	return (MemReader) { data, size, false };
}

void memReaderSkip(MemReader *reader, size_t size)
{
	reader->error |= reader->size < size;

	if (reader->error)
		return;

	reader->data += size;
	reader->size -= size;
}

uint8_t memReadU8(MemReader *reader)
{
	reader->error |= reader->size < 1;

	if (reader->error)
		return 0;

	uint8_t value = reader->data[0];
	reader->data += 1;
	reader->size -= 1;
	return value;
}

void memReadU8Array(MemReader *reader, uint8_t *values, size_t count)
{
	reader->error |= reader->size < count;

	if (reader->error)
	{
		for (; count > 0; --count)
		{
			*values = 0;
			values += 1;
		}
		return;
	}

	for (; count > 0; --count)
	{
		*values = reader->data[0];
		values += 1;
		reader->data += 1;
		reader->size -= 1;
	}
}

static inline uint16_t loadU16LE(const uint8_t *data)
{
	// One byte at a time because some platforms do not allow unaligned loads.
	return ((uint16_t)data[0]) |
	       ((uint16_t)data[1] << 8);
}

uint16_t memReadU16LE(MemReader *reader)
{
	reader->error |= reader->size < 2;

	if (reader->error)
		return 0;

	uint16_t value = loadU16LE(reader->data);
	reader->data += 2;
	reader->size -= 2;
	return value;
}

void memReadU16LEArray(MemReader *reader, uint16_t *values, size_t count)
{
	reader->error |= reader->size / 2 < count;

	if (reader->error)
	{
		for (; count > 0; --count)
		{
			*values = 0;
			values += 1;
		}
		return;
	}

	for (; count > 0; --count)
	{
		*values = loadU16LE(reader->data);
		values += 1;
		reader->data += 2;
		reader->size -= 2;
	}
}

static inline uint32_t loadU32LE(const uint8_t *data)
{
	// One byte at a time because some platforms do not allow unaligned loads.
	return ((uint32_t)data[0]) |
	       ((uint32_t)data[1] << 8) |
	       ((uint32_t)data[2] << 16) |
	       ((uint32_t)data[3] << 24);
}

uint32_t memReadU32LE(MemReader *reader)
{
	reader->error |= reader->size < 4;

	if (reader->error)
		return 0;

	uint32_t value = loadU32LE(reader->data);
	reader->data += 4;
	reader->size -= 4;
	return value;
}

void memReadU32LEArray(MemReader *reader, uint32_t *values, size_t count)
{
	reader->error |= reader->size / 4 < count;

	if (reader->error)
	{
		for (; count > 0; --count)
		{
			*values = 0;
			values += 1;
		}
		return;
	}

	for (; count > 0; --count)
	{
		*values = loadU32LE(reader->data);
		values += 1;
		reader->data += 4;
		reader->size -= 4;
	}
}
