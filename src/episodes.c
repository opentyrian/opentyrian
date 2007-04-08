/*
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
JE_boolean episodeAvail[EpisodeMax]; /* [1..episodemax] */
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

    if(episodeNum > 3)
    {
        JE_resetFileExt(&lvlFile, levelFile, FALSE);
        fseek(lvlFile, lvlPos[lvlNum], SEEK_SET);
    } else {
        JE_resetFileExt(&lvlFile, "TYRIAN.HDT", FALSE);
        fread(&episode1DataLoc, 1, sizeof(episode1DataLoc), lvlFile);
        fseek(lvlFile, episode1DataLoc, SEEK_SET);
    }

    fread(&itemNum,    1, sizeof(itemNum), lvlFile);
    fread(&weapons,    1, sizeof(weapons), lvlFile);
    fread(&weaponPort, 1, sizeof(weaponPort), lvlFile);
    fread(&special,    1, sizeof(special), lvlFile);
    fread(&powerSys,   1, sizeof(powerSys), lvlFile);
    fread(&ships,      1, sizeof(ships), lvlFile);
    fread(&options,    1, sizeof(options), lvlFile);
    fread(&shields,    1, sizeof(shields), lvlFile);
    fread(&enemyDat,   1, sizeof(enemyDat), lvlFile);

    fclose(lvlFile);
}

void JE_initEpisode( JE_byte newEpisode )
{
    if(newEpisode != episodeNum)
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

    for(temp = 0; temp < EpisodeMax; temp++)
    {
        sprintf(buf, "TYRIAN%d.LVL", temp + 1);
        episodeAvail[temp] = (dir != "") & JE_find(buf);

        sprintf(buf, "%sTYRIAN%d.LVL", dir, temp + 1);
        episodeAvail[temp] |= JE_find(buf);
    }
}
