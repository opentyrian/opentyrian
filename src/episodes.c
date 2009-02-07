/* 
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
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

#include "config.h"
#include "episodes.h"
#include "error.h"
#include "lvllib.h"
#include "lvlmast.h"
#include "opentyr.h"


/* MAIN Weapons Data */
JE_WeaponPortType weaponPort;
JE_WeaponType     weapons;

/* Items */
JE_PowerType   powerSys;
JE_ShipType    ships;
JE_OptionType  options;
JE_ShieldType  shields;
JE_SpecialType special;

/* Enemy data */
JE_EnemyDatType enemyDat;

/* EPISODE variables */
JE_byte    episodeNum = 0;
JE_boolean episodeAvail[EPISODE_MAX]; /* [1..episodemax] */
char       macroFile[13], cubeFile[13]; /* string [12] */

JE_longint episode1DataLoc;

/* Tells the game whether the level currently loaded is a bonus level. */
JE_boolean bonusLevel;

/* Tells if the game jumped back to Episode 1 */
JE_boolean jumpBackToEpisode1;

void JE_loadItemDat( void )
{
	FILE *lvlFile;
	JE_word itemNum[7]; /* [1..7] */

	int i, j;

	if (episodeNum > 3)
	{
		JE_resetFile(&lvlFile, levelFile);
		fseek(lvlFile, lvlPos[lvlNum-1], SEEK_SET);
	} else {
		JE_resetFile(&lvlFile, "tyrian.hdt");
		efread(&episode1DataLoc, sizeof(JE_longint), 1, lvlFile);
		fseek(lvlFile, episode1DataLoc, SEEK_SET);
	}

	efread(&itemNum, sizeof(JE_word), 7, lvlFile);

	for (i = 0; i < WEAP_NUM + 1; i++)
	{
		efread(&weapons[i].drain,           sizeof(JE_word), 1, lvlFile);
		efread(&weapons[i].shotrepeat,      sizeof(JE_byte), 1, lvlFile);
		efread(&weapons[i].multi,           sizeof(JE_byte), 1, lvlFile);
		efread(&weapons[i].weapani,         sizeof(JE_word), 1, lvlFile);
		efread(&weapons[i].max,             sizeof(JE_byte), 1, lvlFile);
		efread(&weapons[i].tx,              sizeof(JE_byte), 1, lvlFile);
		efread(&weapons[i].ty,              sizeof(JE_byte), 1, lvlFile);
		efread(&weapons[i].aim,             sizeof(JE_byte), 1, lvlFile);
		efread(&weapons[i].attack,          sizeof(JE_byte), 8, lvlFile);
		efread(&weapons[i].del,             sizeof(JE_byte), 8, lvlFile);
		efread(&weapons[i].sx,              sizeof(JE_shortint), 8, lvlFile);
		efread(&weapons[i].sy,              sizeof(JE_shortint), 8, lvlFile);
		efread(&weapons[i].bx,              sizeof(JE_shortint), 8, lvlFile);
		efread(&weapons[i].by,              sizeof(JE_shortint), 8, lvlFile);
		efread(&weapons[i].sg,              sizeof(JE_word), 8, lvlFile);
		efread(&weapons[i].acceleration,    sizeof(JE_shortint), 1, lvlFile);
		efread(&weapons[i].accelerationx,   sizeof(JE_shortint), 1, lvlFile);
		efread(&weapons[i].circlesize,      sizeof(JE_byte), 1, lvlFile);
		efread(&weapons[i].sound,           sizeof(JE_byte), 1, lvlFile);
		efread(&weapons[i].trail,           sizeof(JE_byte), 1, lvlFile);
		efread(&weapons[i].shipblastfilter, sizeof(JE_byte), 1, lvlFile);
	}

	for (i = 0; i < PORT_NUM + 1; i++)
	{
		fseek(lvlFile, 1, SEEK_CUR); /* skip string length */
		efread(&weaponPort[i].name,        1, 30, lvlFile);
		weaponPort[i].name[30] = '\0';
		efread(&weaponPort[i].opnum,       sizeof(JE_byte), 1, lvlFile);
		for (j = 0; j < 2; j++)
		{
			efread(&weaponPort[i].op[j],   sizeof(JE_word), 11, lvlFile);
		}
		efread(&weaponPort[i].cost,        sizeof(JE_word), 1, lvlFile);
		efread(&weaponPort[i].itemgraphic, sizeof(JE_word), 1, lvlFile);
		efread(&weaponPort[i].poweruse,    sizeof(JE_word), 1, lvlFile);
	}

	for (i = 0; i < SPECIAL_NUM + 1; i++)
	{
		fseek(lvlFile, 1, SEEK_CUR); /* skip string length */
		efread(&special[i].name,        1, 30, lvlFile);
		special[i].name[30] = '\0';
		efread(&special[i].itemgraphic, sizeof(JE_word), 1, lvlFile);
		efread(&special[i].pwr,         sizeof(JE_byte), 1, lvlFile);
		efread(&special[i].stype,       sizeof(JE_byte), 1, lvlFile);
		efread(&special[i].wpn,         sizeof(JE_word), 1, lvlFile);
	}

	for (i = 0; i < POWER_NUM + 1; i++)
	{
		fseek(lvlFile, 1, SEEK_CUR); /* skip string length */
		efread(&powerSys[i].name,        1, 30, lvlFile);
		powerSys[i].name[30] = '\0';
		efread(&powerSys[i].itemgraphic, sizeof(JE_word), 1, lvlFile);
		efread(&powerSys[i].power,       sizeof(JE_shortint), 1, lvlFile);
		efread(&powerSys[i].speed,       sizeof(JE_byte), 1, lvlFile);
		efread(&powerSys[i].cost,        sizeof(JE_word), 1, lvlFile);
	}

	for (i = 0; i < SHIP_NUM + 1; i++)
	{
		fseek(lvlFile, 1, SEEK_CUR); /* skip string length */
		efread(&ships[i].name,           1, 30, lvlFile);
		ships[i].name[30] = '\0';
		efread(&ships[i].shipgraphic,    sizeof(JE_word), 1, lvlFile);
		efread(&ships[i].itemgraphic,    sizeof(JE_word), 1, lvlFile);
		efread(&ships[i].ani,            sizeof(JE_byte), 1, lvlFile);
		efread(&ships[i].spd,            sizeof(JE_shortint), 1, lvlFile);
		efread(&ships[i].dmg,            sizeof(JE_byte), 1, lvlFile);
		efread(&ships[i].cost,           sizeof(JE_word), 1, lvlFile);
		efread(&ships[i].bigshipgraphic, sizeof(JE_byte), 1, lvlFile);
	}

	for (i = 0; i < OPTION_NUM + 1; i++)
	{
		fseek(lvlFile, 1, SEEK_CUR); /* skip string length */
		efread(&options[i].name,        1, 30, lvlFile);
		options[i].name[30] = '\0';
		efread(&options[i].pwr,         sizeof(JE_byte), 1, lvlFile);
		efread(&options[i].itemgraphic, sizeof(JE_word), 1, lvlFile);
		efread(&options[i].cost,        sizeof(JE_word), 1, lvlFile);
		efread(&options[i].tr,          sizeof(JE_byte), 1, lvlFile);
		efread(&options[i].option,      sizeof(JE_byte), 1, lvlFile);
		efread(&options[i].opspd,       sizeof(JE_shortint), 1, lvlFile);
		efread(&options[i].ani,         sizeof(JE_byte), 1, lvlFile);
		efread(&options[i].gr,          sizeof(JE_word), 20, lvlFile);
		efread(&options[i].wport,       sizeof(JE_byte), 1, lvlFile);
		efread(&options[i].wpnum,       sizeof(JE_word), 1, lvlFile);
		efread(&options[i].ammo,        sizeof(JE_byte), 1, lvlFile);
		efread(&options[i].stop,        1, 1, lvlFile); /* override sizeof(JE_boolean) */
		efread(&options[i].icongr,      sizeof(JE_byte), 1, lvlFile);
	}

	for (i = 0; i < SHIELD_NUM + 1; i++)
	{
		fseek(lvlFile, 1, SEEK_CUR); /* skip string length */
		efread(&shields[i].name,        1, 30, lvlFile);
		shields[i].name[30] = '\0';
		efread(&shields[i].tpwr,        sizeof(JE_byte), 1, lvlFile);
		efread(&shields[i].mpwr,        sizeof(JE_byte), 1, lvlFile);
		efread(&shields[i].itemgraphic, sizeof(JE_word), 1, lvlFile);
		efread(&shields[i].cost,        sizeof(JE_word), 1, lvlFile);
	}

	for (i = 0; i < ENEMY_NUM + 1; i++)
	{
		efread(&enemyDat[i].ani,           sizeof(JE_byte), 1, lvlFile);
		efread(&enemyDat[i].tur,           sizeof(JE_byte), 3, lvlFile);
		efread(&enemyDat[i].freq,          sizeof(JE_byte), 3, lvlFile);
		efread(&enemyDat[i].xmove,         sizeof(JE_shortint), 1, lvlFile);
		efread(&enemyDat[i].ymove,         sizeof(JE_shortint), 1, lvlFile);
		efread(&enemyDat[i].xaccel,        sizeof(JE_shortint), 1, lvlFile);
		efread(&enemyDat[i].yaccel,        sizeof(JE_shortint), 1, lvlFile);
		efread(&enemyDat[i].xcaccel,       sizeof(JE_shortint), 1, lvlFile);
		efread(&enemyDat[i].ycaccel,       sizeof(JE_shortint), 1, lvlFile);
		efread(&enemyDat[i].startx,        sizeof(JE_integer), 1, lvlFile);
		efread(&enemyDat[i].starty,        sizeof(JE_integer), 1, lvlFile);
		efread(&enemyDat[i].startxc,       sizeof(JE_shortint), 1, lvlFile);
		efread(&enemyDat[i].startyc,       sizeof(JE_shortint), 1, lvlFile);
		efread(&enemyDat[i].armor,         sizeof(JE_byte), 1, lvlFile);
		efread(&enemyDat[i].esize,         sizeof(JE_byte), 1, lvlFile);
		efread(&enemyDat[i].egraphic,      sizeof(JE_word), 20, lvlFile);
		efread(&enemyDat[i].explosiontype, sizeof(JE_byte), 1, lvlFile);
		efread(&enemyDat[i].animate,       sizeof(JE_byte), 1, lvlFile);
		efread(&enemyDat[i].shapebank,     sizeof(JE_byte), 1, lvlFile);
		efread(&enemyDat[i].xrev,          sizeof(JE_shortint), 1, lvlFile);
		efread(&enemyDat[i].yrev,          sizeof(JE_shortint), 1, lvlFile);
		efread(&enemyDat[i].dgr,           sizeof(JE_word), 1, lvlFile);
		efread(&enemyDat[i].dlevel,        sizeof(JE_shortint), 1, lvlFile);
		efread(&enemyDat[i].dani,          sizeof(JE_shortint), 1, lvlFile);
		efread(&enemyDat[i].elaunchfreq,   sizeof(JE_byte), 1, lvlFile);
		efread(&enemyDat[i].elaunchtype,   sizeof(JE_word), 1, lvlFile);
		efread(&enemyDat[i].value,         sizeof(JE_integer), 1, lvlFile);
		efread(&enemyDat[i].eenemydie,     sizeof(JE_word), 1, lvlFile);
	}

	fclose(lvlFile);
}

void JE_initEpisode( JE_byte newEpisode )
{
	if (newEpisode != episodeNum)
	{
		episodeNum = newEpisode;

		sprintf(levelFile, "tyrian%d.lvl", episodeNum);
		sprintf(cubeFile,  "cubetxt%d.dat", episodeNum);
		sprintf(macroFile, "levels%d.dat", episodeNum);

		JE_analyzeLevel();
		JE_loadItemDat();
	}
}

void JE_scanForEpisodes( void )
{
	JE_byte temp;

	char buf[sizeof(dir) + 11];

	JE_findTyrian("tyrian1.lvl"); /* need to know where to scan */

	for (temp = 0; temp < EPISODE_MAX; temp++)
	{
		sprintf(buf, "%styrian%d.lvl", dir, temp + 1);
		episodeAvail[temp] = JE_find(buf);
	}
}

unsigned int JE_findNextEpisode( void )
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

// kate: tab-width 4; vim: set noet:
