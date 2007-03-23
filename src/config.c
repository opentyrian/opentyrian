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

#define NO_EXTERNS
#include "config.h"
#undef NO_EXTERNS

#include <stdio.h>
#include <string.h>

/* Configuration Load/Save handler */

const JE_byte CryptKey[10] = /* [1..10] */
{
    15,50,89,240,147,34,86, 9,32,208
};

const JE_KeySettingType DefaultKeySettings =
{
    72,80,75,77,57,28,29,56
};

const char DefaultHighScoreNames[34][23] = /* [1..34] of string [22] */
{/*1P*/
/*TYR*/     "The Prime Chair",    /*13*/
            "Transon Lohk",
            "Javi Onukala",
            "Mantori",
            "Nortaneous",
            "Dougan",
            "Reid",
            "General Zinglon",
            "Late Gyges Phildren",
            "Vykromod",
            "Beppo",
            "Borogar",
            "ShipMaster Carlos",

/*OTHER*/   "Jill",               /*5*/
            "Darcy",
            "Jake Stone",
            "Malvineous Havershim",
            "Marta Louise Velasquez",

/*JAZZ*/    "Jazz Jackrabbit",    /*3*/
            "Eva Earlong",
            "Devan Shell",

/*OMF*/     "Crystal Devroe",     /*11*/
            "Steffan Tommas",
            "Milano Angston",
            "Christian",
            "Shirro",
            "Jean-Paul",
            "Ibrahim Hothe",
            "Angel",
            "Cossette Akira",
            "Raven",
            "Hans Kreissack",

/*DARE*/    "Tyler",              /*2*/
            "Rennis the Rat Guard"
};

const char DefaultTeamNames[22][25] = /* [1..22] of string [24] */
{
    "Jackrabbits",
    "Team Tyrian",
    "The Elam Brothers",
    "Dare to Dream Team",
    "Pinball Freaks",
    "Extreme Pinball Freaks",
    "Team Vykromod",
    "Epic All-Stars",
    "Hans Keissack's WARriors",
    "Team Overkill",
    "Pied Pipers",
    "Gencore Growlers",
    "Microsol Masters",
    "Beta Warriors",
    "Team Loco",
    "The Shellians",
    "Jungle Jills",
    "Murderous Malvineous",
    "The Traffic Department",
    "Clan Mikal",
    "Clan Patrok",
    "Carlos' Crawlers"
};


const JE_EditorItemAvailType InitialItemAvail =
{
    1,1,1,0,0,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0, /*Front/Rear Weapons  1-38*/
    0,0,0,0,0,0,0,0,0,0,1,                                                           /*Fill*/
    1,0,0,0,0,1,0,0,0,1,1,0,1,0,0,0,0,0,                                             /*Sidekicks          51-68*/
    0,0,0,0,0,0,0,0,0,0,0,                                                           /*Fill*/
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                                   /*Special Weapons    81-93*/
    0,0,0,0,0                                                                        /*Fill*/
};

/*  Last 2 bytes = Word

    Max Value = 1680
    X div  60 = Armor  (1-28)
    X div 168 = Shield (1-12)
    X div 280 = Engine (1-06)
*/


JE_boolean smoothies[9]; /* [1..9] */
JE_byte StarShowVGASpecialCode;

/* Stars */
struct
{
    JE_byte SC;
    JE_word SLoc;
    JE_word SMov;
} StarDat[MaxStars]; /* [1..Maxstars] */
JE_word StarY;


/* CubeData */
JE_word lastcubemax, cubemax;
JE_word cubelist[4]; /* [1..4] */

/* High-Score Stuff */
JE_boolean gamehasrepeated;

/* Difficulty */
JE_shortint DifficultyLevel, OldDifficultyLevel, InitialDifficulty;

/* Player Stuff */
JE_longint score, score2;

JE_integer    power, LastPower, PowerAdd;
JE_PItemsType PItems, PItemsPlayer2, PItemsBack, PItemsBack2;
JE_shortint   shield, ShieldMax, ShieldSet;
JE_shortint   shield2, ShieldMax2;
JE_integer    ArmorLevel, ArmorLevel2;
JE_byte       ShieldWait, ShieldT;

JE_byte          ShotRepeat[11], ShotMultiPos[11]; /* [1..11] */  /* 7,8 = Superbomb */
JE_byte          PortConfig[10]; /* [1..10] */
JE_boolean       PortConfigDone;
JE_PortPowerType PortPower, LastPortPower;

JE_boolean ResetVersion;

/* Level Data */
char    LastLevelName[11], LevelName[11]; /* string [10] */
JE_byte Mainlevel, NextLevel, SaveLevel;   /*Current Level #*/

/* Keyboard Junk */
JE_KeySettingType KeySettings;

/* Configuration */
JE_shortint LevelFilter, LevelFilterNew, LevelBrightness, LevelBrightnessChg;
JE_boolean  FiltrationAvail, FilterActive, FilterFade, FilterFadeStart;

JE_boolean GameJustLoaded;

JE_boolean GalagaMode;

JE_boolean ExtraGame;

JE_boolean TwoPlayerMode, TwoPlayerLinked, OnePlayerAction, SuperTyrian, TrentWin;
JE_byte    SuperArcadeMode;

JE_byte    SuperArcadePowerup;

JE_real LinkGunDirec;
JE_byte PlayerDevice1, PlayerDevice2;
JE_byte InputDevice1, InputDevice2;

JE_byte SecretHint;
JE_byte Background3Over;
JE_byte Background2Over;
JE_byte GammaCorrection;
JE_boolean SuperPause,
           ExplosionTransparent,
           YouAreCheating,
           DisplayScore,
           SoundHasChanged,
           Background2, SmoothScroll, Wild, SuperWild, StarActive,
           TopEnemyOver,
           SkyEnemyOverALL,
           Background2NotTransparent,
           tempb;
JE_byte temp;
JE_word tempw;

JE_byte VersionNum;   /*SW 1.0 and SW/Reg 1.1 = 0 or 1
                        EA 1.2 = 2*/

JE_byte    FastPlay;
JE_boolean PentiumMode;

/* Savegame files */
JE_boolean PlayerPasswordInput;
JE_byte    InputDevice;  /* 0=Mouse   1=Joystick   2=Gravis GamePad */
JE_byte    GameSpeed;
JE_byte    ProcessorType;  /* 1=386  2=486  3=Pentium Hyper */

JE_SaveFilesType SaveFiles; /*array[1..savefilesnum] of savefiletype;*/
JE_word savefilesseg, savefilesofs;
JE_SaveGameTemp SaveTemp;
JE_word savetempofs, savetempseg;

JE_word EditorLevel;   /*Initial value 800*/

JE_word x;


const JE_byte StringCryptKey[10] = {99, 204, 129, 63, 255, 71, 19, 25, 62, 1};

void JE_decryptstring( JE_string s, JE_byte len )
{
    int i;

    for (i = len-1; i >= 0; i--)
    {
        s[i] ^= StringCryptKey[((i+1) % 10)];
        if (i > 0)
        {
            s[i] ^= s[i-1];
        }
    }
}

void JE_readcryptln( FILE* f, JE_string s )
{
    int size;

    size = getc(f);

    fread(s, size, 1, f);
/*    s[0] = (char)size;*/

    JE_decryptstring(s, size);

}

void JE_skipcryptln( FILE* f )
{
    char size;

    size = getc(f);

    fseek(f, size, SEEK_CUR);
}
