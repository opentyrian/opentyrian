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
#ifndef MEMWRITER_H
#define MEMWRITER_H

#include "memreader.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct MemWriter
{
	uint8_t *data;
	size_t size;
	bool error;  // Indicates that an operation failed and no further operations will be performed.
} MemWriter;

MemWriter memWriterSplit(MemWriter *writer, size_t size);

void memWriterSkip(MemWriter *writer, size_t size);

void memWriteFill(MemWriter *writer, uint8_t value, size_t size);

void memWriteRead(MemWriter *writer, MemReader *reader, size_t size);

void memWriteU8(MemWriter *writer, uint8_t value);
void memWriteU8Array(MemWriter *writer, const uint8_t *values, size_t count);
void memWriteU16(MemWriter *writer, uint16_t value);
void memWriteU16Array(MemWriter *writer, const uint16_t *values, size_t count);
void memWriteU16BE(MemWriter *writer, uint16_t value);
void memWriteU32(MemWriter *writer, uint32_t value);
void memWriteU32Array(MemWriter *writer, const uint32_t *values, size_t count);

static inline void memWriteBool(MemWriter *writer, bool value)
{
	memWriteU8(writer, value ? 1 : 0);
}

static inline void memWriteChar(MemWriter *writer, char value)
{
	memWriteU8(writer, value);
}

static inline void memWriteCharArray(MemWriter *writer, const char *values, size_t count)
{
	memWriteU8Array(writer, (const uint8_t *)values, count);
}

static inline void memWriteS8(MemWriter *writer, int8_t value)
{
	memWriteU8(writer, value);
}

static inline void memWriteS8Array(MemWriter *writer, const int8_t *values, size_t count)
{
	memWriteU8Array(writer, (const uint8_t *)values, count);
}

static inline void memWriteS16(MemWriter *writer, int8_t value)
{
	memWriteU16(writer, value);
}

static inline void memWriteS16Array(MemWriter *writer, const int16_t *values, size_t count)
{
	memWriteU16Array(writer, (const uint16_t *)values, count);
}

static inline void memWriteS32(MemWriter *writer, int32_t value)
{
	memWriteU32(writer, value);
}

static inline void memWriteS32Array(MemWriter *writer, int32_t *values, size_t count)
{
	memWriteU32Array(writer, (const uint32_t *)values, count);
}

#endif /* MEMWRITER_H */
