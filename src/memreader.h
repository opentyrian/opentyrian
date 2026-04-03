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
#ifndef MEMREADER_H
#define MEMREADER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct MemReader
{
	const uint8_t *data;
	size_t size;
	bool error;  // Indicates that an operation failed and no further operations will be performed.
} MemReader;

MemReader memReaderSplit(MemReader *reader, size_t size);

void memReaderSkip(MemReader *reader, size_t size);

uint8_t memReadU8(MemReader *reader);
void memReadU8Array(MemReader *reader, uint8_t *values, size_t count);
uint16_t memReadU16LE(MemReader *reader);
void memReadU16LEArray(MemReader *reader, uint16_t *values, size_t count);
uint32_t memReadU32LE(MemReader *reader);
void memReadU32LEArray(MemReader *reader, uint32_t *values, size_t count);

static inline bool memReadBool(MemReader *reader)
{
	return memReadU8(reader) != 0;
}

static inline char memReadChar(MemReader *reader)
{
	return memReadU8(reader);
}

static inline void memReadCharArray(MemReader *reader, char *values, size_t count)
{
	memReadU8Array(reader, (uint8_t *)values, count);
}

static inline int8_t memReadS8(MemReader *reader)
{
	return memReadU8(reader);
}

static inline void memReadS8Array(MemReader *reader, int8_t *values, size_t count)
{
	memReadU8Array(reader, (uint8_t *)values, count);
}

static inline int8_t memReadS16LE(MemReader *reader)
{
	return memReadU16LE(reader);
}

static inline void memReadS16LEArray(MemReader *reader, int16_t *values, size_t count)
{
	memReadU16LEArray(reader, (uint16_t *)values, count);
}

static inline int32_t memReadS32LE(MemReader *reader)
{
	return memReadU32LE(reader);
}

static inline void memReadS32LEArray(MemReader *reader, int32_t *values, size_t count)
{
	memReadU32LEArray(reader, (uint32_t *)values, count);
}

#endif /* MEMREADER_H */
