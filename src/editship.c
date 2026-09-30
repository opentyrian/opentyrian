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
#include "editship.h"

#include "file.h"

#define EXTRA_SHIPS_FILE_SIZE (sizeof(JE_ShipsType) - 4)

static const Uint8 extraCryptKey[10] /* [1..10] */ =
{
	58, 23, 16, 192, 254, 82, 113, 147, 62, 99
};

bool extraAvail = false;
JE_ShipsType extraShips;
Sprite2_array extraShapes;

static bool decryptExtraShipsData(Uint8 *data);

void loadExtraShapes(void)
{
	assert(extraShapes.data == NULL);

	File file = userFileOpen("newsh$.shp", "rb");
	if (file.error)
		return;

	long fileLength = fileGetLength(&file);
	if ((size_t)fileLength >= sizeof extraShips)
	{
		extraShapes.size = (size_t)fileLength - sizeof extraShips;
		extraShapes.data = malloc(extraShapes.size);
		fileReadExactly(&file, extraShapes.data, extraShapes.size);
		fileReadExactly(&file, extraShips, sizeof extraShips);

		extraAvail = !file.error;

		extraAvail &= decryptExtraShipsData(extraShips);
	}

	fileClose(&file);
}

bool decryptExtraShipsData(Uint8 *data)
{
	for (size_t i = EXTRA_SHIPS_FILE_SIZE - 1; ; --i)
	{
		data[i] ^= extraCryptKey[(i + 1) % 10];
		if (i > 0)
			data[i] ^= data[i - 1];
		else
			break;
	}

	Uint8 y;

	y = 0;
	for (size_t i = 0; i < EXTRA_SHIPS_FILE_SIZE; ++i)
		y += data[i];
	if (data[EXTRA_SHIPS_FILE_SIZE + 0] != y)
		return false;

	y = 0;
	for (size_t i = 0; i < EXTRA_SHIPS_FILE_SIZE; ++i)
		y -= data[i];
	if (data[EXTRA_SHIPS_FILE_SIZE + 1] != y)
		return false;

	y = 1;
	for (size_t i = 0; i < EXTRA_SHIPS_FILE_SIZE; ++i)
		y = y * data[i] + 1;
	if (data[EXTRA_SHIPS_FILE_SIZE + 2] != y)
		return false;

	y = 0;
	for (size_t i = 0; i < EXTRA_SHIPS_FILE_SIZE; ++i)
		y ^= data[i];
	if (data[EXTRA_SHIPS_FILE_SIZE + 3] != y)
		return false;

	return true;
}
