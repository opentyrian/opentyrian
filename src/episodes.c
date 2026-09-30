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
#include "episodes.h"

#include "config.h"
#include "file.h"
#include "logging.h"
#include "lvllib.h"
#include "lvlmast.h"
#include "opentyr.h"

/* MAIN Weapons Data */
JE_WeaponPortType weaponPort;
JE_WeaponType     weapons[WEAP_NUM + 1]; /* [0..weapnum] */

/* Items */
JE_PowerType   powerSys;
JE_ShipType    ships;
JE_OptionType  options[OPTION_NUM + 1]; /* [0..optionnum] */
JE_ShieldType  shields;
JE_SpecialType special;

/* Enemy data */
JE_EnemyDatType enemyDat;

/* EPISODE variables */
JE_byte    initial_episode_num, episodeNum = 0;
JE_boolean episodeAvail[EPISODE_MAX]; /* [1..episodemax] */
char       episodeFilename[13];  // FKA Episodes.macroFile
char       cubeFilename[13];  // FKA Episodes.cubeFile

/* Tells the game whether the level currently loaded is a bonus level. */
JE_boolean bonusLevel;

/* Tells if the game jumped back to Episode 1 */
JE_boolean jumpBackToEpisode1;

void JE_loadItemDat(void)
{
	const char *filename;

	File file;
	
	if (episodeNum <= 3)
	{
		filename = "tyrian.hdt";

		file = dataFileOpen(filename, "rb");
		if (file.error)
		{
			logFatal("Failed to open file '%s': %s", filename, fileGetError(&file));
			exit(EXIT_FAILURE);
		}

		long position = fileReadU32(&file);

		fileSetPosition(&file, position);
	}
	else
	{
		// Episode 4 stores item data in the level file.
		filename = levelFilename;

		file = dataFileOpen(filename, "rb");
		if (file.error)
		{
			logFatal("Failed to open file '%s': %s", filename, fileGetError(&file));
			exit(EXIT_FAILURE);
		}

		fileSetPosition(&file, lvlPos[lvlNum-1]);
	}

	Uint16 counts[7];
	fileReadU16Array(&file, counts, COUNTOF(counts));

	assert(counts[0] == WEAP_NUM);
	assert(counts[1] == PORT_NUM);
	assert(counts[2] == POWER_NUM);
	assert(counts[3] == SHIP_NUM);
	assert(counts[4] == OPTION_NUM);
	assert(counts[5] == SHIELD_NUM);
	assert(counts[6] == ENEMY_NUM);

	for (size_t i = 0; i < WEAP_NUM + 1; ++i)
	{
		fileReadU16Array(&file, &weapons[i].drain,           1);
		fileReadU8Array( &file, &weapons[i].shotrepeat,      1);
		fileReadU8Array( &file, &weapons[i].multi,           1);
		fileReadU16Array(&file, &weapons[i].weapani,         1);
		fileReadU8Array( &file, &weapons[i].max,             1);
		fileReadU8Array( &file, &weapons[i].tx,              1);
		fileReadU8Array( &file, &weapons[i].ty,              1);
		fileReadU8Array( &file, &weapons[i].aim,             1);
		fileReadU8Array( &file,  weapons[i].attack,          8);
		fileReadU8Array( &file,  weapons[i].del,             8);
		fileReadS8Array( &file,  weapons[i].sx,              8);
		fileReadS8Array( &file,  weapons[i].sy,              8);
		fileReadS8Array( &file,  weapons[i].bx,              8);
		fileReadS8Array( &file,  weapons[i].by,              8);
		fileReadU16Array(&file,  weapons[i].sg,              8);
		fileReadS8Array( &file, &weapons[i].acceleration,    1);
		fileReadS8Array( &file, &weapons[i].accelerationx,   1);
		fileReadU8Array( &file, &weapons[i].circlesize,      1);
		fileReadU8Array( &file, &weapons[i].sound,           1);
		fileReadU8Array( &file, &weapons[i].trail,           1);
		fileReadU8Array( &file, &weapons[i].shipblastfilter, 1);
	}
	
	for (size_t i = 0; i < PORT_NUM + 1; ++i)
	{
		Uint8 nameLen = fileReadU8(&file);
		fileReadCharArray(&file,  weaponPort[i].name,       30);
		weaponPort[i].name[MIN(nameLen, 30)] = '\0';
		fileReadU8Array(  &file, &weaponPort[i].opnum,       1);
		fileReadU16Array( &file,  weaponPort[i].op[0],      11);
		fileReadU16Array( &file,  weaponPort[i].op[1],      11);
		fileReadU16Array( &file, &weaponPort[i].cost,        1);
		fileReadU16Array( &file, &weaponPort[i].itemgraphic, 1);
		fileReadU16Array( &file, &weaponPort[i].poweruse,    1);
	}

	for (size_t i = 0; i < SPECIAL_NUM + 1; ++i)
	{
		Uint8 nameLen = fileReadU8(&file);
		fileReadCharArray(&file,  special[i].name,       30);
		special[i].name[MIN(nameLen, 30)] = '\0';
		fileReadU16Array( &file, &special[i].itemgraphic, 1);
		fileReadU8Array(  &file, &special[i].pwr,         1);
		fileReadU8Array(  &file, &special[i].stype,       1);
		fileReadU16Array( &file, &special[i].wpn,         1);
	}

	for (size_t i = 0; i < POWER_NUM + 1; ++i)
	{
		Uint8 nameLen = fileReadU8(&file);
		fileReadCharArray(&file,  powerSys[i].name,       30);
		powerSys[i].name[MIN(nameLen, 30)] = '\0';
		fileReadU16Array( &file, &powerSys[i].itemgraphic, 1);
		fileReadU8Array(  &file, &powerSys[i].power,       1);
		fileReadS8Array(  &file, &powerSys[i].speed,       1);
		fileReadU16Array( &file, &powerSys[i].cost,        1);
	}

	for (size_t i = 0; i < SHIP_NUM + 1; ++i)
	{
		Uint8 nameLen = fileReadU8(&file);
		fileReadCharArray(&file,  ships[i].name,          30);
		ships[i].name[MIN(nameLen, 30)] = '\0';
		fileReadU16Array( &file, &ships[i].shipgraphic,    1);
		fileReadU16Array( &file, &ships[i].itemgraphic,    1);
		fileReadU8Array(  &file, &ships[i].ani,            1);
		fileReadS8Array(  &file, &ships[i].spd,            1);
		fileReadU8Array(  &file, &ships[i].dmg,            1);
		fileReadU16Array( &file, &ships[i].cost,           1);
		fileReadU8Array(  &file, &ships[i].bigshipgraphic, 1);
	}

	for (size_t i = 0; i < OPTION_NUM + 1; ++i)
	{
		Uint8 nameLen = fileReadU8(&file);
		fileReadCharArray(&file,  options[i].name,       30);
		options[i].name[MIN(nameLen, 30)] = '\0';
		fileReadU8Array(  &file, &options[i].pwr,         1);
		fileReadU16Array( &file, &options[i].itemgraphic, 1);
		fileReadU16Array( &file, &options[i].cost,        1);
		fileReadU8Array(  &file, &options[i].tr,          1);
		fileReadU8Array(  &file, &options[i].option,      1);
		fileReadS8Array(  &file, &options[i].opspd,       1);
		fileReadU8Array(  &file, &options[i].ani,         1);
		fileReadU16Array( &file,  options[i].gr,         20);
		fileReadU8Array(  &file, &options[i].wport,       1);
		fileReadU16Array( &file, &options[i].wpnum,       1);
		fileReadU8Array(  &file, &options[i].ammo,        1);
		fileReadBoolArray(&file, &options[i].stop,        1);
		fileReadU8Array(  &file, &options[i].icongr,      1);
	}

	for (size_t i = 0; i < SHIELD_NUM + 1; ++i)
	{
		Uint8 nameLen = fileReadU8(&file);
		fileReadCharArray(&file,  shields[i].name,       30);
		shields[i].name[MIN(nameLen, 30)] = '\0';
		fileReadU8Array(  &file, &shields[i].tpwr,        1);
		fileReadU8Array(  &file, &shields[i].mpwr,        1);
		fileReadU16Array( &file, &shields[i].itemgraphic, 1);
		fileReadU16Array( &file, &shields[i].cost,        1);
	}
	
	for (size_t i = 0; i < ENEMY_NUM + 1; ++i)
	{
		fileReadU8Array( &file, &enemyDat[i].ani,           1);
		fileReadU8Array( &file,  enemyDat[i].tur,           3);
		fileReadU8Array( &file,  enemyDat[i].freq,          3);
		fileReadS8Array( &file, &enemyDat[i].xmove,         1);
		fileReadS8Array( &file, &enemyDat[i].ymove,         1);
		fileReadS8Array( &file, &enemyDat[i].xaccel,        1);
		fileReadS8Array( &file, &enemyDat[i].yaccel,        1);
		fileReadS8Array( &file, &enemyDat[i].xcaccel,       1);
		fileReadS8Array( &file, &enemyDat[i].ycaccel,       1);
		fileReadS16Array(&file, &enemyDat[i].startx,        1);
		fileReadS16Array(&file, &enemyDat[i].starty,        1);
		fileReadS8Array( &file, &enemyDat[i].startxc,       1);
		fileReadS8Array( &file, &enemyDat[i].startyc,       1);
		fileReadU8Array( &file, &enemyDat[i].armor,         1);
		fileReadU8Array( &file, &enemyDat[i].esize,         1);
		fileReadU16Array(&file,  enemyDat[i].egraphic,     20);
		fileReadU8Array( &file, &enemyDat[i].explosiontype, 1);
		fileReadU8Array( &file, &enemyDat[i].animate,       1);
		fileReadU8Array( &file, &enemyDat[i].shapebank,     1);
		fileReadS8Array( &file, &enemyDat[i].xrev,          1);
		fileReadS8Array( &file, &enemyDat[i].yrev,          1);
		fileReadU16Array(&file, &enemyDat[i].dgr,           1);
		fileReadS8Array( &file, &enemyDat[i].dlevel,        1);
		fileReadS8Array( &file, &enemyDat[i].dani,          1);
		fileReadU8Array( &file, &enemyDat[i].elaunchfreq,   1);
		fileReadU16Array(&file, &enemyDat[i].elaunchtype,   1);
		fileReadS16Array(&file, &enemyDat[i].value,         1);
		fileReadU16Array(&file, &enemyDat[i].eenemydie,     1);
	}
	
	if (file.error)
	{
		logFatal("Failed to read from file '%s': %s", filename, fileGetError(&file));
		exit(EXIT_FAILURE);
	}

	fileClose(&file);
}

void JE_initEpisode(JE_byte newEpisode)
{
	if (newEpisode == episodeNum)
		return;
	
	episodeNum = newEpisode;
	
	snprintf(levelFilename,   sizeof levelFilename,   "tyrian%d.lvl",  episodeNum);
	snprintf(cubeFilename,    sizeof cubeFilename,    "cubetxt%d.dat", episodeNum);
	snprintf(episodeFilename, sizeof episodeFilename, "levels%d.dat",  episodeNum);
	
	analyzeLevel();
	JE_loadItemDat();
}

void JE_scanForEpisodes(void)
{
	for (int i = 0; i < EPISODE_MAX; ++i)
	{
		char filename[13];
		snprintf(filename, sizeof filename, "tyrian%d.lvl", i + 1);
		episodeAvail[i] = dataFileExists(filename);
	}
}

unsigned int JE_findNextEpisode(void)
{
	unsigned int newEpisode = episodeNum;
	
	jumpBackToEpisode1 = false;
	
	while (true)
	{
		newEpisode++;
		
		if (newEpisode > EPISODE_MAX)
		{
			newEpisode = 1;
			jumpBackToEpisode1 = true;
			gameHasRepeated = true;
		}
		
		if (episodeAvail[newEpisode-1] || newEpisode == episodeNum)
		{
			break;
		}
	}
	
	return newEpisode;
}
