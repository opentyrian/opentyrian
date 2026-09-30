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
#include "lvllib.h"

#include "file.h"
#include "logging.h"
#include "opentyr.h"

#include <assert.h>
#include <stdlib.h>

JE_LvlPosType lvlPos;

char levelFilename[13]; /* string [12] */ // FKA LvlLib.levelFile
JE_word lvlNum;

void analyzeLevel(void)
{
	File file = dataFileOpen(levelFilename, "rb");
	if (file.error)
	{
		logFatal("Failed to open file '%s': %s", levelFilename, fileGetError(&file));
		exit(EXIT_FAILURE);
	}

	lvlNum = fileReadU16(&file);
	assert(lvlNum <= COUNTOF(lvlPos) - 1);
	lvlNum = MIN(lvlNum, COUNTOF(lvlPos) - 1);

	for (size_t i = 0; i < lvlNum; ++i)
		lvlPos[i] = fileReadU32(&file);

	long fileLength = fileGetLength(&file);
	for (size_t i = lvlNum; i < COUNTOF(lvlPos); ++i)
		lvlPos[i] = fileLength;

	if (file.error)
	{
		logFatal("Failed to read from file '%s': %s", levelFilename, fileGetError(&file));
		exit(EXIT_FAILURE);
	}

	fileClose(&file);
}
