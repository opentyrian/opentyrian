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
#ifndef CONFIG_H
#define CONFIG_H

#include "opentyr.h"

#include <stdio.h>


#define SaveFilesNum (11 * 2)

#define MaxStars 100

#define SaveFileSize  (sizeof(SaveGameTemp) - 4)
#define SaveFilesSize (sizeof(JE_SaveFilesType))

typedef JE_byte JE_KeySettingType[8]; /* [1..8] */
typedef JE_byte JE_PItemsType[12]; /* [1..12] */

typedef JE_byte JE_EditorItemAvailType[100]; /* [1..100] */

typedef struct
{
    JE_word       encode;
    JE_word       level;
    JE_PItemsType items;
    JE_longint    score;
    JE_longint    score2;
    char          levelname[10]; /* string [9]; */
    JE_char       name[14]; /* [1..14] */
    JE_byte       cubes;
    JE_byte       power[2]; /* [1..2] */
    JE_byte       episode;
    JE_PItemsType lastitems;
    JE_byte       difficulty;
    JE_byte       secrethint;
    JE_byte       input1;
    JE_byte       input2;
    JE_boolean    gamehasrepeated;   /*See if you went from one episode to another*/
    JE_byte       initialdifficulty;

    /* High Scores - Each episode has both sets of 1&2 player selections - with 3 in each */
    JE_longint    highscore1,
                  highscore2;
    char          highscorename[30]; /* string [29] */
    JE_byte       highscorediff;
} JE_SaveFileType;

typedef JE_SaveFileType JE_SaveFilesType[SaveFilesNum]; /* [1..savefilesnum] */
typedef JE_byte JE_SaveGameTemp[sizeof(JE_SaveFilesType) + 4 + 100]; /* [1..sizeof(savefilestype) + 4 + 100] */

typedef JE_byte JE_PortPowerType[7]; /* [1..7] */

#ifndef NO_EXTERNS
extern const JE_byte CryptKey[10];
extern const JE_KeySettingType DefaultKeySettings;
extern const char DefaultHighScoreNames[34][23];
extern const char DefaultTeamNames[22][25];
extern const JE_EditorItemAvailType InitialItemAvail;
extern JE_boolean smoothies[9];
extern JE_byte StarShowVGASpecialCode;
extern struct
{
    JE_byte SC;
    JE_word SLoc;
    JE_word SMov;
} StarDat[MaxStars];
extern JE_word StarY;
extern JE_word lastcubemax, cubemax;
extern JE_word cubelist[4];
extern JE_boolean gamehasrepeated;
extern JE_shortint DifficultyLevel, OldDifficultyLevel, InitialDifficulty;
extern JE_longint score, score2;
extern JE_integer power, LastPower, PowerAdd;
extern JE_PItemsType PItems, PItemsPlayer2, PItemsBack, PItemsBack2;
extern JE_shortint shield, ShieldMax, ShieldSet;
extern JE_shortint shield2, ShieldMax2;
extern JE_integer ArmorLevel, ArmorLevel2;
extern JE_byte ShieldWait, ShieldT;
extern JE_byte ShotRepeat[11], ShotMultiPos[11];
extern JE_byte PortConfig[10];
extern JE_boolean PortConfigDone;
extern JE_PortPowerType PortPower, LastPortPower;
extern JE_boolean ResetVersion;
extern char LastLevelName[11], LevelName[11];
extern JE_byte Mainlevel, NextLevel, SaveLevel;
extern JE_KeySettingType KeySettings;
extern JE_shortint LevelFilter, LevelFilterNew, LevelBrightness, LevelBrightnessChg;
extern JE_boolean FiltrationAvail, FilterActive, FilterFade, FilterFadeStart;
extern JE_boolean GameJustLoaded;
extern JE_boolean GalagaMode;
extern JE_boolean ExtraGame;
extern JE_boolean TwoPlayerMode, TwoPlayerLinked, OnePlayerAction, SuperTyrian, TrentWin;
extern JE_byte SuperArcadeMode;
extern JE_byte SuperArcadePowerup;
extern JE_real LinkGunDirec;
extern JE_byte PlayerDevice1, PlayerDevice2;
extern JE_byte InputDevice1, InputDevice2;
extern JE_byte SecretHint;
extern JE_byte Background3Over;
extern JE_byte Background2Over;
extern JE_byte GammaCorrection;
extern JE_boolean SuperPause, ExplosionTransparent, YouAreCheating, DisplayScore, SoundHasChanged, Background2, SmoothScroll, Wild, SuperWild, StarActive, TopEnemyOver,SkyEnemyOverALL, Background2NotTransparent, tempb;
extern JE_byte temp;
extern JE_word tempw;
extern JE_byte VersionNum;
extern JE_byte FastPlay;
extern JE_boolean PentiumMode;
extern JE_boolean PlayerPasswordInput;
extern JE_byte InputDevice;
extern JE_byte GameSpeed;
extern JE_byte ProcessorType;
extern JE_SaveFilesType SaveFiles;
extern JE_word savefilesseg, savefilesofs;
extern JE_SaveGameTemp SaveTemp;
extern JE_word savetempofs, savetempseg;
extern JE_word EditorLevel;
extern JE_word x;

extern const JE_byte StringCryptKey[10];
#endif

void JE_InitProcessorType( void );
void JE_setnewgamespeed( void );
void JE_loadconfiguration( void );
void JE_saveconfiguration( void );
void JE_readcryptln( FILE* f, JE_string s );
void JE_skipcryptln( FILE* f );

void JE_SetupStars( void );

void JE_savegame( JE_byte slot, JE_string name );
void JE_loadgame( JE_byte slot );

void JE_decryptstring( JE_string s, JE_byte len );

#endif /* CONFIG_H */
