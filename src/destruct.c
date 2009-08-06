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

/* File notes:
 * Two players duke it out in a Scorched Earth style game.
 * Most of the variables referring to the players are global as
 * they are often edited and that's how the original was written.
 *
 * *This file has been restructured to break down the game loop
 * into smaller pieces.  While I was going I also added brackets
 * like an obsession and fixed some small bugs I ran into along the
 * way.  I can only remember a few though; misused variables, magnets
 * pushing choppers, really with all the changes it's hard to remember.
 *
 * I've tried to get rid of globals.  Most temp vars are just fine on the stack.
 * I also haven't forced temp var size restrictions.  An unsigned int going from
 * 0 to 20 should be faster than a byte on all non-8 bit systems.
 * I've replaced a lot of constants with enums.  Many more to go though.
 * I've also added in a lot of comments.  Finding stuff should be easier.
 * Inputs are now recorded as actions, not keys.  The CPU player already
 * sort've did this; I just expanded it to fit both players and removed the
 * duality.  This will make adding two AIs easier :)
 *
 * There is still much to do, but right now things still work as well as they
 * did prior.  I am committing not because I think it's useful but because
 * a commit will make it easier to track improvements later.
 */

/*** Headers ***/
#include "opentyr.h"
#include "destruct.h"

#include "config.h"
#include "fonthand.h"
#include "helptext.h"
#include "keyboard.h"
#include "loudness.h"
#include "mtrand.h"
#include "newshape.h"
#include "nortsong.h"
#include "palette.h"
#include "picload.h"
#include "varz.h"
#include "vga256d.h"
#include "video.h"

/*** Enums and Defines ***/
#define WALL_MAX 20
#define SHOT_MAX 40
#define EXPLO_MAX 40
#define KEY_OPTION_MAX 4

#define SHOT_TYPES 17
#define SYSTEM_TYPES 8

#define MAX_INSTALLATIONS 20

/* There are an awful lot of index-1s scattered throughout the code.
 * I plan to eventually change them all and to be able to properly
 * enumerate from 0 up.
 */
enum de_player_t { PLAYER_LEFT = 0, PLAYER_RIGHT = 1, PLAYER_MAX = 2 };
enum de_mode_t { MODE_5CARDWAR = 1, MODE_TRADITIONAL = 2, MODE_HELIASSAULT = 3,
                 MODE_HELIDEFENSE = 4, MODE_OUTGUNNED = 5 };
enum de_unit_t { UNIT_TANK = 1, UNIT_NUKE = 2, UNIT_DIRT = 3, UNIT_SATELLITE = 4,
                 UNIT_MAGNET = 5, UNIT_LASER = 6, UNIT_JUMPER = 7, UNIT_HELI = 8 };
enum de_weapon_t { WEAPON_TRACER = 1, WEAPON_SMALL = 2, WEAPON_LARGE = 3,
                   WEAPON_MICRO = 4, WEAPON_SUPER = 5, WEAPON_DEMO = 6,
                   WEAPON_SMALLNUKE = 7, WEAPON_LARGENUKE = 8,
                   WEAPON_SMALLDIRT = 9, WEAPON_LARGEDIRT = 10,
                   WEAPON_MAGNET = 11, WEAPON_MINILASER = 12,
                   WEAPON_MEGALASER = 13, WEAPON_LASERTRACER = 14,
                   WEAPON_MEGABLAST = 15, WEAPON_MINI = 16, WEAPON_BOMB = 17 };
enum de_trails_t { TRAILS_NONE = 0, TRAILS_NORMAL = 1, TRAILS_FULL = 2 };
/* keys and moves should line up. */
enum de_keys_t {  KEY_LEFT = 0,  KEY_RIGHT = 1,  KEY_UP = 2,  KEY_DOWN = 3,  KEY_CHANGE = 4,  KEY_FIRE = 5,  KEY_CYUP = 6,  KEY_CYDN = 7,  KEY_MAX = 8};
enum de_move_t { MOVE_LEFT = 0, MOVE_RIGHT = 1, MOVE_UP = 2, MOVE_DOWN = 3, MOVE_CHANGE = 4, MOVE_FIRE = 5, MOVE_CYUP = 6, MOVE_CYDN = 7, MOVE_MAX = 8};

/* The tracerlaser is dummied out.  It works but (probably due to the low
 * SHOT_MAX) is not assigned to anything.  The bomb does not work.
 */


/*** Structs ***/
struct destruct_unit_s {

	unsigned int ani_frame;
	unsigned int health;
};
struct destruct_shot_s {

	double x;
	double y;
	double xmov;
	double ymov;
	bool gravity;
	unsigned int shottype;
	int shotdur; /* This looks to be unused */
	unsigned int trail4x, trail4y, trail4c,
	             trail3x, trail3y, trail3c,
	             trail2x, trail2y, trail2c,
	             trail1x, trail1y, trail1c;
};
struct destruct_explo_s {

	JE_word x;
	JE_byte y;
	JE_byte explowidth;
	JE_byte explomax;
	JE_byte explofill;
	JE_byte explocolor;
};
struct destruct_moves_s {
	bool actions[MOVE_MAX];
};
struct destruct_keys_s {
	SDLKey Config[KEY_MAX][KEY_OPTION_MAX];
};
struct destruct_ai_s {

	int c_Angle, c_Power, c_Fire;
	unsigned int c_noDown;
};
struct destruct_player_s {

	bool is_cpu;
	struct destruct_ai_s aiMemory;

	struct destruct_moves_s moves;
	struct destruct_keys_s  keys;

	unsigned int score;
};

/*** Function decs ***/
void JE_destructMain( void );
void JE_introScreen( void );
void JE_modeSelect( void );

void JE_generateTerrain( void );
void JE_aliasDirt( void );


void DE_ResetPlayers( void );
void DE_ResetUnits( void );
void DE_ResetWeapons( void );
void DE_ResetLevel( void );
void DE_ResetActions( void );

void DE_RunTick( void );
void DE_RunTickGravity( void );
void DE_RunTickAnimate( void );
void DE_RunTickDrawWalls( void );
void DE_RunTickExplosions( void );
void DE_RunTickShots( void );
void DE_RunTickAI(enum de_player_t);
void DE_RunTickDrawCrosshairs( void );
void DE_RunTickDrawHUD( void );
void DE_RunTickGetInput( void );
void DE_RunTickProcessInput( void );
bool DE_RunTickCheckEndgame( void );
void DE_RunTickPlaySounds( void );




JE_byte JE_placementPosition( JE_word x, JE_byte width );
JE_boolean JE_stabilityCheck( JE_integer x, JE_integer y );

void JE_tempScreenChecking( void );

void JE_makeExplosion( JE_word destructTempX, JE_word destructTempY, JE_byte shottype );
void JE_eSound( JE_byte sound );
void JE_superPixel( JE_word loc );

void JE_helpScreen( void );
void JE_pauseScreen( void );



/*** Weapon configurations ***/

const JE_boolean demolish[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {false, false, false, false, false, true, true, true, false, false, false, false, true, false, true, false, true};
const JE_byte shotGr[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 101};
const JE_byte shotTrail[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {TRAILS_NONE, TRAILS_NONE, TRAILS_NONE, TRAILS_NORMAL, TRAILS_NORMAL, TRAILS_NORMAL, TRAILS_FULL, TRAILS_FULL, TRAILS_NONE, TRAILS_NONE, TRAILS_NONE, TRAILS_NORMAL, TRAILS_FULL, TRAILS_NORMAL, TRAILS_FULL, TRAILS_NORMAL, TRAILS_NONE};
const JE_byte shotFuse[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};
const JE_byte shotDelay[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {10, 30, 80, 20, 60, 100, 140, 200, 20, 60, 5, 15, 50, 5, 80, 16, 0};
const JE_byte shotSound[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {8, 2, 1, 7, 7, 9, 22, 22, 5, 13, 10, 15, 15, 26, 14, 7, 7};
const JE_byte exploSize[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {4, 20, 30, 14, 22, 16, 40, 60, 10, 30, 0, 5, 10, 3, 15, 7, 0};
const JE_boolean shotBounce[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, false, true};
const JE_byte exploDensity[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {  2,  5, 10, 15, 20, 15, 25, 30, 40, 80, 0, 30, 30,  4, 30, 5, 0};
const JE_byte shotDirt[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {252, 252, 252, 252, 252, 252, 252, 252, 25, 25, 1, 252, 252, 252, 252, 252, 0};
const JE_byte shotColor[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 10, 10, 10, 10, 16, 0};

const JE_byte defaultWeapon[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {2, 4, 9, 0, 11, 12, 4, 16};
const JE_byte defaultCpuWeapon[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {2, 4, 6, 0, 11, 12, 4, 16};
const JE_byte defaultCpuWeaponB[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {6, 7, 6, 0, 11, 13, 4, 16};
const JE_boolean systemAngle[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {true, true, true, false, false, true, false, false};
const JE_word baseDamage[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {200, 120, 400, 300, 80, 150, 600, 40};
const JE_boolean systemAni[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {false, false, false, true, false, false, false, true};

const JE_byte weaponSystems[SYSTEM_TYPES][SHOT_TYPES] /*[1..SYSTEM_TYPES, 1..SHOT_TYPES]*/ =
{
	{1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // normal
	{0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // nuke
	{0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0}, // dirt
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // worthless
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}, // magnet
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0}, // laser
	{1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, // jumper
	{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}  // helicopter
};

const JE_byte leftGraphicBase[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {1, 6, 11, 58, 63, 68, 96, 153};
const JE_byte rightGraphicBase[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {20, 25, 30, 77, 82, 87, 115, 172};

const JE_byte lModeScore[DESTRUCT_MODES] /*[1..destructmodes]*/ = {1, 0, 0, 5, 0};
const JE_byte rModeScore[DESTRUCT_MODES] /*[1..destructmodes]*/ = {1, 0, 5, 0, 1};

const SDLKey defaultKeyConfig[PLAYER_MAX][KEY_MAX][KEY_OPTION_MAX] =
{
	{	{SDLK_c},
		{SDLK_v},
		{SDLK_a},
		{SDLK_z},
		{SDLK_LALT},
		{SDLK_x, SDLK_LSHIFT},
		{SDLK_LCTRL},
		{SDLK_SPACE}
	},
	{	{SDLK_LEFT, SDLK_KP4},
		{SDLK_RIGHT, SDLK_KP6},
		{SDLK_UP, SDLK_KP8},
		{SDLK_DOWN, SDLK_KP2},
		{SDLK_BACKSLASH, SDLK_KP5},
		{SDLK_INSERT, SDLK_RETURN, SDLK_KP0, SDLK_KP_ENTER},
		{SDLK_PAGEUP, SDLK_KP9},
		{SDLK_PAGEDOWN, SDLK_KP3}
	}
};


/*** Globals ***/
SDL_Surface *destructTempScreen;
JE_byte endDelay;
JE_boolean died = false;
JE_boolean destructFirstTime;

JE_byte destructMode;  /*Game Mode - See Tyrian.HTX*/

JE_byte arenaType;
JE_boolean haveWalls;




struct destruct_player_s player[PLAYER_MAX];
struct destruct_unit_s unit[PLAYER_MAX][MAX_INSTALLATIONS];

JE_byte leftAvail, rightAvail;
JE_byte leftSel, rightSel;
JE_byte leftshotDelay, rightshotDelay;

JE_shortint leftLastMove[MAX_INSTALLATIONS], rightLastMove[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/

JE_byte leftShotType[MAX_INSTALLATIONS], leftSystem[MAX_INSTALLATIONS],
        rightShotType[MAX_INSTALLATIONS], rightSystem[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/

JE_word leftX[MAX_INSTALLATIONS], rightX[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/

JE_real leftAngle[MAX_INSTALLATIONS], leftPower[MAX_INSTALLATIONS], leftYMov[MAX_INSTALLATIONS], leftY[MAX_INSTALLATIONS],
        rightAngle[MAX_INSTALLATIONS], rightPower[MAX_INSTALLATIONS], rightYMov[MAX_INSTALLATIONS], rightY[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/

JE_boolean leftYInAir[MAX_INSTALLATIONS],
           rightYInAir[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/

JE_boolean wallExist[20]; /*[1..20]*/
JE_byte wallsX[20], wallsY[20]; /*[1..20]*/

JE_byte dirtHeight[320]; /*[0..319]*/

JE_boolean destructShotAvail[SHOT_MAX]; /*[1..shotmax]*/

struct destruct_shot_s shotRec[SHOT_MAX]; /*[1..shotmax]*/

JE_boolean endOfGame, destructQuit;


JE_boolean destructFound;
JE_real destructTempR;

JE_boolean explosionAvail[EXPLO_MAX]; /*[1..explomax]*/

struct destruct_explo_s exploRec[EXPLO_MAX]; /*[1..explomax]*/


void JE_destructGame( void )
{
	destructFirstTime = true;
	JE_clr256();
	JE_showVGA();
	endOfGame = false;
	destructTempScreen = game_screen;
	JE_loadCompShapes(&eShapes1, &eShapes1Size, '~');
	JE_fadeBlack(1);

	JE_destructMain();
}

void JE_destructMain( void )
{
	JE_loadPic(11, false);
	JE_introScreen();

	DE_ResetPlayers();
	player[PLAYER_LEFT].is_cpu = true;

	do {
		JE_modeSelect();

		if (!destructQuit)
		{
			do
			{
				destructQuit = false;
				endOfGame = false;

				destructFirstTime = true;
				JE_loadPic(11, false);

				haveWalls = mt_rand() % 2; /* I don't like having this here */

				DE_ResetLevel();
				do
				{
					DE_RunTick();
				} while (!destructQuit && !(died && endDelay == 0));

				destructQuit = false;
				died = false;
				JE_fadeBlack(25);
			}
			while (!endOfGame);
		}
	}
	while (!destructQuit);
}

void JE_introScreen( void )
{
	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->h * VGAScreen2->pitch);
	JE_outText(JE_fontCenter(specialName[8-1], TINY_FONT), 90, specialName[8-1], 12, 5);
	JE_outText(JE_fontCenter(miscText[65-1], TINY_FONT), 180, miscText[65-1], 15, 2);
	JE_outText(JE_fontCenter(miscText[66-1], TINY_FONT), 190, miscText[66-1], 15, 2);
	JE_showVGA();
	JE_fadeColor(15);

	newkey = false;
	while (!newkey)
	{
		service_SDL_events(false);
		SDL_Delay(16);
	}

	JE_fadeBlack(15);
	memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->h * VGAScreen->pitch);
	JE_showVGA();
}

/* JE_modeSelect
 *
 * This function prints the DESTRUCT mode selection menu.
 * Its main purpose is to set the variable 'destructMode'.
 */
void JE_modeSelect( void )
{
	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->h * VGAScreen2->pitch);
	destructMode = MODE_5CARDWAR;

	destructFirstTime = true;
	destructQuit = false;

	do {
		for (int x = 1; x <= DESTRUCT_MODES; x++)
		{   /* This code prints the available game modes */
			temp = (x == destructMode) * 4; /* This odd looking code just highlights the selected menu option */
			JE_textShade(JE_fontCenter(destructModeName[x-1], TINY_FONT), 70 + x * 12, destructModeName[x-1], 12, temp, FULL_SHADE);
		}
		JE_showVGA();

		if (destructFirstTime)
		{   /* This code fades us in... */
			JE_fadeColor(15);
			destructFirstTime = false;
		}

		/* This code just grabs keys and edits destructmode as needed */
		newkey = false;
		while (!newkey)
		{
			service_SDL_events(false);
			SDL_Delay(16);
		}
		if (keysactive[SDLK_ESCAPE])
		{
			destructQuit = true;
			break;
		}
		if (keysactive[SDLK_RETURN])
		{
			break;
		}
		if (keysactive[SDLK_UP])
		{
			destructMode--;
			if (destructMode < 1)
				destructMode = 5;
		}
		if (keysactive[SDLK_DOWN])
		{
			destructMode++;
			if (destructMode > 5)
				destructMode = 1;
		}


	} while (!destructQuit && !keysactive[SDLK_RETURN]);

	JE_fadeBlack(15);
	memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->h * VGAScreen->pitch);
	JE_showVGA();
}

void JE_generateTerrain( void )
{
	const JE_byte goodsel[14] /*[1..14]*/ = {1, 2, 6, 12, 13, 14, 17, 23, 24, 26, 28, 29, 32, 33};

	const JE_byte basetypes[8][11] /*[1..8, 1..11]*/ = /*0 is amount of units*/
	{
		{5, 1, 1, 2, 3, 3, 4, 5, 6, 7, 8},   /*Normal*/
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},   /*Traditional*/
		{4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},   /*Weak   Heli attack fleet*/
		{8, 1, 1, 1, 2, 2, 2, 3, 5, 6, 7},   /*Strong Heli defense fleet*/
		{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},   /*Strong Heli attack fleet*/
		{4, 1, 1, 1, 1, 2, 2, 3, 5, 7, 7},   /*Weak   Heli defense fleet*/
		{8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2},   /*Overpowering fleet*/
		{4, 1, 1, 2, 3, 1, 6, 7, 8, 2, 7}    /*Weak fleet*/
	};

	const JE_byte Lbaselookup[DESTRUCT_MODES] /*[1..destructmodes]*/ = {1, 2, 4, 5, 7};
	const JE_byte Rbaselookup[DESTRUCT_MODES] /*[1..destructmodes]*/ = {1, 2, 3, 6, 8};

	JE_byte newheight, oldheight;
	JE_shortint heightchange;
	JE_real sinewave, sinewave2, cosinewave, cosinewave2;
	JE_word x, y, z;
	JE_byte HC1;
	JE_byte temp;
	JE_word destructTempX, destructTempY, destructTempX2, destructTempY2;
	JE_byte arenaType2;

	arenaType = (mt_rand() % 4) + 1;
	arenaType2 = (mt_rand() % 4) + 1;

	/*  Type                 Type2
	    1 = normal           1 = Normal
	    2 = fuzzy walls
	    3 = ballzies
	    4 = highwalls
	*/

	play_song(goodsel[mt_rand() % 14] - 1);
	heightchange = (mt_rand() % 3) - 1;

	sinewave = ((float)mt_rand() / MT_RAND_MAX) * M_PI / 50.0f + 0.01f;
	sinewave2 = ((float)mt_rand() / MT_RAND_MAX) * M_PI / 50.0f + 0.01f;
	cosinewave = ((float)mt_rand() / MT_RAND_MAX) * M_PI / 50.0f + 0.01f;
	cosinewave2 = ((float)mt_rand() / MT_RAND_MAX) * M_PI / 50.0f + 0.01f;
	HC1 = 20;

	switch (arenaType)
	{
		case 1:
			sinewave = M_PI - ((float)mt_rand() / MT_RAND_MAX) * 0.3f;
			sinewave2 = M_PI - ((float)mt_rand() / MT_RAND_MAX) * 0.3f;
			break;
		case 2:
			HC1 = 100;
			break;
	}

	for (x = 1; x <= 318; x++)
	{
		newheight = round(
			sin(sinewave * x) * HC1 + sin(sinewave2 * x) * 15.0f +
			cos(cosinewave * x) * 10.0f + sin(cosinewave2 * x) * 15.0f
		) + 130;

		if (newheight < 40)
			newheight = 40;
		if (newheight > 195)
			newheight = 195;

		dirtHeight[x] = newheight;
	}

	tempW = 0;
	leftAvail = 0;

	for (x = 0; x < basetypes[Lbaselookup[destructMode-1]-1][1-1]; x++)
	{
		leftX[x] = (mt_rand() % 120) + 10;
		leftY[x] = JE_placementPosition(leftX[x] - 1, 14);
		leftSystem[x] = basetypes[Lbaselookup[destructMode-1]-1][(mt_rand() % 10) + 2-1];
		if (leftSystem[x] == 4)
		{
			if (tempW == MAX_INSTALLATIONS - 1)
			{
				leftSystem[x] = 1;
			} else {
				leftY[x] = 30 + (mt_rand() % 40);
				tempW++;
			}
		}
		unit[PLAYER_LEFT][x].health = baseDamage[leftSystem[x]-1];
		if (leftSystem[x] != 4)
			leftAvail++;
	}

	tempW = 0;
	rightAvail = 0;

	for (x = 0; x < basetypes[Rbaselookup[destructMode-1]-1][1-1]; x++)
	{
		rightX[x] = 320 - ((mt_rand() % 120) + 22);
		rightY[x] = JE_placementPosition(rightX[x] - 1, 14);
		rightYMov[x] = 0;
		rightYInAir[x] = false;
		rightSystem[x] = basetypes[Rbaselookup[destructMode-1]-1][(mt_rand() % 10) + 2-1];
		if (rightSystem[x] == 4)
		{
			if (tempW == MAX_INSTALLATIONS - 1)
			{
				rightSystem[x] = 1;
			} else {
				rightY[x] = 30 + (mt_rand() % 40);
				tempW++;
			}
		}
		unit[PLAYER_RIGHT][x].health = baseDamage[rightSystem[x]-1];
		if (rightSystem[x] != 4)
			rightAvail++;
	}

	for (z = 0; z < WALL_MAX; z++)
		wallExist[z] = false;

	if (haveWalls)
	{
		tempW = 20;
		do {

			temp = (mt_rand() % 5) + 1;
			if (temp > tempW)
				temp = tempW;

			do {
				x = (mt_rand() % 300) + 10;

				destructFound = true;

				for (z = 0; z < 4; z++)
					if ((x > leftX[z] - 12) && (x < leftX[z] + 13))
						destructFound = false;
				for (z = 0; z < 4; z++)
					if ((x > rightX[z] - 12) && (x < rightX[z] + 13))
						destructFound = false;

			} while (!destructFound);

			for (z = 1; z <= temp; z++)
			{
				wallExist[tempW - z + 1-1] = true;
				wallsX[tempW - z + 1-1] = x;

				wallsY[tempW - z + 1-1] = JE_placementPosition(x, 12) - 14 * z;
			}

			tempW -= temp;

		} while (tempW != 0);
	}

	for (x = 1; x <= 318; x++)
		JE_rectangle(x, dirtHeight[x], x, 199, 25);

	if (arenaType == 3)
	{ /*RINGIES!!!!*/
		int rings = mt_rand() % 6 + 1;
		for (x = 1; x <= rings; x++)
		{
			destructTempX = (mt_rand() % 320);
			destructTempY = (mt_rand() % 160) + 20;
			y = (mt_rand() % 40) + 10;  /*Size*/

			for (z = 1; z <= y * y * 2; z++)
			{
				destructTempR = ((float)mt_rand() / MT_RAND_MAX) * (M_PI * 2);
				destructTempY2 = destructTempY + round(cos(destructTempR) * (((float)mt_rand() / MT_RAND_MAX) * 0.1f + 0.9f) * y);
				destructTempX2 = destructTempX + round(sin(destructTempR) * (((float)mt_rand() / MT_RAND_MAX) * 0.1f + 0.9f) * y);
				if ((destructTempY2 > 12) && (destructTempY2 < 200) && (destructTempX2 > 0) && (destructTempX2 < 319))
					((Uint8 *)VGAScreen->pixels)[destructTempX2 + destructTempY2 * VGAScreen->pitch] = 25;
			}
		}
	}
	if (arenaType2 == 2)
	{ /*HOLES*/
		int holes = (mt_rand() % 6) + 1;
		for (x = 1; x <= holes; x++)
		{
			destructTempX = (mt_rand() % 320);
			destructTempY = (mt_rand() % 160) + 20;
			y = (mt_rand() % 40) + 10;  /*Size*/

			for (z = 1; z < y * y * 2; z++)
			{
				destructTempR = ((float)mt_rand() / MT_RAND_MAX) * (M_PI * 2);
				destructTempY2 = destructTempY + round(cos(destructTempR) * (((float)mt_rand() / MT_RAND_MAX) * 0.1f + 0.9f) * y);
				destructTempX2 = destructTempX + round(sin(destructTempR) * (((float)mt_rand() / MT_RAND_MAX) * 0.1f + 0.9f) * y);
				if ((destructTempY2 > 12) && (destructTempY2 < 200) && (destructTempX2 > 0) && (destructTempX2 < 319))
					((Uint8 *)VGAScreen->pixels)[destructTempX2 + destructTempY2 * VGAScreen->pitch] = 0;
			}
		}
	}

	JE_aliasDirt();

	JE_showVGA();

	memcpy(destructTempScreen->pixels, VGAScreen->pixels, destructTempScreen->pitch * destructTempScreen->h);
}

void JE_aliasDirt( void )
{
	Uint8 *s = VGAScreen->pixels;
	s += 12 * VGAScreen->pitch;

	for (int y = 12; y < VGAScreen->h; y++)
	{
		for (int x = 0; x < VGAScreen->pitch; x++)
		{
			if (*s == 0)
			{
				int temp = 0;
				if (*(s - VGAScreen->pitch) == 25)
					temp += 1;
				if (y < VGAScreen2->h - 1)
					if (*(s + VGAScreen->pitch) == 25)
						temp += 3;
				if (x > 0)
					if (*(s - 1) == 25)
						temp += 2;
				if (x < VGAScreen2->pitch - 1)
					if (*(s + 1) == 25)
						temp += 2;
				if (temp)
					*s = temp + 16;
			}
			s++;
		}
	}
}

JE_byte JE_placementPosition( JE_word x, JE_byte width )
{
	JE_word y, z;

	y = 0;
	for (z = x; z <= x + width - 1; z++)
		if (y < dirtHeight[z])
			y = dirtHeight[z];

	for (z = x; z <= x + width - 1; z++)
		dirtHeight[z] = y;

	return y;
}

JE_boolean JE_stabilityCheck( JE_integer x, JE_integer y )
{
	JE_word z, tempW, tempW2;

	tempW2 = 0;
	tempW = x + y * destructTempScreen->pitch - 2;
	for (z = 1; z <= 12; z++)
		if (((Uint8 *)destructTempScreen->pixels)[tempW + z] == 25)
			tempW2++;

	return (tempW2 < 10);
}

void JE_tempScreenChecking( void ) /*and copy to vgascreen*/
{
	Uint8 *s = VGAScreen->pixels;
	s += 12 * VGAScreen->pitch;

	Uint8 *temps = destructTempScreen->pixels;
	temps += 12 * destructTempScreen->pitch;

	for (int y = 12; y < VGAScreen->h; y++)
	{
		for (int x = 0; x < VGAScreen->pitch; x++)
		{
			if (*temps & 0x80 && *temps >= 241)
			{
				if (*temps == 241)
					*temps = 0;
				else
					(*temps)--;
			}
			*s = *temps;

			s++;
			temps++;
		}
	}
}

void JE_makeExplosion( JE_word destructTempX, JE_word destructTempY, JE_byte shottype )
{
	JE_word tempW = 0;
	JE_byte temp;

	for (temp = 0; temp < EXPLO_MAX; temp++)
		if (explosionAvail[temp])
			tempW = temp + 1;

	if (tempW > 0)
	{
		explosionAvail[tempW-1] = false;

		temp = exploSize[shottype-1];
		if (temp < 5)
			JE_eSound(3);
		else if (temp < 15)
			JE_eSound(4);
		else if (temp < 20)
			JE_eSound(12);
		else if (temp < 40)
			JE_eSound(11);
		else {
			JE_eSound(12);
			JE_eSound(11);
		}

		exploRec[tempW-1].x = destructTempX;
		exploRec[tempW-1].y = destructTempY;
		exploRec[tempW-1].explowidth = 2;

		if (shottype > 0)
		{
			exploRec[tempW-1].explomax = exploSize[shottype-1];
			exploRec[tempW-1].explofill = exploDensity[shottype-1];
			exploRec[tempW-1].explocolor = shotDirt[shottype-1];
		} else {
			exploRec[tempW-1].explomax = (mt_rand() % 40) + 10;
			exploRec[tempW-1].explofill = (mt_rand() % 60) + 20;
			exploRec[tempW-1].explocolor = 252;
		}
	}
}

void JE_eSound( JE_byte sound )
{
	static int exploSoundChannel = 0;

	if (++exploSoundChannel > 5)
		exploSoundChannel = 1;

	soundQueue[exploSoundChannel] = sound;
}

void JE_superPixel( JE_word loc )
{
	Uint8 *s = destructTempScreen->pixels;
	int loc_max = destructTempScreen->pitch * destructTempScreen->h;

	if (loc > 0 && loc < loc_max)
	{
		/* center */
		if (s[loc] < 252)
			s[loc] = 252;
		else if (s[loc] < 255 - 4)
			s[loc] += 4;
		else
			s[loc] = 255;
	}

	if (loc - 1 > 0 && loc - 1 < loc_max)
	{
		/* left 1 */
		if (s[loc - 1] < 249)
			s[loc - 1] = 249;
		else if (s[loc - 1] < 255 - 2)
			s[loc - 1] += 2;
		else
			s[loc - 1] = 255;
	}

	if (loc - 2 > 0 && loc - 2 < loc_max)
	{
		/* left 2 */
		if (s[loc - 2] < 246)
			s[loc - 2] = 246;
		else if (s[loc - 2] < 255 - 1)
			s[loc - 2] += 1;
		else
			s[loc - 2] = 255;
	}

	if (loc + 1 > 0 && loc + 1 < loc_max)
	{
		/* right 1 */
		if (s[loc + 1] < 249)
			s[loc + 1] = 249;
		else if (s[loc + 1] < 255 - 2)
			s[loc + 1] += 2;
		else
			s[loc + 1] = 255;
	}

	if (loc + 2 > 0 && loc + 2 < loc_max)
	{
		/* right 2 */
		if (s[loc + 2] < 246)
			s[loc + 2] = 246;
		else if (s[loc + 2] < 255 - 1)
			s[loc + 2] += 1;
		else
			s[loc + 2] = 255;
	}

	if (loc - destructTempScreen->pitch > 0 && loc - destructTempScreen->pitch < loc_max)
	{
		/* up 1 */
		if (s[loc - destructTempScreen->pitch] < 249)
			s[loc - destructTempScreen->pitch] = 249;
		else if (s[loc - destructTempScreen->pitch] < 255 - 2)
			s[loc - destructTempScreen->pitch] += 2;
		else
			s[loc - destructTempScreen->pitch] = 255;
	}

	if (loc - destructTempScreen->pitch - 1 > 0 && loc - destructTempScreen->pitch - 1 < loc_max)
	{
		/* up 1, left 1 */
		if (s[loc - destructTempScreen->pitch - 1] < 247)
			s[loc - destructTempScreen->pitch - 1] = 247;
		else if (s[loc - destructTempScreen->pitch - 1] < 255 - 1)
			s[loc - destructTempScreen->pitch - 1] += 1;
		else
			s[loc - destructTempScreen->pitch - 1] = 255;
	}

	if (loc - destructTempScreen->pitch + 1 > 0 && loc - destructTempScreen->pitch + 1 < loc_max)
	{
		/* up 1, right 1 */
		if (s[loc - destructTempScreen->pitch + 1] < 249)
			s[loc - destructTempScreen->pitch + 1] = 249;
		else if (s[loc - destructTempScreen->pitch + 1] < 255 - 2)
			s[loc - destructTempScreen->pitch + 1] += 2;
		else
			s[loc - destructTempScreen->pitch + 1] = 255;
	}

	if (loc - destructTempScreen->pitch * 2 > 0 && loc - destructTempScreen->pitch * 2 < loc_max)
	{
		/* up 2 */
		if (s[loc - destructTempScreen->pitch * 2] < 246)
			s[loc - destructTempScreen->pitch * 2] = 246;
		else if (s[loc - destructTempScreen->pitch * 2] < 255 - 1)
			s[loc - destructTempScreen->pitch * 2] += 1;
		else
			s[loc - destructTempScreen->pitch * 2] = 255;
	}

	if (loc + destructTempScreen->pitch > 0 && loc + destructTempScreen->pitch < loc_max)
	{
		/* down 1 */
		if (s[loc + destructTempScreen->pitch] < 249)
			s[loc + destructTempScreen->pitch] = 249;
		else if (s[loc + destructTempScreen->pitch] < 255 - 2)
			s[loc + destructTempScreen->pitch] += 2;
		else
			s[loc + destructTempScreen->pitch] = 255;
	}

	if (loc + destructTempScreen->pitch - 1 > 0 && loc + destructTempScreen->pitch - 1 < loc_max)
	{
		/* down 1, left 1 */
		if (s[loc + destructTempScreen->pitch - 1] < 247)
			s[loc + destructTempScreen->pitch - 1] = 247;
		else if (s[loc + destructTempScreen->pitch - 1] < 255 - 1)
			s[loc + destructTempScreen->pitch - 1] += 1;
		else
			s[loc + destructTempScreen->pitch - 1] = 255;
	}

	if (loc + destructTempScreen->pitch + 1 > 0 && loc + destructTempScreen->pitch + 1 < loc_max)
	{
		/* down 1, right 1 */
		if (s[loc + destructTempScreen->pitch + 1] < 247)
			s[loc + destructTempScreen->pitch + 1] = 247;
		else if (s[loc + destructTempScreen->pitch + 1] < 255 - 1)
			s[loc + destructTempScreen->pitch + 1] += 1;
		else
			s[loc + destructTempScreen->pitch + 1] = 255;
	}

	if (loc + destructTempScreen->pitch * 2 > 0 && loc + destructTempScreen->pitch * 2 < loc_max)
	{
		/* down 2 */
		if (s[loc + destructTempScreen->pitch * 2] < 246)
			s[loc + destructTempScreen->pitch * 2] = 246;
		else if (s[loc + destructTempScreen->pitch * 2] < 255 - 1)
			s[loc + destructTempScreen->pitch * 2] += 1;
		else
			s[loc + destructTempScreen->pitch * 2] = 255;
	}
}

void JE_helpScreen( void )
{
	//JE_getVGA();  didn't do anything anyway?
	JE_fadeBlack(15);
	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->h * VGAScreen2->pitch);
	JE_clr256();

	for(x = 0; x < 2; x++)
	{
		JE_outText(100, 5 + x * 90, destructHelp[x * 12 + 1-1], 2, 4);
		JE_outText(100, 15 + x * 90, destructHelp[x * 12 + 2-1], 2, 1);
		for (y = 3; y <= 12; y++)
		{
			JE_outText(((y - 1) % 2) * 160 + 10, 15 + ((y - 1) / 2) * 12 + x * 90, destructHelp[x * 12 + y-1], 1, 3);
		}
	}
	JE_outText(30, 190, destructHelp[25-1], 3, 4);
	JE_showVGA();
	JE_fadeColor(15);

	do {
		service_SDL_events(true);
		SDL_Delay(16);
	} while (!newkey);

	JE_fadeBlack(15);
	memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->h * VGAScreen->pitch);
	JE_showVGA();
	JE_fadeColor(15);
}

void JE_pauseScreen( void )
{
	set_volume(tyrMusicVolume / 2, fxVolume);

	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->h * VGAScreen2->pitch);
	JE_outText(JE_fontCenter(miscText[23-1], TINY_FONT), 90, miscText[23-1], 12, 5);
	JE_showVGA();

	do {
		service_SDL_events(true);
		SDL_Delay(16);
	} while (!newkey);

	memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->h * VGAScreen->pitch);
	JE_showVGA();

	set_volume(tyrMusicVolume, fxVolume);
}


/*** Functions that break Destruct down into more manageable pieces ***/

/* DE_ResetX
 *
 * The reset functions clear the state of whatefer they are assigned to.
 */
void DE_ResetPlayers( void )
{
	unsigned int i;


	for (i = 0; i < PLAYER_MAX; ++i)
	{
		player[i].is_cpu = false;
		player[i].score = 0;
		player[i].aiMemory.c_Angle = 0;
		player[i].aiMemory.c_Power = 0;
		player[i].aiMemory.c_Fire = 0;
		player[i].aiMemory.c_noDown = 0;
		memcpy(player[i].keys.Config, defaultKeyConfig[i], sizeof(player[i].keys.Config));
	}
}
void DE_ResetUnits( void )
{
	for (unsigned int p = 0; p < COUNTOF(unit); ++p)
	{
		for (unsigned int u = 0; u < COUNTOF(unit[p]); ++u)
		{
			unit[p][u].ani_frame = 0;
			unit[p][u].health = 0;
		}
	}
}
void DE_ResetWeapons( void )
{
	unsigned int i;


	for (i = 0; i < SHOT_MAX; i++)
	{
		destructShotAvail[i] = true;
	}
	for (i = 0; i < EXPLO_MAX; i++)
	{
		explosionAvail[i] = true;
	}
}
void DE_ResetLevel( void )
{
	/* Okay, let's prep the arena */
	DE_ResetUnits();
	DE_ResetWeapons();

	JE_generateTerrain();

	leftSel = 1;
	rightSel = 1;

	for (z = 0; z < MAX_INSTALLATIONS; z++)
	{
		leftLastMove[z] = 0;
		rightLastMove[z] = 0;
		leftYMov[z] = 0;
		rightYMov[z] = 0;

		if (player[PLAYER_LEFT].is_cpu)
		{
			if (systemAngle[leftSystem[z]-1] || leftSystem[z] == 8)
			{
				leftAngle[z] = M_PI / 4.0f;
				leftPower[z] = 4;
			} else {
				leftAngle[z] = 0;
				leftPower[z] = 4;
			}
			if (haveWalls)
				leftShotType[z] = defaultCpuWeaponB[leftSystem[z]-1];
			else
				leftShotType[z] = defaultCpuWeapon[leftSystem[z]-1];
		} else {
			leftAngle[z] = 0;
			leftPower[z] = 3;
			leftShotType[z] = defaultWeapon[leftSystem[z]-1];
		}
		rightAngle[z] = 0;
		rightPower[z] = 3;
		rightShotType[z] = defaultWeapon[rightSystem[z]-1];
	}
}
void DE_ResetActions( void )
{
	unsigned int i;


	for(i = 0; i < PLAYER_MAX; i++)
	{	/* Zero it all.  A memset would do the trick */
		memset(&(player[i].moves), 0, sizeof(player[i].moves));
	}
}
/* DE_RunTick
 *
 * Runs one tick.  One tick involves handling physics, drawing crap,
 * moving projectiles and explosions, and getting input.
 */
void DE_RunTick( void )
{
	unsigned int i;


	setjasondelay(1);

	memset(soundQueue, 0, sizeof(soundQueue));
	JE_tempScreenChecking();

	if (!died)
	{
		DE_ResetActions();

		/* This code automatically switches the active unit if it is destroyed
		 * and skips over the useless satellite */
		while (leftShotType[leftSel-1] == 0 || unit[0][leftSel-1].health == 0)
		{
			leftSel++;
			if (leftSel > MAX_INSTALLATIONS)
				leftSel = 1;
		}

		/*RightSkipNoShooters*/
		while (rightShotType[rightSel-1] == 0 || unit[1][rightSel-1].health == 0)
		{
			rightSel++;
			if (rightSel > MAX_INSTALLATIONS)
				rightSel = 1;
		}
	}

	DE_RunTickGravity();

	/* This fixes the power of sats.  I'd really like to remove it and fix it properly */
	for (z = 0; z < MAX_INSTALLATIONS; z++)
	{
		if (leftSystem[z] == 6)
			leftPower[z] = 6;
		if (rightSystem[z] == 6)
			rightPower[z] = 6;
	}

	DE_RunTickAnimate();
	DE_RunTickDrawWalls();
	DE_RunTickExplosions();
	DE_RunTickShots();

	/* These are used by the AI */
	/* Todo: move when appropriate */
	for(i = 0; i < PLAYER_MAX; i++)
	{
		if (player[i].is_cpu)
		{
			DE_RunTickAI(i);
		}
	}

	DE_RunTickDrawCrosshairs();
	DE_RunTickDrawHUD();
	JE_showVGA();

	if (destructFirstTime)
	{
		JE_fadeColor(25);
		destructFirstTime = false;
	}

	DE_RunTickGetInput();
	DE_RunTickProcessInput();


	if (!died)
	{
		died = DE_RunTickCheckEndgame();
	}

	DE_RunTickPlaySounds();

	/* The rest of this cruft needs to be put in appropriate sections */
	if (keysactive[SDLK_F10])
	{
		player[PLAYER_LEFT].is_cpu = !player[PLAYER_LEFT].is_cpu;
		keysactive[SDLK_F10] = false;
	}

	if (keysactive[SDLK_p])
	{
		JE_pauseScreen();
		keysactive[lastkey_sym] = false;
	}

	if (keysactive[SDLK_F1])
	{
		JE_helpScreen();
		keysactive[lastkey_sym] = false;
	}

	wait_delay();

	if (keysactive[SDLK_ESCAPE])
	{
		destructQuit = true;
		endOfGame = true;
		keysactive[SDLK_ESCAPE] = false;
	}

	if (keysactive[SDLK_BACKSPACE])
	{
		destructQuit = true;
		keysactive[SDLK_BACKSPACE] = false;
	}

	if (endDelay > 0)
		endDelay--;
}

/* DE_RunTickX
 *
 * Handles something that we do once per tick, such as
 * track ammo and move asplosions.
 */
void DE_RunTickGravity( void )
{
	unsigned int z;

	for (z = 0; z < MAX_INSTALLATIONS; z++)
	{
		if (unit[PLAYER_LEFT][z].health > 0)
		{
			if (leftSystem[z] != 4)
			{
				if (leftYInAir[z])
				{
					if (leftY[z] + leftYMov[z] > 199)
					{
						leftYMov[z] = 0;
						leftYInAir[z] = false;
					}
					leftY[z] += leftYMov[z];
					if (leftY[z] < 26)
					{
						leftYMov[z] = 0;
						leftY[z] = 26;
					}
					if (leftSystem[z] == UNIT_HELI)
					{
						leftYMov[z] += 0.0001f;
					} else {
						leftYMov[z] += 0.03f;
					}
					if (!JE_stabilityCheck(leftX[z], round(leftY[z])))
					{
						leftYMov[z] = 0;
						leftYInAir[z] = false;
					}
				} else if (leftY[z] < 199) {
					if (JE_stabilityCheck(leftX[z], round(leftY[z])))
						leftY[z] += 1;
				}
				temp = leftGraphicBase[leftSystem[z]-1] + unit[0][z].ani_frame;
				if (leftSystem[z] == UNIT_HELI)
				{
					if (leftLastMove[z] < -2)
					{
						temp += 5;
					} else if (leftLastMove[z] > 2) {
						temp += 10;
					}
				} else {
					temp += floor(leftAngle[z] * 9.99f / M_PI);
				}

				JE_drawShape2(leftX[z], round(leftY[z]) - 13, temp, eShapes1);
			} else {
				JE_drawShape2(leftX[z], round(leftY[z]) - 13, leftGraphicBase[leftSystem[z]-1] + unit[0][z].ani_frame, eShapes1);
			}
		}
	}
	for (z = 0; z < MAX_INSTALLATIONS; z++)
	{
		if (unit[PLAYER_RIGHT][z].health > 0)
		{
			if (rightSystem[z] != 4)
			{
				if (rightYInAir[z])
				{
					if (rightY[z] + rightYMov[z] > 199)
					{
						rightYMov[z] = 0;
						rightYInAir[z] = false;
					}
					rightY[z] += rightYMov[z];
					if (rightY[z] < 24)
					{
						rightYMov[z] = 0;
						rightY[z] = 24;
					}
					if (rightSystem[z] == UNIT_HELI) /*HELI*/
					{
						rightYMov[z] += 0.0001f;
					} else {
						rightYMov[z] += 0.03f;
					}
					if (!JE_stabilityCheck(rightX[z], round(rightY[z])))
					{
						rightYMov[z] = 0;
						rightYInAir[z] = false;
					}
				} else if (rightY[z] < 199)
					if (JE_stabilityCheck(rightX[z], round(rightY[z])))
					{
						if (rightSystem[z] == UNIT_HELI)
						{
							rightYMov[z] += 0.01f;
						} else {
							rightY[z] += 1;
						}
					}

				temp = rightGraphicBase[rightSystem[z]-1] + unit[1][z].ani_frame;
				if (rightSystem[z] == 8)
				{
					if (rightLastMove[z] < -2)
					{
						temp += 5;
					} else if (rightLastMove[z] > 2) {
						temp += 10;
					}
				} else {
					temp += floor(rightAngle[z] * 9.99f / M_PI);
				}

				JE_drawShape2(rightX[z], round(rightY[z]) - 13, temp, eShapes1);
			} else {
				JE_drawShape2(rightX[z], round(rightY[z]) - 13, rightGraphicBase[rightSystem[z]-1] + unit[1][z].ani_frame, eShapes1);
			}
		}
	}
}
void DE_RunTickAnimate( void )
{
	unsigned int p, u;

	for (p = 0; p < PLAYER_MAX; ++p)
	{
		for (u = 0; u < MAX_INSTALLATIONS; ++u)
		{
			// if unit does not animate continuously and is not currently animated
			if (!systemAni[ (p == 0 ? leftSystem[u] : rightSystem[u]) -1] && unit[p][u].ani_frame == 0)
			{
				continue;
			}

			if (++unit[p][u].ani_frame > 3)
			{
				unit[p][u].ani_frame = 0;
			}
		}
	}
}
void DE_RunTickDrawWalls( void )
{
	unsigned int x;

	for (x = 0; x < WALL_MAX; x++)
	{
		if (wallExist[x])
		{
			JE_drawShape2(wallsX[x], wallsY[x], 42, eShapes1);
		}
	}
}
void DE_RunTickExplosions( void )
{
	unsigned int i, j, k;
	unsigned int tempPixelIndex;
	int tempPosX, tempPosY;
	double tempRadius;


	/* Run through all open explosions.  They are not sorted in any way */
	for (i = 0; i < EXPLO_MAX; i++)
	{
		if (explosionAvail[i] == true) { continue; } /* Nothing to do */

		for (j = 0; j < exploRec[i].explofill; j++)
		{
			/* An explosion is comprised of multiple 'flares' that fan out.
			   Calculate where this 'flare' will end up */
			tempRadius = ((float)mt_rand() / MT_RAND_MAX) * (M_PI * 2);
			tempPosY = exploRec[i].y + round(cos(tempRadius) * ((float)mt_rand() / MT_RAND_MAX) * exploRec[i].explowidth);
			tempPosX = exploRec[i].x + round(sin(tempRadius) * ((float)mt_rand() / MT_RAND_MAX) * exploRec[i].explowidth);

			/* Okay, now draw the flare (as long as it isn't out of bounds) */
			if (tempPosY < 200 && tempPosY > 15)
			{
				tempPixelIndex = tempPosX + tempPosY * destructTempScreen->pitch;
				if (exploRec[i].explocolor == 252) /* 'explocolor' is misleading.  Basically all damaging shots are 252 */
				{
					JE_superPixel(tempPixelIndex); /* Draw the flare */
				}
				else
				{
					((Uint8 *)destructTempScreen->pixels)[tempPixelIndex] = exploRec[i].explocolor;
				}
			}

			if (exploRec[i].explocolor == 252) /* Destructive shots only.  Damage structures. */
			{
				for (k = 0; k < MAX_INSTALLATIONS; k++)
				{
					/* See if we collided with any units */
					if (unit[PLAYER_LEFT][k].health > 0 && tempPosX > leftX[k] && tempPosX < leftX[k] + 11 && tempPosY > leftY[k] - 11 && tempPosY < leftY[k])
					{
						unit[PLAYER_LEFT][k].health--;
						if (unit[PLAYER_LEFT][k].health == 0)
						{
							JE_makeExplosion(leftX[k] + 5, round(leftY[k]) - 5, (leftSystem[k] == 8) * 2);
							if (leftSystem[k] != 4)
							{
								leftAvail--;
								player[PLAYER_RIGHT].score++;
							}
						}
					}
					if (unit[PLAYER_RIGHT][k].health > 0 && tempPosX > rightX[k] && tempPosX < rightX[k] + 11 && tempPosY > rightY[k] - 11 && tempPosY < rightY[k])
					{
						unit[PLAYER_RIGHT][k].health--;
						if (unit[PLAYER_RIGHT][k].health == 0)
						{
							JE_makeExplosion(rightX[k] + 5, round(rightY[k]) - 5, (rightSystem[k] == 8) * 2);
							if (rightSystem[k] != 4)
							{
								rightAvail--;
								player[PLAYER_LEFT].score++;
							}
						}
					}
				}
			}
		}

		/* Widen the explosion and delete it if necessary. */
		exploRec[i].explowidth++;
		if (exploRec[i].explowidth == exploRec[i].explomax)
		{
			explosionAvail[i] = true;
		}
	}
}
void DE_RunTickShots( void )
{
	unsigned int i, j;
	int tempPosX, tempPosY;
	unsigned int tempTrails;


	for (i = 0; i < SHOT_MAX; i++) {
		if (destructShotAvail[i] == true) { continue; } /* Nothing to do */

		/* Move the shot.  Simple displacement */
		shotRec[i].x += shotRec[i].xmov;
		shotRec[i].y += shotRec[i].ymov;

		/* If the shot can bounce off the map, bounce it */
		if (shotBounce[shotRec[i].shottype-1])
		{
			if (shotRec[i].y > 199 || shotRec[i].y < 14)
			{
				shotRec[i].y -= shotRec[i].ymov;
				shotRec[i].ymov = -shotRec[i].ymov;
			}
			if (shotRec[i].x < 1 || shotRec[i].x > 318)
			{
				shotRec[i].x -= shotRec[i].xmov;
				shotRec[i].xmov = -shotRec[i].xmov;
			}
		} else { /* If it cannot, apply normal physics */
			shotRec[i].ymov += 0.05f; /* add gravity */

			if (shotRec[i].y > 199) /* We hit the floor */
			{
				shotRec[i].y -= shotRec[i].ymov;
				shotRec[i].ymov = -shotRec[i].ymov * 0.8f; /* bounce at reduced velocity */

				/* Don't allow a bouncing shot to bounce straight up and down */
				if (shotRec[i].xmov == 0)
				{
					shotRec[i].xmov += ((float)mt_rand() / MT_RAND_MAX) - 0.5f;
				}
			}
		}

		/* Shot has gone out of bounds. Eliminate it. */
		if (shotRec[i].x > 318 || shotRec[i].x < 1)
		{
			destructShotAvail[i] = true;
			continue;
		}

		/* Now check for collisions. */
		if (shotRec[i].y <= 14)
		{
			/* Don't bother checking for collisions above the map :) */
			shotRec[i].trail2c = 0;
			shotRec[i].trail1c = 0;
			continue;
		}

		tempPosX = round(shotRec[i].x);
		tempPosY = round(shotRec[i].y);

		/*Check player1's hits*/
		for (j = 0; j < MAX_INSTALLATIONS; j++)
		{
			if (unit[PLAYER_LEFT][j].health > 0)
			{
				if (tempPosX > leftX[j] && tempPosX < leftX[j] + 11 && tempPosY > leftY[j] - 13 && tempPosY < leftY[j])
				{
					destructShotAvail[i] = true;
					JE_makeExplosion(tempPosX, tempPosY, shotRec[i].shottype);
				}
			}
		}
		/*Now player2*/
		for (j = 0; j < MAX_INSTALLATIONS; j++)
		{
			if (unit[PLAYER_RIGHT][j].health > 0)
			{
				if (tempPosX > rightX[j] && tempPosX < rightX[j] + 11 && tempPosY > rightY[j] - 13 && tempPosY < rightY[j])
				{
					destructShotAvail[i] = true;
					JE_makeExplosion(tempPosX, tempPosY, shotRec[i].shottype);
				}
			}
		}

		tempTrails = (shotColor[shotRec[i].shottype-1] << 4) - 3;
		JE_pixCool(tempPosX, tempPosY, tempTrails);

		/*Draw the shot trail (if applicable) */
		switch (shotTrail[shotRec[i].shottype-1])
		{
		case TRAILS_NONE:
			break;

		case TRAILS_NORMAL:
			if (shotRec[i].trail2c > 0 && shotRec[i].trail2y > 12)
			{
				JE_pixCool(shotRec[i].trail2x, shotRec[i].trail2y, shotRec[i].trail2c);
			}
			shotRec[i].trail2x = shotRec[i].trail1x;
			shotRec[i].trail2y = shotRec[i].trail1y;
			if (shotRec[i].trail1c > 0)
			{
				shotRec[i].trail2c = shotRec[i].trail1c - 4;
			}
			if (shotRec[i].trail1c > 0 && shotRec[i].trail1y > 12)
			{
				JE_pixCool(shotRec[i].trail1x, shotRec[i].trail1y, shotRec[i].trail1c);
			}
			shotRec[i].trail1x = tempPosX;
			shotRec[i].trail1y = tempPosY;
			shotRec[i].trail1c = tempTrails - 3;
			break;

		case TRAILS_FULL:
			if (shotRec[i].trail4c > 0 && shotRec[i].trail4y > 12)
			{
				JE_pixCool(shotRec[i].trail4x, shotRec[i].trail4y, shotRec[i].trail4c);
			}
			shotRec[i].trail4x = shotRec[i].trail3x;
			shotRec[i].trail4y = shotRec[i].trail3y;
			if (shotRec[i].trail3c > 0)
			{
				shotRec[i].trail4c = shotRec[i].trail3c - 3;
			}
			if (shotRec[i].trail3c > 0 && shotRec[i].trail3y > 12)
			{
				JE_pixCool(shotRec[i].trail3x, shotRec[i].trail3y, shotRec[i].trail3c);
			}
			shotRec[i].trail3x = shotRec[i].trail2x;
			shotRec[i].trail3y = shotRec[i].trail2y;
			if (shotRec[i].trail2c > 0)
			{
				shotRec[i].trail3c = shotRec[i].trail2c - 3;
			}
			if (shotRec[i].trail2c > 0 && shotRec[i].trail2y > 12)
			{
				JE_pixCool(shotRec[i].trail2x, shotRec[i].trail2y, shotRec[i].trail2c);
			}
			shotRec[i].trail2x = shotRec[i].trail1x;
			shotRec[i].trail2y = shotRec[i].trail1y;
			if (shotRec[i].trail1c > 0)
			{
				shotRec[i].trail2c = shotRec[i].trail1c - 3;
			}
			if (shotRec[i].trail1c > 0 && shotRec[i].trail1y > 12)
			{
				JE_pixCool(shotRec[i].trail1x, shotRec[i].trail1y, shotRec[i].trail1c);
			}
			shotRec[i].trail1x = tempPosX;
			shotRec[i].trail1y = tempPosY;
			shotRec[i].trail1c = tempTrails - 1;
			break;
		}

		/* Bounce off of or destroy walls */
		for (j = 0; j < WALL_MAX; j++)
		{
			if (wallExist[j] && tempPosX >= wallsX[j] && tempPosX <= wallsX[j] + 11 && tempPosY >= wallsY[j] && tempPosY <= wallsY[j] + 14)
			{
				if (demolish[shotRec[i].shottype-1])
				{
					/* Blow up the wall and remove the shot. */
					wallExist[j] = false;
					destructShotAvail[i] = true;
					JE_makeExplosion(tempPosX, tempPosY, shotRec[i].shottype);
					continue;
				} else {
					/* Otherwise, bounce. */
					if (shotRec[i].x - shotRec[i].xmov < wallsX[j] || shotRec[i].x - shotRec[i].xmov > wallsX[j] + 11)
					{
						shotRec[i].xmov = -shotRec[i].xmov;
					}
					if (shotRec[i].y - shotRec[i].ymov < wallsY[j] || shotRec[i].y - shotRec[i].ymov > wallsY[j] + 14)
					{
						if (shotRec[i].ymov < 0)
						{
							shotRec[i].ymov = -shotRec[i].ymov;
						}
						else
						{
							shotRec[i].ymov = -shotRec[i].ymov * 0.8f;
						}
					}

					tempPosX = round(shotRec[i].x);
					tempPosY = round(shotRec[i].y);
				}
			}
		}

		/* Our last collision check, at least for now.  We hit dirt (I think) */
		if((((Uint8 *)destructTempScreen->pixels)[tempPosX + tempPosY * destructTempScreen->pitch]) == 25)
		{
			destructShotAvail[i] = true;
			JE_makeExplosion(tempPosX, tempPosY, shotRec[i].shottype);
			continue;
		}
	}
}
void DE_RunTickAI(enum de_player_t player_index)
{
	unsigned int i;


	if (player[player_index].aiMemory.c_noDown > 0)
	{
		player[player_index].aiMemory.c_noDown--;
	}

	/* Until all structs are properly divvied up this must only apply to player1 */
	if (mt_rand() % 100 > 80)
	{
		player[player_index].aiMemory.c_Angle += (mt_rand() % 3) - 1;
		if (player[player_index].aiMemory.c_Angle > 1)
		{
			player[player_index].aiMemory.c_Angle = 1;
		}
		if (player[player_index].aiMemory.c_Angle < -1)
		{
			player[player_index].aiMemory.c_Angle = -1;
		}
	}
	if (mt_rand() % 100 > 90)
	{
		if (player[player_index].aiMemory.c_Angle > 0 && leftAngle[leftSel-1] > (M_PI / 2.0f) - (M_PI / 9.0f))
		{
			player[player_index].aiMemory.c_Angle = 0;
		}
		if (player[player_index].aiMemory.c_Angle < 0 && leftAngle[leftSel-1] < M_PI / 8.0f)
		{
			player[player_index].aiMemory.c_Angle = 0;
		}
	}

	if (mt_rand() % 100 > 93)
	{
		player[player_index].aiMemory.c_Power += (mt_rand() % 3) - 1;
		if (player[player_index].aiMemory.c_Power > 1)
		{
			player[player_index].aiMemory.c_Power = 1;
		}
		if (player[player_index].aiMemory.c_Power < -1)
		{
			player[player_index].aiMemory.c_Power = -1;
		}
	}
	if (mt_rand() % 100 > 90)
	{
		if (player[player_index].aiMemory.c_Power > 0 && leftPower[leftSel-1] > 4)
		{
			player[player_index].aiMemory.c_Power = 0;
		}
		if (player[player_index].aiMemory.c_Power < 0 && leftPower[leftSel-1] < 3)
		{
			player[player_index].aiMemory.c_Power = 0;
		}
		if (leftPower[leftSel-1] < 2)
		{
			player[player_index].aiMemory.c_Power = 1;
		}
	}

	// prefer helicopter
	for (i = 0; i < MAX_INSTALLATIONS; i++)
	{
		if (leftSystem[i] == UNIT_HELI && unit[player_index][i].health > 0)
		{
			leftSel = i + 1;
			break;
		}
	}

	if (leftSystem[leftSel-1] == UNIT_HELI)
	{
		if (!leftYInAir[leftSel-1])
		{
			player[player_index].aiMemory.c_Power = 1;
		}
		if (mt_rand() % leftX[leftSel-1] > 100)
		{
			player[player_index].aiMemory.c_Power = 1;
		}
		if (mt_rand() % 240 > leftX[leftSel-1])
		{
			player[player_index].moves.actions[MOVE_RIGHT] = true;
		}
		else if ((mt_rand() % 20) + 300 < leftX[leftSel-1])
		{
			player[player_index].moves.actions[MOVE_LEFT] = true;
		}
		else if (mt_rand() % 30 == 1)
		{
			player[player_index].aiMemory.c_Angle = (mt_rand() % 3) - 1;
		}
		if (leftX[leftSel-1] > 295 && leftLastMove[leftSel-1] > 1)
		{
			player[player_index].moves.actions[MOVE_LEFT] = true;
			player[player_index].moves.actions[MOVE_RIGHT] = false;
		}
		if (leftSystem[leftSel-1] != UNIT_HELI || leftLastMove[leftSel-1] > 3 || (leftX[leftSel-1] > 160 && leftLastMove[leftSel-1] > -3))
		{
			if (mt_rand() % (int)round(leftY[leftSel-1]) < 150 && leftYMov[leftSel-1] < 0.01f && (leftX[leftSel-1] < 160 || leftLastMove[leftSel-1] < 2))
			{
				player[player_index].moves.actions[MOVE_FIRE] = true;
			}
			player[player_index].aiMemory.c_noDown = (5 - abs(leftLastMove[leftSel-1])) * (5 - abs(leftLastMove[leftSel-1])) + 3;
			player[player_index].aiMemory.c_Power = 1;
		} else
		{
			player[player_index].moves.actions[MOVE_FIRE] = false;
		}

		for (i = 0; i < MAX_INSTALLATIONS; i++)
			if (abs(rightX[i] - leftX[leftSel-1]) < 8)
			{
				if (rightSystem[i] == 4)
					player[player_index].moves.actions[MOVE_FIRE] = false;
				else {
					player[player_index].moves.actions[MOVE_LEFT] = false;
					player[player_index].moves.actions[MOVE_RIGHT] = false;
					if (leftLastMove[leftSel-1] < -1)
						leftLastMove[leftSel-1]++;
					else if (leftLastMove[leftSel-1] > 1)
						leftLastMove[leftSel-1]--;
				}
			}
	} else {
		player[player_index].moves.actions[MOVE_FIRE] = 1;
	}

	if (mt_rand() % 200 > 198)
	{
		player[player_index].moves.actions[MOVE_CHANGE] = true;
		player[player_index].aiMemory.c_Angle = 0;
		player[player_index].aiMemory.c_Power = 0;
		player[player_index].aiMemory.c_Fire = 0;
	}

	if (mt_rand() % 100 > 98 || leftShotType[leftSel-1] == 1)
	{
		player[player_index].moves.actions[MOVE_CYDN] = true;
	}
	if (player[player_index].aiMemory.c_Angle > 0)
	{
		player[player_index].moves.actions[MOVE_LEFT] = true;
	}
	if (player[player_index].aiMemory.c_Angle < 0)
	{
		player[player_index].moves.actions[MOVE_RIGHT] = true;
	}
	if (player[player_index].aiMemory.c_Power > 0)
	{
		player[player_index].moves.actions[MOVE_UP] = true;
	}
	if (player[player_index].aiMemory.c_Power < 0 && player[player_index].aiMemory.c_noDown == 0)
	{
		player[player_index].moves.actions[MOVE_DOWN] = true;
	}
	if (player[player_index].aiMemory.c_Fire > 0)
	{
		player[player_index].moves.actions[MOVE_FIRE] = true;
	}

	if (leftYMov[leftSel-1] < -0.1f && leftSystem[leftSel-1] == 8)
	{
		player[player_index].moves.actions[MOVE_FIRE] = false;
	}

	/* This last hack was down in the processing section.
	 * What exactly it was doing there I do not know */
	if(leftSystem[leftSel-1] == UNIT_LASER || leftYInAir[leftSel-1]) {
		player[player_index].aiMemory.c_Power = 0;
	}
}
void DE_RunTickDrawCrosshairs( void )
{

	int tempPosX, tempPosY;

	/* Draw the crosshairs.  Most vehicles aim left or right.  Helis can aim
	 * either way and this must be accounted for.
	 * I'd really like to just add a 'direction' stat so as not to
	 * have two different logics here
	 */
	if (leftSystem[leftSel-1] == UNIT_HELI)
	{
		tempPosX = leftX[leftSel-1] + round(0.1f * leftLastMove[leftSel-1] * leftLastMove[leftSel-1] * leftLastMove[leftSel-1]) + 5;
		tempPosY = round(leftY[leftSel-1]) + 1;
	} else {
		tempPosX = round(leftX[leftSel-1] + 6 + cos(leftAngle[leftSel-1]) * (leftPower[leftSel-1] * 8 + 7));
		tempPosY = round(leftY[leftSel-1] - 7 - sin(leftAngle[leftSel-1]) * (leftPower[leftSel-1] * 8 + 7));
	}
	JE_pix(tempPosX,     tempPosY,     14);
	JE_pix(tempPosX + 3, tempPosY,      3);
	JE_pix(tempPosX - 3, tempPosY,      3);
	JE_pix(tempPosX,     tempPosY + 2,  3);
	JE_pix(tempPosX,     tempPosY - 2,  3);

	if (rightSystem[rightSel-1] == UNIT_HELI)
	{  /*Heli*/
		tempPosX = rightX[rightSel-1] + round(0.1f * rightLastMove[rightSel-1] * rightLastMove[rightSel-1] * rightLastMove[rightSel-1]) + 5;
		tempPosY = round(rightY[rightSel-1]) + 1;
	} else {
		tempPosX = round(rightX[rightSel-1] + 6 - cos(rightAngle[rightSel-1]) * (rightPower[rightSel-1] * 8 + 7));
		tempPosY = round(rightY[rightSel-1] - 7 - sin(rightAngle[rightSel-1]) * (rightPower[rightSel-1] * 8 + 7));
	}
	JE_pix(tempPosX,     tempPosY,     14);
	JE_pix(tempPosX + 3, tempPosY,      3);
	JE_pix(tempPosX - 3, tempPosY,      3);
	JE_pix(tempPosX,     tempPosY + 2,  3);
	JE_pix(tempPosX,     tempPosY - 2,  3);
}
void DE_RunTickDrawHUD( void )
{
	char tempstr[256];


	JE_bar( 5, 3, 14, 8, 241);
	JE_rectangle( 4, 2, 15, 9, 242);
	JE_rectangle( 3, 1, 16, 10, 240);
	JE_bar(18, 3, 140, 8, 241);
	JE_rectangle(17, 2, 143, 9, 242);
	JE_rectangle(16, 1, 144, 10, 240);
	JE_drawShape2(  4, 0, 190 + leftShotType[leftSel-1], eShapes1);
	JE_outText( 20, 3, weaponNames[leftShotType[leftSel-1]-1], 15, 2);
	sprintf(tempstr, "dmg~%d~", unit[PLAYER_LEFT][leftSel-1].health);
	JE_outText( 75, 3, tempstr, 15, 0);
	sprintf(tempstr, "pts~%d~", player[PLAYER_LEFT].score);
	JE_outText(110, 3, tempstr, 15, 0);

	JE_bar(175, 3, 184, 8, 241);
	JE_rectangle(174, 2, 185, 9, 242);
	JE_rectangle(173, 1, 186, 10, 240);
	JE_bar(188, 3, 310, 8, 241);
	JE_rectangle(187, 2, 312, 9, 242);
	JE_rectangle(186, 1, 313, 10, 240);
	JE_drawShape2(174, 0, 190 + rightShotType[rightSel-1], eShapes1);
	JE_outText(190, 3, weaponNames[rightShotType[rightSel-1]-1], 15, 2);
	sprintf(tempstr, "dmg~%d~", unit[PLAYER_RIGHT][rightSel-1].health);
	JE_outText(245, 3, tempstr, 15, 0);
	sprintf(tempstr, "pts~%d~", player[PLAYER_RIGHT].score);
	JE_outText(280, 3, tempstr, 15, 0);
}
void DE_RunTickGetInput( void )
{
	unsigned int player_index, key_index, slot_index;
	SDLKey key;

	/* player.keys holds our key config.  Players will eventually be allowed
	 * to can change their key mappings.  player.moves and player.keys
	 * line up; rather than manually checking left and right we can
	 * just loop through the indexes and set the actions as needed. */
	service_SDL_events(true);

	for(player_index = 0; player_index < PLAYER_MAX; player_index++)
	{
		for(key_index = 0; key_index < KEY_MAX; key_index++)
		{
			for(slot_index = 0; slot_index < KEY_OPTION_MAX; slot_index++)
			{
				key = player[player_index].keys.Config[key_index][slot_index];
				if(key != SDLK_UNKNOWN && keysactive[key] == true)
				{
					/* The right key was clearly pressed */
					player[player_index].moves.actions[key_index] = true;

					/* Some keys we want to toggle afterwards */
					if(key_index == KEY_CHANGE ||
					   key_index == KEY_CYUP   ||
					   key_index == KEY_CYDN)
					{
						keysactive[key] = false;
					}
				}
			}
		}
	}
}
void DE_RunTickProcessInput( void )
{
	unsigned int emptyShot, i;
	unsigned int player_index;


	if (leftAvail > 0)
	{
		player_index = 0;
		if (systemAngle[leftSystem[leftSel-1]-1])
		{
			/*leftAnglechange*/
			if (player[player_index].moves.actions[MOVE_LEFT] == true)
			{
				leftAngle[leftSel-1] += 0.01f;
				if (leftAngle[leftSel-1] > M_PI / 2)
				{
					leftAngle[leftSel-1] = M_PI / 2 - 0.01f;
				}
			}
			/*rightAnglechange*/
			if (player[player_index].moves.actions[MOVE_RIGHT] == true)
			{
				leftAngle[leftSel-1] -= 0.01f;
				if (leftAngle[leftSel-1] < 0)
				{
					leftAngle[leftSel-1] = 0;
				}
			}
		} else if (leftSystem[leftSel-1] == UNIT_HELI) {
			if (player[player_index].moves.actions[MOVE_LEFT] == true && leftX[leftSel-1] > 5)
				if (JE_stabilityCheck(leftX[leftSel-1] - 5, round(leftY[leftSel-1])))
				{
					if (leftLastMove[leftSel-1] > -5)
					{
						leftLastMove[leftSel-1]--;
					}
					leftX[leftSel-1]--;
					if (JE_stabilityCheck(leftX[leftSel-1], round(leftY[leftSel-1])))
					{
						leftYInAir[leftSel-1] = true;
					}
				}
			if (player[player_index].moves.actions[MOVE_RIGHT] == true && leftX[leftSel-1] < 305)
				if (JE_stabilityCheck(leftX[leftSel-1] + 5, round(leftY[leftSel-1])))
				{
					if (leftLastMove[leftSel-1] < 5)
					{
						leftLastMove[leftSel-1]++;
					}
					leftX[leftSel-1]++;
					if (JE_stabilityCheck(leftX[leftSel-1], round(leftY[leftSel-1])))
					{
						leftYInAir[leftSel-1] = true;
					}
				}
		}

		/*Leftincreasepower*/
		if (player[player_index].moves.actions[MOVE_UP] == true)
		{
			if (leftSystem[leftSel-1] == UNIT_HELI)
			{
				leftYInAir[leftSel-1] = true;
				leftYMov[leftSel-1] -= 0.1f;
			} else if (leftSystem[leftSel-1] != 7 || leftYInAir[leftSel-1]) {
				leftPower[leftSel-1] += 0.05f;
				if (leftPower[leftSel-1] > 5)
				{
					leftPower[leftSel-1] = 5;
				}
			} else {
				leftYMov[leftSel-1] = -3;
				leftYInAir[leftSel-1] = true;
			}
		}
		/*Leftdecreasepower*/
		if (player[player_index].moves.actions[MOVE_DOWN] == true)
		{
			if (leftSystem[leftSel-1] == UNIT_HELI && leftYInAir[leftSel-1])
			{
				leftYMov [leftSel-1] += 0.1f;
			} else {
				leftPower[leftSel-1] -= 0.05f;
				if (leftPower[leftSel-1] < 1)
				{
					leftPower[leftSel-1] = 1;
				}
			}
		}
		/*Leftupweapon*/
		if (player[player_index].moves.actions[MOVE_CYUP] == true)
		{
			leftShotType[leftSel-1]++;
			if (leftShotType[leftSel-1] > SHOT_TYPES)
			{
				leftShotType[leftSel-1] = 1;
			}

			while (weaponSystems[leftSystem[leftSel-1]-1][leftShotType[leftSel-1]-1] == 0)
			{
				leftShotType[leftSel-1]++;
				if (leftShotType[leftSel-1] > SHOT_TYPES)
				{
					leftShotType[leftSel-1] = 1;
				}
			}
		}
		/*Leftdownweapon*/
		if (player[player_index].moves.actions[MOVE_CYDN] == true)
		{
			leftShotType[leftSel-1]--;
			if (leftShotType[leftSel-1] < 1)
			{
				leftShotType[leftSel-1] = SHOT_TYPES;
			}

			while (weaponSystems[leftSystem[leftSel-1]-1][leftShotType[leftSel-1]-1] == 0)
			{
				leftShotType[leftSel-1]--;
				if (leftShotType[leftSel-1] < 1)
				{
					leftShotType[leftSel-1] = SHOT_TYPES;
				}
			}
		}

		/*Leftchange*/
		if (player[player_index].moves.actions[MOVE_CHANGE] == true)
		{
			leftSel++;
			if (leftSel > MAX_INSTALLATIONS)
			{
				leftSel = 1;
			}
		}

		/*Newshot*/
		if (leftshotDelay > 0)
		{
			leftshotDelay--;
		}
		if (player[player_index].moves.actions[MOVE_FIRE] == true && (leftshotDelay == 0))
		{
			leftshotDelay = shotDelay[leftShotType[leftSel-1]-1];

			if (shotDirt[leftShotType[leftSel-1]-1] > 20)
			{
				emptyShot = 0;
				for (i = 0; i < SHOT_MAX; i++)
				{
					if (destructShotAvail[i])
					{
						emptyShot = i + 1;
						break;
					}
				}

				if (emptyShot > 0 && (leftSystem[leftSel-1] != UNIT_HELI || leftYInAir[leftSel-1]))
				{
					soundQueue[0] = shotSound[leftShotType[leftSel-1]-1];

					if (leftSystem[leftSel-1] == UNIT_HELI)
					{
						shotRec[emptyShot-1].x = leftX[leftSel-1] + leftLastMove[leftSel-1] * 2 + 5;
						shotRec[emptyShot-1].y = leftY[leftSel-1] + 1;
						shotRec[emptyShot-1].ymov = 0.5f + leftYMov[leftSel-1] * 0.1f;
						shotRec[emptyShot-1].xmov = 0.02f * leftLastMove[leftSel-1] * leftLastMove[leftSel-1] * leftLastMove[leftSel-1];
						if (player[player_index].moves.actions[MOVE_UP] && leftY[leftSel-1] < 30) /* For some odd reason if we're too high we ignore most of our computing. */
						{
							shotRec[emptyShot-1].ymov = 0.1f;
							if (shotRec[emptyShot-1].xmov < 0)
							{
								shotRec[emptyShot-1].xmov += 0.1f;
							}
							else if (shotRec[emptyShot-1].xmov > 0)
							{
								shotRec[emptyShot-1].xmov -= 0.1f;
							}
							shotRec[emptyShot-1].y = leftY[leftSel-1];
						}
					} else {
						shotRec[emptyShot-1].x = leftX[leftSel-1] + 6 + cos(leftAngle[leftSel-1]) * 10;
						shotRec[emptyShot-1].y = leftY[leftSel-1] - 7 - sin(leftAngle[leftSel-1]) * 10;
						shotRec[emptyShot-1].ymov = -sin(leftAngle[leftSel-1]) * leftPower[leftSel-1];
						shotRec[emptyShot-1].xmov =  cos(leftAngle[leftSel-1]) * leftPower[leftSel-1];
					}

					shotRec[emptyShot-1].shottype = leftShotType[leftSel-1];

					destructShotAvail[emptyShot-1] = false;

					shotRec[emptyShot-1].shotdur = shotFuse[shotRec[emptyShot-1].shottype-1];

					shotRec[emptyShot-1].trail1c = 0;
					shotRec[emptyShot-1].trail2c = 0;
				}
			} else {
				switch (shotDirt[leftShotType[leftSel-1]-1])
				{
				case 1: /* magnet */
					for (i = 0; i < SHOT_MAX; i++)
					{
						if (destructShotAvail[i] == false)
						{
							if (shotRec[i].x > leftX[leftSel-1])
							{
								shotRec[i].xmov += leftPower[leftSel-1] * 0.1f;
							}
						}
					}
					for (i = 0; i < MAX_INSTALLATIONS; i++) /* magnets push coptors */
					{
						if (rightSystem[i] == UNIT_HELI && rightYInAir[i] && (rightX[i] + 11)< 318) /* changed to properly align border */
						{
							rightX[i] += 2;
						}
					}
					unit[PLAYER_LEFT][leftSel-1].ani_frame = 1;
					break;
				}
			}
		}
	}

	if (rightAvail > 0)
	{
		player_index = 1;
		if (systemAngle[rightSystem[rightSel-1]-1])
		{
			/*rightAnglechange*/
			if (player[player_index].moves.actions[MOVE_RIGHT]== true)
			{
				rightAngle[rightSel-1] += 0.01f;
				if (rightAngle[rightSel-1] > M_PI / 2)
				{
					rightAngle[rightSel-1] = M_PI / 2 - 0.01f;
				}
			}
			/*rightAnglechange*/
			if (player[player_index].moves.actions[MOVE_LEFT] == true)
			{
				rightAngle[rightSel-1] -= 0.01f;
				if (rightAngle[rightSel-1] < 0)
				{
					rightAngle[rightSel-1] = 0;
				}
			}
		} else if (rightSystem[rightSel-1] == 8) { /*Helicopter*/
			if (player[player_index].moves.actions[MOVE_LEFT] == true && rightX[rightSel-1] > 5)
			{
				if (JE_stabilityCheck(rightX[rightSel-1] - 5, round(rightY[rightSel-1])))
				{
					if (rightLastMove[rightSel-1] > -5)
					{
						rightLastMove[rightSel-1]--;
					}
					rightX[rightSel-1]--;
					if (JE_stabilityCheck(rightX[rightSel-1], round(rightY[rightSel-1])))
					{
						rightYInAir[rightSel-1] = true;
					}
				}
			}
			if (player[player_index].moves.actions[MOVE_RIGHT] == true && rightX[rightSel-1] < 305)
			{
				if (JE_stabilityCheck(rightX[rightSel-1] + 5, round(rightY[rightSel-1])))
				{
					if (rightLastMove[rightSel-1] < 5)
					{
						rightLastMove[rightSel-1]++;
					}
					rightX[rightSel-1]++;
					if (JE_stabilityCheck(rightX[rightSel-1], round(rightY[rightSel-1])))
					{
						rightYInAir[rightSel-1] = true;
					}
				}
			}
		}

		/*Rightincreasepower*/
		if (player[player_index].moves.actions[MOVE_UP] == true)
		{
			if (rightSystem[rightSel-1] == UNIT_HELI)
			{ /*HELI*/
				rightYInAir[rightSel-1] = true;
				rightYMov[rightSel-1] -= 0.1f;
			} else if (rightSystem[rightSel-1] != 7 || rightYInAir[rightSel-1]) {
				rightPower[rightSel-1] += 0.05f;
				if (rightPower[rightSel-1] > 5)
				{
					rightPower[rightSel-1] = 5;
				}
			} else {
				rightYMov[rightSel-1] = -3;
				rightYInAir[rightSel-1] = true;
			}
		}
		/*Rightdecreasepower*/
		if (player[player_index].moves.actions[MOVE_DOWN] == true)
		{
			if (rightSystem[rightSel-1] == UNIT_HELI && rightYInAir[rightSel-1])
			{ /*HELI*/
				rightYMov[rightSel-1] += 0.1f;
			} else {
				rightPower[rightSel-1] -= 0.05f;
				if (rightPower[rightSel-1] < 1)
				{
					rightPower[rightSel-1] = 1;
				}
			}
		}
		/*Rightupweapon*/
		if (player[player_index].moves.actions[MOVE_CYUP]== true)
		{
			rightShotType[rightSel-1]++;
			if (rightShotType[rightSel-1] > SHOT_TYPES)
			{
				rightShotType[rightSel-1] = 1;
			}
			while (weaponSystems[rightSystem[rightSel-1]-1][rightShotType[rightSel-1]-1] == 0)
			{
				rightShotType[rightSel-1]++;
				if (rightShotType[rightSel-1] > SHOT_TYPES)
				{
					rightShotType[rightSel-1] = 1;
				}
			}
		}
		/*Rightdownweapon*/
		if (player[player_index].moves.actions[MOVE_CYDN] == true)
		{
			rightShotType[rightSel-1]--;
			if (rightShotType[rightSel-1] < 1)
			{
				rightShotType[rightSel-1] = SHOT_TYPES;
			}

			while (weaponSystems[rightSystem[rightSel-1]-1][rightShotType[rightSel-1]-1] == 0)
			{
				rightShotType[rightSel-1]--;
				if (rightShotType[rightSel-1] < 1)
				{
					rightShotType[rightSel-1] = SHOT_TYPES;
				}
			}
		}

		/*Rightchange*/
		if (player[player_index].moves.actions[MOVE_CHANGE] == true)
		{
			rightSel++;
			if (rightSel > MAX_INSTALLATIONS)
			{
				rightSel = 1;
			}
		}

		/*Newshot*/
		if (rightshotDelay > 0)
		{
			rightshotDelay--;
		}
		if (player[player_index].moves.actions[MOVE_FIRE] == true && rightshotDelay == 0)
		{
			rightshotDelay = shotDelay[rightShotType[rightSel-1]-1];

			emptyShot = 0;
			for (i = 0; i < SHOT_MAX; i++)
			{
				if (destructShotAvail[i])
				{
					emptyShot = i + 1;
				}
			}

			if (shotDirt[rightShotType[rightSel-1]-1] > 20)
			{
				if (emptyShot > 0 && (rightSystem[rightSel-1] != 8 || rightYInAir[rightSel-1]))
				{
					soundQueue[1] = shotSound[rightShotType[rightSel-1]-1];

					if (rightSystem[rightSel-1] == 8)
					{
						shotRec[emptyShot-1].x = rightX[rightSel-1] + rightLastMove[rightSel-1] * 2 + 5;
						shotRec[emptyShot-1].y = rightY[rightSel-1] + 1;
						shotRec[emptyShot-1].ymov = 0.5f;
						shotRec[emptyShot-1].xmov = 0.02f * rightLastMove[rightSel-1] * rightLastMove[rightSel-1] * rightLastMove[rightSel-1];
						if ((keysactive[SDLK_KP8] || keysactive[SDLK_UP]) && rightY[rightSel-1] < 30)
						{
							shotRec[emptyShot-1].ymov = 0.1f;
							if (shotRec[emptyShot-1].xmov < 0)
							{
								shotRec[emptyShot-1].xmov += 0.1f;
							}
							else if (shotRec[emptyShot-1].xmov > 0) {
								shotRec[emptyShot-1].xmov -= 0.1f;
							}
							shotRec[emptyShot-1].y = rightY[rightSel-1];
						}
					} else {
						shotRec[emptyShot-1].x = rightX [rightSel-1] + 6 - cos(rightAngle[rightSel-1]) * 10;
						shotRec[emptyShot-1].y = rightY [rightSel-1] - 7 - sin(rightAngle[rightSel-1]) * 10;
						shotRec[emptyShot-1].ymov = -sin(rightAngle[rightSel-1]) * rightPower[rightSel-1];
						shotRec[emptyShot-1].xmov = -cos(rightAngle[rightSel-1]) * rightPower[rightSel-1];
					}

					if (rightSystem[rightSel-1] == 7)
					{
						shotRec[emptyShot-1].x = rightX[rightSel-1] + 2;
						if (rightYInAir[rightSel-1])
						{
							shotRec[emptyShot-1].ymov = 1;
							shotRec[emptyShot-1].y = rightY[rightSel-1] + 2;
						} else {
							shotRec[emptyShot-1].ymov = -2;
							shotRec[emptyShot-1].y = rightY[rightSel-1] - 12;
						}
					}

					shotRec[emptyShot-1].shottype = rightShotType[rightSel-1];

					destructShotAvail[emptyShot-1] = false;

					shotRec[emptyShot-1].shotdur = shotFuse[shotRec[emptyShot-1].shottype-1];

					shotRec[emptyShot-1].trail1c = 0;
					shotRec[emptyShot-1].trail2c = 0;
				}
			} else {
				switch (shotDirt[rightShotType[rightSel-1]-1])
				{
				case 1:
					for (i = 0; i < SHOT_MAX; i++)
					{
						if (!destructShotAvail[i])
						{
							if (shotRec[i].x < rightX[rightSel-1])
							{
								shotRec[i].xmov -= rightPower[rightSel-1] * 0.1f;
							}
						}
					}
					for (i = 0; i < MAX_INSTALLATIONS; i++)
					{
						if (leftSystem[i] == 8 && leftYInAir[i] && leftX[i] > 1)
						{
							leftX[i] -= 2;
						}
					}

					unit[PLAYER_RIGHT][rightSel-1].ani_frame = 1;
					break;
				}
			}
		}
	}
}
bool DE_RunTickCheckEndgame( void )
{
	if (leftAvail == 0)
	{
		player[PLAYER_RIGHT].score += lModeScore[destructMode-1];
		died = true;
		soundQueue[7] = V_CLEARED_PLATFORM;
		endDelay = 80;
	}
	if (rightAvail == 0)
	{
		player[PLAYER_LEFT].score += rModeScore[destructMode-1];
		died = true;
		soundQueue[7] = V_CLEARED_PLATFORM;
		endDelay = 80;
	}
	return(leftAvail == 0 || rightAvail == 0);
}
void DE_RunTickPlaySounds( void )
{
	unsigned int i, tempSampleIndex, tempVolume;


	for (i = 0; i < COUNTOF(soundQueue); i++)
	{
		if (soundQueue[i] != S_NONE)
		{
			tempSampleIndex = soundQueue[i];
			if (i == 7)
			{
				tempVolume = fxPlayVol;
			}
			else
			{
				tempVolume = fxPlayVol / 2;
			}

			JE_multiSamplePlay(digiFx[tempSampleIndex-1], fxSize[tempSampleIndex-1], i, tempVolume);
			soundQueue[i] = S_NONE;
		}
	}

}


// kate: tab-width 4; vim: set noet:
