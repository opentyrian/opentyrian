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
#ifndef SWAR_H
#define SWAR_H

#include <stdint.h>
#include <string.h>

 // Unsigned integer type with 1x, 2x, 4x, or 8x 8-bit lanes.
#if UINTPTR_MAX >= UINT64_MAX
#define Swar8 uint64_t
#define SWAR8_LANES 8
#elif UINTPTR_MAX >= UINT32_MAX
#define Swar8 uint32_t
#define SWAR8_LANES 4
#elif UINTPTR_MAX >= UINT16_MAX
#define Swar8 uint16_t
#define SWAR8_LANES 2
#else
#define Swar8 uint8_t
#define SWAR8_LANES 1
#endif

// Replicate a value into all lanes.
#define SWAR8(x) ((Swar8)((uint8_t)(x) * (Swar8)((Swar8)~(Swar8)0 / 0xFFU)))

#if SWAR8_LANES != 1

static inline Swar8 swar8Read(const uint8_t *src)
{
	Swar8 values;
	memcpy(&values, src, sizeof values);
	return values;
}

static inline void swar8Write(uint8_t *dst, Swar8 values)
{
	memcpy(dst, &values, sizeof values);
}

#else

static inline Swar8 swar8Read(const uint8_t *src)
{
	return *src;
}

static inline void swar8Write(uint8_t *dst, Swar8 values)
{
	*dst = values;
}

#endif

#endif /* SWAR_H */
