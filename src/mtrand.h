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
#ifndef MTRAND_H
#define MTRAND_H

#include <stddef.h>

#define MT_RAND_MAX 0xffffffffUL
#define MT_RAND_N 624

typedef struct
{
	unsigned long x[MT_RAND_N];  /* the array for the state vector */
	size_t p0, p1, pm;           /* indexes into x */
} MtState;

/* An MtState must be seeded with mt_srand_r() before use. */
void mt_srand_r(MtState *mt, unsigned long s);
unsigned long mt_rand_r(MtState *mt);
float mt_rand_1_r(MtState *mt);
float mt_rand_lt1_r(MtState *mt);

/* The generator used by mt_rand() etc. */
extern MtState mt_global;

static inline void mt_srand(unsigned long s)
{
	mt_srand_r(&mt_global, s);
}

static inline unsigned long mt_rand(void)
{
	return mt_rand_r(&mt_global);
}

static inline float mt_rand_1(void)
{
	return mt_rand_1_r(&mt_global);
}

static inline float mt_rand_lt1(void)
{
	return mt_rand_lt1_r(&mt_global);
}

#endif /* MTRAND_H */
