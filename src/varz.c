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

JE_byte fromTime;
JE_integer tempDat, tempDat2, tempDat3;
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

const JE_byte specialArcadeWeapon[PortNum] /* [1..Portnum] */ = 
{
    17,17,18,0,0,0,10,0,0,0,0,0,44,0,10,0,19,0,0,-0,0,0,0,0,0,0,
    -0,0,0,0,45,0,0,0,0,0,0,0,0,0,0,0
};

const JE_byte optionSelect[16][3][2] /* [0..15, 1..3, 1..2] */ = 
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

const JE_byte PItemButtonMap[7] /* [1..7] */ = {12,1,2,10,6,4,5}; /*Financial Stuff*/

const JE_word linkGunWeapons[38] /* [1..38] */ =
{
    0,0,0,0,0,0,0,0,444,445,446,447,0,448,449,0,0,0,0,0,450,451,0,506,0,564,
      445,446,447,448,449,445,446,447,448,449,450,451
};
const JE_word chargeGunWeapons[38] /* [1..38] */ =
{
    0,0,0,0,0,0,0,0,476,458,464,482,0,488,470,0,0,0,0,0,494,500,0,528,0,558,
      458,458,458,458,458,458,458,458,458,458,458,458
};
const JE_word linkMultiGr[17] /* [0..16] */ =
    {77,221,183,301,1,282,164,202,58,201,163,281,39,300,182,220,77};
const JE_word linkSonicGr[17] /* [0..16] */ =
    {85,242,131,303,47,284,150,223,66,224,149,283,9,302,130,243,85};
const JE_word linkMult2Gr[17] /* [0..16] */ =
    {78,299,295,297,2,278,276,280,59,279,275,277,40,296,294,298,78};

const JE_byte randomEnemyLaunchSounds[3] /* [1..3] */ = {13,6,26};

const JE_byte keyboardCombos[26][8] /* [1..26, 1..8] */ = 
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

const JE_byte shipCombosB[21] /* [1..21] */ = 
    {15,16,17,18,19,20,21,22,23,24, 7, 8, 5,25,14, 4, 6, 3, 9, 2,26};
  /*!! SUPER Tyrian !!*/
const JE_byte superTyrianSpecials[4] /* [1..4] */ = {1,2,4,5};

const JE_byte shipCombos[13][3] /* [0..12, 1..3] */ = 
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
JE_boolean stoppedDemo;

/*Special General Data*/
JE_byte lvlFileNum;
JE_word maxEvent, eventLoc;
/*JE_word maxenemies;*/
JE_word tempBackMove, explodeMove; /*Speed of background movement*/
JE_byte levelEnd;
JE_word levelEndFxWait;
JE_shortint levelEndWarp;
JE_boolean showMemLeft, endLevel, reallyEndLevel, waitToEndLevel, playerEndLevel,
           normalBonuslevelCurrent, bonusLevelCurrent,
           smallEnemyAdjust, readyToEndLevel, quitRequested;

JE_byte newPL[10]; /* [0..9] */ /*Eventsys event 75 parameter*/
JE_word returnLoc;
JE_boolean returnActive;
JE_word galagaShotFreq;
JE_longint galagaLife;

JE_boolean debug;    /*Debug Mode*/
JE_word debugTime, lastDebugTime;
JE_longint debugHistCount;
JE_real debugHist;
JE_word curLoc;      /*Current Pixel location of background 1*/

JE_boolean firstGameOver, gameLoaded,
           enemyStillExploding;

/* Error Checking */
JE_word tempSS;


/* Destruction Ratio */
JE_word totalenemy;
JE_word enemyKilled;

JE_byte statBar[2], statCol[2]; /* [1..2] */

/* Buffer */
JE_map1buffer *map1BufferTop, *map1BufferBot;

/* Shape/Map Data - All in one Segment! */
struct JE_megadatatype1 *megaData1;
struct JE_megadatatype2 *megaData2;
struct JE_megadatatype3 *megaData3;

/* Secret Level Display */
JE_byte flash;
JE_shortint flashChange;
JE_byte displayTime;

/* Demo Stuff */
JE_boolean firstEvent;
FILE *recordFile;
JE_word recordFileNum;
JE_byte lastKey[8]; /* [1..8] */
JE_word lastMoveWait, nextMoveWait;
JE_byte nextDemoOperation;
JE_boolean playDemo;
JE_byte playDemoNum;

/* Sound Effects Queue */
JE_byte soundQueue[8]; /* [0..7] */

/*Level Event Data*/
JE_boolean enemyContinualDamage;
JE_boolean enemiesActive;
JE_boolean forceEvents;
JE_boolean stopBackgrounds;
JE_byte stopBackgroundNum;
JE_byte damageRate;  /*Rate at which a player takes damage*/
JE_boolean background3X1;  /*Background 3 enemies use Background 1 X offset*/
JE_boolean background3X1B; /*Background 3 enemies moved 8 pixels left*/

JE_boolean levelTimer;
JE_word    levelTimerCountdown;
JE_word    levelTimerJumpTo;
JE_boolean randomExplosions;

JE_boolean editShip1, editShip2;

JE_boolean globalFlags[10]; /* [1..10] */
JE_byte levelSong;

JE_boolean drawGameSaved;

JE_boolean repause;

/* DESTRUCT game */
JE_boolean loadDestruct;

/* MapView Data */
JE_word mapOrigin, mapPNum;
JE_byte mapPlanet[5], mapSection[5]; /* [1..5] */

/* Interface Constants */
JE_boolean loadTitleScreen;
JE_boolean moveTyrianLogoUp;
JE_boolean skipStarShowVGA;

/*EnemyData*/
JE_EnemyType enemy;
JE_EnemyAvailType enemyAvail;
JE_word enemyAvailOfs, topEnemyAvailOfs, groundEnemyAvailOfs, groundEnemyAvailOfs2, enemyOffset;
JE_word enemyOnScreen;
JE_byte enemyShapeTables[6]; /* [1..6] */
JE_boolean uniqueEnemy;
JE_word superEnemy254Jump;

/*ExplosionData*/
struct
{
    JE_word explodeLoc;
    JE_word explodeGr;
    JE_byte followPlayer;
    JE_byte fixedExplode;
    JE_word fixedMovement;
} explosions[ExplosionMax]; /* [1..ExplosionMax] */
JE_byte explodeAvail[ExplosionMax]; /* [1..ExplosionMax] */
JE_integer explosionFollowAmount;
JE_boolean playerFollow, fixedExplosions;
JE_integer explosionMoveUp;

/*EnemyShotData*/
JE_boolean fireButtonHeld;
JE_boolean enemyShotAvail[EnemyShotMax]; /* [1..Enemyshotmax] */
struct
{
    JE_integer Sx, Sy;
    JE_integer SxM, SyM;
    JE_shortint SxC, SyC;
    JE_byte Tx, Ty;
    JE_word SGR;
    JE_byte SDMG;
    JE_byte duration;
    JE_word animate;
    JE_word animax;
    JE_byte fill[12]; /* [1..12] */
} enemyShot[EnemyShotMax]; /* [1..Enemyshotmax]  */

/* Player Shot Data */
JE_byte     zinglonDuration;
JE_byte     astralDuration;
JE_word     flareDuration;
JE_boolean  flareStart;
JE_shortint flareColChg;
JE_byte     specialWait;
JE_byte     nextSpecialWait;
JE_boolean  spraySpecial;
JE_byte     doIced;
JE_boolean  infiniteShot;

JE_byte superBomb[2]; /* [1..2] */

JE_integer tempShotX, tempShotY;
struct
{
    JE_integer  shotX;
    JE_integer  shotY;
    JE_integer  shotXM;
    JE_integer  shotYM;
    JE_integer  shotXC;  /*10*/
    JE_integer  shotYC;
    JE_boolean  shotComplicated;
    JE_integer  shotDevX;
    JE_integer  shotDirX;
    JE_integer  shotDevY;  /*19*/
    JE_integer  shotDirY;
    JE_integer  shotCirSizeX;
    JE_integer  shotCirSizeY;
    JE_byte     shotTrail;
    JE_word     shotGR;
    JE_word     shotAni;     /*30*/
    JE_word     shotAniMax;
    JE_shortint shotDmg;
    JE_byte     shotBlastFilter;
    JE_byte     chainReaction;     /*35*/
    JE_byte     playerNumber;
    JE_byte     aimAtEnemy;
    JE_byte     aimDelay;
    JE_byte     aimDelayMax;     /*39*/
    JE_byte     fill[1]; /* [1..1] */
} playerShotData[MaxPWeapon + 1]; /* [1..MaxPWeapon+1] */

JE_byte chain;

/*PlayerData*/
JE_boolean allPlayersGone;  /*Both players dead and finished exploding*/

JE_byte shotAvail[MaxPWeapon]; /* [1..MaxPWeapon] */   /*0:Avail 1-255:Duration left*/
JE_byte shadowyDist;
JE_byte purpleBallsRemaining[2]; /* [1..2] */

JE_boolean playerAlive, playerAliveB;
JE_byte playerStillExploding, playerStillExploding2;
JE_byte *eShapes1, *eShapes2, *eShapes3, *eShapes4, *eShapes5, *eShapes6;

JE_byte *shapesC1, *shapes6, *shapes9, *shapesW2;

JE_word eShapes1Size,
        eShapes2Size,
        eShapes3Size,
        eShapes4Size,
        eShapes5Size,
        eShapes6Size,
        shapesC1Size,
        shapes6Size,
        shapes9Size,
        shapesW2Size;

JE_byte SAni;
JE_integer SAniX, SAniY, SAniXNeg, SAniYNeg;  /* X,Y ranges of field of hit */
JE_integer baseSpeedOld, baseSpeedOld2, baseSpeed, baseSpeedB, baseSpeed2, baseSpeed2B,
           baseSpeedKeyH, baseSpeedKeyV;
JE_boolean keyMoveWait;

JE_boolean makeMouseDelay;

JE_word playerInvulnerable1, playerInvulnerable2;

JE_integer lastPXShotMove, lastPYShotMove;

JE_integer PXB, PYB, lastPXB, lastPYB, lastPX2B, lastPY2B, PXChangeB, PYChangeB,
           lastTurnB, lastTurn2B, tempLastTurn2B;
JE_byte stopWaitXB, stopWaitYB;
JE_word mouseXB, mouseYB;

JE_integer PX, PY, lastPX, lastPY, lastPX2, lastPY2, PXChange, PYChange,
           lastTurn, lastTurn2, tempLastTurn2;
JE_byte StopwaitX, StopwaitY;

JE_integer PYHist[3], PYHistB[3]; /* [1..3] */

/*JE_shortint optionMoveX[10], optionMoveY[10]; \* [1..10] *\ */
JE_word option1Draw, option2Draw, option1Item, option2Item;
JE_byte option1AmmoMax, option2AmmoMax;
JE_word option1AmmoRechargeWait, option2AmmoRechargeWait,
        option1AmmoRechargeWaitMax, option2AmmoRechargeWaitMax;
JE_integer option1Ammo, option2Ammo;
JE_integer optionAni1, optionAni2, optionCharge1, optionCharge2, optionCharge1Wait, optionCharge2Wait,
           option1X, option1LastX, option1Y, option1LastY,
           option2X, option2LastX, option2Y, option2LastY,
           option1MaxX, option1MinX, option2MaxX, option2MinX,
           option1MaxY, option1MinY, option2MaxY, option2MinY;
JE_boolean optionAni1Go, optionAni2Go, option1Stop, option2Stop;
JE_real optionSatelliteRotate;

JE_integer optionAttachmentMove;
JE_boolean optionAttachmentLinked, optionAttachmentReturn;


JE_byte chargeWait, chargeLevel, chargeMax, chargeGr, chargeGrWait;

JE_boolean playerHNotReady;
JE_word playerHX[20], playerHY[20]; /* [1..20] */

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
JE_word lastSP;

/*MegaData*/
JE_word megaDataOfs, megaData2Ofs, megaData3Ofs;

/*Temporary Numbers*/
JE_word avail;
JE_word tempCount;
JE_integer tempI, tempI2, tempI3, tempI4, tempI5;
JE_longint tempL;
JE_real tempR, tempR2;
/*JE_integer tempX, tempY;*/

JE_boolean tempB;
JE_byte temp, temp2, temp3, temp4, temp5, tempREX, tempPos;
JE_word tempX, tempY, tempX2, tempY2;
JE_word tempW, tempW2, tempW3, tempW4, tempW5, tempOfs;


JE_boolean doNotSaveBackup;

JE_boolean tempSpecial;

JE_word x, y;
JE_integer a, b, c, d, z, zz;
JE_byte playerNum;

JE_word BKWrap1to, BKWrap2To, BKWrap3To,
        BKWrap1, BKWrap2, BKWrap3;

JE_byte min, max;

JE_shortint specialWeaponFilter, specialWeaponFreq;
JE_word     specialWeaponWpn;
JE_boolean  linkToPlayer;

JE_integer baseArmor, baseArmor2;
JE_word shipGR, shipGR2;
JE_word shipGRSeg, shipGROfs, shipGR2Seg, shipGR2Ofs;
