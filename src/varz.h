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
#ifndef VARZ_H
#define VARZ_H

#include "opentyr.h"
#include "episodes.h"

#define SA 7

#define MaxPWeapon    81 /* 81*/
#define ExplosionMax 200 /*200*/
#define EnemyShotMax  60 /* 60*/

#define CurrentKeySpeed 1  /*Keyboard/Joystick movement rate*/

#define MaxSP 100

struct JE_singleenemytype
{
    JE_byte     fillbyte;
    JE_integer  ex, ey;     /* POSITION */
    JE_shortint exc, eyc;   /* CURRENT SPEED */
    JE_shortint exca, eyca; /* RANDOM ACCELERATION */
    JE_shortint excc, eycc; /* FIXED ACCELERATION WAITTIME */
    JE_byte     exccw, eyccw;
    JE_byte     armorleft;
    JE_byte     eshotwait[3], eshotmultipos[3]; /* [1..3] */
    JE_byte     enemycycle;
    JE_byte     ani;
    JE_word     egr[20]; /* [1..20] */
    JE_byte     size;
    JE_byte     linknum;
    JE_byte     aniactive;
    JE_byte     animax;
    JE_byte     aniwhenfire;
    JE_word     shapeseg;
    JE_shortint exrev,eyrev;
    JE_integer  exccadd,eyccadd;
    JE_byte     exccwmax,eyccwmax;
    JE_word     enemydatofs;
    JE_boolean  edamaged;
    JE_word     enemytype;
    JE_byte     animin;
    JE_word     edgr;
    JE_shortint edlevel;
    JE_shortint edani;
    JE_byte     fill1;
    JE_byte     filter;
    JE_integer  evalue;
    JE_integer  fixedmovey;
    JE_byte     freq[3]; /* [1..3] */
    JE_byte     launchwait;
    JE_word     launchtype;
    JE_byte     launchfreq;
    JE_byte     xaccel;
    JE_byte     yaccel;
    JE_byte     tur[3]; /* [1..3] */
    JE_word     enemydie;   /* Enemy created when this one dies */
    JE_boolean  enemyground;
    JE_byte     explonum;
    JE_word     mapoffset;
    JE_boolean  scoreitem;

    JE_boolean  special;
    JE_byte     flagnum;
    JE_boolean  setto;

    JE_byte     iced;      /*Duration*/

    JE_byte     launchspecial;

    JE_integer  xminbounce;
    JE_integer  xmaxbounce;
    JE_integer  yminbounce;
    JE_integer  ymaxbounce;
    JE_byte     fill[3]; /* [1..3] */
};

typedef struct JE_singleenemytype JE_multienemytype[100]; /* [1..100] */

typedef JE_word JE_dancshape[(24 * 28) / 2]; /* [1..(24*28) div 2] */

typedef JE_char JE_charstring[256]; /* [1..256] */

typedef JE_byte JE_map1buffer[24 * 28 * 13 * 4]; /* [1..24*28*13*4] */

typedef JE_word JE_maptype[300][14]; /* [1..300, 1..14] */
typedef JE_word JE_maptype2[300][14]; /* [1..600, 1..14] */
typedef JE_word JE_maptype3[600][15]; /* [1..600, 1..15] */

struct JE_eventrectype
{
    JE_word     EventTime;
    JE_byte     EventType;
    JE_integer  EventDat, EventDat2;
    JE_shortint EventDat3, EventDat5, EventDat6;
    JE_byte     EventDat4;
};

struct JE_megadatatype1
{
    JE_maptype mainmap;
    struct
    {
        JE_dancshape sh;
    } shapes[72]; /* [0..71] */
    JE_byte    tempdat1;
    /*JE_dancshape filler;*/
};

struct JE_megadatatype2
{
    JE_maptype2 mainmap;
    struct
    {
        JE_byte nothing[3]; /* [1..3] */
        JE_byte fill;
        JE_dancshape sh;
    } shapes[71]; /* [0..70] */
    JE_byte     tempdat2;
};

struct JE_megadatatype3
{
    JE_maptype3 mainmap;
    struct
    {
        JE_byte nothing[3]; /* [1..3] */
        JE_byte fill;
        JE_dancshape sh;
    } shapes[70]; /* [0..69] */
    JE_byte tempdat3;
};

typedef JE_multienemytype JE_EnemyType;
typedef JE_byte JE_EnemyAvailType[100]; /* [1..100] */

typedef JE_byte JE_REXtype[20]; /* [1..20] */

#ifndef NO_EXTERNS
extern JE_byte fromtime;
extern JE_integer tempdat, tempdat2, tempdat3;
extern JE_boolean tempb2;
extern JE_word test1;
extern const JE_byte SANextShip[SA + 2];
extern const JE_word SASpecialWeapon[SA];
extern const JE_word SASpecialWeaponB[SA];
extern const JE_byte SAShip[SA];
extern const JE_word SAWeapon[SA][5];
extern const JE_byte SpecialArcadeWeapon[PortNum];
extern const JE_byte OptionSelect[16][3][2];
extern const JE_word PGR[21];
extern const JE_byte PAni[21];
extern const JE_byte pitembuttonmap[7];
extern const JE_word LinkGunWeapons[38];
extern const JE_word ChargeGunWeapons[38];
extern const JE_word LinkMultiGr[17];
extern const JE_word LinkSonicGr[17];
extern const JE_word LinkMult2Gr[17];
extern const JE_byte RandomEnemyLaunchSounds[3];
extern const JE_byte KeyboardCombos[26][8];
extern const JE_byte ShipCombosB[21];
extern const JE_byte SuperTyrianSpecials[4];
extern const JE_byte ShipCombos[13][3];
extern JE_byte SFCurrentcode[2][21];
extern JE_byte SFExecuted[2];
extern JE_boolean StoppedDemo;
extern JE_byte lvlfilenum;
extern JE_word MaxEvent, EventLoc;
extern JE_word tempbackmove, explodemove;
extern JE_byte levelend;
extern JE_word levelendfxwait;
extern JE_shortint levelendwarp;
extern JE_boolean showmemleft, endlevel, reallyendlevel, waittoendlevel, playerendlevel, normalbonuslevelcurrent, bonuslevelcurrent, smallenemyadjust, readytoendlevel, quitrequested;
extern JE_byte NewPL[10];
extern JE_word ReturnLoc;
extern JE_boolean ReturnActive;
extern JE_word GalagaShotFreq;
extern JE_longint GalagaLife;
extern JE_boolean debug;
extern JE_word debugtime, lastdebugtime;
extern JE_longint debughistcount;
extern JE_real debughist;
extern JE_word curloc;
extern JE_boolean firstgameover, gameloaded, enemystillexploding;
extern JE_word TempSS;
extern JE_word Totalenemy;
extern JE_word EnemyKilled;
extern JE_byte Statbar[2], Statcol[2];
extern JE_map1buffer *map1buffertop, *map1bufferbot;
extern struct JE_megadatatype1 *megadata1;
extern struct JE_megadatatype2 *megadata2;
extern struct JE_megadatatype3 *megadata3;
extern JE_byte Flash;
extern JE_shortint FlashChange;
extern JE_byte DisplayTime;
extern JE_boolean FirstEvent;
extern FILE *RecordFile;
extern JE_word RecordFileNum;
extern JE_byte LastKey[8];
extern JE_word LastMoveWait, NextMoveWait;
extern JE_byte NextDemoOperation;
extern JE_boolean PlayDemo;
extern JE_byte PlayDemoNum;
extern JE_byte SoundQueue[8];
extern JE_boolean EnemyContinualDamage;
extern JE_boolean EnemiesActive;
extern JE_boolean ForceEvents;
extern JE_boolean StopBackgrounds;
extern JE_byte StopBackgroundNum;
extern JE_byte DamageRate;
extern JE_boolean Background3X1;
extern JE_boolean Background3X1b;
extern JE_boolean LevelTimer;
extern JE_word LevelTimerCountdown;
extern JE_word LevelTimerJumpTo;
extern JE_boolean RandomExplosions;
extern JE_boolean EditShip1, EditShip2;
extern JE_boolean GlobalFlags[10];
extern JE_byte levelsong;
extern JE_boolean drawgamesaved;
extern JE_boolean repause;
extern JE_boolean LoadDestruct;
extern JE_word MapOrigin, MapPnum;
extern JE_byte MapPlanet[5], MapSection[5];
extern JE_boolean LoadTitleScreen;
extern JE_boolean MoveTyrianLogoUp;
extern JE_boolean SkipStarShowVGA;
extern JE_EnemyType Enemy;
extern JE_EnemyAvailType EnemyAvail;
extern JE_word EnemyAvailOfs, TopEnemyAvailOfs, GroundEnemyAvailOfs, GroundEnemyAvailOfs2, EnemyOffset;
extern JE_word Enemyonscreen;
extern JE_byte EnemyShapeTables[6];
extern JE_boolean UniqueEnemy;
extern JE_word SuperEnemy254Jump;
extern struct { JE_word ExplodeLoc; JE_word ExplodeGr; JE_byte FollowPlayer; JE_byte FixedExplode; JE_word FixedMovement; } Explosions[ExplosionMax];
extern JE_byte ExplodeAvail[ExplosionMax];
extern JE_integer ExplosionFollowAmount;
extern JE_boolean PlayerFollow,FixedExplosions;
extern JE_integer ExplosionMoveUp;
extern JE_boolean FireButtonHeld;
extern JE_boolean EnemyShotAvail[EnemyShotMax];
extern struct { JE_integer Sx, Sy; JE_integer SxM, SyM; JE_shortint SxC, SyC; JE_byte Tx, Ty; JE_word SGR; JE_byte SDMG; JE_byte Duration; JE_word Animate; JE_word Animax; JE_byte Fill[12]; } EnemyShot[EnemyShotMax];
extern JE_byte ZinglonDuration;
extern JE_byte AstralDuration;
extern JE_word FlareDuration;
extern JE_boolean FlareStart;
extern JE_shortint FlareColChg;
extern JE_byte SpecialWait;
extern JE_byte NextSpecialWait;
extern JE_boolean SpraySpecial;
extern JE_byte DoIced;
extern JE_boolean InfiniteShot;
extern JE_byte SuperBomb[2];
extern JE_integer TempShotX, TempShotY;
extern struct { JE_integer ShotX; JE_integer ShotY; JE_integer ShotXM; JE_integer ShotYM; JE_integer ShotXC; JE_integer ShotYC; JE_boolean ShotComplicated; JE_integer ShotDevX; JE_integer ShotDirX; JE_integer ShotDevY; JE_integer ShotDirY; JE_integer ShotCirSizeX; JE_integer ShotCirSizeY; JE_byte ShotTrail; JE_word ShotGR; JE_word ShotAni; JE_word ShotAniMax; JE_shortint ShotDmg; JE_byte ShotBlastFilter; JE_byte ChainReaction; JE_byte PlayerNumber; JE_byte AimAtEnemy; JE_byte AimDelay; JE_byte AimDelayMax; JE_byte fill[1]; } PlayerShotData[MaxPWeapon + 1];
extern JE_byte chain;
extern JE_boolean AllPlayersGone;
extern JE_byte ShotAvail[MaxPWeapon];
extern JE_byte shadowydist;
extern JE_byte purpleballsremaining[2];
extern JE_boolean playeralive,playeraliveB;
extern JE_byte playerstillexploding, playerstillexploding2;
extern void *eshapes1, *eshapes2, *eshapes3, *eshapes4, *eshapes5, *eshapes6;
extern void *ShapesC1, *Shapes6, *Shapes9, *ShapesW2;
extern JE_word EShapes1Ofs, EShapes1Seg, EShapes2Ofs, EShapes2Seg, EShapes3Ofs, EShapes3Seg, EShapes4Ofs, EShapes4Seg, EShapes5Ofs, EShapes5Seg, EShapes6Ofs, EShapes6Seg, ShapesC1Ofs, ShapesC1Seg, Shapes9Ofs, Shapes9Seg, ShapesW2Ofs, ShapesW2Seg;
extern JE_word EShapes1Size, EShapes2Size, EShapes3Size, EShapes4Size, EShapes5Size, EShapes6Size, ShapesC1Size, Shapes6Size, Shapes9Size, ShapesW2Size;
extern JE_byte SAni;
extern JE_integer SAniX, SAniY, SaniXNeg, SaniYNeg;
extern JE_integer BaseSpeedOld, BaseSpeedOld2, BaseSpeed, BaseSpeedB, BaseSpeed2, BaseSpeed2B, BaseSpeedKeyH, BaseSpeedKeyV;
extern JE_boolean KeyMoveWait;
extern JE_boolean MakeMouseDelay;
extern JE_word playerinvulnerable1, playerinvulnerable2;
extern JE_integer LastPXShotMove, LastPYShotMove;
extern JE_integer PXB, PYB, LastPXB, LastPYB, LastPX2B, LastPY2B, PXChangeB, PYChangeB, LastTurnB, LastTurn2B, templastturn2B;
extern JE_byte StopwaitXB, StopwaitYB;
extern JE_word MouseXB, MouseYB;
extern JE_integer PX, PY, LastPX, LastPY, LastPX2, LastPY2, PXChange, PYChange, LastTurn, LastTurn2, templastturn2;
extern JE_byte StopwaitX, StopwaitY;
extern JE_integer PYHist[3], PYHistB[3];
extern JE_word Option1Draw, Option2Draw, Option1Item, Option2Item;
extern JE_byte Option1AmmoMax, Option2AmmoMax;
extern JE_word Option1AmmoRechargeWait, Option2AmmoRechargeWait, Option1AmmoRechargeWaitMax, Option2AmmoRechargeWaitMax;
extern JE_integer Option1Ammo, Option2Ammo;
extern JE_integer OptionAni1, OptionAni2, OptionCharge1, OptionCharge2, OptionCharge1Wait, OptionCharge2Wait, Option1X, Option1LastX, Option1Y, Option1LastY, Option2X, Option2LastX, Option2Y, Option2LastY, Option1MaxX, Option1MinX, Option2MaxX, Option2MinX, Option1MaxY, Option1MinY, Option2MaxY, Option2MinY;JE_boolean OptionAni1Go, OptionAni2Go, Option1Stop, Option2Stop;
extern JE_real OptionSatelliteRotate;
extern JE_integer OptionAttachmentMove;
extern JE_boolean OptionAttachmentLinked, OptionAttachmentReturn;
extern JE_byte ChargeWait, ChargeLevel, ChargeMax, ChargeGr, ChargeGrWait;
extern JE_boolean PlayerHNotReady;
extern JE_word PlayerHX[20], PlayerHY[20];
extern JE_word neat;
extern JE_REXtype REXAvail;
extern struct { JE_byte delay; JE_word ex, ey; JE_boolean big; } REXDat[20];
extern JE_byte SPZ[MaxSP + 1];
extern struct { JE_word location; JE_word movement; JE_byte color; } SPL[MaxSP + 1];
extern JE_word LastSP;
extern JE_word megadataofs, megadata2ofs, megadata3ofs;
extern JE_word avail;
extern JE_word tempCount;
extern JE_integer tempi, tempi2, tempi3, tempi4, tempi5;
extern JE_longint templ;
extern JE_real tempr, tempr2;
extern JE_boolean tempb;
extern JE_byte temp, temp2, temp3, temp4, temp5, tempREX, temppos;
extern JE_word tempx, tempy, tempx2, tempy2;
extern JE_word tempw, tempw2, tempw3, tempw4, tempw5, tempofs;
extern JE_boolean DoNotSaveBackup;
extern JE_boolean TempSpecial;
extern JE_word X, Y;
extern JE_integer A, B, C, D, Z, ZZ;
extern JE_byte playernum;
extern JE_word BKwrap1to, BKwrap2to, BKwrap3to, BKwrap1, BKwrap2, BKwrap3;
extern JE_byte min, max;
extern JE_shortint SpecialWeaponFilter, SpecialWeaponFreq;
extern JE_word SpecialWeaponWpn;
extern JE_boolean LinktoPlayer;
extern JE_integer BaseArmor, BaseArmor2;
extern JE_word ShipGR, ShipGR2;
extern JE_word ShipGRSeg, ShipGROfs, ShipGR2Seg, ShipGR2Ofs;
#endif

void JE_GetShipInfo( void );
JE_word JE_SGR( JE_word ship, JE_word *ofs, JE_word *seg );

void JE_CalcPurpleBall( JE_byte playernum );
void JE_DrawOptions( void );

void JE_tyrianhalt( JE_byte code );     /* This ends the game */
void JE_initplayershot( JE_word portnum, JE_byte temp, JE_word px, JE_word py,
                        JE_word mousex, JE_word mousey,
                        JE_word wpnum, JE_byte playernum );
void JE_DoSpecialShot( JE_byte playernum, JE_integer *armor, JE_shortint *shield );

void JE_powerup( JE_byte port );
void JE_WipeShieldArmorBars( void );
JE_byte JE_PlayerDamage( JE_word tempx, JE_word tempy, JE_byte temp,
                         JE_integer *px, JE_integer *py,
                         JE_boolean *PlayerAlive,
                         JE_byte *PlayerStillExploding,
                         JE_integer *ArmorLevel,
                         JE_shortint *Shield );

void SetupExplosion( JE_integer x, JE_integer y, JE_integer explodetype );
void setupexplosionlarge( JE_boolean enemyground, JE_byte explonum, JE_integer x, JE_integer y );

void DrawShield( void );
void DrawArmor( void );

void PortConfigs( void );

void ResetPlayerH( void );

void DoSP( JE_word x, JE_word y, JE_word num, JE_byte explowidth, JE_byte color );  /*SuperPixels*/
void DrawSP( void );

void DrawOptionLevel( void );

#endif /* VARZ_H */
