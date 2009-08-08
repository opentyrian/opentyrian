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

/*** Defines ***/
#define MAX_WALL 20
#define MAX_EXPLO 40
#define MAX_KEY_OPTIONS 4
#define MAX_INSTALLATIONS 20

/*** Macros ***/
/* I believe in having a strict hierarchy rather than hordes of globabls.
 * This has the benefit of it being clear who controls what, but the downside
 * of sometimes requiring very complicated nested statements comprised of
 * structs within structs, etc.  All for what, in assembly, is usually
 * no more than a simple move.
 *
 * 90% of these time we're referencing the selected unit.  Because of that,
 * these macros should help simplify things greatly while still being readable.
 */

/* Don't use ++ and -- operators with macros.  They may look innocent
 * but given half the chance they will crash you and everyone
 * you care about.  It's a good practice to NOT embed them. */
#define CURUNIT(x)           (player[(x)].unitSelected-1)
#define CURUNIT_unitX(x)     (player[(x)].unit[CURUNIT(x)].unitX)
#define CURUNIT_unitY(x)     (player[(x)].unit[CURUNIT(x)].unitY)
#define CURUNIT_unitYMov(x)  (player[(x)].unit[CURUNIT(x)].unitYMov)
#define CURUNIT_isYInAir(x)  (player[(x)].unit[CURUNIT(x)].isYInAir)
#define CURUNIT_unitType(x)  (player[(x)].unit[CURUNIT(x)].unitType)
#define CURUNIT_shotType(x)  (player[(x)].unit[CURUNIT(x)].shotType)
#define CURUNIT_angle(x)     (player[(x)].unit[CURUNIT(x)].angle)
#define CURUNIT_power(x)     (player[(x)].unit[CURUNIT(x)].power)
#define CURUNIT_lastMove(x)  (player[(x)].unit[CURUNIT(x)].lastMove)
#define CURUNIT_ani_frame(x) (player[(x)].unit[CURUNIT(x)].ani_frame)
#define CURUNIT_health(x)    (player[(x)].unit[CURUNIT(x)].health)



/*** Enums ***/
/* There are an awful lot of index-1s scattered throughout the code.
 * I plan to eventually change them all and to be able to properly
 * enumerate from 0 up.
 */
enum de_player_t { PLAYER_LEFT = 0, PLAYER_RIGHT = 1, MAX_PLAYERS = 2 };
enum de_mode_t { MODE_5CARDWAR = 1, MODE_TRADITIONAL = 2, MODE_HELIASSAULT = 3,
                 MODE_HELIDEFENSE = 4, MODE_OUTGUNNED = 5, MAX_MODES = 5 };
enum de_unit_t { UNIT_TANK = 1, UNIT_NUKE = 2, UNIT_DIRT = 3, UNIT_SATELLITE = 4,
                 UNIT_MAGNET = 5, UNIT_LASER = 6, UNIT_JUMPER = 7, UNIT_HELI = 8, MAX_UNITS = 8 };
enum de_shot_t { SHOT_TRACER = 1, SHOT_SMALL = 2, SHOT_LARGE = 3,
                 SHOT_MICRO = 4, SHOT_SUPER = 5, SHOT_DEMO = 6,
                 SHOT_SMALLNUKE = 7, SHOT_LARGENUKE = 8,
                 SHOT_SMALLDIRT = 9, SHOT_LARGEDIRT = 10,
                 SHOT_MAGNET = 11, SHOT_MINILASER = 12,
                 SHOT_MEGALASER = 13, SHOT_LASERTRACER = 14,
                 SHOT_MEGABLAST = 15, SHOT_MINI = 16, SHOT_BOMB = 17,
                 SHOT_FIRST = 1, SHOT_LAST = 17, MAX_SHOTS = 17, SHOT_INVALID = 0 };
enum de_trails_t { TRAILS_NONE = 0, TRAILS_NORMAL = 1, TRAILS_FULL = 2 };
/* keys and moves should line up. */
enum de_keys_t {  KEY_LEFT = 0,  KEY_RIGHT = 1,  KEY_UP = 2,  KEY_DOWN = 3,  KEY_CHANGE = 4,  KEY_FIRE = 5,  KEY_CYUP = 6,  KEY_CYDN = 7,  MAX_KEY = 8};
enum de_move_t { MOVE_LEFT = 0, MOVE_RIGHT = 1, MOVE_UP = 2, MOVE_DOWN = 3, MOVE_CHANGE = 4, MOVE_FIRE = 5, MOVE_CYUP = 6, MOVE_CYDN = 7, MAX_MOVE = 8};

/* The tracerlaser is dummied out.  It works but (probably due to the low
 * MAX_SHOTS) is not assigned to anything.  The bomb does not work.
 */


/*** Structs ***/
struct destruct_unit_s {

	/* Positioning/movement */
	unsigned int unitX; /* yep, one's an int and the other is a real */
	double       unitY;
	double       unitYMov;
	bool         isYInAir;

	/* What it is and what it fires */
	unsigned int unitType;
	unsigned int shotType;

	/* What it's pointed */
	double angle;
	double power;

	/* Misc */
	int lastMove;
	unsigned int ani_frame;
	unsigned int health;
};

JE_shortint leftLastMove[MAX_INSTALLATIONS], rightLastMove[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/
JE_word leftX[MAX_INSTALLATIONS], rightX[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/
JE_real leftAngle[MAX_INSTALLATIONS], leftPower[MAX_INSTALLATIONS], leftYMov[MAX_INSTALLATIONS], leftY[MAX_INSTALLATIONS],
        rightAngle[MAX_INSTALLATIONS], rightPower[MAX_INSTALLATIONS], rightYMov[MAX_INSTALLATIONS], rightY[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/


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
	bool actions[MAX_MOVE];
};
struct destruct_keys_s {
	SDLKey Config[MAX_KEY][MAX_KEY_OPTIONS];
};
struct destruct_ai_s {

	int c_Angle, c_Power, c_Fire;
	unsigned int c_noDown;
};
struct destruct_player_s {

	bool is_cpu;
	struct destruct_ai_s aiMemory;

	struct destruct_unit_s unit[MAX_INSTALLATIONS];
	struct destruct_moves_s moves;
	struct destruct_keys_s  keys;

	unsigned int unitsRemaining;
	unsigned int unitSelected;
	unsigned int shotDelay;
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
void DE_RunTickCycleDeadUnits( void );
void DE_RunTickGravity( void );
void DE_RunTickAnimate( void );
void DE_RunTickDrawWalls( void );
void DE_RunTickExplosions( void );
void DE_RunTickShots( void );
void DE_RunTickAI( void );
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

const JE_boolean demolish[MAX_SHOTS] /*[1..MAX_SHOTS]*/ = {false, false, false, false, false, true, true, true, false, false, false, false, true, false, true, false, true};
//const JE_byte shotGr[MAX_SHOTS] /*[1..MAX_SHOTS]*/ = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 101};
const JE_byte shotTrail[MAX_SHOTS] /*[1..MAX_SHOTS]*/ = {TRAILS_NONE, TRAILS_NONE, TRAILS_NONE, TRAILS_NORMAL, TRAILS_NORMAL, TRAILS_NORMAL, TRAILS_FULL, TRAILS_FULL, TRAILS_NONE, TRAILS_NONE, TRAILS_NONE, TRAILS_NORMAL, TRAILS_FULL, TRAILS_NORMAL, TRAILS_FULL, TRAILS_NORMAL, TRAILS_NONE};
const JE_byte shotFuse[MAX_SHOTS] /*[1..MAX_SHOTS]*/ = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};
const JE_byte shotDelay[MAX_SHOTS] /*[1..MAX_SHOTS]*/ = {10, 30, 80, 20, 60, 100, 140, 200, 20, 60, 5, 15, 50, 5, 80, 16, 0};
const JE_byte shotSound[MAX_SHOTS] /*[1..MAX_SHOTS]*/ = {8, 2, 1, 7, 7, 9, 22, 22, 5, 13, 10, 15, 15, 26, 14, 7, 7};
const JE_byte exploSize[MAX_SHOTS] /*[1..MAX_SHOTS]*/ = {4, 20, 30, 14, 22, 16, 40, 60, 10, 30, 0, 5, 10, 3, 15, 7, 0};
const JE_boolean shotBounce[MAX_SHOTS] /*[1..MAX_SHOTS]*/ = {false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, false, true};
const JE_byte exploDensity[MAX_SHOTS] /*[1..MAX_SHOTS]*/ = {  2,  5, 10, 15, 20, 15, 25, 30, 40, 80, 0, 30, 30,  4, 30, 5, 0};
const JE_byte shotDirt[MAX_SHOTS] /*[1..MAX_SHOTS]*/ = {252, 252, 252, 252, 252, 252, 252, 252, 25, 25, 1, 252, 252, 252, 252, 252, 0};
const JE_byte shotColor[MAX_SHOTS] /*[1..MAX_SHOTS]*/ = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 10, 10, 10, 10, 16, 0};

const JE_byte defaultWeapon[MAX_UNITS]     /*[1..MAX_UNITS]*/ = {SHOT_SMALL, SHOT_MICRO,     SHOT_SMALLDIRT, SHOT_INVALID, SHOT_MAGNET, SHOT_MINILASER, SHOT_MICRO, SHOT_MINI};
const JE_byte defaultCpuWeapon[MAX_UNITS]  /*[1..MAX_UNITS]*/ = {SHOT_SMALL, SHOT_MICRO,     SHOT_DEMO,      SHOT_INVALID, SHOT_MAGNET, SHOT_MINILASER, SHOT_MICRO, SHOT_MINI};
const JE_byte defaultCpuWeaponB[MAX_UNITS] /*[1..MAX_UNITS]*/ = {SHOT_DEMO,  SHOT_SMALLNUKE, SHOT_DEMO,      SHOT_INVALID, SHOT_MAGNET, SHOT_MEGALASER, SHOT_MICRO, SHOT_MINI};
const JE_boolean systemAngle[MAX_UNITS] /*[1..MAX_UNITS]*/ = {true, true, true, false, false, true, false, false};
const JE_word baseDamage[MAX_UNITS] /*[1..MAX_UNITS]*/ = {200, 120, 400, 300, 80, 150, 600, 40};
const JE_boolean systemAni[MAX_UNITS] /*[1..MAX_UNITS]*/ = {false, false, false, true, false, false, false, true};

const JE_byte weaponSystems[MAX_UNITS][MAX_SHOTS] /*[1..MAX_UNITS, 1..MAX_SHOTS]*/ =
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

const JE_byte leftGraphicBase[MAX_UNITS] /*[1..MAX_UNITS]*/ = {1, 6, 11, 58, 63, 68, 96, 153};
const JE_byte rightGraphicBase[MAX_UNITS] /*[1..MAX_UNITS]*/ = {20, 25, 30, 77, 82, 87, 115, 172};

const JE_byte lModeScore[DESTRUCT_MODES] /*[1..destructmodes]*/ = {1, 0, 0, 5, 0};
const JE_byte rModeScore[DESTRUCT_MODES] /*[1..destructmodes]*/ = {1, 0, 5, 0, 1};

const SDLKey defaultKeyConfig[MAX_PLAYERS][MAX_KEY][MAX_KEY_OPTIONS] =
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

struct destruct_player_s player[MAX_PLAYERS];

JE_boolean wallExist[20]; /*[1..20]*/
JE_byte wallsX[20], wallsY[20]; /*[1..20]*/

JE_byte dirtHeight[320]; /*[0..319]*/

JE_boolean destructShotAvail[MAX_SHOTS]; /*[1..shotmax]*/

struct destruct_shot_s shotRec[MAX_SHOTS]; /*[1..shotmax]*/

JE_boolean endOfGame, destructQuit;


JE_boolean destructFound;
JE_real destructTempR;

JE_boolean explosionAvail[MAX_EXPLO]; /*[1..explomax]*/

struct destruct_explo_s exploRec[MAX_EXPLO]; /*[1..explomax]*/


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
	player[PLAYER_LEFT].unitsRemaining = 0;

	for (x = 0; x < basetypes[Lbaselookup[destructMode-1]-1][1-1]; x++)
	{
		leftX[x] = (mt_rand() % 120) + 10;
		leftY[x] = JE_placementPosition(leftX[x] - 1, 14);
		/* Added 2 lines.  Feel left/right should be mirrored */
		leftYMov[x] = 0;
		player[PLAYER_LEFT].unit[x].isYInAir = false;
		player[PLAYER_LEFT].unit[x].unitType = basetypes[Lbaselookup[destructMode-1]-1][(mt_rand() % 10) + 2-1];
		if (player[PLAYER_LEFT].unit[x].unitType == UNIT_SATELLITE)
		{
			if (tempW == MAX_INSTALLATIONS - 1)
			{ /* Makes sure there's always at least one tank */
				player[PLAYER_LEFT].unit[x].unitType = UNIT_TANK;
			} else {
				leftY[x] = 30 + (mt_rand() % 40);
				tempW++;
			}
		}
		player[PLAYER_LEFT].unit[x].health = baseDamage[player[PLAYER_LEFT].unit[x].unitType-1];
		if (player[PLAYER_LEFT].unit[x].unitType != UNIT_SATELLITE)
		{
			player[PLAYER_LEFT].unitsRemaining++;
		}
	}

	tempW = 0;
	player[PLAYER_RIGHT].unitsRemaining = 0;

	for (x = 0; x < basetypes[Rbaselookup[destructMode-1]-1][1-1]; x++)
	{
		rightX[x] = 320 - ((mt_rand() % 120) + 22);
		rightY[x] = JE_placementPosition(rightX[x] - 1, 14);
		rightYMov[x] = 0;
		player[PLAYER_RIGHT].unit[x].isYInAir = false;
		player[PLAYER_RIGHT].unit[x].unitType = basetypes[Rbaselookup[destructMode-1]-1][(mt_rand() % 10) + 2-1];
		if (player[PLAYER_RIGHT].unit[x].unitType == UNIT_SATELLITE)
		{
			if (tempW == MAX_INSTALLATIONS - 1)
			{
				player[PLAYER_RIGHT].unit[x].unitType = UNIT_TANK;
			} else {
				rightY[x] = 30 + (mt_rand() % 40);
				tempW++;
			}
		}
		player[PLAYER_RIGHT].unit[x].health = baseDamage[player[PLAYER_RIGHT].unit[x].unitType-1];
		if (player[PLAYER_RIGHT].unit[x].unitType != UNIT_SATELLITE)
		{
			player[PLAYER_RIGHT].unitsRemaining++;
		}
	}

	for (z = 0; z < MAX_WALL; z++)
	{
		wallExist[z] = false;
	}

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

	for (temp = 0; temp < MAX_EXPLO; temp++)
	{
		if (explosionAvail[temp])
		{
			tempW = temp + 1;
			break;
		}
	}

	if (tempW > 0)
	{
		explosionAvail[tempW-1] = false;
		exploRec[tempW-1].x = destructTempX;
		exploRec[tempW-1].y = destructTempY;
		exploRec[tempW-1].explowidth = 2;

		/* Bug fix!  shottype == 0 when a unit is destroyed and actually pulls
		 * data out of another table.*/
		if(shottype != SHOT_INVALID)
		{
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

			exploRec[tempW-1].explomax = exploSize[shottype-1];
			exploRec[tempW-1].explofill = exploDensity[shottype-1];
			exploRec[tempW-1].explocolor = shotDirt[shottype-1];		}
		else
		{
			JE_eSound(4);
			exploRec[tempW-1].explomax = exploSize[shottype-1];
			exploRec[tempW-1].explofill = exploDensity[shottype-1];
			exploRec[tempW-1].explocolor = shotDirt[shottype-1];
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


	for (i = 0; i < MAX_PLAYERS; ++i)
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
	unsigned int p, u;


	for (p = 0; p < MAX_PLAYERS; ++p)
	{
		for (u = 0; u < MAX_INSTALLATIONS; ++u)
		{
			player[p].unit[u].ani_frame = 0;
			player[p].unit[u].health = 0;
		}
	}
}
void DE_ResetWeapons( void )
{
	unsigned int i;


	for (i = 0; i < MAX_SHOTS; i++)
	{
		destructShotAvail[i] = true;
	}
	for (i = 0; i < MAX_EXPLO; i++)
	{
		explosionAvail[i] = true;
	}
}
void DE_ResetLevel( void )
{
	unsigned int i, j;

	/* Okay, let's prep the arena */
	DE_ResetUnits();
	DE_ResetWeapons();

	JE_generateTerrain();

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		player[i].unitSelected = 1;
		for( j = 0; j < MAX_INSTALLATIONS; j++)
		{
			leftLastMove[j] = 0;
			rightLastMove[j] = 0;
			leftYMov[j] = 0;
			rightYMov[j] = 0;

			if (player[i].is_cpu)
			{
				if (systemAngle[player[i].unit[j].unitType-1] || player[i].unit[j].unitType == UNIT_HELI)
				{
					leftAngle[j] = M_PI / 4.0f;
					leftPower[j] = 4;
				} else {
					leftAngle[j] = 0;
					leftPower[j] = 4;
				}
				if (haveWalls)
				{
					player[i].unit[j].shotType = defaultCpuWeaponB[player[i].unit[j].unitType-1];
				}
				else
				{
					player[i].unit[j].shotType = defaultCpuWeapon[player[i].unit[j].unitType-1];
				}
			} else {
				leftAngle[j] = 0;
				leftPower[j] = 3;
				player[i].unit[j].shotType = defaultWeapon[player[i].unit[j].unitType-1];
			}
		}
	}
}
void DE_ResetActions( void )
{
	unsigned int i;


	for(i = 0; i < MAX_PLAYERS; i++)
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
	setjasondelay(1);

	memset(soundQueue, 0, sizeof(soundQueue));
	JE_tempScreenChecking();

	DE_ResetActions();
	DE_RunTickCycleDeadUnits();


	DE_RunTickGravity();

	/* This fixes the power of sats.  I'd really like to remove it and fix it properly */
	for (z = 0; z < MAX_INSTALLATIONS; z++)
	{
		if (player[PLAYER_LEFT].unit[z].unitType == UNIT_LASER)
		{
			leftPower[z] = 6;
		}
		if (player[PLAYER_RIGHT].unit[z].unitType == UNIT_LASER)
		{
			rightPower[z] = 6;
		}
	}

	DE_RunTickAnimate();
	DE_RunTickDrawWalls();
	DE_RunTickExplosions();
	DE_RunTickShots();
	DE_RunTickAI();
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
void DE_RunTickCycleDeadUnits( void )
{
	unsigned int i;


	/* This code automatically switches the active unit if it is destroyed
	 * and skips over the useless satellite */
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (player[i].unitsRemaining == 0) { continue; }

		while (CURUNIT_shotType(i) == SHOT_INVALID || CURUNIT_health(i) == 0)
		{
			player[i].unitSelected++;
			if (player[i].unitSelected > MAX_INSTALLATIONS)
			{
				player[i].unitSelected = 1;
			}
		}
	}
}
void DE_RunTickGravity( void )
{
	unsigned int z;

	for (z = 0; z < MAX_INSTALLATIONS; z++)
	{
		if (player[PLAYER_LEFT].unit[z].health > 0)
		{
			if (player[PLAYER_LEFT].unit[z].unitType != UNIT_SATELLITE)
			{
				if (player[PLAYER_LEFT].unit[z].isYInAir == true)
				{
					if (leftY[z] + leftYMov[z] > 199)
					{
						leftYMov[z] = 0;
						player[PLAYER_LEFT].unit[z].isYInAir = false;
					}
					leftY[z] += leftYMov[z];
					if (leftY[z] < 26)
					{
						leftYMov[z] = 0;
						leftY[z] = 26;
					}
					if (player[PLAYER_LEFT].unit[z].unitType == UNIT_HELI)
					{
						leftYMov[z] += 0.0001f;
					} else {
						leftYMov[z] += 0.03f;
					}
					if (!JE_stabilityCheck(leftX[z], round(leftY[z])))
					{
						leftYMov[z] = 0;
						player[PLAYER_LEFT].unit[z].isYInAir = false;
					}
				} else if (leftY[z] < 199) {
					if (JE_stabilityCheck(leftX[z], round(leftY[z])))
						leftY[z] += 1;
				}
				temp = leftGraphicBase[player[PLAYER_LEFT].unit[z].unitType-1] + player[PLAYER_LEFT].unit[z].ani_frame;
				if (player[PLAYER_LEFT].unit[z].unitType == UNIT_HELI)
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
				JE_drawShape2(leftX[z], round(leftY[z]) - 13, leftGraphicBase[player[PLAYER_LEFT].unit[z].unitType-1] + player[PLAYER_LEFT].unit[z].ani_frame, eShapes1);
			}
		}
	}
	for (z = 0; z < MAX_INSTALLATIONS; z++)
	{
		if (player[PLAYER_RIGHT].unit[z].health > 0)
		{
			if (player[PLAYER_RIGHT].unit[z].unitType != UNIT_SATELLITE)
			{
				if (player[PLAYER_RIGHT].unit[z].isYInAir == true)
				{
					if (rightY[z] + rightYMov[z] > 199)
					{
						rightYMov[z] = 0;
						player[PLAYER_RIGHT].unit[z].isYInAir = false;
					}
					rightY[z] += rightYMov[z];
					if (rightY[z] < 24)
					{
						rightYMov[z] = 0;
						rightY[z] = 24;
					}
					if (player[PLAYER_RIGHT].unit[z].unitType == UNIT_HELI) /*HELI*/
					{
						rightYMov[z] += 0.0001f;
					} else {
						rightYMov[z] += 0.03f;
					}
					if (!JE_stabilityCheck(rightX[z], round(rightY[z])))
					{
						rightYMov[z] = 0;
						player[PLAYER_RIGHT].unit[z].isYInAir = false;
					}
				} else if (rightY[z] < 199)
					if (JE_stabilityCheck(rightX[z], round(rightY[z])))
					{
						if (player[PLAYER_RIGHT].unit[z].unitType == UNIT_HELI)
						{
							rightYMov[z] += 0.01f;
						} else {
							rightY[z] += 1;
						}
					}

				temp = rightGraphicBase[player[PLAYER_RIGHT].unit[z].unitType-1] + player[PLAYER_RIGHT].unit[z].ani_frame;
				if (player[PLAYER_RIGHT].unit[z].unitType == UNIT_HELI)
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
				JE_drawShape2(rightX[z], round(rightY[z]) - 13, rightGraphicBase[player[PLAYER_RIGHT].unit[z].unitType-1] + player[PLAYER_RIGHT].unit[z].ani_frame, eShapes1);
			}
		}
	}
}
void DE_RunTickAnimate( void )
{
	unsigned int p, u;

	for (p = 0; p < MAX_PLAYERS; ++p)
	{
		for (u = 0; u < MAX_INSTALLATIONS; ++u)
		{
			// if unit does not animate continuously and is not currently animated
			if (!systemAni[player[p].unit[u].unitType-1] && player[p].unit[u].ani_frame == 0)
			{
				continue;
			}

			if (++(player[p].unit[u].ani_frame) > 3)
			{
				player[p].unit[u].ani_frame = 0;
			}
		}
	}
}
void DE_RunTickDrawWalls( void )
{
	unsigned int x;

	for (x = 0; x < MAX_WALL; x++)
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
	for (i = 0; i < MAX_EXPLO; i++)
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
			/* Actually an X out of bounds (wrapping around) doesn't seem too
			 * bad.  It's that way in the original too.  To fix this relatively
			 * harmless bug, just confine tempPosX */
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
					if (player[PLAYER_LEFT].unit[k].health > 0 && tempPosX > leftX[k] && tempPosX < leftX[k] + 11 && tempPosY > leftY[k] - 11 && tempPosY < leftY[k])
					{
						player[PLAYER_LEFT].unit[k].health--;
						if (player[PLAYER_LEFT].unit[k].health == 0)
						{
							JE_makeExplosion(leftX[k] + 5, round(leftY[k]) - 5, (player[PLAYER_LEFT].unit[k].unitType == UNIT_HELI) * 2);
							if (player[PLAYER_LEFT].unit[k].unitType != UNIT_SATELLITE)
							{
								player[PLAYER_LEFT].unitsRemaining--;
								player[PLAYER_RIGHT].score++;
							}
						}
					}
					if (player[PLAYER_RIGHT].unit[k].health > 0 && tempPosX > rightX[k] && tempPosX < rightX[k] + 11 && tempPosY > rightY[k] - 11 && tempPosY < rightY[k])
					{
						player[PLAYER_RIGHT].unit[k].health--;
						if (player[PLAYER_RIGHT].unit[k].health == 0)
						{
							JE_makeExplosion(rightX[k] + 5, round(rightY[k]) - 5, (player[PLAYER_RIGHT].unit[k].unitType == UNIT_HELI) * 2);
							if (player[PLAYER_RIGHT].unit[k].unitType != UNIT_SATELLITE)
							{
								player[PLAYER_RIGHT].unitsRemaining--;
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


	for (i = 0; i < MAX_SHOTS; i++) {
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
			if (player[PLAYER_LEFT].unit[j].health > 0)
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
			if (player[PLAYER_RIGHT].unit[j].health > 0)
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
		for (j = 0; j < MAX_WALL; j++)
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
void DE_RunTickAI( void )
{
	unsigned int i;
	unsigned int player_index, player_target;


	for (player_index = 0; player_index < MAX_PLAYERS; player_index++)
	{
		if (player[player_index].is_cpu == false)
		{
			continue;
		}


		/* I've been thinking, purely hypothetically, about what it would take
		 * to have multiple computer opponents.  The answer?  A lot of crap
		 * and a 'target' variable in the player struct. */

		player_target = player_index + 1;
		if (player_target >= MAX_PLAYERS)
		{
			player_target = 0;
		}

		/* This is the start of the original AI.  Heh.  AI. */
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
			if (player[player_index].aiMemory.c_Angle > 0 && leftAngle[CURUNIT(player_index)] > (M_PI / 2.0f) - (M_PI / 9.0f))
			{
				player[player_index].aiMemory.c_Angle = 0;
			}
			if (player[player_index].aiMemory.c_Angle < 0 && leftAngle[CURUNIT(player_index)] < M_PI / 8.0f)
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
			if (player[player_index].aiMemory.c_Power > 0 && leftPower[CURUNIT(player_index)] > 4)
			{
				player[player_index].aiMemory.c_Power = 0;
			}
			if (player[player_index].aiMemory.c_Power < 0 && leftPower[CURUNIT(player_index)] < 3)
			{
				player[player_index].aiMemory.c_Power = 0;
			}
			if (leftPower[CURUNIT(player_index)] < 2)
			{
				player[player_index].aiMemory.c_Power = 1;
			}
		}

		// prefer helicopter
		for (i = 0; i < MAX_INSTALLATIONS; i++)
		{
			if (player[player_index].unit[i].unitType == UNIT_HELI && player[player_index].unit[i].health > 0)
			{
				player[player_index].unitSelected = i + 1;
				break;
			}
		}

		if (CURUNIT_unitType(player_index) == UNIT_HELI)
		{
			if (CURUNIT_isYInAir(player_index) == false)
			{
				player[player_index].aiMemory.c_Power = 1;
			}
			if (mt_rand() % leftX[CURUNIT(player_index)] > 100)
			{
				player[player_index].aiMemory.c_Power = 1;
			}
			if (mt_rand() % 240 > leftX[CURUNIT(player_index)])
			{
				player[player_index].moves.actions[MOVE_RIGHT] = true;
			}
			else if ((mt_rand() % 20) + 300 < leftX[CURUNIT(player_index)])
			{
				player[player_index].moves.actions[MOVE_LEFT] = true;
			}
			else if (mt_rand() % 30 == 1)
			{
				player[player_index].aiMemory.c_Angle = (mt_rand() % 3) - 1;
			}
			if (leftX[CURUNIT(player_index)] > 295 && leftLastMove[CURUNIT(player_index)] > 1)
			{
				player[player_index].moves.actions[MOVE_LEFT] = true;
				player[player_index].moves.actions[MOVE_RIGHT] = false;
			}
			if (CURUNIT_unitType(player_index) != UNIT_HELI || leftLastMove[CURUNIT(player_index)] > 3 || (leftX[CURUNIT(player_index)] > 160 && leftLastMove[CURUNIT(player_index)] > -3))
			{
				if (mt_rand() % (int)round(leftY[CURUNIT(player_index)]) < 150 && leftYMov[CURUNIT(player_index)] < 0.01f && (leftX[CURUNIT(player_index)] < 160 || leftLastMove[CURUNIT(player_index)] < 2))
				{
					player[player_index].moves.actions[MOVE_FIRE] = true;
				}
				player[player_index].aiMemory.c_noDown = (5 - abs(leftLastMove[CURUNIT(player_index)])) * (5 - abs(leftLastMove[CURUNIT(player_index)])) + 3;
				player[player_index].aiMemory.c_Power = 1;
			} else
			{
				player[player_index].moves.actions[MOVE_FIRE] = false;
			}

			for (i = 0; i < MAX_INSTALLATIONS; i++)
			{
				if (abs(rightX[i] - leftX[CURUNIT(player_index)]) < 8)
				{
					/* I get it.  This makes helicoptors hover over
					 * their enemies. */
					if (player[player_target].unit[i].unitType == UNIT_SATELLITE)
					{
						player[player_index].moves.actions[MOVE_FIRE] = false;
					}
					else
					{
						player[player_index].moves.actions[MOVE_LEFT] = false;
						player[player_index].moves.actions[MOVE_RIGHT] = false;
						if (leftLastMove[CURUNIT(player_index)] < -1)
						{
							leftLastMove[CURUNIT(player_index)]++;
						}
						else if (leftLastMove[CURUNIT(player_index)] > 1)
						{
							leftLastMove[CURUNIT(player_index)]--;
						}
					}
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

		if (mt_rand() % 100 > 98 || CURUNIT_shotType(player_index) == SHOT_TRACER)
		{   /* Clearly the CPU doesn't like the tracer :) */
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

		if (leftYMov[CURUNIT(player_index)] < -0.1f && CURUNIT_unitType(player_index) == UNIT_HELI)
		{
			player[player_index].moves.actions[MOVE_FIRE] = false;
		}

		/* This last hack was down in the processing section.
		 * What exactly it was doing there I do not know */
		if(CURUNIT_unitType(player_index) == UNIT_LASER || CURUNIT_isYInAir(player_index) == true) {
			player[player_index].aiMemory.c_Power = 0;
		}
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
	if (CURUNIT_unitType(PLAYER_LEFT) == UNIT_HELI)
	{
		tempPosX = leftX[CURUNIT(PLAYER_LEFT)] + round(0.1f * leftLastMove[CURUNIT(PLAYER_LEFT)] * leftLastMove[CURUNIT(PLAYER_LEFT)] * leftLastMove[CURUNIT(PLAYER_LEFT)]) + 5;
		tempPosY = round(leftY[CURUNIT(PLAYER_LEFT)]) + 1;
	} else {
		tempPosX = round(leftX[CURUNIT(PLAYER_LEFT)] + 6 + cos(leftAngle[CURUNIT(PLAYER_LEFT)]) * (leftPower[CURUNIT(PLAYER_LEFT)] * 8 + 7));
		tempPosY = round(leftY[CURUNIT(PLAYER_LEFT)] - 7 - sin(leftAngle[CURUNIT(PLAYER_LEFT)]) * (leftPower[CURUNIT(PLAYER_LEFT)] * 8 + 7));
	}
	JE_pix(tempPosX,     tempPosY,     14);
	JE_pix(tempPosX + 3, tempPosY,      3);
	JE_pix(tempPosX - 3, tempPosY,      3);
	JE_pix(tempPosX,     tempPosY + 2,  3);
	JE_pix(tempPosX,     tempPosY - 2,  3);

	if (CURUNIT_unitType(PLAYER_RIGHT) == UNIT_HELI)
	{  /*Heli*/
		tempPosX = rightX[CURUNIT(PLAYER_RIGHT)] + round(0.1f * rightLastMove[CURUNIT(PLAYER_RIGHT)] * rightLastMove[CURUNIT(PLAYER_RIGHT)] * rightLastMove[CURUNIT(PLAYER_RIGHT)]) + 5;
		tempPosY = round(rightY[CURUNIT(PLAYER_RIGHT)]) + 1;
	} else {
		tempPosX = round(rightX[CURUNIT(PLAYER_RIGHT)] + 6 - cos(rightAngle[CURUNIT(PLAYER_RIGHT)]) * (rightPower[CURUNIT(PLAYER_RIGHT)] * 8 + 7));
		tempPosY = round(rightY[CURUNIT(PLAYER_RIGHT)] - 7 - sin(rightAngle[CURUNIT(PLAYER_RIGHT)]) * (rightPower[CURUNIT(PLAYER_RIGHT)] * 8 + 7));
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
	JE_drawShape2(  4, 0, 190 + CURUNIT_shotType(PLAYER_LEFT), eShapes1);
	JE_outText( 20, 3, weaponNames[CURUNIT_shotType(PLAYER_LEFT)-1], 15, 2);
	sprintf(tempstr, "dmg~%d~", CURUNIT_health(PLAYER_LEFT));
	JE_outText( 75, 3, tempstr, 15, 0);
	sprintf(tempstr, "pts~%d~", player[PLAYER_LEFT].score);
	JE_outText(110, 3, tempstr, 15, 0);

	JE_bar(175, 3, 184, 8, 241);
	JE_rectangle(174, 2, 185, 9, 242);
	JE_rectangle(173, 1, 186, 10, 240);
	JE_bar(188, 3, 310, 8, 241);
	JE_rectangle(187, 2, 312, 9, 242);
	JE_rectangle(186, 1, 313, 10, 240);
	JE_drawShape2(174, 0, 190 + CURUNIT_shotType(PLAYER_RIGHT), eShapes1);
	JE_outText(190, 3, weaponNames[CURUNIT_shotType(PLAYER_RIGHT)-1], 15, 2);
	sprintf(tempstr, "dmg~%d~", CURUNIT_health(PLAYER_RIGHT));
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

	for(player_index = 0; player_index < MAX_PLAYERS; player_index++)
	{
		for(key_index = 0; key_index < MAX_KEY; key_index++)
		{
			for(slot_index = 0; slot_index < MAX_KEY_OPTIONS; slot_index++)
			{
				key = player[player_index].keys.Config[key_index][slot_index];
				if(key == SDLK_UNKNOWN) { break; }
				if(keysactive[key] == true)
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
					break;
				}
			}
		}
	}
}
void DE_RunTickProcessInput( void )
{
	unsigned int emptyShot, i;
	unsigned int player_index;


	player_index = 0;
	if (player[player_index].unitsRemaining > 0)
	{
		if (systemAngle[CURUNIT_unitType(player_index)-1])
		{
			/*leftAnglechange*/
			if (player[player_index].moves.actions[MOVE_LEFT] == true)
			{
				leftAngle[CURUNIT(player_index)] += 0.01f;
				if (leftAngle[CURUNIT(player_index)] > M_PI / 2)
				{
					leftAngle[CURUNIT(player_index)] = M_PI / 2 - 0.01f;
				}
			}
			/*rightAnglechange*/
			if (player[player_index].moves.actions[MOVE_RIGHT] == true)
			{
				leftAngle[CURUNIT(player_index)] -= 0.01f;
				if (leftAngle[CURUNIT(player_index)] < 0)
				{
					leftAngle[CURUNIT(player_index)] = 0;
				}
			}
		} else if (CURUNIT_unitType(player_index) == UNIT_HELI) {
			if (player[player_index].moves.actions[MOVE_LEFT] == true && leftX[CURUNIT(player_index)] > 5)
				if (JE_stabilityCheck(leftX[CURUNIT(player_index)] - 5, round(leftY[CURUNIT(player_index)])))
				{
					if (leftLastMove[CURUNIT(player_index)] > -5)
					{
						leftLastMove[CURUNIT(player_index)]--;
					}
					leftX[CURUNIT(player_index)]--;
					if (JE_stabilityCheck(leftX[CURUNIT(player_index)], round(leftY[CURUNIT(player_index)])))
					{
						CURUNIT_isYInAir(player_index) = true;
					}
				}
			if (player[player_index].moves.actions[MOVE_RIGHT] == true && leftX[CURUNIT(player_index)] < 305)
				if (JE_stabilityCheck(leftX[CURUNIT(player_index)] + 5, round(leftY[CURUNIT(player_index)])))
				{
					if (leftLastMove[CURUNIT(player_index)] < 5)
					{
						leftLastMove[CURUNIT(player_index)]++;
					}
					leftX[CURUNIT(player_index)]++;
					if (JE_stabilityCheck(leftX[CURUNIT(player_index)], round(leftY[CURUNIT(player_index)])))
					{
						CURUNIT_isYInAir(player_index) = true;
					}
				}
		}

		/*Leftincreasepower*/
		if (player[player_index].moves.actions[MOVE_UP] == true)
		{
			if (CURUNIT_unitType(player_index) == UNIT_HELI)
			{
				CURUNIT_isYInAir(player_index) = true;
				leftYMov[CURUNIT(player_index)] -= 0.1f;
			} else if (CURUNIT_unitType(player_index) != UNIT_JUMPER
			        || CURUNIT_isYInAir(player_index) == true) {
				leftPower[CURUNIT(player_index)] += 0.05f;
				if (leftPower[CURUNIT(player_index)] > 5)
				{
					leftPower[CURUNIT(player_index)] = 5;
				}
			} else {
				leftYMov[CURUNIT(player_index)] = -3;
				CURUNIT_isYInAir(player_index) = true;
			}
		}
		/*Leftdecreasepower*/
		if (player[player_index].moves.actions[MOVE_DOWN] == true)
		{
			if (CURUNIT_unitType(player_index) == UNIT_HELI && CURUNIT_isYInAir(player_index) == true)
			{
				leftYMov [CURUNIT(player_index)] += 0.1f;
			} else {
				leftPower[CURUNIT(player_index)] -= 0.05f;
				if (leftPower[CURUNIT(player_index)] < 1)
				{
					leftPower[CURUNIT(player_index)] = 1;
				}
			}
		}
		/*Leftupweapon*/
		if (player[player_index].moves.actions[MOVE_CYUP] == true)
		{
			do
			{
				CURUNIT_shotType(player_index)++;
				if (CURUNIT_shotType(player_index) > SHOT_LAST)
				{
					CURUNIT_shotType(player_index) = SHOT_FIRST;
				}
			} while (weaponSystems[CURUNIT_unitType(player_index)-1][CURUNIT_shotType(player_index)-1] == 0);
		}
		/*Leftdownweapon*/
		if (player[player_index].moves.actions[MOVE_CYDN] == true)
		{
			do
			{
				CURUNIT_shotType(player_index)--;
				if (CURUNIT_shotType(player_index) < SHOT_FIRST)
				{
					CURUNIT_shotType(player_index) = SHOT_LAST;
				}
			} while (weaponSystems[CURUNIT_unitType(player_index)-1][CURUNIT_shotType(player_index)-1] == 0);
		}

		/*Leftchange*/
		if (player[player_index].moves.actions[MOVE_CHANGE] == true)
		{
			player[player_index].unitSelected++;
			if (player[player_index].unitSelected > MAX_INSTALLATIONS)
			{
				player[player_index].unitSelected = 1;
			}
		}

		/*Newshot*/
		if (player[player_index].shotDelay > 0)
		{
			player[player_index].shotDelay--;
		}
		if (player[player_index].moves.actions[MOVE_FIRE] == true && (player[player_index].shotDelay == 0))
		{
			player[player_index].shotDelay = shotDelay[CURUNIT_shotType(player_index)-1];

			if (shotDirt[CURUNIT_shotType(player_index)-1] > 20)
			{
				emptyShot = 0;
				for (i = 0; i < MAX_SHOTS; i++)
				{
					if (destructShotAvail[i])
					{
						emptyShot = i + 1;
						break;
					}
				}

				if (emptyShot > 0 && (CURUNIT_unitType(player_index) != UNIT_HELI
				 || CURUNIT_isYInAir(player_index) == true))
				{
					soundQueue[0] = shotSound[CURUNIT_shotType(player_index)-1];

					if (CURUNIT_unitType(player_index) == UNIT_HELI)
					{
						shotRec[emptyShot-1].x = leftX[CURUNIT(player_index)] + leftLastMove[CURUNIT(player_index)] * 2 + 5;
						shotRec[emptyShot-1].y = leftY[CURUNIT(player_index)] + 1;
						shotRec[emptyShot-1].ymov = 0.5f + leftYMov[CURUNIT(player_index)] * 0.1f;
						/* This next line is responsible for the odd 'if I'm
						 * moving up while firing at a very slight angle it
						 * shifts to the other side' 'bug'.
						 * It's present in the original BTW. */
						shotRec[emptyShot-1].xmov = 0.02f * leftLastMove[CURUNIT(player_index)] * leftLastMove[CURUNIT(player_index)] * leftLastMove[CURUNIT(player_index)];

						if (player[player_index].moves.actions[MOVE_UP] && leftY[CURUNIT(player_index)] < 30) /* For some odd reason if we're too high we ignore most of our computing. */
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
							shotRec[emptyShot-1].y = leftY[CURUNIT(player_index)];
						}
					} else {
						shotRec[emptyShot-1].x = leftX[CURUNIT(player_index)] + 6 + cos(leftAngle[CURUNIT(player_index)]) * 10;
						shotRec[emptyShot-1].y = leftY[CURUNIT(player_index)] - 7 - sin(leftAngle[CURUNIT(player_index)]) * 10;
						shotRec[emptyShot-1].ymov = -sin(leftAngle[CURUNIT(player_index)]) * leftPower[CURUNIT(player_index)];
						shotRec[emptyShot-1].xmov =  cos(leftAngle[CURUNIT(player_index)]) * leftPower[CURUNIT(player_index)];
					}

					shotRec[emptyShot-1].shottype = CURUNIT_shotType(player_index);

					destructShotAvail[emptyShot-1] = false;

					shotRec[emptyShot-1].shotdur = shotFuse[shotRec[emptyShot-1].shottype-1];

					shotRec[emptyShot-1].trail1c = 0;
					shotRec[emptyShot-1].trail2c = 0;
				}
			} else {
				switch (shotDirt[CURUNIT_shotType(player_index)-1])
				{
				case 1: /* magnet */
					for (i = 0; i < MAX_SHOTS; i++)
					{
						if (destructShotAvail[i] == false)
						{
							if (shotRec[i].x > leftX[CURUNIT(player_index)])
							{
								shotRec[i].xmov += leftPower[CURUNIT(player_index)] * 0.1f;
							}
						}
					}
					for (i = 0; i < MAX_INSTALLATIONS; i++) /* magnets push coptors */
					{
						if (player[player_index].unit[i].unitType == UNIT_HELI
						 && player[player_index].unit[i].isYInAir && (rightX[i] + 11) < 318) /* changed to properly align border */
						{
							rightX[i] += 2;
						}
					}
					CURUNIT_ani_frame(player_index) = 1;
					break;
				}
			}
		}
	}

	player_index = 1;
	if (player[player_index].unitsRemaining > 0)
	{
		if (systemAngle[CURUNIT_unitType(player_index)-1])
		{
			/*rightAnglechange*/
			if (player[player_index].moves.actions[MOVE_RIGHT]== true)
			{
				rightAngle[CURUNIT(player_index)] += 0.01f;
				if (rightAngle[CURUNIT(player_index)] > M_PI / 2)
				{
					rightAngle[CURUNIT(player_index)] = M_PI / 2 - 0.01f;
				}
			}
			/*rightAnglechange*/
			if (player[player_index].moves.actions[MOVE_LEFT] == true)
			{
				rightAngle[CURUNIT(player_index)] -= 0.01f;
				if (rightAngle[CURUNIT(player_index)] < 0)
				{
					rightAngle[CURUNIT(player_index)] = 0;
				}
			}
		} else if (CURUNIT_unitType(player_index) == UNIT_HELI) { /*Helicopter*/
			if (player[player_index].moves.actions[MOVE_LEFT] == true && rightX[CURUNIT(player_index)] > 5)
			{
				if (JE_stabilityCheck(rightX[CURUNIT(player_index)] - 5, round(rightY[CURUNIT(player_index)])))
				{
					if (rightLastMove[CURUNIT(player_index)] > -5)
					{
						rightLastMove[CURUNIT(player_index)]--;
					}
					rightX[CURUNIT(player_index)]--;
					if (JE_stabilityCheck(rightX[CURUNIT(player_index)], round(rightY[CURUNIT(player_index)])))
					{
						CURUNIT_isYInAir(player_index) = true;
					}
				}
			}
			if (player[player_index].moves.actions[MOVE_RIGHT] == true && rightX[CURUNIT(player_index)] < 305)
			{
				if (JE_stabilityCheck(rightX[CURUNIT(player_index)] + 5, round(rightY[CURUNIT(player_index)])))
				{
					if (rightLastMove[CURUNIT(player_index)] < 5)
					{
						rightLastMove[CURUNIT(player_index)]++;
					}
					rightX[CURUNIT(player_index)]++;
					if (JE_stabilityCheck(rightX[CURUNIT(player_index)], round(rightY[CURUNIT(player_index)])))
					{
						CURUNIT_isYInAir(player_index) = true;
					}
				}
			}
		}

		/*Rightincreasepower*/
		if (player[player_index].moves.actions[MOVE_UP] == true)
		{
			if (CURUNIT_unitType(player_index) == UNIT_HELI)
			{
				CURUNIT_isYInAir(player_index) = true;
				rightYMov[CURUNIT(player_index)] -= 0.1f;
			} else if (CURUNIT_unitType(player_index) != UNIT_JUMPER
			        || CURUNIT_isYInAir(player_index) == true) {
				rightPower[CURUNIT(player_index)] += 0.05f;
				if (rightPower[CURUNIT(player_index)] > 5)
				{
					rightPower[CURUNIT(player_index)] = 5;
				}
			} else {
				rightYMov[CURUNIT(player_index)] = -3;
				CURUNIT_isYInAir(player_index) = true;
			}
		}
		/*Rightdecreasepower*/
		if (player[player_index].moves.actions[MOVE_DOWN] == true)
		{
			if (CURUNIT_unitType(player_index) == UNIT_HELI && CURUNIT_isYInAir(player_index) == true)
			{ /*HELI*/
				rightYMov[CURUNIT(player_index)] += 0.1f;
			} else {
				rightPower[CURUNIT(player_index)] -= 0.05f;
				if (rightPower[CURUNIT(player_index)] < 1)
				{
					rightPower[CURUNIT(player_index)] = 1;
				}
			}
		}
		/*Rightupweapon*/
		if (player[player_index].moves.actions[MOVE_CYUP]== true)
		{
			do
			{
				CURUNIT_shotType(player_index)++;
				if (CURUNIT_shotType(player_index) > SHOT_LAST)
				{
					CURUNIT_shotType(player_index) = SHOT_FIRST;
				}
			} while (weaponSystems[CURUNIT_unitType(player_index)-1][CURUNIT_shotType(player_index)-1] == 0);
		}
		/*Rightdownweapon*/
		if (player[player_index].moves.actions[MOVE_CYDN] == true)
		{
			do
			{
				CURUNIT_shotType(player_index)--;
				if (CURUNIT_shotType(player_index) < SHOT_FIRST)
				{
					CURUNIT_shotType(player_index) = SHOT_LAST;
				}
			} while (weaponSystems[CURUNIT_unitType(player_index)-1][CURUNIT_shotType(player_index)-1] == 0);

		}

		/*Rightchange*/
		if (player[player_index].moves.actions[MOVE_CHANGE] == true)
		{
			player[player_index].unitSelected++;
			if (player[player_index].unitSelected > MAX_INSTALLATIONS)
			{
				player[player_index].unitSelected = 1;
			}
		}

		/*Newshot*/
		if (player[player_index].shotDelay > 0)
		{
			player[player_index].shotDelay--;
		}
		if (player[player_index].moves.actions[MOVE_FIRE] == true && player[player_index].shotDelay == 0)
		{
			player[player_index].shotDelay = shotDelay[CURUNIT_shotType(player_index)-1];

			emptyShot = 0;
			for (i = 0; i < MAX_SHOTS; i++)
			{
				if (destructShotAvail[i])
				{
					emptyShot = i + 1;
				}
			}

			if (shotDirt[CURUNIT_shotType(player_index)-1] > 20)
			{
				if (emptyShot > 0 && (CURUNIT_unitType(player_index) != UNIT_HELI
				 || CURUNIT_isYInAir(player_index) == true))
				{
					soundQueue[1] = shotSound[CURUNIT_shotType(player_index)-1];

					if (CURUNIT_unitType(player_index) == UNIT_HELI)
					{
						shotRec[emptyShot-1].x = rightX[CURUNIT(player_index)] + rightLastMove[CURUNIT(player_index)] * 2 + 5;
						shotRec[emptyShot-1].y = rightY[CURUNIT(player_index)] + 1;
						shotRec[emptyShot-1].ymov = 0.5f;
						shotRec[emptyShot-1].xmov = 0.02f * rightLastMove[CURUNIT(player_index)] * rightLastMove[CURUNIT(player_index)] * rightLastMove[CURUNIT(player_index)];
						if ((keysactive[SDLK_KP8] || keysactive[SDLK_UP]) && rightY[CURUNIT(player_index)] < 30)
						{
							shotRec[emptyShot-1].ymov = 0.1f;
							if (shotRec[emptyShot-1].xmov < 0)
							{
								shotRec[emptyShot-1].xmov += 0.1f;
							}
							else if (shotRec[emptyShot-1].xmov > 0) {
								shotRec[emptyShot-1].xmov -= 0.1f;
							}
							shotRec[emptyShot-1].y = rightY[CURUNIT(player_index)];
						}
					} else {
						shotRec[emptyShot-1].x = rightX [CURUNIT(player_index)] + 6 - cos(rightAngle[CURUNIT(player_index)]) * 10;
						shotRec[emptyShot-1].y = rightY [CURUNIT(player_index)] - 7 - sin(rightAngle[CURUNIT(player_index)]) * 10;
						shotRec[emptyShot-1].ymov = -sin(rightAngle[CURUNIT(player_index)]) * rightPower[CURUNIT(player_index)];
						shotRec[emptyShot-1].xmov = -cos(rightAngle[CURUNIT(player_index)]) * rightPower[CURUNIT(player_index)];
					}

					if (CURUNIT_unitType(player_index) == UNIT_JUMPER)
					{
						shotRec[emptyShot-1].x = rightX[CURUNIT(player_index)] + 2;
						if (CURUNIT_isYInAir(player_index) == true)
						{
							shotRec[emptyShot-1].ymov = 1;
							shotRec[emptyShot-1].y = rightY[CURUNIT(player_index)] + 2;
						} else {
							shotRec[emptyShot-1].ymov = -2;
							shotRec[emptyShot-1].y = rightY[CURUNIT(player_index)] - 12;
						}
					}

					shotRec[emptyShot-1].shottype = CURUNIT_shotType(player_index);

					destructShotAvail[emptyShot-1] = false;

					shotRec[emptyShot-1].shotdur = shotFuse[shotRec[emptyShot-1].shottype-1];

					shotRec[emptyShot-1].trail1c = 0;
					shotRec[emptyShot-1].trail2c = 0;
				}
			} else {
				switch (shotDirt[CURUNIT_shotType(player_index)-1])
				{
				case 1:
					for (i = 0; i < MAX_SHOTS; i++)
					{
						if (!destructShotAvail[i])
						{
							if (shotRec[i].x < rightX[CURUNIT(player_index)])
							{
								shotRec[i].xmov -= rightPower[CURUNIT(player_index)] * 0.1f;
							}
						}
					}
					for (i = 0; i < MAX_INSTALLATIONS; i++)
					{
						if (player[player_index].unit[i].unitType == UNIT_HELI
						 && player[player_index].unit[i].isYInAir == true
						 && leftX[i] > 1)
						{
							leftX[i] -= 2;
						}
					}

					CURUNIT_ani_frame(player_index) = 1;
					break;
				}
			}
		}
	}
}
bool DE_RunTickCheckEndgame( void )
{
	if (player[PLAYER_LEFT].unitsRemaining == 0)
	{
		player[PLAYER_RIGHT].score += lModeScore[destructMode-1];
		died = true;
		soundQueue[7] = V_CLEARED_PLATFORM;
		endDelay = 80;
	}
	if (player[PLAYER_RIGHT].unitsRemaining == 0)
	{
		player[PLAYER_LEFT].score += rModeScore[destructMode-1];
		died = true;
		soundQueue[7] = V_CLEARED_PLATFORM;
		endDelay = 80;
	}
	return(player[PLAYER_LEFT].unitsRemaining == 0 || player[PLAYER_RIGHT].unitsRemaining == 0);
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
