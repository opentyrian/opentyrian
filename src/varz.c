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
#include "episodes.h"

#define NO_EXTERNS
#include "varz.h"
#undef NO_EXTERNS

JE_byte fromtime;
JE_integer tempdat, tempdat2, tempdat3;
JE_boolean tempb2;

JE_word test1;

const JE_byte SANextShip[SA + 2] /* [0..SA + 1] */ = { 3, 8, 6, 2, 5, 1, 4, 3, 7};
const JE_word SASpecialWeapon[SA] /* [1..SA] */  = { 7, 8, 9,10,11,12,13};
const JE_word SASpecialWeaponB[SA] /* [1..SA] */ = {37, 6,15,40,16,14,41};
const JE_byte SAShip[SA] /* [1..SA] */ = {3,1,5,10,2,11,12};
const JE_word SAWeapon[SA][5] /* [1..SA, 1..5] */ = 
{  /* R Bl Bk G  P */
    { 9,31,32,33,34},  /* Stealth Ship */
    {19, 8,22,41,34},  /* StormWind    */
    {27, 5,20,42,31},  /* Techno       */
    {15, 3,28,22,12},  /* Enemy        */
    {23,35,25,14, 6},  /* Weird        */
    { 2, 5,21, 4, 7},  /* Unknown      */
    {40,38,37,41,36}   /* NortShip Z   */
};

const JE_byte SpecialArcadeWeapon[PortNum] /* [1..Portnum] */ = 
{
    17,17,18,0,0,0,10,0,0,0,0,0,44,0,10,0,19,0,0,-0,0,0,0,0,0,0,
    -0,0,0,0,45,0,0,0,0,0,0,0,0,0,0,0
};

const JE_byte OptionSelect[16][3][2] /* [0..15, 1..3, 1..2] */ = 
{   /* MAIN     OPT    FRONT */
    { { 0, 0},{ 0, 0},{ 0, 0} },  /**/
    { { 1, 1},{16,16},{30,30} },  /*Single Shot*/
    { { 2, 2},{29,29},{29,20} },  /*Dual Shot*/
    { { 3, 3},{21,21},{12, 0} },  /*Charge Cannon*/
    { { 4, 4},{18,18},{16,23} },  /*Vulcan*/
    { { 0, 0},{ 0, 0},{ 0, 0} },  /**/
    { { 6, 6},{29,16},{ 0,22} },  /*Super Missile*/
    { { 7, 7},{19,19},{19,28} },  /*Atom Bomb*/
    { { 0, 0},{ 0, 0},{ 0, 0} },  /**/
    { { 0, 0},{ 0, 0},{ 0, 0} },  /**/
    { {10,10},{21,21},{21,27} },  /*Mini Missile*/
    { { 0, 0},{ 0, 0},{ 0, 0} },  /**/
    { { 0, 0},{ 0, 0},{ 0, 0} },  /**/
    { {13,13},{17,17},{13,26} },  /*MicroBomb*/
    { { 0, 0},{ 0, 0},{ 0, 0} },  /**/
    { {15,15},{15,16},{15,16} }   /*Post-It*/
};

const JE_word PGR[21] /* [1..21] */ = 
{
    4,
    1,2,3,
    41-21,57-21,73-21,89-21,105-21,
    121-21,137-21,153-21,
    151,151,151,151,73-21,73-21,1,2,4
    /*151,151,151*/
};
const JE_byte PAni[21] /* [1..21] */ = {1,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1};

const JE_byte pitembuttonmap[7] /* [1..7] */ = {12,1,2,10,6,4,5}; /*Financial Stuff*/

const JE_word LinkGunWeapons[38] /* [1..38] */ =
{
    0,0,0,0,0,0,0,0,444,445,446,447,0,448,449,0,0,0,0,0,450,451,0,506,0,564,
      445,446,447,448,449,445,446,447,448,449,450,451
};
const JE_word ChargeGunWeapons[38] /* [1..38] */ =
{
    0,0,0,0,0,0,0,0,476,458,464,482,0,488,470,0,0,0,0,0,494,500,0,528,0,558,
      458,458,458,458,458,458,458,458,458,458,458,458
};
const JE_word LinkMultiGr[17] /* [0..16] */ =
    {77,221,183,301,1,282,164,202,58,201,163,281,39,300,182,220,77};
const JE_word LinkSonicGr[17] /* [0..16] */ =
    {85,242,131,303,47,284,150,223,66,224,149,283,9,302,130,243,85};
const JE_word LinkMult2Gr[17] /* [0..16] */ =
    {78,299,295,297,2,278,276,280,59,279,275,277,40,296,294,298,78};

const JE_byte RandomEnemyLaunchSounds[3] /* [1..3] */ = {13,6,26};

const JE_byte KeyboardCombos[26][8] /* [1..26, 1..8] */ = 
{
    {  2,  1,  2,  5,137,          0,  0,  0},  /*Invulnerability*/
    {  4,  3,  2,  5,138,          0,  0,  0},  /*Atom Bomb*/
    {  3,  4,  6,139,          0,  0,  0,  0},  /*Seeker Bombs*/
    {  2,  5,142,          0,  0,  0,  0,  0},  /*Ice Blast*/
    {  6,  2,  6,143,          0,  0,  0,  0},  /*Auto Repair*/
    {  6,  7,  5,  8,  6,  7,  5,112        },  /*Spin Wave*/
    {  7,  8,101,          0,  0,  0,  0,  0},  /*Repulsor*/
    {  1,  7,  6,146,          0,  0,  0,  0},  /*Protron Field*/
    {  8,  6,  7,  1,120,          0,  0,  0},  /*Minefield*/
    {  3,  6,  8,  5,121,          0,  0,  0},  /*Post-It Blast*/
    {  1,  2,  7,  8,119,          0,  0,  0},  /*Drone Ship - TBC*/
    {  3,  4,  3,  6,123,          0,  0,  0},  /*Repair Player 2*/
    {  6,  7,  5,  8,124,          0,  0,  0},  /*Super Bomb - TBC*/
    {  1,  6,125,          0,  0,  0,  0,  0},  /*Hot Dog*/
    {  9,  5,126,          0,  0,  0,  0,  0},  /*Lightning UP      */
    {  1,  7,127,          0,  0,  0,  0,  0},  /*Lightning UP+LEFT */
    {  1,  8,128,          0,  0,  0,  0,  0},  /*Lightning UP+RIGHT*/
    {  9,  7,129,          0,  0,  0,  0,  0},  /*Lightning    LEFT */
    {  9,  8,130,          0,  0,  0,  0,  0},  /*Lightning    RIGHT*/
    {  4,  2,  3,  5,131,          0,  0,  0},  /*Warfly            */
    {  3,  1,  2,  8,132,          0,  0,  0},  /*FrontBlaster      */
    {  2,  4,  5,133,          0,  0,  0,  0},  /*Gerund            */
    {  3,  4,  2,  8,134,          0,  0,  0},  /*FireBomb          */
    {  1,  4,  6,135,          0,  0,  0,  0},  /*Indigo            */
    {  1,  3,  6,137,          0,  0,  0,  0},  /*Invulnerability [easier] */
    {  1,  4,  3,  4,  7,136,          0,  0}   /*D-Media Protron Drone    */
};

const JE_byte ShipCombosB[21] /* [1..21] */ = 
    {15,16,17,18,19,20,21,22,23,24, 7, 8, 5,25,14, 4, 6, 3, 9, 2,26};
  /*!! SUPER Tyrian !!*/
const JE_byte SuperTyrianSpecials[4] /* [1..4] */ = {1,2,4,5};

const JE_byte ShipCombos[13][3] /* [0..12, 1..3] */ = 
{
    { 5, 4, 7},  /*2nd Player ship*/
    { 1, 2, 0},  /*USP Talon*/
    {14, 4, 0},  /*Super Carrot*/
    { 4, 5, 0},  /*Gencore Phoenix*/
    { 6, 5, 0},  /*Gencore Maelstrom*/
    { 7, 8, 0},  /*MicroCorp Stalker*/
    { 7, 9, 0},  /*MicroCorp Stalker-B*/
    {10, 3, 5},  /*Prototype Stalker-C*/
    { 5, 8, 9},  /*Stalker*/
    { 1, 3, 0},  /*USP Fang*/
    { 7,16,17},  /*U-Ship*/
    { 2,11,12},  /*1st Player ship*/
    { 3, 8,10}   /*Nort ship*/
};

/*Street-Fighter Commands*/
JE_byte SFCurrentcode[2][21]; /* [1..2, 1..21] */
JE_byte SFExecuted[2]; /* [1..2] */


/*Happy*/
JE_boolean StoppedDemo;

/*Special General Data*/
JE_byte lvlfilenum;
JE_word MaxEvent, EventLoc;
/*JE_word maxenemies;*/
JE_word tempbackmove, explodemove; /*Speed of background movement*/
JE_byte levelend;
JE_word levelendfxwait;
JE_shortint levelendwarp;
JE_boolean showmemleft, endlevel, reallyendlevel, waittoendlevel, playerendlevel,
           normalbonuslevelcurrent, bonuslevelcurrent,
           smallenemyadjust, readytoendlevel, quitrequested;

JE_byte NewPL[10]; /* [0..9] */ /*Eventsys event 75 parameter*/
JE_word ReturnLoc;
JE_boolean ReturnActive;
JE_word GalagaShotFreq;
JE_longint GalagaLife;

JE_boolean debug;    /*Debug Mode*/
JE_word debugtime, lastdebugtime;
JE_longint debughistcount;
JE_real debughist;
JE_word curloc;      /*Current Pixel location of background 1*/

JE_boolean firstgameover, gameloaded,
           enemystillexploding;

/* Error Checking */
JE_word TempSS;


/* Destruction Ratio */
JE_word Totalenemy;
JE_word EnemyKilled;

JE_byte Statbar[2], Statcol[2]; /* [1..2] */

/* Buffer */
JE_map1buffer *map1buffertop, *map1bufferbot;

/* Shape/Map Data - All in one Segment! */
struct JE_megadatatype1 *megadata1;
struct JE_megadatatype2 *megadata2;
struct JE_megadatatype3 *megadata3;

/* Secret Level Display */
JE_byte Flash;
JE_shortint FlashChange;
JE_byte DisplayTime;

/* Demo Stuff */
JE_boolean FirstEvent;
FILE *RecordFile;
JE_word RecordFileNum;
JE_byte LastKey[8]; /* [1..8] */
JE_word LastMoveWait, NextMoveWait;
JE_byte NextDemoOperation;
JE_boolean PlayDemo;
JE_byte PlayDemoNum;

/* Sound Effects Queue */
JE_byte SoundQueue[8]; /* [0..7] */

/*Level Event Data*/
JE_boolean EnemyContinualDamage;
JE_boolean EnemiesActive;
JE_boolean ForceEvents;
JE_boolean StopBackgrounds;
JE_byte StopBackgroundNum;
JE_byte DamageRate;  /*Rate at which a player takes damage*/
JE_boolean Background3X1;  /*Background 3 enemies use Background 1 X offset*/
JE_boolean Background3X1b; /*Background 3 enemies moved 8 pixels left*/

JE_boolean LevelTimer;
JE_word    LevelTimerCountdown;
JE_word    LevelTimerJumpTo;
JE_boolean RandomExplosions;

JE_boolean EditShip1, EditShip2;

JE_boolean GlobalFlags[10]; /* [1..10] */
JE_byte levelsong;

JE_boolean drawgamesaved;

JE_boolean repause;

/* DESTRUCT game */
JE_boolean LoadDestruct;

/* MapView Data */
JE_word MapOrigin, MapPnum;
JE_byte MapPlanet[5], MapSection[5]; /* [1..5] */

/* Interface Constants */
JE_boolean LoadTitleScreen;
JE_boolean MoveTyrianLogoUp;
JE_boolean SkipStarShowVGA;

/*EnemyData*/
JE_EnemyType Enemy;
JE_EnemyAvailType EnemyAvail;
JE_word EnemyAvailOfs, TopEnemyAvailOfs, GroundEnemyAvailOfs, GroundEnemyAvailOfs2, EnemyOffset;
JE_word Enemyonscreen;
JE_byte EnemyShapeTables[6]; /* [1..6] */
JE_boolean UniqueEnemy;
JE_word SuperEnemy254Jump;

/*ExplosionData*/
struct
{
    JE_word ExplodeLoc;
    JE_word ExplodeGr;
    JE_byte FollowPlayer;
    JE_byte FixedExplode;
    JE_word FixedMovement;
} Explosions[ExplosionMax]; /* [1..ExplosionMax] */
JE_byte ExplodeAvail[ExplosionMax]; /* [1..ExplosionMax] */
JE_integer ExplosionFollowAmount;
JE_boolean PlayerFollow,FixedExplosions;
JE_integer ExplosionMoveUp;

/*EnemyShotData*/
JE_boolean FireButtonHeld;
JE_boolean EnemyShotAvail[EnemyShotMax]; /* [1..Enemyshotmax] */
struct
{
    JE_integer Sx, Sy;
    JE_integer SxM, SyM;
    JE_shortint SxC, SyC;
    JE_byte Tx, Ty;
    JE_word SGR;
    JE_byte SDMG;
    JE_byte Duration;
    JE_word Animate;
    JE_word Animax;
    JE_byte Fill[12]; /* [1..12] */
} EnemyShot[EnemyShotMax]; /* [1..Enemyshotmax]  */

/* Player Shot Data */
JE_byte     ZinglonDuration;
JE_byte     AstralDuration;
JE_word     FlareDuration;
JE_boolean  FlareStart;
JE_shortint FlareColChg;
JE_byte     SpecialWait;
JE_byte     NextSpecialWait;
JE_boolean  SpraySpecial;
JE_byte     DoIced;
JE_boolean  InfiniteShot;

JE_byte SuperBomb[2]; /* [1..2] */

JE_integer TempShotX, TempShotY;
struct
{
    JE_integer  ShotX;
    JE_integer  ShotY;
    JE_integer  ShotXM;
    JE_integer  ShotYM;
    JE_integer  ShotXC;  /*10*/
    JE_integer  ShotYC;
    JE_boolean  ShotComplicated;
    JE_integer  ShotDevX;
    JE_integer  ShotDirX;
    JE_integer  ShotDevY;  /*19*/
    JE_integer  ShotDirY;
    JE_integer  ShotCirSizeX;
    JE_integer  ShotCirSizeY;
    JE_byte     ShotTrail;
    JE_word     ShotGR;
    JE_word     ShotAni;     /*30*/
    JE_word     ShotAniMax;
    JE_shortint ShotDmg;
    JE_byte     ShotBlastFilter;
    JE_byte     ChainReaction;     /*35*/
    JE_byte     PlayerNumber;
    JE_byte     AimAtEnemy;
    JE_byte     AimDelay;
    JE_byte     AimDelayMax;     /*39*/
    JE_byte     fill[1]; /* [1..1] */
} PlayerShotData[MaxPWeapon + 1]; /* [1..MaxPWeapon+1] */

JE_byte chain;

/*PlayerData*/
JE_boolean AllPlayersGone;  /*Both players dead and finished exploding*/

JE_byte ShotAvail[MaxPWeapon]; /* [1..MaxPWeapon] */   /*0:Avail 1-255:Duration left*/
JE_byte shadowydist;
JE_byte purpleballsremaining[2]; /* [1..2] */

JE_boolean playeralive,playeraliveB;
JE_byte playerstillexploding, playerstillexploding2;
void *eshapes1, *eshapes2, *eshapes3, *eshapes4, *eshapes5, *eshapes6;

void *ShapesC1, *Shapes6, *Shapes9, *ShapesW2;

JE_word EShapes1Ofs, EShapes1Seg,
        EShapes2Ofs, EShapes2Seg,
        EShapes3Ofs, EShapes3Seg,
        EShapes4Ofs, EShapes4Seg,
        EShapes5Ofs, EShapes5Seg,
        EShapes6Ofs, EShapes6Seg,
        ShapesC1Ofs, ShapesC1Seg,
        Shapes9Ofs,  Shapes9Seg,
        ShapesW2Ofs, ShapesW2Seg;     /* 6 is in NewShape */

JE_word EShapes1Size,
        EShapes2Size,
        EShapes3Size,
        EShapes4Size,
        EShapes5Size,
        EShapes6Size,
        ShapesC1Size,
        Shapes6Size,
        Shapes9Size,
        ShapesW2Size;

JE_byte SAni;
JE_integer SAniX, SAniY, SaniXNeg, SaniYNeg;  /* X,Y ranges of field of hit */
JE_integer BaseSpeedOld, BaseSpeedOld2, BaseSpeed, BaseSpeedB, BaseSpeed2, BaseSpeed2B,
           BaseSpeedKeyH, BaseSpeedKeyV;
JE_boolean KeyMoveWait;

JE_boolean MakeMouseDelay;

JE_word playerinvulnerable1, playerinvulnerable2;

JE_integer LastPXShotMove, LastPYShotMove;

JE_integer PXB, PYB, LastPXB, LastPYB, LastPX2B, LastPY2B, PXChangeB, PYChangeB,
           LastTurnB, LastTurn2B, templastturn2B;
JE_byte StopwaitXB, StopwaitYB;
JE_word MouseXB, MouseYB;

JE_integer PX, PY, LastPX, LastPY, LastPX2, LastPY2, PXChange, PYChange,
           LastTurn, LastTurn2, templastturn2;
JE_byte StopwaitX, StopwaitY;

JE_integer PYHist[3], PYHistB[3]; /* [1..3] */

/*JE_shortint OptionMoveX[10], OptionMoveY[10]; \* [1..10] *\ */
JE_word Option1Draw, Option2Draw, Option1Item, Option2Item;
JE_byte Option1AmmoMax, Option2AmmoMax;
JE_word Option1AmmoRechargeWait, Option2AmmoRechargeWait,
        Option1AmmoRechargeWaitMax, Option2AmmoRechargeWaitMax;
JE_integer Option1Ammo, Option2Ammo;
JE_integer OptionAni1, OptionAni2, OptionCharge1, OptionCharge2, OptionCharge1Wait, OptionCharge2Wait,
           Option1X, Option1LastX, Option1Y, Option1LastY,
           Option2X, Option2LastX, Option2Y, Option2LastY,
           Option1MaxX, Option1MinX, Option2MaxX, Option2MinX,
           Option1MaxY, Option1MinY, Option2MaxY, Option2MinY;
JE_boolean OptionAni1Go, OptionAni2Go, Option1Stop, Option2Stop;
JE_real OptionSatelliteRotate;

JE_integer OptionAttachmentMove;
JE_boolean OptionAttachmentLinked, OptionAttachmentReturn;


JE_byte ChargeWait, ChargeLevel, ChargeMax, ChargeGr, ChargeGrWait;

JE_boolean PlayerHNotReady;
JE_word PlayerHX[20], PlayerHY[20]; /* [1..20] */

JE_word neat;


/*Repeating Explosions*/
JE_REXtype REXAvail;
struct
{
    JE_byte    delay;
    JE_word    ex, ey;
    JE_boolean big;
    /*JE_byte    fill[2] \* [1..2] \* */
} REXDat[20]; /* [1..20] */

/*SuperPixels*/
JE_byte SPZ[MaxSP + 1]; /* [0..MaxSP] */
struct
{
    JE_word location;
    JE_word movement;
    JE_byte color;
} SPL[MaxSP + 1]; /* [0..MaxSP] */
JE_word LastSP;

/*MegaData*/
JE_word megadataofs, megadata2ofs, megadata3ofs;

/*Temporary Numbers*/
JE_word avail;
JE_word tempCount;
JE_integer tempi, tempi2, tempi3, tempi4, tempi5;
JE_longint templ;
JE_real tempr, tempr2;
/*JE_integer tempx, tempy;*/

JE_boolean tempb;
JE_byte temp, temp2, temp3, temp4, temp5, tempREX, temppos;
JE_word tempx, tempy, tempx2, tempy2;
JE_word tempw, tempw2, tempw3, tempw4, tempw5, tempofs;


JE_boolean DoNotSaveBackup;

JE_boolean TempSpecial;

JE_word X, Y;
JE_integer A, B, C, D, Z, ZZ;
JE_byte playernum;

JE_word BKwrap1to, BKwrap2to, BKwrap3to,
        BKwrap1, BKwrap2, BKwrap3;

JE_byte min, max;

JE_shortint SpecialWeaponFilter, SpecialWeaponFreq;
JE_word     SpecialWeaponWpn;
JE_boolean  LinktoPlayer;

JE_integer BaseArmor, BaseArmor2;
JE_word ShipGR, ShipGR2;
JE_word ShipGRSeg, ShipGROfs, ShipGR2Seg, ShipGR2Ofs;
