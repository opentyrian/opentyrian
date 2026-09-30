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
#include "demo.h"

#include "config.h"
#include "episodes.h"
#include "file.h"
#include "keyboard.h"
#include "logging.h"
#include "memreader.h"
#include "memwriter.h"
#include "mainint.h"
#include "mtrand.h"
#include "varz.h"

bool playDemo = false;
bool recordDemo = false;
bool stoppedDemo = false;

static Uint8 demoNum = 0;
static File demoFile = { 0 };

static Uint8 demoKeys;
static Uint16 demoKeysWait;  // FKA Varz.lastMoveWait

static const unsigned long seed = 32402394;

void beginPlayDemo(void)
{
	assert(demoFile.f == NULL);

	if (++demoNum > 5)
		demoNum = 1;

	char demoFilename[9];
	snprintf(demoFilename, sizeof(demoFilename), "demo.%d", demoNum);

	logDebug("Playing demo '%s'.", demoFilename);

	demoFile = dataFileOpen(demoFilename, "rb");
	if (demoFile.error)
	{
		logFatal("Failed to open file '%s': %s", demoFilename, fileGetError(&demoFile));
		exit(EXIT_FAILURE);
	}

	mt_srand(seed);

	difficultyLevel = DIFFICULTY_NORMAL;

	Uint8 newEpisode                           = fileReadU8(&demoFile);
	JE_initEpisode(newEpisode);
	fileReadCharArray(&demoFile, levelName, 10);
	levelName[10] = '\0';
	lvlFileNum                                 = fileReadU8(&demoFile);
	player[0].items.weapon[FRONT_WEAPON].id    = fileReadU8(&demoFile);
	player[0].items.weapon[REAR_WEAPON].id     = fileReadU8(&demoFile);
	player[0].items.super_arcade_mode          = fileReadU8(&demoFile);
	player[0].items.sidekick[LEFT_SIDEKICK]    = fileReadU8(&demoFile);
	player[0].items.sidekick[RIGHT_SIDEKICK]   = fileReadU8(&demoFile);
	player[0].items.generator                  = fileReadU8(&demoFile);
	player[0].items.sidekick_level             = fileReadU8(&demoFile);
	player[0].items.sidekick_series            = fileReadU8(&demoFile);
	initial_episode_num                        = fileReadU8(&demoFile);
	player[0].items.shield                     = fileReadU8(&demoFile);
	player[0].items.special                    = fileReadU8(&demoFile);
	player[0].items.ship                       = fileReadU8(&demoFile);
	player[0].items.weapon[FRONT_WEAPON].power = fileReadU8(&demoFile);
	player[0].items.weapon[REAR_WEAPON].power  = fileReadU8(&demoFile);
	fileReadExactly(&demoFile, (Uint8[3]) { 0, 0, 0 }, 3); // unused
	levelSong                                  = fileReadU8(&demoFile);

	Uint8 data[2];
	size_t size = fileReadAtMost(&demoFile, data, sizeof(data));
	if (demoFile.error)
	{
		logFatal("Failed to read from demo recording file: %s", fileGetError(&demoFile));
		exit(EXIT_FAILURE);
	}

	MemReader reader = { data, size, false };

	demoKeys = 0;
	demoKeysWait = memReadU16BE(&reader);

	assert(reader.size == 0 || reader.error);
}

bool playDemoKeys(void)
{
	while (demoKeysWait == 0)
	{
		Uint8 data[3];
		size_t size = fileReadAtMost(&demoFile, data, sizeof data);
		if (demoFile.error)
		{
			logFatal("Failed to read from demo recording file: %s", fileGetError(&demoFile));
			exit(EXIT_FAILURE);
		}

		if (size < sizeof data)
			return false;

		MemReader reader = { data, size, false };

		demoKeys = memReadU8(&reader);
		demoKeysWait = memReadU16BE(&reader);

		assert(reader.size == 0 && !reader.error);
	}

	demoKeysWait--;

	if (demoKeys & (1 << KEY_SETTING_UP))
		player[0].y -= CURRENT_KEY_SPEED;
	if (demoKeys & (1 << KEY_SETTING_DOWN))
		player[0].y += CURRENT_KEY_SPEED;

	if (demoKeys & (1 << KEY_SETTING_LEFT))
		player[0].x -= CURRENT_KEY_SPEED;
	if (demoKeys & (1 << KEY_SETTING_RIGHT))
		player[0].x += CURRENT_KEY_SPEED;

	button[0] = (bool)(demoKeys & (1 << KEY_SETTING_FIRE));
	button[3] = (bool)(demoKeys & (1 << KEY_SETTING_CHANGE_FIRE));
	button[1] = (bool)(demoKeys & (1 << KEY_SETTING_LEFT_SIDEKICK));
	button[2] = (bool)(demoKeys & (1 << KEY_SETTING_RIGHT_SIDEKICK));

	return true;
}

void endPlayDemo(void)
{
	fileClose(&demoFile);
}

void beginRecordDemo(void)
{
	assert(demoFile.f == NULL);

	char newDemoFilename[12];
	for (Uint8 newDemoNum = 1; ; ++newDemoNum)
	{
		snprintf(newDemoFilename, sizeof(newDemoFilename), "demorec.%d", newDemoNum);

		if (!userFileExists(newDemoFilename))
			break;

		if (newDemoNum == UINT8_MAX)
		{
			logFatal("No more demo recording files can be created.");
			exit(EXIT_FAILURE);
		}
	}

	logDebug("Recording demo '%s'.", newDemoFilename);

	demoFile = userFileOpen(newDemoFilename, "wb");
	if (demoFile.error)
	{
		logFatal("Failed to open file '%s': %s", newDemoFilename, fileGetError(&demoFile));
		exit(EXIT_FAILURE);
	}

	mt_srand(seed);

	difficultyLevel = DIFFICULTY_NORMAL;

	for (size_t i = strlen(levelName); i < sizeof(levelName); ++i)
		levelName[i] = '\0';

	fileWriteU8(&demoFile, episodeNum);
	fileWriteCharArray(&demoFile, levelName, 10);
	fileWriteU8(&demoFile, lvlFileNum);
	fileWriteU8(&demoFile, player[0].items.weapon[FRONT_WEAPON].id);
	fileWriteU8(&demoFile, player[0].items.weapon[REAR_WEAPON].id);
	fileWriteU8(&demoFile, player[0].items.super_arcade_mode);
	fileWriteU8(&demoFile, player[0].items.sidekick[LEFT_SIDEKICK]);
	fileWriteU8(&demoFile, player[0].items.sidekick[RIGHT_SIDEKICK]);
	fileWriteU8(&demoFile, player[0].items.generator);
	fileWriteU8(&demoFile, player[0].items.sidekick_level);
	fileWriteU8(&demoFile, player[0].items.sidekick_series);
	fileWriteU8(&demoFile, initial_episode_num);
	fileWriteU8(&demoFile, player[0].items.shield);
	fileWriteU8(&demoFile, player[0].items.special);
	fileWriteU8(&demoFile, player[0].items.ship);
	fileWriteU8(&demoFile, player[0].items.weapon[FRONT_WEAPON].power);
	fileWriteU8(&demoFile, player[0].items.weapon[REAR_WEAPON].power);
	fileWriteU8Array(&demoFile, (Uint8[]) { 0, 0, 0 }, 3);  // unused
	fileWriteU8(&demoFile, levelSong);

	demoKeys = 0;
	demoKeysWait = 0;
}

void recordDemoKeys(void)
{
	Uint8 oldDemoKeys = demoKeys;

	demoKeys = 0;
	for (size_t i = 0; i < 8; ++i)
		demoKeys |= keysactive[keySettings[i]] << i;

	if (demoKeys != oldDemoKeys || demoKeysWait == UINT16_MAX)
	{
		Uint8 data[3];

		MemWriter writer = { data, sizeof data, false };

		memWriteU16BE(&writer, demoKeysWait);
		memWriteU8(&writer, demoKeys);

		assert(writer.size == 0 && !writer.error);

		fileWrite(&demoFile, data, sizeof data);

		demoKeysWait = 0;
	}

	demoKeysWait++;
}

void endRecordDemo(void)
{
	Uint8 data[2];

	MemWriter writer = { data, sizeof data, false };

	memWriteU16BE(&writer, demoKeysWait);

	assert(writer.size == 0 && !writer.error);

	fileWrite(&demoFile, data, sizeof data);
	fileFlush(&demoFile);

	if (demoFile.error)
	{
		logFatal("Failed to write to demo recording file: %s", fileGetError(&demoFile));
		exit(EXIT_FAILURE);
	}

	fileClose(&demoFile);
}
