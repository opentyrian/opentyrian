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
#ifndef EPISODES_H
#define EPISODES_H

#include "opentyr.h"
#include "lvlmast.h"

/* Episodes and general data */

#define FirstLevel 1
#define EpisodeMax 5

typedef struct
{
    JE_word     drain;
    JE_byte     shotrepeat;
    JE_byte     multi;
    JE_word     weapani;
    JE_byte     max;
    JE_byte     tx, ty, aim;
    JE_byte     attack[8], del[8]; /* [1..8] */
    JE_shortint sx[8], sy[8]; /* [1..8] */
    JE_shortint bx[8], by[8]; /* [1..8] */
    JE_word     sg[8]; /* [1..8] */
    JE_shortint acceleration, accelerationx;
    JE_byte     circlesize;
    JE_byte     sound;
    JE_byte     trail;
    JE_byte     shipblastfilter;
} JE_WeaponType[WeapNum + 1]; /* [0..weapnum] */

typedef struct
{
    char    name[31]; /* string [30] */
    JE_byte opnum;
    JE_word op[2][11]; /* [1..2, 1..11] */
    JE_word cost;
    JE_word itemgraphic;
    JE_word poweruse;
} JE_WeaponPortType[PortNum + 1]; /* [0..portnum] */

typedef struct
{
    char        name[31]; /* string [30] */
    JE_word     itemgraphic;
    JE_byte     power;
    JE_shortint speed;
    JE_word     cost;
} JE_PowerType[PowerNum + 1]; /* [0..powernum] */

typedef struct
{
    char    name[31]; /* string [30] */
    JE_word itemgraphic;
    JE_byte pwr;
    JE_byte stype;
    JE_word wpn;
} JE_SpecialType[SpecialNum + 1]; /* [0..specialnum] */

typedef struct
{
    char        name[31]; /* string [30] */
    JE_word     itemgraphic;
    JE_shortint power;
    JE_byte     speed;
    JE_word     cost;
} JE_EngineType[EngineNum + 1]; /* [0..enginenum] */

typedef struct
{
    char        name[31]; /* string [30] */
    JE_byte     pwr;
    JE_word     itemgraphic;
    JE_word     cost;
    JE_byte     tr, option;
    JE_shortint opspd;
    JE_byte     ani;
    JE_word     gr[20]; /* [1..20] */
    JE_byte     wport;
    JE_word     wpnum;
    JE_byte     ammo;
    JE_boolean  stop;
    JE_byte     icongr;
} JE_OptionType[OptionNum + 1]; /* [0..optionnum] */

typedef struct
{
    char    name[31]; /* string [30] */
    JE_byte tpwr;
    JE_byte mpwr;
    JE_word itemgraphic;
    JE_word cost;
} JE_ShieldType[ShieldNum + 1]; /* [0..shieldnum] */

typedef struct
{
    char        name[31]; /* string [30] */
    JE_word     shipgraphic;
    JE_word     itemgraphic;
    JE_byte     ani;
    JE_shortint spd;
    JE_byte     dmg;
    JE_word     cost;
    JE_byte     bigshipgraphic;
} JE_ShipType[ShipNum + 1]; /* [0..shipnum] */

/* EnemyData */
typedef struct
{
    JE_byte     ani;
    JE_byte     tur[3]; /* [1..3] */
    JE_byte     freq[3]; /* [1..3] */
    JE_shortint xmove;
    JE_shortint ymove;
    JE_shortint xaccel;
    JE_shortint yaccel;
    JE_shortint xcaccel;
    JE_shortint ycaccel;
    JE_integer  startx;
    JE_integer  starty;
    JE_shortint startxc;
    JE_shortint startyc;
    JE_byte     armor;
    JE_byte     esize;
    JE_word     Egraphic[20]; /* [1..20] */
    JE_byte     ExplosionType;
    JE_byte     animate;        /* 0:Not Yet   1:Always   2:When Firing Only */
    JE_byte     shapebank;      /* See LEVELMAK.DOC */
    JE_shortint xrev, yrev;
    JE_word     dgr;
    JE_shortint dlevel;
    JE_shortint dani;
    JE_byte     elaunchfreq;
    JE_word     elaunchtype;
    JE_integer  value;
    JE_word     eenemydie;
} JE_EnemyDatType[EnemyNum + 1]; /* [0..enemynum] */

#ifndef NO_EXTERNS
extern JE_WeaponPortType *WeaponPort;
extern JE_WeaponType     *Weapons;
extern JE_PowerType   *PowerSys;
extern JE_ShipType    *Ships;
extern JE_OptionType  *Options;
extern JE_ShieldType  *Shields;
extern JE_SpecialType *Special;
extern JE_EnemyDatType *EnemyDat;
extern JE_byte    EpisodeNum;
extern JE_boolean EpisodeAvail[EpisodeMax]; /* [1..episodemax] */
extern char       MacroFile[13], CubeFile[13]; /* string [12] */
extern JE_longint Episode1DataLoc;
extern JE_boolean BonusLevel;
extern JE_boolean JumpBacktoEpisode1;
#endif

void JE_loaditemdat( void );
void JE_initepisode( JE_byte newepisode );
JE_boolean FindNextEpisode( void );
void scanforepisodes( void );

#endif /* EPISODES_H */
