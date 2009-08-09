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
 * Right now most (but not quite all) of the left/right vars have been
 * made arrays and placed in appropriate structs.  Functions are being
 * analyzed, split up, and simplified when possible.  Maintenance should
 * already be a lot easier.
 *
 * Things I wanted to do but can't: Remove references to VGAScreen.
 * VGAScreen is a global.  I thought I could draw everything to
 * destructTempScreen and blit it later; that's what happens most of the time.
 * Unfortunately functions like JE_rectangle will NOT allow you to specify
 * the screen you want to blit to.  So that'll have to wait until we switch
 * away from 256 colors.
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
#define MAX_SHOTS 40
#define MAX_WALLS 20
#define MAX_EXPLO 40
#define MAX_KEY_OPTIONS 4
#define MAX_INSTALLATIONS 20

/*** Macros ***/
/* I believe in having a strict hierarchy of objects.  Units are owned
 * by players and players are owned by the world.  I feel it makes things
 * clearer, but it can result in a pretty nasty wall of text for what is
 * essentially a simple MOV.
 *
 * This macro helps a little.  Yuriks thinks we should use pointers, and
 * normally I'd agree except the code base is just about as far removed from
 * pointers and passing variables as possible.  We'l work it out eventually.
 */

/* Don't use ++ and -- operators with macros.  They may look innocent
 * but given half the chance they will crash you and everyone
 * you care about.  It's a good practice to NOT embed them. */
#define CURUNIT(x) player[(x)].unit[(player[(x)].unitSelected-1)]

/* Todo: create 'makeshot' function which will zero out trails + fix that bug.*/

/*** Enums ***/
/* There are an awful lot of index-1s scattered throughout the code.
 * I plan to eventually change them all and to be able to properly
 * enumerate from 0 up.
 */
enum de_player_t { PLAYER_LEFT = 0, PLAYER_RIGHT = 1, MAX_PLAYERS = 2 };
enum de_mode_t { MODE_5CARDWAR = 1, MODE_TRADITIONAL = 2, MODE_HELIASSAULT = 3,
                 MODE_HELIDEFENSE = 4, MODE_OUTGUNNED = 5, MAX_MODES = 5 };
enum de_unit_t { UNIT_TANK = 1, UNIT_NUKE, UNIT_DIRT, UNIT_SATELLITE,
                 UNIT_MAGNET, UNIT_LASER, UNIT_JUMPER, UNIT_HELI,
                 UNIT_FIRST = UNIT_TANK, UNIT_LAST = UNIT_HELI,
                 MAX_UNITS = 8 };
enum de_shot_t { SHOT_TRACER = 1, SHOT_SMALL, SHOT_LARGE, SHOT_MICRO,
                 SHOT_SUPER, SHOT_DEMO, SHOT_SMALLNUKE, SHOT_LARGENUKE,
                 SHOT_SMALLDIRT, SHOT_LARGEDIRT, SHOT_MAGNET, SHOT_MINILASER,
                 SHOT_MEGALASER, SHOT_LASERTRACER, SHOT_MEGABLAST, SHOT_MINI,
                 SHOT_BOMB,
                 SHOT_FIRST = SHOT_TRACER, SHOT_LAST = SHOT_BOMB,
                 MAX_SHOT_TYPES = 17, SHOT_INVALID = 0 };
enum de_trails_t { TRAILS_NONE = 0, TRAILS_NORMAL = 1, TRAILS_FULL = 2 };
enum de_pixel_t { PIXEL_BLACK = 0, PIXEL_DIRT = 25,
                  PIXEL_RED_MIN = 241, PIXEL_EXPLODE = 252 };
enum de_mapflags_t { MAP_NORMAL = 0x00, MAP_WALLS = 0x01, MAP_RINGS = 0x02,
					 MAP_HOLES = 0x04, MAP_FUZZY = 0x08, MAP_TALL = 0x10 };

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
struct destruct_wall_s {

	bool wallExist;
	unsigned int wallX, wallY;
};
struct destruct_world_s {

	/* Map data & screen pointer */
	unsigned int baseMap[320];
	SDL_Surface * VGAScreen;
	struct destruct_wall_s mapWalls[MAX_WALLS];

	/* Map configuration */
	unsigned int mapFlags;
};

/*** Function decs ***/
void JE_destructMain( void );
void JE_introScreen( void );
void JE_modeSelect( void );

void JE_generateTerrain( void );
void DE_generateBaseTerrain( unsigned int, unsigned int *);
void DE_drawBaseTerrain( unsigned int * );
void DE_generateUnits( unsigned int * );
void DE_generateWalls( struct destruct_world_s * );
void DE_generateRings(SDL_Surface *, Uint8 );
void JE_aliasDirt( SDL_Surface * );


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




unsigned int JE_placementPosition( unsigned int, unsigned int, unsigned int * );
JE_boolean JE_stabilityCheck( JE_integer x, JE_integer y );

void JE_tempScreenChecking( void );

void JE_makeExplosion( JE_word destructTempX, JE_word destructTempY, JE_byte shottype );
void JE_eSound( JE_byte sound );
void JE_superPixel( JE_word loc );

void JE_helpScreen( void );
void JE_pauseScreen( void );



/*** Weapon configurations ***/

/* Part of me wants to leave these as bytes to save space. */
const bool     demolish[MAX_SHOT_TYPES] = {false, false, false, false, false, true, true, true, false, false, false, false, true, false, true, false, true};
const int        shotGr[MAX_SHOT_TYPES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 101};
const int     shotTrail[MAX_SHOT_TYPES] = {TRAILS_NONE, TRAILS_NONE, TRAILS_NONE, TRAILS_NORMAL, TRAILS_NORMAL, TRAILS_NORMAL, TRAILS_FULL, TRAILS_FULL, TRAILS_NONE, TRAILS_NONE, TRAILS_NONE, TRAILS_NORMAL, TRAILS_FULL, TRAILS_NORMAL, TRAILS_FULL, TRAILS_NORMAL, TRAILS_NONE};
const int      shotFuse[MAX_SHOT_TYPES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};
const int     shotDelay[MAX_SHOT_TYPES] = {10, 30, 80, 20, 60, 100, 140, 200, 20, 60, 5, 15, 50, 5, 80, 16, 0};
const int     shotSound[MAX_SHOT_TYPES] = {8, 2, 1, 7, 7, 9, 22, 22, 5, 13, 10, 15, 15, 26, 14, 7, 7};
const int     exploSize[MAX_SHOT_TYPES] = {4, 20, 30, 14, 22, 16, 40, 60, 10, 30, 0, 5, 10, 3, 15, 7, 0};
const bool   shotBounce[MAX_SHOT_TYPES] = {false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, false, true};
const int  exploDensity[MAX_SHOT_TYPES] = {  2,  5, 10, 15, 20, 15, 25, 30, 40, 80, 0, 30, 30,  4, 30, 5, 0};
const int      shotDirt[MAX_SHOT_TYPES] = {252, 252, 252, 252, 252, 252, 252, 252, 25, 25, 1, 252, 252, 252, 252, 252, 0};
const int     shotColor[MAX_SHOT_TYPES] = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 10, 10, 10, 10, 16, 0};

const int     defaultWeapon[MAX_UNITS] = {SHOT_SMALL, SHOT_MICRO,     SHOT_SMALLDIRT, SHOT_INVALID, SHOT_MAGNET, SHOT_MINILASER, SHOT_MICRO, SHOT_MINI};
const int  defaultCpuWeapon[MAX_UNITS] = {SHOT_SMALL, SHOT_MICRO,     SHOT_DEMO,      SHOT_INVALID, SHOT_MAGNET, SHOT_MINILASER, SHOT_MICRO, SHOT_MINI};
const int defaultCpuWeaponB[MAX_UNITS] = {SHOT_DEMO,  SHOT_SMALLNUKE, SHOT_DEMO,      SHOT_INVALID, SHOT_MAGNET, SHOT_MEGALASER, SHOT_MICRO, SHOT_MINI};
const int       systemAngle[MAX_UNITS] = {true, true, true, false, false, true, false, false};
const int        baseDamage[MAX_UNITS] = {200, 120, 400, 300, 80, 150, 600, 40};
const int         systemAni[MAX_UNITS] = {false, false, false, true, false, false, false, true};

const bool weaponSystems[MAX_UNITS][MAX_SHOT_TYPES] =
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

/* More constant configuration settings. */
/* Music that destruct will play.  You can check out musmast.c to see what is what. */
const JE_byte goodsel[14] /*[1..14]*/ = {1, 2, 6, 12, 13, 14, 17, 23, 24, 26, 28, 29, 32, 33};

/* Unit creation.  Need to move this later: Doesn't belong here */
const JE_byte basetypes[8][11] /*[1..8, 1..11]*/ = /*0 is amount of units*/
{
	{5, UNIT_TANK, UNIT_TANK, UNIT_NUKE, UNIT_DIRT,      UNIT_DIRT,   UNIT_SATELLITE, UNIT_MAGNET, UNIT_LASER,  UNIT_JUMPER, UNIT_HELI},   /*Normal*/
	{8, UNIT_TANK, UNIT_TANK, UNIT_TANK, UNIT_TANK,      UNIT_TANK,   UNIT_TANK,      UNIT_TANK,   UNIT_TANK,   UNIT_TANK,   UNIT_TANK},   /*Traditional*/
	{4, UNIT_HELI, UNIT_HELI, UNIT_HELI, UNIT_HELI,      UNIT_HELI,   UNIT_HELI,      UNIT_HELI,   UNIT_HELI,   UNIT_HELI,   UNIT_HELI},   /*Weak   Heli attack fleet*/
	{8, UNIT_TANK, UNIT_TANK, UNIT_TANK, UNIT_NUKE,      UNIT_NUKE,   UNIT_NUKE,      UNIT_DIRT,   UNIT_MAGNET, UNIT_LASER,  UNIT_JUMPER},   /*Strong Heli defense fleet*/
	{8, UNIT_HELI, UNIT_HELI, UNIT_HELI, UNIT_HELI,      UNIT_HELI,   UNIT_HELI,      UNIT_HELI,   UNIT_HELI,   UNIT_HELI,   UNIT_HELI},   /*Strong Heli attack fleet*/
	{4, UNIT_TANK, UNIT_TANK, UNIT_TANK, UNIT_TANK,      UNIT_NUKE,   UNIT_NUKE,      UNIT_DIRT,   UNIT_MAGNET, UNIT_JUMPER, UNIT_JUMPER},   /*Weak   Heli defense fleet*/
	{8, UNIT_TANK, UNIT_NUKE, UNIT_DIRT, UNIT_SATELLITE, UNIT_MAGNET, UNIT_LASER,     UNIT_JUMPER, UNIT_HELI,   UNIT_TANK,   UNIT_NUKE},   /*Overpowering fleet*/
	{4, UNIT_TANK, UNIT_TANK, UNIT_NUKE, UNIT_DIRT,      UNIT_TANK,   UNIT_LASER,     UNIT_JUMPER, UNIT_HELI,   UNIT_NUKE,   UNIT_JUMPER}    /*Weak fleet*/
};
const unsigned int baseLookup[MAX_PLAYERS][DESTRUCT_MODES] =
{
	{1, 2, 4, 5, 7},
	{1, 2, 3, 6, 8}
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

struct destruct_player_s player[MAX_PLAYERS];
struct destruct_world_s  world;
struct destruct_shot_s shotRec[MAX_SHOTS]; /*[1..shotmax]*/
struct destruct_explo_s exploRec[MAX_EXPLO]; /*[1..explomax]*/


JE_boolean destructShotAvail[MAX_SHOTS]; /*[1..shotmax]*/


JE_boolean endOfGame, destructQuit;
JE_boolean explosionAvail[MAX_EXPLO]; /*[1..explomax]*/




void JE_destructGame( void )
{
	/* This is the entry function.  Any one-time actions we need to
	 * perform can go in here. */
	destructFirstTime = true;
	JE_clr256();
	JE_showVGA();
	endOfGame = false;
	destructTempScreen = game_screen;
	world.VGAScreen = VGAScreen;
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
	/* The unique modifiers:
	    Altered generation (really tall)
	    Fuzzy hills
	    Rings of dirt

	   The non-unique ones;:
	    Rings of not dirt (holes)
	    Walls
	*/

	world.mapFlags = MAP_NORMAL;

	if(mt_rand() % 2 == 0)
	{
		world.mapFlags |= MAP_WALLS;
	}
	if(mt_rand() % 4 == 0)
	{
		world.mapFlags |= MAP_HOLES;
	}
	switch(mt_rand() % 4)
	{
	case 0:
		world.mapFlags |= MAP_FUZZY;
		break;

	case 1:
		world.mapFlags |= MAP_TALL;
		break;

	case 2:
		world.mapFlags |= MAP_RINGS;
		break;
	}

	play_song(goodsel[mt_rand() % 14] - 1);

	/* Todo: create world struct w walls and dirt. Pass it instead */
	DE_generateBaseTerrain(world.mapFlags, world.baseMap);
	DE_generateUnits(world.baseMap);
	DE_generateWalls(&world);
	DE_drawBaseTerrain(world.baseMap);

	if (world.mapFlags & MAP_RINGS)
	{
		DE_generateRings(world.VGAScreen, PIXEL_DIRT);
	}
	if (world.mapFlags &MAP_HOLES)
	{
		DE_generateRings(world.VGAScreen, PIXEL_BLACK);
	}

	JE_aliasDirt(world.VGAScreen);
	JE_showVGA();

	memcpy(destructTempScreen->pixels, VGAScreen->pixels, destructTempScreen->pitch * destructTempScreen->h);
}
void DE_generateBaseTerrain( unsigned int mapFlags, unsigned int * baseWorld)
{
	unsigned int i;
	unsigned int newheight, HeightMul;
	double sinewave, sinewave2, cosinewave, cosinewave2;


	/* The 'terrain' is actually the video buffer :).  If it's brown, flu... er,
	 * brown pixels are what we check for collisions with. */

	/* The ranges here are between .01 and roughly 0.07283...*/
	sinewave    = (mt_rand_lt1()) * M_PI / 50.0 + 0.01;
	sinewave2   = (mt_rand_lt1()) * M_PI / 50.0 + 0.01;
	cosinewave  = (mt_rand_lt1()) * M_PI / 50.0 + 0.01;
	cosinewave2 = (mt_rand_lt1()) * M_PI / 50.0 + 0.01;
	HeightMul = 20;

	/* This block just exists to mix things up. */
	if(mapFlags & MAP_FUZZY)
	{
		sinewave  = M_PI - mt_rand_lt1() * 0.3f;
		sinewave2 = M_PI - mt_rand_lt1() * 0.3f;
	}
	if(mapFlags & MAP_TALL)
	{
		HeightMul = 100;
	}

	/* Now compute a height for each of our lines. */
	for (i = 1; i <= 318; i++)
	{
		newheight = round(sin(sinewave   * i) * HeightMul + sin(sinewave2   * i) * 15.0 +
		                  cos(cosinewave * i) * 10.0      + sin(cosinewave2 * i) * 15.0) + 130;

		/* Bind it; we have mins and maxs */
		if (newheight < 40)
		{
			newheight = 40;
		}
		else if (newheight > 195) {
			newheight = 195;
		}
		baseWorld[i] = newheight;
	}
	/* The base world has been created. */
}
void DE_drawBaseTerrain( unsigned int * baseWorld)
{
	unsigned int i;


	for (i = 1; i <= 318; i++)
	{
		JE_rectangle(i, baseWorld[i], i, 199, PIXEL_DIRT);
	}
}

void DE_generateUnits( unsigned int * baseWorld )
{
	unsigned int i, j, numSatellites;


	for (i = 0; i < MAX_PLAYERS; i++)
	{

		numSatellites = 0;
		player[i].unitsRemaining = 0;

		for (j = 0; j < basetypes[baseLookup[i][destructMode-1]-1][0]; j++)
		{
			/* Not everything is the same between players */
			if(i == PLAYER_LEFT)
			{
				player[i].unit[j].unitX = (mt_rand() % 120) + 10;
			}
			else
			{
				player[i].unit[j].unitX = 320 - ((mt_rand() % 120) + 22);
			}

			player[i].unit[j].unitY = JE_placementPosition(player[i].unit[j].unitX - 1, 14, baseWorld);
			player[i].unit[j].unitYMov = 0;
			player[i].unit[j].isYInAir = false;
			player[i].unit[j].unitType = basetypes[baseLookup[i][destructMode-1]-1][(mt_rand() % 10) + 1];

			/* Sats are special cases since they are useless.  They don't count
			 * as active units and we can't have a team of all sats */
			if (player[i].unit[j].unitType == UNIT_SATELLITE)
			{
				if (numSatellites == basetypes[baseLookup[i][destructMode-1]-1][0])
				{
					player[i].unit[j].unitType = UNIT_TANK;
					player[i].unitsRemaining++;
				} else {
					player[i].unit[j].unitY = 30 + (mt_rand() % 40);
					numSatellites++;
				}
			}
			else
			{
				player[i].unitsRemaining++;
			}

			/* Now just fill in the rest of the unit's values. */
			player[i].unit[j].health = baseDamage[player[i].unit[j].unitType-1];
			player[i].unit[j].ani_frame = 0;
		}
	}
}
void DE_generateWalls( struct destruct_world_s * gameWorld )
{
	unsigned int i, j, wallX;
	unsigned int wallHeight, remainWalls;


	if ((world.mapFlags & MAP_WALLS) == false)
	{
		/* Just clear them out */
		for (i = 0; i < MAX_WALLS; i++)
		{
			gameWorld->mapWalls[i].wallExist = false;
		}
		return;
	}

	remainWalls = MAX_WALLS;

	do {

		/* Create a wall.  Decide how tall the wall will be */
		wallHeight = (mt_rand() % 5) + 1;
		if(wallHeight > remainWalls)
		{
			wallHeight = remainWalls;
		}

		/* Now find a good place to put the wall. */
		while(1) {
label_outercontinue:

			wallX = (mt_rand() % 300) + 10;

			/* Is this place already occupied? Apparently we only care enough
			 * to leave four units unmasked (I can certainly recall having
			 * units hidden behind walls) */
			for (i = 0; i < MAX_PLAYERS; i++)
			{
				for (j = 0; j < 4; j++)
				{
					if ((wallX > player[i].unit[j].unitX - 12)
					 && (wallX < player[i].unit[j].unitX + 13))
					{
						goto label_outercontinue; /* Breaking out of nested loops is perhaps one of goto's only legitimate uses. */
					}
				}
			}
			break;
		}


		/* We now have a valid X.  Create the wall. */
		for (i = 1; i <= wallHeight; i++)
		{
			gameWorld->mapWalls[remainWalls - i].wallExist = true;
			gameWorld->mapWalls[remainWalls - i].wallX = wallX;
			gameWorld->mapWalls[remainWalls - i].wallY = JE_placementPosition(wallX, 12, gameWorld->baseMap) - 14 * i;
		}

		remainWalls -= wallHeight;

	} while (remainWalls != 0);
}
void DE_generateRings( SDL_Surface * screen, Uint8 pixel )
{
	unsigned int i, j, tempSize;
	int tempPosX1, tempPosY1, tempPosX2, tempPosY2;
	double tempRadian;


	int rings = mt_rand() % 6 + 1;
	for (i = 1; i <= rings; i++)
	{
		tempPosX1 = (mt_rand() % 320);
		tempPosY1 = (mt_rand() % 160) + 20;
		tempSize = (mt_rand() % 40) + 10;  /*Size*/

		for (j = 1; j <= tempSize * tempSize * 2; j++)
		{
			tempRadian = mt_rand_lt1() * (M_PI * 2);
			tempPosY2 = tempPosY1 + round(cos(tempRadian) * (mt_rand_lt1() * 0.1f + 0.9f) * tempSize);
			tempPosX2 = tempPosX1 + round(sin(tempRadian) * (mt_rand_lt1() * 0.1f + 0.9f) * tempSize);
			if ((tempPosY2 > 12) && (tempPosY2 < 200)
			 && (tempPosX2 > 0) && (tempPosX2 < 319))
			{
				((Uint8 *)screen->pixels)[tempPosX2 + tempPosY2 * screen->pitch] = pixel;
			}
		}
	}
}

void JE_aliasDirt( SDL_Surface * screen )
{
	/* This complicated looking function goes through the whole screen
	 * looking for brown pixels which just happen to be next to non-brown
	 * pixels.  It's an aliaser, just like it says. */
	unsigned int x, y, newColor;


	/* This is a pointer to a screen.  If you don't like pointer arithmetic,
	 * you won't like this function. */
	Uint8 *s = screen->pixels;
	s += 12 * screen->pitch;

	for (y = 12; y < screen->h; y++)
	{
		for (x = 0; x < screen->pitch; x++)
		{
			if (*s == PIXEL_BLACK)
			{
				newColor = PIXEL_BLACK;
				if (*(s - screen->pitch) == PIXEL_DIRT) /* look up */
				{
					newColor += 1;
				}
				if (y < screen->h - 1)
				{
					if (*(s + screen->pitch) == PIXEL_DIRT) /* look down */
					{
						newColor += 3;
					}
				}
				if (x > 0)
				{
					if (*(s - 1) == PIXEL_DIRT) /* look left */
					{
						newColor += 2;
					}
				}
				if (x < screen->pitch - 1)
				{
					if (*(s + 1) == PIXEL_DIRT) /* look right */
					{
						newColor += 2;
					}
				}
				if (newColor)
				{
					*s = newColor + 16; /* 16 must be the start of the brown pixels. */
				}
			}

			s++;
		}
	}
}

unsigned int JE_placementPosition( unsigned int passed_x, unsigned int width, unsigned int * world )
{
	unsigned int i, new_y;


	/* This is the function responsible for carving out chunks of land.
	 * There's a bug here, but it's a pretty major gameplay altering one:
	 * areas can be carved out for units that are aerial or in mountains.
	 * This can result in huge caverns.  Ergo, it's a feature :) */
	new_y = 0;
	for (i = passed_x; i <= passed_x + width - 1; i++)
	{
		if (new_y < world[i])
		{
			new_y = world[i];
		}
	}

	for (i = passed_x; i <= passed_x + width - 1; i++)
	{
		world[i] = new_y;
	}

	return new_y;
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
			/* This block is what fades out explosions. */
			if (*temps >= PIXEL_RED_MIN)
			{
				if (*temps == PIXEL_RED_MIN)
				{
					*temps = PIXEL_BLACK;
				}
				else
				{
					(*temps)--;
				}
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
			{
				JE_eSound(3);
			}
			else if (temp < 15)
			{
				JE_eSound(4);
			}
			else if (temp < 20)
			{
				JE_eSound(12);
			}
			else if (temp < 40)
			{
				JE_eSound(11);
			}
			else {
				JE_eSound(12);
				JE_eSound(11);
			}

			exploRec[tempW-1].explomax = exploSize[shottype-1];
			exploRec[tempW-1].explofill = exploDensity[shottype-1];
			exploRec[tempW-1].explocolor = shotDirt[shottype-1];
		}
		else
		{
			JE_eSound(4);
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
			player[i].unit[j].lastMove = 0;
			player[i].unit[j].unitYMov = 0;

			if (player[i].is_cpu) /* Todo: Split this off into a 'resetai' function */
			{
				if (systemAngle[player[i].unit[j].unitType-1] || player[i].unit[j].unitType == UNIT_HELI)
				{
					player[i].unit[j].angle = M_PI / 4.0f;
					player[i].unit[j].power = 4;
				} else {
					player[i].unit[j].angle = 0;
					player[i].unit[j].power = 4;
				}
				if (world.mapFlags & MAP_WALLS)
				{
					player[i].unit[j].shotType = defaultCpuWeaponB[player[i].unit[j].unitType-1];
				}
				else
				{
					player[i].unit[j].shotType = defaultCpuWeapon[player[i].unit[j].unitType-1];
				}
			} else {
				/* Players don't get to start with cool angles.*/
				player[i].unit[j].angle = 0;
				player[i].unit[j].power = 3;
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
			player[PLAYER_LEFT].unit[z].power = 6;
		}
		if (player[PLAYER_RIGHT].unit[z].unitType == UNIT_LASER)
		{
			player[PLAYER_RIGHT].unit[z].power = 6;
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

		while (CURUNIT(i).shotType == SHOT_INVALID || CURUNIT(i).health == 0)
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
					if (player[PLAYER_LEFT].unit[z].unitY + player[PLAYER_LEFT].unit[z].unitYMov > 199)
					{
						player[PLAYER_LEFT].unit[z].unitYMov = 0;
						player[PLAYER_LEFT].unit[z].isYInAir = false;
					}
					player[PLAYER_LEFT].unit[z].unitY += player[PLAYER_LEFT].unit[z].unitYMov;
					if (player[PLAYER_LEFT].unit[z].unitY < 26)
					{
						player[PLAYER_LEFT].unit[z].unitYMov = 0;
						player[PLAYER_LEFT].unit[z].unitY = 26;
					}
					if (player[PLAYER_LEFT].unit[z].unitType == UNIT_HELI)
					{
						player[PLAYER_LEFT].unit[z].unitYMov += 0.0001f;
					} else {
						player[PLAYER_LEFT].unit[z].unitYMov += 0.03f;
					}
					if (!JE_stabilityCheck(player[PLAYER_LEFT].unit[z].unitX, round(player[PLAYER_LEFT].unit[z].unitY)))
					{
						player[PLAYER_LEFT].unit[z].unitYMov = 0;
						player[PLAYER_LEFT].unit[z].isYInAir = false;
					}
				} else if (player[PLAYER_LEFT].unit[z].unitY < 199) {
					if (JE_stabilityCheck(player[PLAYER_LEFT].unit[z].unitX, round(player[PLAYER_LEFT].unit[z].unitY)))
						player[PLAYER_LEFT].unit[z].unitY += 1;
				}
				temp = leftGraphicBase[player[PLAYER_LEFT].unit[z].unitType-1] + player[PLAYER_LEFT].unit[z].ani_frame;
				if (player[PLAYER_LEFT].unit[z].unitType == UNIT_HELI)
				{
					if (player[PLAYER_LEFT].unit[z].lastMove < -2)
					{
						temp += 5;
					} else if (player[PLAYER_LEFT].unit[z].lastMove > 2) {
						temp += 10;
					}
				} else {
					temp += floor(player[PLAYER_LEFT].unit[z].angle * 9.99f / M_PI);
				}

				JE_drawShape2(player[PLAYER_LEFT].unit[z].unitX, round(player[PLAYER_LEFT].unit[z].unitY) - 13, temp, eShapes1);
			} else {
				JE_drawShape2(player[PLAYER_LEFT].unit[z].unitX, round(player[PLAYER_LEFT].unit[z].unitY) - 13, leftGraphicBase[player[PLAYER_LEFT].unit[z].unitType-1] + player[PLAYER_LEFT].unit[z].ani_frame, eShapes1);
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
					if (player[PLAYER_RIGHT].unit[z].unitY + player[PLAYER_RIGHT].unit[z].unitYMov > 199)
					{
						player[PLAYER_RIGHT].unit[z].unitYMov = 0;
						player[PLAYER_RIGHT].unit[z].isYInAir = false;
					}
					player[PLAYER_RIGHT].unit[z].unitY += player[PLAYER_RIGHT].unit[z].unitYMov;
					if (player[PLAYER_RIGHT].unit[z].unitY < 24)
					{
						player[PLAYER_RIGHT].unit[z].unitYMov = 0;
						player[PLAYER_RIGHT].unit[z].unitY = 24;
					}
					if (player[PLAYER_RIGHT].unit[z].unitType == UNIT_HELI) /*HELI*/
					{
						player[PLAYER_RIGHT].unit[z].unitYMov += 0.0001f;
					} else {
						player[PLAYER_RIGHT].unit[z].unitYMov += 0.03f;
					}
					if (!JE_stabilityCheck(player[PLAYER_RIGHT].unit[z].unitX, round(player[PLAYER_RIGHT].unit[z].unitY)))
					{
						player[PLAYER_RIGHT].unit[z].unitYMov = 0;
						player[PLAYER_RIGHT].unit[z].isYInAir = false;
					}
				} else if (player[PLAYER_RIGHT].unit[z].unitY < 199)
					if (JE_stabilityCheck(player[PLAYER_RIGHT].unit[z].unitX, round(player[PLAYER_RIGHT].unit[z].unitY)))
					{
						if (player[PLAYER_RIGHT].unit[z].unitType == UNIT_HELI)
						{
							player[PLAYER_RIGHT].unit[z].unitYMov += 0.01f;
						} else {
							player[PLAYER_RIGHT].unit[z].unitY += 1;
						}
					}

				temp = rightGraphicBase[player[PLAYER_RIGHT].unit[z].unitType-1] + player[PLAYER_RIGHT].unit[z].ani_frame;
				if (player[PLAYER_RIGHT].unit[z].unitType == UNIT_HELI)
				{
					if (player[PLAYER_RIGHT].unit[z].lastMove < -2)
					{
						temp += 5;
					} else if (player[PLAYER_RIGHT].unit[z].lastMove > 2) {
						temp += 10;
					}
				} else {
					temp += floor(player[PLAYER_RIGHT].unit[z].angle * 9.99f / M_PI);
				}

				JE_drawShape2(player[PLAYER_RIGHT].unit[z].unitX, round(player[PLAYER_RIGHT].unit[z].unitY) - 13, temp, eShapes1);
			} else {
				JE_drawShape2(player[PLAYER_RIGHT].unit[z].unitX, round(player[PLAYER_RIGHT].unit[z].unitY) - 13, rightGraphicBase[player[PLAYER_RIGHT].unit[z].unitType-1] + player[PLAYER_RIGHT].unit[z].ani_frame, eShapes1);
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

	for (x = 0; x < MAX_WALLS; x++)
	{
		if (world.mapWalls[x].wallExist)
		{
			JE_drawShape2(world.mapWalls[x].wallX, world.mapWalls[x].wallY, 42, eShapes1);
		}
	}
}
void DE_RunTickExplosions( void )
{
	unsigned int i, j, k;
	unsigned int tempPixelIndex;
	int tempPosX, tempPosY;
	double tempRadian;


	/* Run through all open explosions.  They are not sorted in any way */
	for (i = 0; i < MAX_EXPLO; i++)
	{
		if (explosionAvail[i] == true) { continue; } /* Nothing to do */

		for (j = 0; j < exploRec[i].explofill; j++)
		{
			/* An explosion is comprised of multiple 'flares' that fan out.
			   Calculate where this 'flare' will end up */
			tempRadian = mt_rand_lt1() * M_PI * 2;
			tempPosY = exploRec[i].y + round(cos(tempRadian) * mt_rand_lt1() * exploRec[i].explowidth);
			tempPosX = exploRec[i].x + round(sin(tempRadian) * mt_rand_lt1() * exploRec[i].explowidth);

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
					if (player[PLAYER_LEFT].unit[k].health > 0
					 && tempPosX > player[PLAYER_LEFT].unit[k].unitX
					 && tempPosX < player[PLAYER_LEFT].unit[k].unitX + 11
					 && tempPosY > player[PLAYER_LEFT].unit[k].unitY - 11
					 && tempPosY < player[PLAYER_LEFT].unit[k].unitY)
					{
						player[PLAYER_LEFT].unit[k].health--;
						if (player[PLAYER_LEFT].unit[k].health == 0)
						{
							JE_makeExplosion(player[PLAYER_LEFT].unit[k].unitX + 5, round(player[PLAYER_LEFT].unit[k].unitY) - 5, (player[PLAYER_LEFT].unit[k].unitType == UNIT_HELI) * 2);
							if (player[PLAYER_LEFT].unit[k].unitType != UNIT_SATELLITE)
							{
								player[PLAYER_LEFT].unitsRemaining--;
								player[PLAYER_RIGHT].score++;
							}
						}
					}
					if (player[PLAYER_RIGHT].unit[k].health > 0
					 && tempPosX > player[PLAYER_RIGHT].unit[k].unitX
					 && tempPosX < player[PLAYER_RIGHT].unit[k].unitX + 11
					 && tempPosY > player[PLAYER_RIGHT].unit[k].unitY - 11
					 && tempPosY < player[PLAYER_RIGHT].unit[k].unitY)
					{
						player[PLAYER_RIGHT].unit[k].health--;
						if (player[PLAYER_RIGHT].unit[k].health == 0)
						{
							JE_makeExplosion(player[PLAYER_RIGHT].unit[k].unitX + 5, round(player[PLAYER_RIGHT].unit[k].unitY) - 5, (player[PLAYER_RIGHT].unit[k].unitType == UNIT_HELI) * 2);
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


	for (i = 0; i < MAX_SHOTS; i++)
	{
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
					shotRec[i].xmov += mt_rand_lt1() - 0.5f;
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
				if (tempPosX > player[PLAYER_LEFT].unit[j].unitX && tempPosX < player[PLAYER_LEFT].unit[j].unitX + 11 && tempPosY > player[PLAYER_LEFT].unit[j].unitY - 13 && tempPosY < player[PLAYER_LEFT].unit[j].unitY)
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
				if (tempPosX > player[PLAYER_RIGHT].unit[j].unitX && tempPosX < player[PLAYER_RIGHT].unit[j].unitX + 11 && tempPosY > player[PLAYER_RIGHT].unit[j].unitY - 13 && tempPosY < player[PLAYER_RIGHT].unit[j].unitY)
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
		for (j = 0; j < MAX_WALLS; j++)
		{
			if (world.mapWalls[j].wallExist == true
			 && tempPosX >= world.mapWalls[j].wallX && tempPosX <= world.mapWalls[j].wallX + 11
			 && tempPosY >= world.mapWalls[j].wallY && tempPosY <= world.mapWalls[j].wallY + 14)
			{
				if (demolish[shotRec[i].shottype-1])
				{
					/* Blow up the wall and remove the shot. */
					world.mapWalls[j].wallExist = false;
					destructShotAvail[i] = true;
					JE_makeExplosion(tempPosX, tempPosY, shotRec[i].shottype);
					continue;
				} else {
					/* Otherwise, bounce. */
					if (shotRec[i].x - shotRec[i].xmov < world.mapWalls[j].wallX
					 || shotRec[i].x - shotRec[i].xmov > world.mapWalls[j].wallX + 11)
					{
						shotRec[i].xmov = -shotRec[i].xmov;
					}
					if (shotRec[i].y - shotRec[i].ymov < world.mapWalls[j].wallY
					 || shotRec[i].y - shotRec[i].ymov > world.mapWalls[j].wallY + 14)
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
			if (player[player_index].aiMemory.c_Angle > 0 && CURUNIT(player_index).angle > (M_PI / 2.0f) - (M_PI / 9.0f))
			{
				player[player_index].aiMemory.c_Angle = 0;
			}
			if (player[player_index].aiMemory.c_Angle < 0 && CURUNIT(player_index).angle < M_PI / 8.0f)
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
			if (player[player_index].aiMemory.c_Power > 0 && CURUNIT(player_index).power > 4)
			{
				player[player_index].aiMemory.c_Power = 0;
			}
			if (player[player_index].aiMemory.c_Power < 0 && CURUNIT(player_index).power < 3)
			{
				player[player_index].aiMemory.c_Power = 0;
			}
			if (CURUNIT(player_index).power < 2)
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

		if (CURUNIT(player_index).unitType == UNIT_HELI)
		{
			if (CURUNIT(player_index).isYInAir == false)
			{
				player[player_index].aiMemory.c_Power = 1;
			}
			if (mt_rand() % CURUNIT(player_index).unitX > 100)
			{
				player[player_index].aiMemory.c_Power = 1;
			}
			if (mt_rand() % 240 > CURUNIT(player_index).unitX)
			{
				player[player_index].moves.actions[MOVE_RIGHT] = true;
			}
			else if ((mt_rand() % 20) + 300 < CURUNIT(player_index).unitX)
			{
				player[player_index].moves.actions[MOVE_LEFT] = true;
			}
			else if (mt_rand() % 30 == 1)
			{
				player[player_index].aiMemory.c_Angle = (mt_rand() % 3) - 1;
			}
			if (CURUNIT(player_index).unitX > 295 && CURUNIT(player_index).lastMove > 1)
			{
				player[player_index].moves.actions[MOVE_LEFT] = true;
				player[player_index].moves.actions[MOVE_RIGHT] = false;
			}
			if (CURUNIT(player_index).unitType != UNIT_HELI || CURUNIT(player_index).lastMove > 3 || (CURUNIT(player_index).unitX > 160 && CURUNIT(player_index).lastMove > -3))
			{
				if (mt_rand() % (int)round(CURUNIT(player_index).unitY) < 150 && CURUNIT(player_index).unitYMov < 0.01f && (CURUNIT(player_index).unitX < 160 || CURUNIT(player_index).lastMove < 2))
				{
					player[player_index].moves.actions[MOVE_FIRE] = true;
				}
				player[player_index].aiMemory.c_noDown = (5 - abs(CURUNIT(player_index).lastMove)) * (5 - abs(CURUNIT(player_index).lastMove)) + 3;
				player[player_index].aiMemory.c_Power = 1;
			} else
			{
				player[player_index].moves.actions[MOVE_FIRE] = false;
			}

			for (i = 0; i < MAX_INSTALLATIONS; i++)
			{
				if (abs(player[player_target].unit[i].unitX - CURUNIT(player_index).unitX) < 8)
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
						if (CURUNIT(player_index).lastMove < -1)
						{
							CURUNIT(player_index).lastMove++;
						}
						else if (CURUNIT(player_index).lastMove > 1)
						{
							CURUNIT(player_index).lastMove--;
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

		if (mt_rand() % 100 > 98 || CURUNIT(player_index).shotType == SHOT_TRACER)
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

		if (CURUNIT(player_index).unitYMov < -0.1f && CURUNIT(player_index).unitType == UNIT_HELI)
		{
			player[player_index].moves.actions[MOVE_FIRE] = false;
		}

		/* This last hack was down in the processing section.
		 * What exactly it was doing there I do not know */
		if(CURUNIT(player_index).unitType == UNIT_LASER || CURUNIT(player_index).isYInAir == true) {
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
	if (CURUNIT(PLAYER_LEFT).unitType == UNIT_HELI)
	{
		tempPosX = CURUNIT(PLAYER_LEFT).unitX + round(0.1f * CURUNIT(PLAYER_LEFT).lastMove * CURUNIT(PLAYER_LEFT).lastMove * CURUNIT(PLAYER_LEFT).lastMove) + 5;
		tempPosY = round(CURUNIT(PLAYER_LEFT).unitY) + 1;
	} else {
		tempPosX = round(CURUNIT(PLAYER_LEFT).unitX + 6 + cos(CURUNIT(PLAYER_LEFT).angle) * (CURUNIT(PLAYER_LEFT).power * 8 + 7));
		tempPosY = round(CURUNIT(PLAYER_LEFT).unitY - 7 - sin(CURUNIT(PLAYER_LEFT).angle) * (CURUNIT(PLAYER_LEFT).power * 8 + 7));
	}
	JE_pix(tempPosX,     tempPosY,     14);
	JE_pix(tempPosX + 3, tempPosY,      3);
	JE_pix(tempPosX - 3, tempPosY,      3);
	JE_pix(tempPosX,     tempPosY + 2,  3);
	JE_pix(tempPosX,     tempPosY - 2,  3);

	if (CURUNIT(PLAYER_RIGHT).unitType == UNIT_HELI)
	{  /*Heli*/
		tempPosX = CURUNIT(PLAYER_RIGHT).unitX + round(0.1f * CURUNIT(PLAYER_RIGHT).lastMove * CURUNIT(PLAYER_RIGHT).lastMove * CURUNIT(PLAYER_RIGHT).lastMove) + 5;
		tempPosY = round(CURUNIT(PLAYER_RIGHT).unitY) + 1;
	} else {
		tempPosX = round(CURUNIT(PLAYER_RIGHT).unitX + 6 - cos(CURUNIT(PLAYER_RIGHT).angle) * (CURUNIT(PLAYER_RIGHT).power * 8 + 7));
		tempPosY = round(CURUNIT(PLAYER_RIGHT).unitY - 7 - sin(CURUNIT(PLAYER_RIGHT).angle) * (CURUNIT(PLAYER_RIGHT).power * 8 + 7));
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
	JE_drawShape2(  4, 0, 190 + CURUNIT(PLAYER_LEFT).shotType, eShapes1);
	JE_outText( 20, 3, weaponNames[CURUNIT(PLAYER_LEFT).shotType-1], 15, 2);
	sprintf(tempstr, "dmg~%d~", CURUNIT(PLAYER_LEFT).health);
	JE_outText( 75, 3, tempstr, 15, 0);
	sprintf(tempstr, "pts~%d~", player[PLAYER_LEFT].score);
	JE_outText(110, 3, tempstr, 15, 0);

	JE_bar(175, 3, 184, 8, 241);
	JE_rectangle(174, 2, 185, 9, 242);
	JE_rectangle(173, 1, 186, 10, 240);
	JE_bar(188, 3, 310, 8, 241);
	JE_rectangle(187, 2, 312, 9, 242);
	JE_rectangle(186, 1, 313, 10, 240);
	JE_drawShape2(174, 0, 190 + CURUNIT(PLAYER_RIGHT).shotType, eShapes1);
	JE_outText(190, 3, weaponNames[CURUNIT(PLAYER_RIGHT).shotType-1], 15, 2);
	sprintf(tempstr, "dmg~%d~", CURUNIT(PLAYER_RIGHT).health);
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
		if (systemAngle[CURUNIT(player_index).unitType-1])
		{
			/*leftAnglechange*/
			if (player[player_index].moves.actions[MOVE_LEFT] == true)
			{
				CURUNIT(player_index).angle += 0.01f;
				if (CURUNIT(player_index).angle > M_PI / 2)
				{
					CURUNIT(player_index).angle = M_PI / 2 - 0.01f;
				}
			}
			/*rightAnglechange*/
			if (player[player_index].moves.actions[MOVE_RIGHT] == true)
			{
				CURUNIT(player_index).angle -= 0.01f;
				if (CURUNIT(player_index).angle < 0)
				{
					CURUNIT(player_index).angle = 0;
				}
			}
		} else if (CURUNIT(player_index).unitType == UNIT_HELI) {
			if (player[player_index].moves.actions[MOVE_LEFT] == true && CURUNIT(player_index).unitX > 5)
				if (JE_stabilityCheck(CURUNIT(player_index).unitX - 5, round(CURUNIT(player_index).unitX)))
				{
					if (CURUNIT(player_index).lastMove > -5)
					{
						CURUNIT(player_index).lastMove--;
					}
					CURUNIT(player_index).unitX--;
					if (JE_stabilityCheck(CURUNIT(player_index).unitX, round(CURUNIT(player_index).unitY)))
					{
						CURUNIT(player_index).isYInAir = true;
					}
				}
			if (player[player_index].moves.actions[MOVE_RIGHT] == true && CURUNIT(player_index).unitX < 305)
				if (JE_stabilityCheck(CURUNIT(player_index).unitX + 5, round(CURUNIT(player_index).unitY)))
				{
					if (CURUNIT(player_index).lastMove < 5)
					{
						CURUNIT(player_index).lastMove++;
					}
					CURUNIT(player_index).unitX++;
					if (JE_stabilityCheck(CURUNIT(player_index).unitX, round(CURUNIT(player_index).unitY)))
					{
						CURUNIT(player_index).isYInAir = true;
					}
				}
		}

		/*Leftincreasepower*/
		if (player[player_index].moves.actions[MOVE_UP] == true)
		{
			if (CURUNIT(player_index).unitType == UNIT_HELI)
			{
				CURUNIT(player_index).isYInAir = true;
				CURUNIT(player_index).unitYMov -= 0.1f;
			} else if (CURUNIT(player_index).unitType != UNIT_JUMPER
			        || CURUNIT(player_index).isYInAir == true) {
				CURUNIT(player_index).power += 0.05f;
				if (CURUNIT(player_index).power > 5)
				{
					CURUNIT(player_index).power = 5;
				}
			} else {
				CURUNIT(player_index).unitYMov = -3;
				CURUNIT(player_index).isYInAir = true;
			}
		}
		/*Leftdecreasepower*/
		if (player[player_index].moves.actions[MOVE_DOWN] == true)
		{
			if (CURUNIT(player_index).unitType == UNIT_HELI && CURUNIT(player_index).isYInAir == true)
			{
				CURUNIT(player_index).unitYMov += 0.1f;
			} else {
				CURUNIT(player_index).power -= 0.05f;
				if (CURUNIT(player_index).power < 1)
				{
					CURUNIT(player_index).power = 1;
				}
			}
		}
		/*Leftupweapon*/
		if (player[player_index].moves.actions[MOVE_CYUP] == true)
		{
			do
			{
				CURUNIT(player_index).shotType++;
				if (CURUNIT(player_index).shotType > SHOT_LAST)
				{
					CURUNIT(player_index).shotType = SHOT_FIRST;
				}
			} while (weaponSystems[CURUNIT(player_index).unitType-1][CURUNIT(player_index).shotType-1] == 0);
		}
		/*Leftdownweapon*/
		if (player[player_index].moves.actions[MOVE_CYDN] == true)
		{
			do
			{
				CURUNIT(player_index).shotType--;
				if (CURUNIT(player_index).shotType < SHOT_FIRST)
				{
					CURUNIT(player_index).shotType = SHOT_LAST;
				}
			} while (weaponSystems[CURUNIT(player_index).unitType-1][CURUNIT(player_index).shotType-1] == 0);
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
			player[player_index].shotDelay = shotDelay[CURUNIT(player_index).shotType-1];

			if (shotDirt[CURUNIT(player_index).shotType-1] > 20)
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

				if (emptyShot > 0 && (CURUNIT(player_index).unitType != UNIT_HELI
				 || CURUNIT(player_index).isYInAir == true))
				{
					soundQueue[0] = shotSound[CURUNIT(player_index).shotType-1];

					if (CURUNIT(player_index).unitType == UNIT_HELI)
					{
						shotRec[emptyShot-1].x = CURUNIT(player_index).unitX + CURUNIT(player_index).lastMove * 2 + 5;
						shotRec[emptyShot-1].y = CURUNIT(player_index).unitY + 1;
						shotRec[emptyShot-1].ymov = 0.5f + CURUNIT(player_index).unitYMov * 0.1f;
						shotRec[emptyShot-1].xmov = 0.02f * CURUNIT(player_index).lastMove * CURUNIT(player_index).lastMove * CURUNIT(player_index).lastMove;

						if (player[player_index].moves.actions[MOVE_UP] && CURUNIT(player_index).unitY < 30) /* For some odd reason if we're too high we ignore most of our computing. */
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
							shotRec[emptyShot-1].y = CURUNIT(player_index).unitY;
						}
					} else {
						shotRec[emptyShot-1].x = CURUNIT(player_index).unitX + 6 + cos(CURUNIT(player_index).angle) * 10;
						shotRec[emptyShot-1].y = CURUNIT(player_index).unitY - 7 - sin(CURUNIT(player_index).angle) * 10;
						shotRec[emptyShot-1].ymov = -sin(CURUNIT(player_index).angle) * CURUNIT(player_index).power;
						shotRec[emptyShot-1].xmov =  cos(CURUNIT(player_index).angle) * CURUNIT(player_index).power;
					}

					shotRec[emptyShot-1].shottype = CURUNIT(player_index).shotType;

					destructShotAvail[emptyShot-1] = false;

					shotRec[emptyShot-1].shotdur = shotFuse[shotRec[emptyShot-1].shottype-1];

					shotRec[emptyShot-1].trail1c = 0;
					shotRec[emptyShot-1].trail2c = 0;
				}
			} else {
				switch (shotDirt[CURUNIT(player_index).shotType-1])
				{
				case 1: /* magnet */
					for (i = 0; i < MAX_SHOTS; i++)
					{
						if (destructShotAvail[i] == false)
						{
							if (shotRec[i].x > CURUNIT(player_index).unitX)
							{
								shotRec[i].xmov += CURUNIT(player_index).power * 0.1f;
							}
						}
					}
					for (i = 0; i < MAX_INSTALLATIONS; i++) /* magnets push coptors */
					{
						if (player[player_index].unit[i].unitType == UNIT_HELI
						 && player[player_index].unit[i].isYInAir == true
						 && player[player_index].unit[i].unitX + 11 < 318) /* changed to properly align border */
						{
							player[player_index].unit[i].unitX += 2;
						}
					}
					CURUNIT(player_index).ani_frame = 1;
					break;
				}
			}
		}
	}

	player_index = 1;
	if (player[player_index].unitsRemaining > 0)
	{
		if (systemAngle[CURUNIT(player_index).unitType-1])
		{
			/*rightAnglechange*/
			if (player[player_index].moves.actions[MOVE_RIGHT]== true)
			{
				CURUNIT(player_index).angle += 0.01f;
				if (CURUNIT(player_index).angle > M_PI / 2)
				{
					CURUNIT(player_index).angle = M_PI / 2 - 0.01f;
				}
			}
			/*rightAnglechange*/
			if (player[player_index].moves.actions[MOVE_LEFT] == true)
			{
				CURUNIT(player_index).angle -= 0.01f;
				if (CURUNIT(player_index).angle < 0)
				{
					CURUNIT(player_index).angle = 0;
				}
			}
		} else if (CURUNIT(player_index).unitType == UNIT_HELI) { /*Helicopter*/
			if (player[player_index].moves.actions[MOVE_LEFT] == true && CURUNIT(player_index).unitX > 5)
			{
				if (JE_stabilityCheck(CURUNIT(player_index).unitX - 5, round(CURUNIT(player_index).unitY)))
				{
					if (CURUNIT(player_index).lastMove > -5)
					{
						CURUNIT(player_index).lastMove--;
					}
					CURUNIT(player_index).unitX--;
					if (JE_stabilityCheck(CURUNIT(player_index).unitX, round(CURUNIT(player_index).unitY)))
					{
						CURUNIT(player_index).isYInAir = true;
					}
				}
			}
			if (player[player_index].moves.actions[MOVE_RIGHT] == true && CURUNIT(player_index).unitX < 305)
			{
				if (JE_stabilityCheck(CURUNIT(player_index).unitX + 5, round(CURUNIT(player_index).unitY)))
				{
					if (CURUNIT(player_index).lastMove < 5)
					{
						CURUNIT(player_index).lastMove++;
					}
					CURUNIT(player_index).unitX++;
					if (JE_stabilityCheck(CURUNIT(player_index).unitX, round(CURUNIT(player_index).unitY)))
					{
						CURUNIT(player_index).isYInAir = true;
					}
				}
			}
		}

		/*Rightincreasepower*/
		if (player[player_index].moves.actions[MOVE_UP] == true)
		{
			if (CURUNIT(player_index).unitType == UNIT_HELI)
			{
				CURUNIT(player_index).isYInAir = true;
				CURUNIT(player_index).unitYMov -= 0.1f;
			} else if (CURUNIT(player_index).unitType != UNIT_JUMPER
			        || CURUNIT(player_index).isYInAir == true) {
				CURUNIT(player_index).power += 0.05f;
				if (CURUNIT(player_index).power > 5)
				{
					CURUNIT(player_index).power = 5;
				}
			} else {
				CURUNIT(player_index).unitYMov = -3;
				CURUNIT(player_index).isYInAir = true;
			}
		}
		/*Rightdecreasepower*/
		if (player[player_index].moves.actions[MOVE_DOWN] == true)
		{
			if (CURUNIT(player_index).unitType == UNIT_HELI && CURUNIT(player_index).isYInAir == true)
			{
				CURUNIT(player_index).unitYMov += 0.1f;
			} else {
				CURUNIT(player_index).power -= 0.05f;
				if (CURUNIT(player_index).power < 1)
				{
					CURUNIT(player_index).power = 1;
				}
			}
		}
		/*Rightupweapon*/
		if (player[player_index].moves.actions[MOVE_CYUP]== true)
		{
			do
			{
				CURUNIT(player_index).shotType++;
				if (CURUNIT(player_index).shotType > SHOT_LAST)
				{
					CURUNIT(player_index).shotType = SHOT_FIRST;
				}
			} while (weaponSystems[CURUNIT(player_index).unitType-1][CURUNIT(player_index).shotType-1] == 0);
		}
		/*Rightdownweapon*/
		if (player[player_index].moves.actions[MOVE_CYDN] == true)
		{
			do
			{
				CURUNIT(player_index).shotType--;
				if (CURUNIT(player_index).shotType < SHOT_FIRST)
				{
					CURUNIT(player_index).shotType = SHOT_LAST;
				}
			} while (weaponSystems[CURUNIT(player_index).unitType-1][CURUNIT(player_index).shotType-1] == 0);

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
			player[player_index].shotDelay = shotDelay[CURUNIT(player_index).shotType-1];

			emptyShot = 0;
			for (i = 0; i < MAX_SHOTS; i++)
			{
				if (destructShotAvail[i])
				{
					emptyShot = i + 1;
				}
			}

			if (shotDirt[CURUNIT(player_index).shotType-1] > 20)
			{
				if (emptyShot > 0 && (CURUNIT(player_index).unitType != UNIT_HELI
				 || CURUNIT(player_index).isYInAir == true))
				{
					soundQueue[1] = shotSound[CURUNIT(player_index).shotType-1];

					if (CURUNIT(player_index).unitType == UNIT_HELI)
					{
						shotRec[emptyShot-1].x = CURUNIT(player_index).unitX + CURUNIT(player_index).lastMove * 2 + 5;
						shotRec[emptyShot-1].y = CURUNIT(player_index).unitY + 1;
						shotRec[emptyShot-1].ymov = 0.5f + CURUNIT(player_index).unitYMov * 0.1f; /* This wasn't here before.  Oversight? */
						shotRec[emptyShot-1].xmov = 0.02f * CURUNIT(player_index).lastMove * CURUNIT(player_index).lastMove * CURUNIT(player_index).lastMove;
						if (player[player_index].moves.actions[MOVE_UP] == true
						 && CURUNIT(player_index).unitY < 30)
						{
							shotRec[emptyShot-1].ymov = 0.1f;
							if (shotRec[emptyShot-1].xmov < 0)
							{
								shotRec[emptyShot-1].xmov += 0.1f;
							}
							else if (shotRec[emptyShot-1].xmov > 0) {
								shotRec[emptyShot-1].xmov -= 0.1f;
							}
							shotRec[emptyShot-1].y = CURUNIT(player_index).unitY;
						}
					} else {
						shotRec[emptyShot-1].x = CURUNIT(player_index).unitX + 6 - cos(CURUNIT(player_index).angle) * 10;
						shotRec[emptyShot-1].y = CURUNIT(player_index).unitY - 7 - sin(CURUNIT(player_index).angle) * 10;
						shotRec[emptyShot-1].ymov = -sin(CURUNIT(player_index).angle) * CURUNIT(player_index).power;
						shotRec[emptyShot-1].xmov = -cos(CURUNIT(player_index).angle) * CURUNIT(player_index).power;
					}

					if (CURUNIT(player_index).unitType == UNIT_JUMPER)
					{
						shotRec[emptyShot-1].x = CURUNIT(player_index).unitX + 2;
						if (CURUNIT(player_index).isYInAir == true)
						{
							shotRec[emptyShot-1].ymov = 1;
							shotRec[emptyShot-1].y = CURUNIT(player_index).unitY + 2;
						} else {
							shotRec[emptyShot-1].ymov = -2;
							shotRec[emptyShot-1].y = CURUNIT(player_index).unitY - 12;
						}
					}

					shotRec[emptyShot-1].shottype = CURUNIT(player_index).shotType;

					destructShotAvail[emptyShot-1] = false;

					shotRec[emptyShot-1].shotdur = shotFuse[shotRec[emptyShot-1].shottype-1];

					shotRec[emptyShot-1].trail1c = 0;
					shotRec[emptyShot-1].trail2c = 0;
				}
			} else {
				switch (shotDirt[CURUNIT(player_index).shotType-1])
				{
				case 1:
					for (i = 0; i < MAX_SHOTS; i++)
					{
						if (!destructShotAvail[i])
						{
							if (shotRec[i].x < CURUNIT(player_index).unitX)
							{
								shotRec[i].xmov -= CURUNIT(player_index).power * 0.1f;
							}
						}
					}
					for (i = 0; i < MAX_INSTALLATIONS; i++)
					{
						if (player[player_index].unit[i].unitType == UNIT_HELI
						 && player[player_index].unit[i].isYInAir == true
						 && player[player_index].unit[i].unitX > 1)
						{
							player[player_index].unit[i].unitX -= 2;
						}
					}

					CURUNIT(player_index).ani_frame = 1;
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
