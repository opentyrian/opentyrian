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
#include "memwriter.h"

#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

MemWriter memWriterSplit(MemWriter *writer, size_t size)
{
	writer->error |= writer->size < size;

	if (writer->error)
		return (MemWriter) { NULL, 0, true };

	uint8_t *data = writer->data;

	writer->data += size;
	writer->size -= size;

	return (MemWriter) { data, size, false };
}

void memWriterSkip(MemWriter *writer, size_t size)
{
	writer->error |= writer->size < size;

	if (writer->error)
		return;

	writer->data += size;
	writer->size -= size;
}

void memWriteFill(MemWriter *writer, uint8_t value, size_t size)
{
	writer->error |= writer->size < size;

	if (writer->error)
		return;

	memset(writer->data, value, size);
	writer->data += size;
	writer->size -= size;
}

void memWriteRead(MemWriter *writer, MemReader *reader, size_t size)
{
	writer->error |= writer->size < size;
	reader->error |= reader->size < size;
	writer->error |= reader->error;
	reader->error = writer->error;

	if (writer->error)
		return;

	memcpy(writer->data, reader->data, size);
	writer->data += size;
	writer->size -= size;
	reader->data += size;
	reader->size -= size;
}

void memWriteU8(MemWriter *writer, uint8_t value)
{
	writer->error |= writer->size < 1;

	if (writer->error)
		return;

	writer->data[0] = value;
	writer->data += 1;
	writer->size -= 1;
}

void memWriteU8Array(MemWriter *writer, const uint8_t *values, size_t count)
{
	writer->error |= writer->size < count;

	if (writer->error)
		return;

	for (; count > 0; --count)
	{
		writer->data[0] = *values;
		values += 1;
		writer->data += 1;
		writer->size -= 1;
	}
}

static inline void storeU16LE(uint8_t *data, uint16_t value)
{
	// One byte at a time because some platforms do not allow unaligned stores.
	data[0] = value;
	data[1] = value >> 8;
}

void memWriteU16LE(MemWriter *writer, uint16_t value)
{
	writer->error |= writer->size < 2;

	if (writer->error)
		return;

	storeU16LE(writer->data, value);
	writer->data += 2;
	writer->size -= 2;
}

void memWriteU16LEArray(MemWriter *writer, const uint16_t *values, size_t count)
{
	writer->error |= writer->size / 2 < count;

	if (writer->error)
		return;

	for (; count > 0; --count)
	{
		storeU16LE(writer->data, *values);
		values += 1;
		writer->data += 2;
		writer->size -= 2;
	}
}

static inline void storeU32LE(uint8_t *data, uint32_t value)
{
	// One byte at a time because some platforms do not allow unaligned stores.
	data[0] = value;
	data[1] = value >> 8;
	data[2] = value >> 16;
	data[3] = value >> 24;
}

void memWriteU32LE(MemWriter *writer, uint32_t value)
{
	writer->error |= writer->size < 4;

	if (writer->error)
		return;

	storeU32LE(writer->data, value);
	writer->data += 4;
	writer->size -= 4;
}

void memWriteU32LEArray(MemWriter *writer, const uint32_t *values, size_t count)
{
	writer->error |= writer->size / 4 < count;

	if (writer->error)
		return;

	for (; count > 0; --count)
	{
		storeU32LE(writer->data, *values);
		values += 1;
		writer->data += 4;
		writer->size -= 4;
	}
}
