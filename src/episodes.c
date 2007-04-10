/* vim: set noet:
 *
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Team
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
#include "opentyr.h"
#include "error.h"
#include "lvlmast.h"
#include "lvllib.h"

#define NO_EXTERNS
#include "episodes.h"
#undef NO_EXTERNS

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
		JE_resetFileExt(&lvlFile, levelFile, FALSE);
		fseek(lvlFile, lvlPos[lvlNum], SEEK_SET);
	} else {
		JE_resetFileExt(&lvlFile, "TYRIAN.HDT", FALSE);
		fread(&episode1DataLoc, 1, sizeof(episode1DataLoc), lvlFile);
		fseek(lvlFile, episode1DataLoc, SEEK_SET);
	}

	fread(&itemNum,    sizeof(JE_word), 7, lvlFile);

	for (i = 0; i < WEAP_NUM + 1; i++)
	{
		fread(&weapons[i].drain,           sizeof(JE_word), 1, lvlFile);
		fread(&weapons[i].shotrepeat,      sizeof(JE_byte), 1, lvlFile);
		fread(&weapons[i].multi,           sizeof(JE_byte), 1, lvlFile);
		fread(&weapons[i].weapani,         sizeof(JE_word), 1, lvlFile);
		fread(&weapons[i].max,             sizeof(JE_byte), 1, lvlFile);
		fread(&weapons[i].tx,              sizeof(JE_byte), 1, lvlFile);
		fread(&weapons[i].ty,              sizeof(JE_byte), 1, lvlFile);
		fread(&weapons[i].aim,             sizeof(JE_byte), 1, lvlFile);
		fread(&weapons[i].attack,          sizeof(JE_byte), 8, lvlFile);
		fread(&weapons[i].del,             sizeof(JE_byte), 8, lvlFile);
		fread(&weapons[i].sx,              sizeof(JE_shortint), 8, lvlFile);
		fread(&weapons[i].sy,              sizeof(JE_shortint), 8, lvlFile);
		fread(&weapons[i].bx,              sizeof(JE_shortint), 8, lvlFile);
		fread(&weapons[i].by,              sizeof(JE_shortint), 8, lvlFile);
		fread(&weapons[i].sg,              sizeof(JE_word), 8, lvlFile);
		fread(&weapons[i].acceleration,    sizeof(JE_shortint), 1, lvlFile);
		fread(&weapons[i].accelerationx,   sizeof(JE_shortint), 1, lvlFile);
		fread(&weapons[i].circlesize,      sizeof(JE_byte), 1, lvlFile);
		fread(&weapons[i].sound,           sizeof(JE_byte), 1, lvlFile);
		fread(&weapons[i].trail,           sizeof(JE_byte), 1, lvlFile);
		fread(&weapons[i].shipblastfilter, sizeof(JE_byte), 1, lvlFile);
	}

	for (i = 0; i < PORT_NUM + 1; i++)
	{
		fseek(lvlFile, 1, SEEK_CUR); /* skip string length */
		fread(&weaponPort[i].name,        1, 30, lvlFile);
		weaponPort[i].name[30] = '\0';
		fread(&weaponPort[i].opnum,       sizeof(JE_byte), 1, lvlFile);
		for (j = 0; j < 2; j++)
		{
			fread(&weaponPort[i].op[j],   sizeof(JE_word), 11, lvlFile);
		}
		fread(&weaponPort[i].cost,        sizeof(JE_word), 1, lvlFile);
		fread(&weaponPort[i].itemgraphic, sizeof(JE_word), 1, lvlFile);
		fread(&weaponPort[i].poweruse,    sizeof(JE_word), 1, lvlFile);
	}

	for (i = 0; i < SPECIAL_NUM + 1; i++)
	{
		fseek(lvlFile, 1, SEEK_CUR); /* skip string length */
		fread(&special[i].name,        1, 30, lvlFile);
		special[i].name[30] = '\0';
		fread(&special[i].itemgraphic, sizeof(JE_word), 1, lvlFile);
		fread(&special[i].pwr,         sizeof(JE_byte), 1, lvlFile);
		fread(&special[i].stype,       sizeof(JE_byte), 1, lvlFile);
		fread(&special[i].wpn,         sizeof(JE_word), 1, lvlFile);
	}

	for (i = 0; i < POWER_NUM + 1; i++)
	{
		fseek(lvlFile, 1, SEEK_CUR); /* skip string length */
		fread(&powerSys[i].name,        1, 30, lvlFile);
		powerSys[i].name[30] = '\0';
		fread(&powerSys[i].itemgraphic, sizeof(JE_word), 1, lvlFile);
		fread(&powerSys[i].power,       sizeof(JE_shortint), 1, lvlFile);
		fread(&powerSys[i].speed,       sizeof(JE_byte), 1, lvlFile);
		fread(&powerSys[i].cost,        sizeof(JE_word), 1, lvlFile);
	}

	for (i = 0; i < SHIP_NUM + 1; i++)
	{
		fseek(lvlFile, 1, SEEK_CUR); /* skip string length */
		fread(&ships[i].name,           1, 30, lvlFile);
		ships[i].name[30] = '\0';
		fread(&ships[i].shipgraphic,    sizeof(JE_word), 1, lvlFile);
		fread(&ships[i].itemgraphic,    sizeof(JE_word), 1, lvlFile);
		fread(&ships[i].ani,            sizeof(JE_byte), 1, lvlFile);
		fread(&ships[i].spd,            sizeof(JE_shortint), 1, lvlFile);
		fread(&ships[i].dmg,            sizeof(JE_byte), 1, lvlFile);
		fread(&ships[i].cost,           sizeof(JE_word), 1, lvlFile);
		fread(&ships[i].bigshipgraphic, sizeof(JE_byte), 1, lvlFile);
	}

	for (i = 0; i < OPTION_NUM + 1; i++)
	{
		fseek(lvlFile, 1, SEEK_CUR); /* skip string length */
		fread(&options[i].name,        1, 30, lvlFile);
		options[i].name[30] = '\0';
		fread(&options[i].pwr,         sizeof(JE_byte), 1, lvlFile);
		fread(&options[i].itemgraphic, sizeof(JE_word), 1, lvlFile);
		fread(&options[i].cost,        sizeof(JE_word), 1, lvlFile);
		fread(&options[i].tr,          sizeof(JE_byte), 1, lvlFile);
		fread(&options[i].option,      sizeof(JE_byte), 1, lvlFile);
		fread(&options[i].opspd,       sizeof(JE_shortint), 1, lvlFile);
		fread(&options[i].ani,         sizeof(JE_byte), 1, lvlFile);
		fread(&options[i].gr,          sizeof(JE_word), 20, lvlFile);
		fread(&options[i].wport,       sizeof(JE_byte), 1, lvlFile);
		fread(&options[i].wpnum,       sizeof(JE_word), 1, lvlFile);
		fread(&options[i].ammo,        sizeof(JE_byte), 1, lvlFile);
		fread(&options[i].stop,        1, 1, lvlFile); /* override sizeof(JE_boolean) */
		fread(&options[i].icongr,      sizeof(JE_byte), 1, lvlFile);
	}

	for (i = 0; i < SHIELD_NUM + 1; i++)
	{
		fseek(lvlFile, 1, SEEK_CUR); /* skip string length */
		fread(&shields[i].name,        1, 30, lvlFile);
		shields[i].name[30] = '\0';
		fread(&shields[i].tpwr,        sizeof(JE_byte), 1, lvlFile);
		fread(&shields[i].mpwr,        sizeof(JE_byte), 1, lvlFile);
		fread(&shields[i].itemgraphic, sizeof(JE_word), 1, lvlFile);
		fread(&shields[i].cost,        sizeof(JE_word), 1, lvlFile);
	}

	for (i = 0; i < ENEMY_NUM + 1; i++)
	{
		fread(&enemyDat[i].ani,           sizeof(JE_byte), 1, lvlFile);
		fread(&enemyDat[i].tur,           sizeof(JE_byte), 3, lvlFile);
		fread(&enemyDat[i].freq,          sizeof(JE_byte), 3, lvlFile);
		fread(&enemyDat[i].xmove,         sizeof(JE_shortint), 1, lvlFile);
		fread(&enemyDat[i].ymove,         sizeof(JE_shortint), 1, lvlFile);
		fread(&enemyDat[i].xaccel,        sizeof(JE_shortint), 1, lvlFile);
		fread(&enemyDat[i].yaccel,        sizeof(JE_shortint), 1, lvlFile);
		fread(&enemyDat[i].xcaccel,       sizeof(JE_shortint), 1, lvlFile);
		fread(&enemyDat[i].ycaccel,       sizeof(JE_shortint), 1, lvlFile);
		fread(&enemyDat[i].startx,        sizeof(JE_integer), 1, lvlFile);
		fread(&enemyDat[i].starty,        sizeof(JE_integer), 1, lvlFile);
		fread(&enemyDat[i].startxc,       sizeof(JE_shortint), 1, lvlFile);
		fread(&enemyDat[i].startyc,       sizeof(JE_shortint), 1, lvlFile);
		fread(&enemyDat[i].armor,         sizeof(JE_byte), 1, lvlFile);
		fread(&enemyDat[i].esize,         sizeof(JE_byte), 1, lvlFile);
		fread(&enemyDat[i].egraphic,      sizeof(JE_word), 20, lvlFile);
		fread(&enemyDat[i].explosiontype, sizeof(JE_byte), 1, lvlFile);
		fread(&enemyDat[i].animate,       sizeof(JE_byte), 1, lvlFile);
		fread(&enemyDat[i].shapebank,     sizeof(JE_byte), 1, lvlFile);
		fread(&enemyDat[i].xrev,          sizeof(JE_shortint), 1, lvlFile);
		fread(&enemyDat[i].yrev,          sizeof(JE_shortint), 1, lvlFile);
		fread(&enemyDat[i].dgr,           sizeof(JE_word), 1, lvlFile);
		fread(&enemyDat[i].dlevel,        sizeof(JE_shortint), 1, lvlFile);
		fread(&enemyDat[i].dani,          sizeof(JE_shortint), 1, lvlFile);
		fread(&enemyDat[i].elaunchfreq,   sizeof(JE_byte), 1, lvlFile);
		fread(&enemyDat[i].elaunchtype,   sizeof(JE_word), 1, lvlFile);
		fread(&enemyDat[i].value,         sizeof(JE_integer), 1, lvlFile);
		fread(&enemyDat[i].eenemydie,     sizeof(JE_word), 1, lvlFile);
	}

	fclose(lvlFile);
}

void JE_initEpisode( JE_byte newEpisode )
{
	if (newEpisode != episodeNum)
	{
		episodeNum = newEpisode;

		sprintf(levelFile, "TYRIAN%d.LVL", episodeNum);
		sprintf(cubeFile,  "CUBETXT%d.DAT", episodeNum);
		sprintf(macroFile, "LEVELS%d.DAT", episodeNum);

		JE_analyzeLevel();
		JE_loadItemDat();
	}
}

void JE_scanForEpisodes( void )
{
	JE_byte temp;

	char buf[sizeof(dir) + 11];

	JE_findTyrian("TYRIAN1.LVL"); /* need to know where to scan */

	for (temp = 0; temp < EPISODE_MAX; temp++)
	{
		sprintf(buf, "TYRIAN%d.LVL", temp + 1);
		episodeAvail[temp] = (dir != "") & JE_find(buf);

		sprintf(buf, "%sTYRIAN%d.LVL", dir, temp + 1);
		episodeAvail[temp] |= JE_find(buf);
	}
}
