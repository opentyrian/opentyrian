/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) The OpenTyrian Development Team
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
#include "config.h"

#include "episodes.h"
#include "file.h"
#include "joystick.h"
#include "loudness.h"
#include "memreader.h"
#include "memwriter.h"
#include "mtrand.h"
#include "nortsong.h"
#include "opentyr.h"
#include "player.h"
#include "varz.h"
#include "vga256d.h"
#include "video.h"
#include "video_scale.h"

#include <stdio.h>

#ifdef _MSC_VER
#include <direct.h>
#define mkdir _mkdir
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#define SAVE_FILES_SIZE (109 * SAVE_FILES_NUM)
#define SAVE_FILE_SIZE (SAVE_FILES_SIZE + 100)

/* Configuration Load/Save handler */

static const JE_byte cryptKey[10] = /* [1..10] */
{
	15, 50, 89, 240, 147, 34, 86, 9, 32, 208
};

const KeySettings defaultKeySettings =
{
	SDL_SCANCODE_UP,
	SDL_SCANCODE_DOWN,
	SDL_SCANCODE_LEFT,
	SDL_SCANCODE_RIGHT,
	SDL_SCANCODE_SPACE,
	SDL_SCANCODE_RETURN,
	SDL_SCANCODE_LCTRL,
	SDL_SCANCODE_LALT,
};

static const char *const keySettingNames[] =
{
	"up",
	"down",
	"left",
	"right",
	"fire",
	"change fire",
	"left sidekick",
	"right sidekick",
};

static const char defaultHighScoreNames[34][23] = /* [1..34] of string [22] */
{/*1P*/
/*TYR*/   "The Prime Chair", /*13*/
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

/*OTHER*/ "Jill", /*5*/
          "Darcy",
          "Jake Stone",
          "Malvineous Havershim",
          "Marta Louise Velasquez",

/*JAZZ*/  "Jazz Jackrabbit", /*3*/
          "Eva Earlong",
          "Devan Shell",

/*OMF*/   "Crystal Devroe", /*11*/
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

/*DARE*/  "Tyler", /*2*/
          "Rennis the Rat Guard"
};

static const char defaultTeamNames[22][25] = /* [1..22] of string [24] */
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

static const JE_EditorItemAvailType initialEditorItemAvail =  // FKA initialItemAvail
{
	1,1,1,0,0,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0, /* Front/Rear Weapons 1-38  */
	0,0,0,0,0,0,0,0,0,0,1,                                                           /* Fill                     */
	1,0,0,0,0,1,0,0,0,1,1,0,1,0,0,0,0,0,                                             /* Sidekicks          51-68 */
	0,0,0,0,0,0,0,0,0,0,0,                                                           /* Fill                     */
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                                   /* Special Weapons    81-93 */
	0,0,0,0,0                                                                        /* Fill                     */
};

/* Last 2 bytes = Word
 *
 * Max Value = 1680
 * X div  60 = Armor  (1-28)
 * X div 168 = Shield (1-12)
 * X div 280 = Engine (1-06)
 */

JE_boolean smoothies[9] = /* [1..9] */
{ 0, 0, 0, 0, 0, 0, 0, 0, 0 };

JE_byte starShowVGASpecialCode;

/* CubeData */
JE_word lastCubeMax, cubeMax;
JE_word cubeList[4]; /* [1..4] */

/* High-Score Stuff */
JE_boolean gameHasRepeated;  // can only get highscore on first play-through

/* Difficulty */
JE_shortint difficultyLevel, oldDifficultyLevel,
            initialDifficulty;  // can only get highscore on initial episode

/* Player Stuff */
uint    power, lastPower, powerAdd;
JE_byte shieldWait, shieldT;

JE_byte          shotRepeat[11], shotMultiPos[11];
JE_boolean       portConfigChange, portConfigDone;

/* Level Data */
char    lastLevelName[11], levelName[11]; /* string [10] */
JE_byte mainLevel, nextLevel, saveLevel;   /*Current Level #*/

/* Keyboard Junk */
KeySettings keySettings;

/* Configuration */
JE_shortint levelFilter, levelFilterNew, levelBrightness, levelBrightnessChg;
JE_boolean  filtrationAvail, filterActive, filterFade, filterFadeStart;

JE_boolean gameJustLoaded;

JE_boolean galagaMode;

JE_boolean extraGame;

JE_boolean twoPlayerMode, twoPlayerLinked, onePlayerAction, superTyrian;
JE_boolean trentWin = false;
JE_byte    superArcadeMode;

JE_byte    superArcadePowerUp;

JE_real linkGunDirec;
JE_byte inputDevice[2] = { 1, 2 }; // 0:any  1:keyboard  2:mouse  3+:joystick

JE_byte secretHint;
JE_byte background3over;
JE_byte background2over;
JE_byte gammaCorrection;
JE_boolean superPause = false;
JE_boolean explosionTransparent,
           youAreCheating,
           displayScore,
           background2, smoothScroll, wild, superWild, starActive,
           topEnemyOver,
           skyEnemyOverAll,
           background2notTransparent;

JE_byte    fastPlay;
JE_boolean pentiumMode;

/* Savegame files */
JE_byte    gameSpeed;
JE_byte    processorType;  /* 1=386 2=486 3=Pentium Hyper */

JE_SaveFilesType saveFiles; /*array[1..saveLevelnum] of savefiletype;*/

JE_EditorItemAvailType editorItemAvail;

JE_word editorLevel;   /*Initial value 800*/

Config opentyrian_config;  // implicitly initialized

// Fields of TYRIAN.CFG that are preserved for compatibility
static Uint8 inputDevice_ = 0;  // FKA inputDevice
static Uint8 jConfigure = 0;  // FKA NortSong.jConfigure
static Uint8 midiPort = 0;  // FKA NortSong.midiPort
static Uint8 soundEffects = 0;  // FKA NortSong.soundEffects
static Uint8 versionNum;   /* SW 1.0 and SW/Reg 1.1 = 0 or 1
                            * EA 1.2 = 2 */
static const Uint8 defaultJoyButtonAssign[4] = { 1, 4, 5, 5 };  // FKA Joystick.defaultJoyButtonAssign
static Uint8 joyButtonAssign[4] = { 0 };  // FKA Joystick.joyButtonAssign
static Uint8 inputDevice1 = 0;
static Uint8 inputDevice2 = 0;
static const Uint8 defaultDosKeySettings[8] = { 72, 80, 75, 77, 57, 28, 29, 56 };  // FKA defaultKeySettings
static Uint8 dosKeySettings[8] = { 0 };  // FKA keySettings

bool load_opentyrian_config(void)
{
	// defaults
	fullscreen_display = -1;
	set_scaler_by_name("Scale2x");
	memcpy(keySettings, defaultKeySettings, sizeof(keySettings));
	
	Config *config = &opentyrian_config;
	
	FILE *file = dir_fopen_warn(get_user_directory(), "opentyrian.cfg", "r");
	if (file == NULL)
		return false;
	
	if (!config_parse(config, file))
	{
		fclose(file);
		
		return false;
	}
	
	ConfigSection *section;
	
	section = config_find_section(config, "video", NULL);
	if (section != NULL)
	{
		config_get_int_option(section, "fullscreen", &fullscreen_display);
		
		const char *scaler;
		if (config_get_string_option(section, "scaler", &scaler))
			set_scaler_by_name(scaler);
		
		const char *scaling_mode;
		if (config_get_string_option(section, "scaling_mode", &scaling_mode))
			set_scaling_mode_by_name(scaling_mode);
	}

	section = config_find_section(config, "keyboard", NULL);
	if (section != NULL)
	{
		for (size_t i = 0; i < COUNTOF(keySettings); ++i)
		{
			const char *keyName;
			if (config_get_string_option(section, keySettingNames[i], &keyName))
			{
				SDL_Scancode scancode = SDL_GetScancodeFromName(keyName);
				if (scancode != SDL_SCANCODE_UNKNOWN)
					keySettings[i] = scancode;
			}
		}
	}

	fclose(file);
	
	return true;
}

bool save_opentyrian_config(void)
{
	Config *config = &opentyrian_config;
	
	ConfigSection *section;
	
	section = config_find_or_add_section(config, "video", NULL);
	if (section == NULL)
		exit(EXIT_FAILURE);  // out of memory
	
	config_set_int_option(section, "fullscreen", fullscreen_display);
	
	config_set_string_option(section, "scaler", scalers[scaler].name);
	
	config_set_string_option(section, "scaling_mode", scaling_mode_names[scaling_mode]);

	section = config_find_or_add_section(config, "keyboard", NULL);
	if (section == NULL)
		exit(EXIT_FAILURE);  // out of memory

	for (size_t i = 0; i < COUNTOF(keySettings); ++i)
	{
		const char *keyName = SDL_GetScancodeName(keySettings[i]);
		if (keyName[0] == '\0')
			keyName = NULL;
		config_set_string_option(section, keySettingNames[i], keyName);
	}

#ifndef TARGET_WIN32
	mkdir(get_user_directory(), 0700);
#else
	mkdir(get_user_directory());
#endif
	
	FILE *file = dir_fopen(get_user_directory(), "opentyrian.cfg", "w");
	if (file == NULL)
		return false;
	
	config_write(config, file);
	
#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
	fsync(fileno(file));
#endif
	fclose(file);
	
	return true;
}

static void playeritems_to_pitems(JE_PItemsType pItems, PlayerItems *items, JE_byte initial_episode_num)
{
	pItems[0]  = items->weapon[FRONT_WEAPON].id;
	pItems[1]  = items->weapon[REAR_WEAPON].id;
	pItems[2]  = items->super_arcade_mode;
	pItems[3]  = items->sidekick[LEFT_SIDEKICK];
	pItems[4]  = items->sidekick[RIGHT_SIDEKICK];
	pItems[5]  = items->generator;
	pItems[6]  = items->sidekick_level;
	pItems[7]  = items->sidekick_series;
	pItems[8]  = initial_episode_num;
	pItems[9]  = items->shield;
	pItems[10] = items->special;
	pItems[11] = items->ship;
}

static void pitems_to_playeritems(PlayerItems *items, JE_PItemsType pItems, JE_byte *initial_episode_num)
{
	items->weapon[FRONT_WEAPON].id  = pItems[0];
	items->weapon[REAR_WEAPON].id   = pItems[1];
	items->super_arcade_mode        = pItems[2];
	items->sidekick[LEFT_SIDEKICK]  = pItems[3];
	items->sidekick[RIGHT_SIDEKICK] = pItems[4];
	items->generator                = pItems[5];
	items->sidekick_level           = pItems[6];
	items->sidekick_series          = pItems[7];
	if (initial_episode_num != NULL)
		*initial_episode_num        = pItems[8];
	items->shield                   = pItems[9];
	items->special                  = pItems[10];
	items->ship                     = pItems[11];
}

void JE_saveGame(JE_byte slot, const char *name)
{
	saveFiles[slot-1].initialDifficulty = initialDifficulty;
	saveFiles[slot-1].gameHasRepeated = gameHasRepeated;
	saveFiles[slot-1].level = saveLevel;
	
	if (superTyrian)
		player[0].items.super_arcade_mode = SA_SUPERTYRIAN;
	else if (superArcadeMode == SA_NONE && onePlayerAction)
		player[0].items.super_arcade_mode = SA_ARCADE;
	else
		player[0].items.super_arcade_mode = superArcadeMode;
	
	playeritems_to_pitems(saveFiles[slot-1].items, &player[0].items, initial_episode_num);
	
	if (twoPlayerMode)
		playeritems_to_pitems(saveFiles[slot-1].lastItems, &player[1].items, 0);
	else
		playeritems_to_pitems(saveFiles[slot-1].lastItems, &player[0].last_items, 0);
	
	saveFiles[slot-1].score  = player[0].cash;
	saveFiles[slot-1].score2 = player[1].cash;
	
	memcpy(&saveFiles[slot-1].levelName, &lastLevelName, sizeof(lastLevelName));
	saveFiles[slot-1].cubes  = lastCubeMax;

	if (strcmp(lastLevelName, "Completed") == 0)
	{
		temp = episodeNum - 1;
		if (temp < 1)
		{
			temp = EPISODE_AVAILABLE; /* JE: {Episodemax is 4 for completion purposes} */
		}
		saveFiles[slot-1].episode = temp;
	}
	else
	{
		saveFiles[slot-1].episode = episodeNum;
	}

	saveFiles[slot-1].difficulty = difficultyLevel;
	saveFiles[slot-1].secretHint = secretHint;
	saveFiles[slot-1].input1 = inputDevice[0];
	saveFiles[slot-1].input2 = inputDevice[1];

	strcpy(saveFiles[slot-1].name, name);
	
	for (uint port = 0; port < 2; ++port)
	{
		// if two-player, use first player's front and second player's rear weapon
		saveFiles[slot-1].power[port] = player[twoPlayerMode ? port : 0].items.weapon[port].power;
	}
	
	saveSaves();
}

void JE_loadGame(JE_byte slot)
{
	superTyrian = false;
	onePlayerAction = false;
	twoPlayerMode = false;
	extraGame = false;
	galagaMode = false;

	initialDifficulty = saveFiles[slot-1].initialDifficulty;
	gameHasRepeated   = saveFiles[slot-1].gameHasRepeated;
	twoPlayerMode     = (slot-1) > 10;
	difficultyLevel   = saveFiles[slot-1].difficulty;
	
	pitems_to_playeritems(&player[0].items, saveFiles[slot-1].items, &initial_episode_num);
	
	superArcadeMode = player[0].items.super_arcade_mode;
	
	if (superArcadeMode == SA_SUPERTYRIAN)
		superTyrian = true;
	if (superArcadeMode != SA_NONE)
		onePlayerAction = true;
	if (superArcadeMode > SA_NORTSHIPZ)
		superArcadeMode = SA_NONE;
	
	if (twoPlayerMode)
	{
		onePlayerAction = false;
		
		pitems_to_playeritems(&player[1].items, saveFiles[slot-1].lastItems, NULL);
	}
	else
	{
		pitems_to_playeritems(&player[0].last_items, saveFiles[slot-1].lastItems, NULL);
	}

	/* Compatibility with old version */
	if (player[1].items.sidekick_level < 101)
	{
		player[1].items.sidekick_level = 101;
		player[1].items.sidekick_series = player[1].items.sidekick[LEFT_SIDEKICK];
	}
	
	player[0].cash = saveFiles[slot-1].score;
	player[1].cash = saveFiles[slot-1].score2;
	
	mainLevel   = saveFiles[slot-1].level;
	cubeMax     = saveFiles[slot-1].cubes;
	lastCubeMax = cubeMax;

	secretHint = saveFiles[slot-1].secretHint;
	inputDevice[0] = saveFiles[slot-1].input1;
	inputDevice[1] = saveFiles[slot-1].input2;

	for (uint port = 0; port < 2; ++port)
	{
		// if two-player, use first player's front and second player's rear weapon
		player[twoPlayerMode ? port : 0].items.weapon[port].power = saveFiles[slot-1].power[port];
	}
	
	int episode = saveFiles[slot-1].episode;

	memcpy(&levelName, &saveFiles[slot-1].levelName, sizeof(levelName));

	if (strcmp(levelName, "Completed") == 0)
	{
		if (episode == EPISODE_AVAILABLE)
			episode = 1;
		else if (episode < EPISODE_AVAILABLE)
			episode++;
		/* Increment episode.  Episode EPISODE_AVAILABLE goes to 1. */
	}

	JE_initEpisode(episode);
	saveLevel = mainLevel;
	memcpy(&lastLevelName, &levelName, sizeof(levelName));
}

void JE_initProcessorType(void)
{
	/* SYN: Originally this proc looked at your hardware specs and chose appropriate options. We don't care, so I'll just set
	   decent defaults here. */

	wild = false;
	superWild = false;
	smoothScroll = true;
	explosionTransparent = true;
	filtrationAvail = false;
	background2 = true;
	displayScore = true;

	switch (processorType)
	{
		case 1: /* 386 */
			background2 = false;
			displayScore = false;
			explosionTransparent = false;
			break;
		case 2: /* 486 - Default */
			break;
		case 3: /* High Detail */
			smoothScroll = false;
			break;
		case 4: /* Pentium */
			wild = true;
			filtrationAvail = true;
			break;
		case 5: /* Nonstandard VGA */
			smoothScroll = false;
			break;
		case 6: /* SuperWild */
			wild = true;
			superWild = true;
			filtrationAvail = true;
			break;
	}

	switch (gameSpeed)
	{
		case 1:  /* Slug Mode */
			fastPlay = 3;
			break;
		case 2:  /* Slower */
			fastPlay = 4;
			break;
		case 3: /* Slow */
			fastPlay = 5;
			break;
		case 4: /* Normal */
			fastPlay = 0;
			break;
		case 5: /* Pentium Hyper */
			fastPlay = 1;
			break;
	}

}

void JE_setNewGameSpeed(void)
{
	pentiumMode = false;

	Uint16 speed;
	switch (fastPlay)
	{
	default:
		assert(false);
		// fall through
	case 0:  // Normal
		speed = 0x4300;
		smoothScroll = true;
		frameCountMax = 2;
		break;
	case 1:  // Pentium Hyper
		speed = 0x3000;
		smoothScroll = true;
		frameCountMax = 2;
		break;
	case 2:
		speed = 0x2000;
		smoothScroll = false;
		frameCountMax = 2;
		break;
	case 3:  // Slug mode
		speed = 0x5300;
		smoothScroll = true;
		frameCountMax = 4;
		break;
	case 4:  // Slower
		speed = 0x4300;
		smoothScroll = true;
		frameCountMax = 3;
		break;
	case 5:  // Slow
		speed = 0x4300;
		smoothScroll = true;
		frameCountMax = 2;
		pentiumMode = true;
		break;
	}

	setFrameSpeed(speed);
	setFrameCount(frameCountMax);
}

const char *get_user_directory(void)
{
	static char user_dir[500] = "";
	
	if (strlen(user_dir) == 0)
	{
#ifndef TARGET_WIN32
		char *xdg_config_home = getenv("XDG_CONFIG_HOME");
		if (xdg_config_home != NULL)
		{
			snprintf(user_dir, sizeof(user_dir), "%s/opentyrian", xdg_config_home);
		}
		else
		{
			char *home = getenv("HOME");
			if (home != NULL)
			{
				snprintf(user_dir, sizeof(user_dir), "%s/.config/opentyrian", home);
			}
			else
			{
				strcpy(user_dir, ".");
			}
		}
#else
		strcpy(user_dir, ".");
#endif
	}
	
	return user_dir;
}

void loadConfiguration(void)
{
	bool invalid = false;

	FILE *f = dir_fopen_warn(get_user_directory(), "tyrian.cfg", "rb");
	if (f == NULL)
	{
		invalid = true;
	}
	else
	{
		Uint8 data[28];
		size_t size = fread(data, 1, sizeof(data), f);

		fseek(f, 0, SEEK_END);
		invalid |= ftell(f) != sizeof(data);

		fclose(f);

		MemReader reader = { data, size, false };

		background2 = memReadBool(&reader);
		gameSpeed = memReadU8(&reader);
		inputDevice_ = memReadU8(&reader);
		jConfigure = memReadU8(&reader);
		versionNum = memReadU8(&reader);
		processorType = memReadU8(&reader);
		midiPort = memReadU8(&reader);
		soundEffects = memReadU8(&reader);
		gammaCorrection = memReadU8(&reader);
		difficultyLevel = memReadS8(&reader);
		memReadU8Array(&reader, joyButtonAssign, COUNTOF(joyButtonAssign));
		tyrMusicVolume = memReadU16LE(&reader);
		fxVolume = memReadU16LE(&reader);
		inputDevice1 = memReadU8(&reader);
		inputDevice2 = memReadU8(&reader);
		memReadU8Array(&reader, dosKeySettings, COUNTOF(dosKeySettings));

		assert(reader.size == 0 || reader.error);
		invalid |= reader.error;

		inputDevice_ = 0;
		if (jConfigure == 0)
			jConfigure = 1;
		// Game resets version number; ShipEdit doesn't.
		versionNum = 2;

		if (tyrMusicVolume > 255)
			tyrMusicVolume = 255;
		if (fxVolume > 255)
			fxVolume = 255;
	}

	if (invalid)
	{
		printf("\nInvalid or missing TYRIAN.CFG! Continuing using defaults.\n\n");
		
		background2 = true;
		gameSpeed = 4;
		inputDevice_ = 0;
		jConfigure = 0;
		versionNum = 2;
		processorType = 3;
		midiPort = 1;
		soundEffects = 1;
		gammaCorrection = 0;
		difficultyLevel = 0;
		memcpy(&joyButtonAssign, &defaultJoyButtonAssign, sizeof(joyButtonAssign));
		tyrMusicVolume = 223;
		fxVolume = 223;
		inputDevice1 = 0;
		inputDevice2 = 0;
		memcpy(&dosKeySettings, &defaultDosKeySettings, sizeof(dosKeySettings));
	}
	
	load_opentyrian_config();

	set_volume(tyrMusicVolume, fxVolume);

	JE_initProcessorType();
}

void saveConfiguration(void)
{
	Uint8 data[28];

	MemWriter writer = { data, sizeof(data), false };

	memWriteBool(&writer, background2);
	memWriteU8(&writer, gameSpeed);
	memWriteU8(&writer, inputDevice_);
	memWriteU8(&writer, jConfigure);
	memWriteU8(&writer, versionNum);
	memWriteU8(&writer, processorType);
	memWriteU8(&writer, midiPort);
	memWriteU8(&writer, soundEffects);
	memWriteU8(&writer, gammaCorrection);
	memWriteS8(&writer, difficultyLevel);
	memWriteU8Array(&writer, joyButtonAssign, COUNTOF(joyButtonAssign));
	memWriteU16LE(&writer, tyrMusicVolume);
	memWriteU16LE(&writer, fxVolume);
	memWriteU8(&writer, inputDevice1);
	memWriteU8(&writer, inputDevice2);
	memWriteU8Array(&writer, dosKeySettings, COUNTOF(dosKeySettings));

	assert(writer.size == 0 && !writer.error);

#ifndef TARGET_WIN32
	mkdir(get_user_directory(), 0700);
#else
	mkdir(get_user_directory());
#endif

	FILE *f = dir_fopen_warn(get_user_directory(), "tyrian.cfg", "wb");
	if (f != NULL)
	{
		if (fwrite(data, 1, sizeof(data), f) != sizeof(data))
			fprintf(stderr, "warning: failed to write to 'tyrian.cfg'\n");

#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
		fsync(fileno(f));
#endif
		fclose(f);
	}
	
	save_opentyrian_config();
}

static bool decryptSaveData(Uint8 *data);

void loadSaves(void)
{
	bool invalid = false;

	FILE *f = dir_fopen_warn(get_user_directory(), "tyrian.sav", "rb");
	if (f == NULL)
	{
		invalid = true;
	}
	else
	{
		Uint8 data[SAVE_FILE_SIZE + 4];
		size_t size = fread(data, 1, sizeof(data), f);

		fclose(f);

		invalid = !decryptSaveData(data);

		MemReader reader = { data, size, false };

		for (size_t i = 0; i < COUNTOF(saveFiles); ++i)
		{
			saveFiles[i].encode = memReadU16LE(&reader);
			saveFiles[i].level = memReadU16LE(&reader);
			memReadU8Array(&reader, saveFiles[i].items, COUNTOF(saveFiles[i].items));
			saveFiles[i].score = memReadU32LE(&reader);
			saveFiles[i].score2 = memReadU32LE(&reader);
			Uint8 levelNameLen = memReadU8(&reader);
			memReadCharArray(&reader, saveFiles[i].levelName, 9);
			saveFiles[i].levelName[MIN(levelNameLen, 9)] = '\0';
			memReadCharArray(&reader, saveFiles[i].name, 14);
			saveFiles[i].name[14] = '\0';
			saveFiles[i].cubes = memReadU8(&reader);
			memReadU8Array(&reader, saveFiles[i].power, COUNTOF(saveFiles[i].power));
			saveFiles[i].episode = memReadU8(&reader);
			memReadU8Array(&reader, saveFiles[i].lastItems, COUNTOF(saveFiles[i].lastItems));
			saveFiles[i].difficulty = memReadU8(&reader);
			saveFiles[i].secretHint = memReadU8(&reader);
			saveFiles[i].input1 = memReadU8(&reader);
			saveFiles[i].input2 = memReadU8(&reader);
			saveFiles[i].gameHasRepeated = memReadBool(&reader);
			saveFiles[i].initialDifficulty = memReadU8(&reader);
			saveFiles[i].highScore1 = memReadS32LE(&reader);
			saveFiles[i].highScore2 = memReadS32LE(&reader);
			Uint8 highScoreNameLen = memReadU8(&reader);
			memReadCharArray(&reader, saveFiles[i].highScoreName, 29);
			saveFiles[i].highScoreName[MIN(highScoreNameLen, 29)] = '\0';
			saveFiles[i].highScoreDiff = memReadU8(&reader);
		}

		memReadU8Array(&reader, editorItemAvail, COUNTOF(editorItemAvail));

		editorLevel = ((Uint16)editorItemAvail[98] << 8) | editorItemAvail[99];

		assert(reader.size == 4 || reader.error);
		invalid |= reader.error;
	}

	if (invalid)
	{
		memset(saveFiles, 0, sizeof(saveFiles));

		for (size_t i = 0; i < SAVE_FILES_NUM; ++i)
		{
			saveFiles[i].level = 0;

			for (size_t j = 0; j < 14; ++j)
				saveFiles[i].name[j] = ' ';
			saveFiles[i].name[14] = '\0';

			saveFiles[i].highScore1 = ((mt_rand() % 20) + 1) * 1000;
			if (i % 6 < 3)
			{
				saveFiles[i].highScore2 = 0;
				strcpy(saveFiles[i].highScoreName, defaultHighScoreNames[mt_rand() % COUNTOF(defaultHighScoreNames)]);
			}
			else
			{
				saveFiles[i].highScore2 = ((mt_rand() % 20) + 1) * 1000;
				strcpy(saveFiles[i].highScoreName, defaultTeamNames[mt_rand() % COUNTOF(defaultTeamNames)]);
			}
			saveFiles[i].highScoreDiff = 0;
		}

		memcpy(editorItemAvail, initialEditorItemAvail, sizeof(editorItemAvail));

		editorLevel = 800;
	}
}

static void encryptSaveData(Uint8 *data);

void saveSaves(void)
{
	Uint8 data[SAVE_FILE_SIZE + 4];

	MemWriter writer = { data, sizeof(data), false };

	for (size_t i = 0; i < COUNTOF(saveFiles); ++i)
	{
		memWriteU16LE(&writer, saveFiles[i].encode);
		memWriteU16LE(&writer, saveFiles[i].level);
		memWriteU8Array(&writer, saveFiles[i].items, COUNTOF(saveFiles[i].items));
		memWriteU32LE(&writer, saveFiles[i].score);
		memWriteU32LE(&writer, saveFiles[i].score2);
		memWriteU8(&writer, strlen(saveFiles[i].levelName));
		memWriteCharArray(&writer, saveFiles[i].levelName, 9);
		memWriteCharArray(&writer, saveFiles[i].name, 14);
		memWriteU8(&writer, saveFiles[i].cubes);
		memWriteU8Array(&writer, saveFiles[i].power, COUNTOF(saveFiles[i].power));
		memWriteU8(&writer, saveFiles[i].episode);
		memWriteU8Array(&writer, saveFiles[i].lastItems, COUNTOF(saveFiles[i].lastItems));
		memWriteU8(&writer, saveFiles[i].difficulty);
		memWriteU8(&writer, saveFiles[i].secretHint);
		memWriteU8(&writer, saveFiles[i].input1);
		memWriteU8(&writer, saveFiles[i].input2);
		memWriteBool(&writer, saveFiles[i].gameHasRepeated);
		memWriteU8(&writer, saveFiles[i].initialDifficulty);
		memWriteS32LE(&writer, saveFiles[i].highScore1);
		memWriteS32LE(&writer, saveFiles[i].highScore2);
		memWriteU8(&writer, strlen(saveFiles[i].highScoreName));
		memWriteCharArray(&writer, saveFiles[i].highScoreName, 29);
		memWriteU8(&writer, saveFiles[i].highScoreDiff);
	}

	editorItemAvail[98] = editorLevel >> 8;
	editorItemAvail[99] = editorLevel;

	memWriteU8Array(&writer, editorItemAvail, COUNTOF(editorItemAvail));

	assert(writer.size == 4 && !writer.error);

	encryptSaveData(data);

#ifndef TARGET_WIN32
	mkdir(get_user_directory(), 0700);
#else
	mkdir(get_user_directory());
#endif

	FILE *f = dir_fopen_warn(get_user_directory(), "tyrian.sav", "wb");
	if (f != NULL)
	{
		if (fwrite(data, 1, sizeof(data), f) != sizeof(data))
			fprintf(stderr, "warning: failed to write to 'tyrian.sav'\n");

#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
		fsync(fileno(f));
#endif
		fclose(f);
	}
}

void encryptSaveData(Uint8 *data)
{
	Uint8 y;

	y = 0;
	for (size_t i = 0; i < SAVE_FILE_SIZE; ++i)
		y += data[i];
	data[SAVE_FILE_SIZE] = y;

	y = 0;
	for (size_t i = 0; i < SAVE_FILE_SIZE; ++i)
		y -= data[i];
	data[SAVE_FILE_SIZE + 1] = y;

	y = 1;
	for (size_t i = 0; i < SAVE_FILE_SIZE; ++i)
		y = (y * data[i]) + 1;
	data[SAVE_FILE_SIZE + 2] = y;

	y = 0;
	for (size_t i = 0; i < SAVE_FILE_SIZE; ++i)
		y ^= data[i];
	data[SAVE_FILE_SIZE + 3] = y;

	for (size_t i = 0; i < SAVE_FILE_SIZE; ++i)
	{
		data[i] ^= cryptKey[(i + 1) % 10];
		if (i > 0)
			data[i] ^= data[i - 1];
	}
}

bool decryptSaveData(Uint8 *data)
{
	for (size_t i = SAVE_FILE_SIZE - 1; ; --i)
	{
		data[i] ^= cryptKey[(i + 1) % 10];
		if (i > 0)
			data[i] ^= data[i - 1];
		else
			break;
	}

	Uint8 y;

	y = 0;
	for (size_t i = 0; i < SAVE_FILE_SIZE; ++i)
		y += data[i];
	if (data[SAVE_FILE_SIZE] != y)
		return false;

	y = 0;
	for (size_t i = 0; i < SAVE_FILE_SIZE; ++i)
		y -= data[i];
	if (data[SAVE_FILE_SIZE + 1] != y)
		return false;

	y = 1;
	for (size_t i = 0; i < SAVE_FILE_SIZE; ++i)
		y = (y * data[i]) + 1;
	if (data[SAVE_FILE_SIZE + 2] != y)
		return false;

	y = 0;
	for (size_t i = 0; i < SAVE_FILE_SIZE; ++i)
		y ^= data[i];
	if (data[SAVE_FILE_SIZE + 3] != y)
		return false;

	return true;
}
