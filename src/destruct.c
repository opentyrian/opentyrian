/* vim: set noet:
 *
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Development Team
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
#include "destruct.h"

#include "config.h"
#include "fonthand.h"
#include "helptext.h"
#include "keyboard.h"
#include "loudness.h"
#include "newshape.h"
#include "nortsong.h"
#include "nortvars.h"
#include "pcxload.h"
#include "picload.h"
#include "picload2.h"
#include "starfade.h"
#include "varz.h"
#include "vga256d.h"


SDL_Surface *destructTempScreen;
JE_byte enddelay;
JE_boolean died = false;
JE_boolean firsttime;

#define SHOT_MAX 40
#define EXPLO_MAX 40

#define SHOT_TYPES 17
#define SYSTEM_TYPES 8

#define MAX_INSTALLATIONS 20

const JE_boolean demolish[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {false, false, false, false, false, true, true, true, false, false, false, false, true, false, true, false, true};
const JE_byte shotgr[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 101};
const JE_byte shottrail[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {0, 0, 0, 1, 1, 1, 2, 2, 0, 0, 0, 1, 2, 1, 2, 1, 0};
const JE_byte shotfuse[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};
const JE_byte shotdelay[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {10, 30, 80, 20, 60, 100, 140, 200, 20, 60, 5, 15, 50, 5, 80, 16, 0};
const JE_byte shotsound[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {8, 2, 1, 7, 7, 9, 22, 22, 5, 13, 10, 15, 15, 26, 14, 7, 7};
const JE_byte explosize[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {4, 20, 30, 14, 22, 16, 40, 60, 10, 30, 0, 5, 10, 3, 15, 7, 0};
const JE_boolean shotbounce[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, false, true};
const JE_byte explodensity[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {  2,  5, 10, 15, 20, 15, 25, 30, 40, 80, 0, 30, 30,  4, 30, 5, 0};
const JE_byte shotdirt[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {252, 252, 252, 252, 252, 252, 252, 252, 25, 25, 1, 252, 252, 252, 252, 252, 0};
const JE_byte shotcolor[SHOT_TYPES] /*[1..SHOT_TYPES]*/ = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 10, 10, 10, 10, 16, 0};

const JE_byte defaultweapon[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {2, 4, 9, 0, 11, 12, 4, 16};
const JE_byte defaultcpuweapon[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {2, 4, 6, 0, 11, 12, 4, 16};
const JE_byte defaultcpuweaponB[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {6, 7, 6, 0, 11, 13, 4, 16};
const JE_boolean systemangle[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {true, true, true, false, false, true, false, false};
const JE_word basedamage[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {200, 120, 400, 300, 80, 150, 600, 40};
const JE_boolean systemani[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {false, false, false, true, false, false, false, true};

const JE_byte weaponsystems[SYSTEM_TYPES][SHOT_TYPES] /*[1..SYSTEM_TYPES, 1..SHOT_TYPES]*/ =
{
	{1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
	{1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
	{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}
};

const JE_byte leftgraphicbase[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {1, 6, 11, 58, 63, 68, 96, 153};
const JE_byte rightgraphicbase[SYSTEM_TYPES] /*[1..SYSTEM_TYPES]*/ = {20, 25, 30, 77, 82, 87, 115, 172};

const JE_byte Lmodescore[DESTRUCT_MODES] /*[1..destructmodes]*/ = {1, 0, 0, 5, 0};
const JE_byte Rmodescore[DESTRUCT_MODES] /*[1..destructmodes]*/ = {1, 0, 5, 0, 1};

JE_byte cpu;
JE_byte mode;  /*Game Mode - See Tyrian.HTX*/

JE_byte arenatype;
JE_boolean havewalls;

JE_integer Lc_Angle, Lc_Power, Lc_Fire;

JE_boolean L_Left, L_Right, L_Up, L_Down, L_Change, L_Fire, L_Weapon;
JE_byte NoL_Down;

JE_word leftscore, rightscore;
JE_byte leftavail, rightavail;

JE_byte leftani[MAX_INSTALLATIONS], rightani[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/

JE_byte leftsel, rightsel;

JE_byte leftshotdelay, rightshotdelay;

JE_shortint leftlastmove[MAX_INSTALLATIONS], rightlastmove[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/

JE_byte leftshottype[MAX_INSTALLATIONS], leftsystem[MAX_INSTALLATIONS],
        rightshottype[MAX_INSTALLATIONS], rightsystem[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/

JE_word leftdmg[MAX_INSTALLATIONS], rightdmg[MAX_INSTALLATIONS],
        leftx[MAX_INSTALLATIONS], rightx[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/

JE_real leftangle[MAX_INSTALLATIONS], leftpower[MAX_INSTALLATIONS], leftymov[MAX_INSTALLATIONS], lefty[MAX_INSTALLATIONS],
        rightangle[MAX_INSTALLATIONS], rightpower[MAX_INSTALLATIONS], rightymov[MAX_INSTALLATIONS], righty[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/

JE_boolean leftyinair[MAX_INSTALLATIONS],
           rightyinair[MAX_INSTALLATIONS]; /*[1..maxinstallations]*/

JE_boolean wallexist[20]; /*[1..20]*/
JE_byte wallsx[20], wallsy[20]; /*[1..20]*/

JE_byte dirtheight[320]; /*[0..319]*/

JE_boolean shotavail[SHOT_MAX]; /*[1..shotmax]*/
struct {
	JE_real x;
	JE_real y;
	JE_real xmov;
	JE_real ymov;
	JE_boolean gravity;
	JE_byte shottype;
	JE_longint shotdur;
	JE_word trail4x, trail4y, trail4c,
	        trail3x, trail3y, trail3c,
	        trail2x, trail2y, trail2c,
	        trail1x, trail1y, trail1c;
} shotrec[SHOT_MAX]; /*[1..shotmax]*/

JE_boolean endofgame, quit;


JE_boolean found;

JE_integer tempx;
JE_integer tempy;
JE_real tempr, tempr2;

JE_byte explosoundchannel;
JE_boolean explosionavail[EXPLO_MAX]; /*[1..explomax]*/
struct {
	JE_word x;
	JE_byte y;
	JE_byte explowidth;
	JE_byte explomax;
	JE_byte explofill;
	JE_byte explocolor;
} explorec[EXPLO_MAX]; /*[1..explomax]*/


void JE_destructGame( void )
{
	firsttime = true;
	JE_clr256();
	JE_showVGA();
	endofgame = false;
	destructTempScreen = game_screen;
	JE_loadCompShapes(&eShapes1, &eShapes1Size, '~');
	JE_fadeBlack(1);
	
	/* TODO callBIOShandler = false; */
	JE_destructMain();
	/* TODO callBIOShandler = false; */
}

void JE_superPixel( JE_word loc )
{
	Uint8 *s = destructTempScreen->pixels;
	int loc_max = destructTempScreen->w * destructTempScreen->h;
	
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
	
	if (loc - destructTempScreen->w > 0 && loc - destructTempScreen->w < loc_max)
	{
		/* up 1 */
		if (s[loc - destructTempScreen->w] < 249)
			s[loc - destructTempScreen->w] = 249;
		else if (s[loc - destructTempScreen->w] < 255 - 2)
			s[loc - destructTempScreen->w] += 2;
		else
			s[loc - destructTempScreen->w] = 255;
	}
	
	if (loc - destructTempScreen->w - 1 > 0 && loc - destructTempScreen->w - 1 < loc_max)
	{
		/* up 1, left 1 */
		if (s[loc - destructTempScreen->w - 1] < 247)
			s[loc - destructTempScreen->w - 1] = 247;
		else if (s[loc - destructTempScreen->w - 1] < 255 - 1)
			s[loc - destructTempScreen->w - 1] += 1;
		else
			s[loc - destructTempScreen->w - 1] = 255;
	}
	
	if (loc - destructTempScreen->w + 1 > 0 && loc - destructTempScreen->w + 1 < loc_max)
	{
		/* up 1, right 1 */
		if (s[loc - destructTempScreen->w + 1] < 249)
			s[loc - destructTempScreen->w + 1] = 249;
		else if (s[loc - destructTempScreen->w + 1] < 255 - 2)
			s[loc - destructTempScreen->w + 1] += 2;
		else
			s[loc - destructTempScreen->w + 1] = 255;
	}
	
	if (loc - destructTempScreen->w * 2 > 0 && loc - destructTempScreen->w * 2 < loc_max)
	{
		/* up 2 */
		if (s[loc - destructTempScreen->w * 2] < 246)
			s[loc - destructTempScreen->w * 2] = 246;
		else if (s[loc - destructTempScreen->w * 2] < 255 - 1)
			s[loc - destructTempScreen->w * 2] += 1;
		else
			s[loc - destructTempScreen->w * 2] = 255;
	}
	
	if (loc + destructTempScreen->w > 0 && loc + destructTempScreen->w < loc_max)
	{
		/* down 1 */
		if (s[loc + destructTempScreen->w] < 249)
			s[loc + destructTempScreen->w] = 249;
		else if (s[loc + destructTempScreen->w] < 255 - 2)
			s[loc + destructTempScreen->w] += 2;
		else
			s[loc + destructTempScreen->w] = 255;
	}
	
	if (loc + destructTempScreen->w - 1 > 0 && loc + destructTempScreen->w - 1 < loc_max)
	{
		/* down 1, left 1 */
		if (s[loc + destructTempScreen->w - 1] < 247)
			s[loc + destructTempScreen->w - 1] = 247;
		else if (s[loc + destructTempScreen->w - 1] < 255 - 1)
			s[loc + destructTempScreen->w - 1] += 1;
		else
			s[loc + destructTempScreen->w - 1] = 255;
	}
	
	if (loc + destructTempScreen->w + 1 > 0 && loc + destructTempScreen->w + 1 < loc_max)
	{
		/* down 1, right 1 */
		if (s[loc + destructTempScreen->w + 1] < 247)
			s[loc + destructTempScreen->w + 1] = 247;
		else if (s[loc + destructTempScreen->w + 1] < 255 - 1)
			s[loc + destructTempScreen->w + 1] += 1;
		else
			s[loc + destructTempScreen->w + 1] = 255;
	}
	
	if (loc + destructTempScreen->w * 2 > 0 && loc + destructTempScreen->w * 2 < loc_max)
	{
		/* down 2 */
		if (s[loc + destructTempScreen->w * 2] < 246)
			s[loc + destructTempScreen->w * 2] = 246;
		else if (s[loc + destructTempScreen->w * 2] < 255 - 1)
			s[loc + destructTempScreen->w * 2] += 1;
		else
			s[loc + destructTempScreen->w * 2] = 255;
	}
}

void JE_pauseScreen( void )
{
	JE_setVol(tyrMusicVolume / 2 + 10, fxVolume);
	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->h * VGAScreen2->pitch);
	JE_outText(JE_fontCenter(miscText[23-1], TINY_FONT), 90, miscText[23-1], 12, 5);
	JE_showVGA();
	
	do {
		service_SDL_events(true);
		SDL_Delay(16);
	} while (!newkey);
	
	memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->h * VGAScreen->pitch);
	JE_showVGA();
	JE_setVol(tyrMusicVolume, fxVolume);
}

void JE_eSound( JE_byte sound )
{
	if (explosoundchannel < 1)
		explosoundchannel = 1;
	explosoundchannel++;
	if (explosoundchannel > 5)
		explosoundchannel = 1;
	
	soundQueue[explosoundchannel] = sound;
}

void JE_helpScreen( void )
{
	/* TODO getvga;*/
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

JE_boolean JE_stabilityCheck( JE_integer x, JE_integer y )
{
	JE_word z, tempw, tempw2;
	
	tempw2 = 0;
	tempw = x + y * destructTempScreen->w - 2;
	for (z = 1; z <= 12; z++)
		if (((Uint8 *)destructTempScreen->pixels)[tempw + z] == 25)
			tempw2++;
	
	return (tempw2 < 10);
}

JE_byte JE_placementPosition( JE_word x, JE_byte width )
{
	JE_word y, z;
	
	y = 0;
	for (z = x; z <= x + width - 1; z++)
		if (y < dirtheight[z])
			y = dirtheight[z];
	
	for (z = x; z <= x + width - 1; z++)
		dirtheight[z] = y;
	
	return y;
}

void JE_destructMain( void )
{
	JE_byte temp, temp2;
	char tempstr[256];
	
	JE_loadPic(11, false);
	JE_introScreen();
	
	cpu = 1;
	leftscore = 0;
	rightscore = 0;
	
	do {
		JE_modeSelect();
		
		if (!quit)
		{
			do {
				quit = false;
				endofgame = false;
				
				firsttime = true;
				JE_loadPic(11, false);
				
				havewalls = rand() % 2;
				
				JE_generateTerrain();
				
				leftsel = 1;
				rightsel = 1;
				
				Lc_Angle = 2;
				Lc_Power = 2;
				Lc_Fire = 0;
				NoL_Down = 0;
				
				for (z = 0; z < MAX_INSTALLATIONS; z++)
				{
					leftlastmove[z] = 0;
					rightlastmove[z] = 0;
					leftymov[z] = 0;
					rightymov[z] = 0;
					leftani[z] = 0;
					rightani[z] = 0;
					if (cpu > 0)
					{
						if (systemangle[leftsystem[z]-1] || leftsystem[z] == 8)
						{
							leftangle[z] = M_PI / 4.0f;
							leftpower[z] = 4;
						} else {
							leftangle[z] = 0;
							leftpower[z] = 4;
						}
						if (havewalls)
							leftshottype[z] = defaultcpuweaponB[leftsystem[z]-1];
						else
							leftshottype[z] = defaultcpuweapon[leftsystem[z]-1];
					} else {
						leftangle[z] = 0;
						leftpower[z] = 3;
						leftshottype[z] = defaultweapon[leftsystem[z]-1];
					}
					rightangle[z] = 0;
					rightpower[z] = 3;
					rightshottype[z] = defaultweapon[rightsystem[z]-1];
				}
				
				for (x = 0; x < SHOT_MAX; x++)
					shotavail[x] = true;
				for (x = 0; x < EXPLO_MAX; x++)
					explosionavail[x] = true;
				
				do {
					setjasondelay(1);
					
					memset(soundQueue, 0, sizeof(soundQueue));
					
					JE_tempScreenChecking();
					
					if (!died)
					{
						/*LeftSkipNoShooters*/
						while (leftshottype[leftsel-1] == 0 || leftdmg[leftsel-1] == 0)
						{
							leftsel++;
							if (leftsel > MAX_INSTALLATIONS)
								leftsel = 1;
						}
						
						/*RightSkipNoShooters*/
						while (rightshottype[rightsel-1] == 0 || rightdmg[rightsel-1] == 0)
						{
							rightsel++;
							if (rightsel > MAX_INSTALLATIONS)
								rightsel = 1;
						}
					}
					
					for (z = 0; z < MAX_INSTALLATIONS; z++)
						if (leftdmg[z] > 0)
						{
							if (leftsystem[z] != 4)
							{
								if (leftyinair[z])
								{
									if (lefty[z] + leftymov[z] > 199)
									{
										leftymov[z] = 0;
										leftyinair[z] = false;
									}
									lefty[z] += leftymov[z];
									if (lefty[z] < 26)
									{
										leftymov[z] = 0;
										lefty[z] = 26;
									}
									if (leftsystem[z] == 8) /*HELI*/
									{
										leftymov[z] += 0.0001f;
									} else {
										leftymov[z] += 0.03f;
									}
									if (!JE_stabilityCheck(leftx[z], round(lefty[z])))
									{
										leftymov[z] = 0;
										leftyinair[z] = false;
									}
								} else if (lefty[z] < 199) {
									if (JE_stabilityCheck(leftx[z], round(lefty[z])))
										lefty[z] += 1;
								}
								temp = leftgraphicbase[leftsystem[z]-1] + leftani[z];
								if (leftsystem[z] == 8)
								{
									if (leftlastmove[z] < -2)
									{
										temp += 5;
									} else if (leftlastmove[z] > 2) {
										temp += 10;
									}
								} else {
									temp += floor(leftangle[z] * 9.99f / M_PI);
								}
								
								JE_drawShape2(leftx[z], round(lefty[z]) - 13, temp, eShapes1);
							} else {
								JE_drawShape2(leftx[z], round(lefty[z]) - 13, leftgraphicbase[leftsystem[z]-1] + leftani[z], eShapes1);
							}
						}
					for (z = 0; z < MAX_INSTALLATIONS; z++)
						if (rightdmg[z] > 0)
						{
							if (rightsystem[z] != 4)
							{
								if (rightyinair[z])
								{
									if (righty[z] + rightymov[z] > 199)
									{
										rightymov[z] = 0;
										rightyinair[z] = false;
									}
									righty[z] += rightymov[z];
									if (righty[z] < 24)
									{
										rightymov[z] = 0;
										righty[z] = 24;
									}
									if (rightsystem[z] == 8) /*HELI*/
									{
										rightymov[z] += 0.0001f;
									} else {
										rightymov[z] += 0.03f;
									}
									if (!JE_stabilityCheck(rightx[z], round(righty[z])))
									{
										rightymov[z] = 0;
										rightyinair[z] = false;
									}
								} else if (righty[z] < 199)
									if (JE_stabilityCheck(rightx[z], round(righty[z])))
									{
										if (rightsystem[z] == 8)
										{
											rightymov[z] += 0.01f;
										} else {
											righty[z] += 1;
										}
									}
								
								temp = rightgraphicbase[rightsystem[z]-1] + rightani[z];
								if (rightsystem[z] == 8)
								{
									if (rightlastmove[z] < -2)
									{
										temp += 5;
									} else if (rightlastmove[z] > 2) {
										temp += 10;
									}
								} else {
									temp += floor(rightangle[z] * 9.99f / M_PI);
								}
								
								JE_drawShape2(rightx[z], round(righty[z]) - 13, temp, eShapes1);
							} else {
								JE_drawShape2(rightx[z], round(righty[z]) - 13, rightgraphicbase[rightsystem[z]-1] + rightani[z], eShapes1);
							}
						}
					
					for (z = 0; z < MAX_INSTALLATIONS; z++)
					{
						if (leftsystem[z] == 6)
							leftpower[z] = 6;
						if (rightsystem[z] == 6)
							rightpower[z] = 6;
						
						if (leftani[z] == 0)
						{
							if (systemani[leftsystem[z]-1])
								leftani[z]++;
						} else {
							leftani[z]++;
							if (leftani[z] >= 4)
								leftani[z] = 0;
						}
						if (rightani[z] == 0)
						{
							if (systemani[rightsystem[z]-1])
								rightani[z]++;
						} else {
							rightani[z]++;
							if (rightani[z] >= 4)
								rightani[z] = 0;
						}
					}
					
					for (x = 0; x < 20; x++)
						if (wallexist[x])
							JE_drawShape2(wallsx[x], wallsy[x], 42, eShapes1);
					
				/*Explosions*/
				for (z = 0; z < EXPLO_MAX; z++)
					if (!explosionavail[z])
					{
						
						for (temp = 0; temp < explorec[z].explofill; temp++)
						{
							tempr = ((float)rand() / RAND_MAX) * (M_PI * 2);
							tempy = explorec[z].y + round(cos(tempr) * ((float)rand() / RAND_MAX) * explorec[z].explowidth);
							tempx = explorec[z].x + round(sin(tempr) * ((float)rand() / RAND_MAX) * explorec[z].explowidth);
							
							if (tempy < 200 && tempy > 15)
							{
								tempW = tempx + tempy * destructTempScreen->w;
								if (explorec[z].explocolor == 252)
									JE_superPixel(tempW);
								else
									((Uint8 *)destructTempScreen->pixels)[tempW] = explorec[z].explocolor;
							}
							
							if (explorec[z].explocolor == 252)
								for (temp2 = 0; temp2 < MAX_INSTALLATIONS; temp2++)
								{
									if (leftdmg[temp2] > 0 && tempx > leftx[temp2] && tempx < leftx[temp2] + 11 && tempy > lefty[temp2] - 11 && tempy < lefty[temp2])
									{
										leftdmg[temp2]--;
										if (leftdmg[temp2] == 0)
										{
											JE_makeExplosion(leftx[temp2] + 5, round(lefty[temp2]) - 5, (leftsystem[temp2] == 8) * 2);
											if (leftsystem[temp2] != 4)
											{
												leftavail--;
												rightscore++;
											}
										}
									}
									if (rightdmg[temp2] > 0 && tempx > rightx[temp2] && tempx < rightx[temp2] + 11 && tempy > righty[temp2] - 11 && tempy < righty[temp2])
									{
										rightdmg[temp2]--;
										if (rightdmg[temp2] == 0)
										{
											JE_makeExplosion(rightx[temp2] + 5, round(righty[temp2]) - 5, (rightsystem[temp2] == 8) * 2);
											if (rightsystem[temp2] != 4)
											{
												rightavail--;
												leftscore++;
											}
										}
									}
								}
						}
						
						explorec[z].explowidth++;
						if (explorec[z].explowidth == explorec[z].explomax)
							explosionavail[z] = true;
					}
					
				/*Shotdraw*/
				for (z = 0; z < SHOT_MAX; z++)
					if (!shotavail[z])
					{
						shotrec[z].x += shotrec[z].xmov;
						shotrec[z].y += shotrec[z].ymov;
						
						if (shotbounce[shotrec[z].shottype-1])
						{
							if (shotrec[z].y > 199 || shotrec[z].y < 14)
							{
								shotrec[z].y -= shotrec[z].ymov;
								shotrec[z].ymov = -shotrec[z].ymov;
							}
							if (shotrec[z].x < 1 || shotrec[z].x > 318)
							{
								shotrec[z].x -= shotrec[z].xmov;
								shotrec[z].xmov = -shotrec[z].xmov;
							}
						} else {
							shotrec[z].ymov += 0.05f;
							if (shotrec[z].y > 199)
							{
								shotrec[z].y -= shotrec[z].ymov;
								shotrec[z].ymov = -shotrec[z].ymov * 0.8f;
								if (shotrec[z].xmov == 0)
									shotrec[z].xmov += ((float)rand() / RAND_MAX) - 0.5f;
							}
						}
						
						if (shotrec[z].x > 318 || shotrec[z].x < 1)
						{
							shotavail[z] = true;
						} else {
							
							tempx = round(shotrec[z].x);
							tempy = round(shotrec[z].y);
							
							if (shotrec[z].y > 14)
							{
								
								/*Check left hits*/
								for (temp2 = 0; temp2 < MAX_INSTALLATIONS; temp2++)
									if (leftdmg[temp2] > 0)
									{
										if (tempx > leftx[temp2] && tempx < leftx[temp2] + 11 && tempy > lefty[temp2] - 13 && tempy < lefty[temp2])
										{
											shotavail[z] = true;
											JE_makeExplosion(tempx, tempy, shotrec[z].shottype);
										}
									}
								
								/*Check right hits*/
								for (temp2 = 0; temp2 < MAX_INSTALLATIONS; temp2++)
									if (rightdmg[temp2] > 0)
									{
										if (tempx > rightx[temp2] && tempx < rightx[temp2] + 11 && tempy > righty[temp2] - 13 && tempy < righty[temp2])
										{
											shotavail[z] = true;
											JE_makeExplosion(tempx, tempy, shotrec[z].shottype);
										}
									}
								
								
								tempW = (shotcolor[shotrec[z].shottype-1] << 4) - 3;
								JE_pixCool(tempx, tempy, tempW);
								
								/*Shottrail*/
								switch (shottrail[shotrec[z].shottype-1])
								{
									case 1:
										if (shotrec[z].trail2c > 0 && shotrec[z].trail2y > 12)
											JE_pixCool(shotrec[z].trail2x, shotrec[z].trail2y, shotrec[z].trail2c);
										shotrec[z].trail2x = shotrec[z].trail1x;
										shotrec[z].trail2y = shotrec[z].trail1y;
										if (shotrec[z].trail1c > 0)
											shotrec[z].trail2c = shotrec[z].trail1c - 4;
										
										if (shotrec[z].trail1c > 0 && shotrec[z].trail1y > 12)
											JE_pixCool(shotrec[z].trail1x, shotrec[z].trail1y, shotrec[z].trail1c);
										shotrec[z].trail1x = tempx;
										shotrec[z].trail1y = tempy;
										shotrec[z].trail1c = tempW - 3;
										break;
									case 2:
										if (shotrec[z].trail4c > 0 && shotrec[z].trail4y > 12)
											JE_pixCool(shotrec[z].trail4x, shotrec[z].trail4y, shotrec[z].trail4c);
										shotrec[z].trail4x = shotrec[z].trail3x;
										shotrec[z].trail4y = shotrec[z].trail3y;
										if (shotrec[z].trail3c > 0)
											shotrec[z].trail4c = shotrec[z].trail3c - 3;
										
										if (shotrec[z].trail3c > 0 && shotrec[z].trail3y > 12)
											JE_pixCool(shotrec[z].trail3x, shotrec[z].trail3y, shotrec[z].trail3c);
										shotrec[z].trail3x = shotrec[z].trail2x;
										shotrec[z].trail3y = shotrec[z].trail2y;
										if (shotrec[z].trail2c > 0)
											shotrec[z].trail3c = shotrec[z].trail2c - 3;
										
										if (shotrec[z].trail2c > 0 && shotrec[z].trail2y > 12)
											JE_pixCool(shotrec[z].trail2x, shotrec[z].trail2y, shotrec[z].trail2c);
										shotrec[z].trail2x = shotrec[z].trail1x;
										shotrec[z].trail2y = shotrec[z].trail1y;
										if (shotrec[z].trail1c > 0)
											shotrec[z].trail2c = shotrec[z].trail1c - 3;
										
										if (shotrec[z].trail1c > 0 && shotrec[z].trail1y > 12)
											JE_pixCool(shotrec[z].trail1x, shotrec[z].trail1y, shotrec[z].trail1c);
										shotrec[z].trail1x = tempx;
										shotrec[z].trail1y = tempy;
										shotrec[z].trail1c = tempW - 1;
										break;
								}
								
								for (temp = 0; temp < 20; temp++)
								{
									if (wallexist[temp] && tempx >= wallsx[temp] && tempx <= wallsx[temp] + 11 && tempy >= wallsy[temp] && tempy <= wallsy[temp] + 14)
									{
										if (demolish[shotrec[z].shottype-1])
										{
											wallexist[temp] = false;
											shotavail[z] = true;
											JE_makeExplosion(tempx, tempy, shotrec[z].shottype);
										} else {
											if (shotrec[z].x - shotrec[z].xmov < wallsx[temp] || shotrec[z].x - shotrec[z].xmov > wallsx[temp] + 11)
												shotrec[z].xmov = -shotrec[z].xmov;
											if (shotrec[z].y - shotrec[z].ymov < wallsy[temp] || shotrec[z].y - shotrec[z].ymov > wallsy[temp] + 14)
											{
												if (shotrec[z].ymov < 0)
													shotrec[z].ymov = -shotrec[z].ymov;
												else
													shotrec[z].ymov = -shotrec[z].ymov * 0.8f;
											}
											
											tempx = round(shotrec[z].x);
											tempy = round(shotrec[z].y);
										}
									}
								}
								
								temp = ((Uint8 *)destructTempScreen->pixels)[tempx + tempy * destructTempScreen->w];
								
								if (temp == 25)
								{
									shotavail[z] = true;
									JE_makeExplosion(tempx, tempy, shotrec[z].shottype);
								}
								
							} else {
								shotrec[z].trail2c = 0;
								shotrec[z].trail1c = 0;
							}
							
						} /*Shotavail out of bounds*/
						
					}
					
					L_Left   = false;
					L_Right  = false;
					L_Up     = false;
					L_Down   = false;
					if (NoL_Down > 0)
						NoL_Down--;
					L_Change = false;
					L_Fire   = false;
					
					L_Weapon = false;
					
					switch (cpu)
					{
						case 1:
							
							if (rand() % 100 > 80)
							{
								Lc_Angle += (rand() % 3) - 1;
								if (Lc_Angle > 1)
									Lc_Angle = 1;
								if (Lc_Angle < -1)
									Lc_Angle = -1;
							}
							if (rand() % 100 > 90)
							{
								if (Lc_Angle > 0 && leftangle[leftsel-1] > (M_PI / 2.0f) - (M_PI / 9.0f))
									Lc_Angle = 0;
								if (Lc_Angle < 0 && leftangle[leftsel-1] < M_PI / 8.0f)
									Lc_Angle = 0;
							}
							
							if (rand() % 100 > 93)
							{
								Lc_Power += (rand() % 3) - 1;
								if (Lc_Power > 1)
									Lc_Power = 1;
								if (Lc_Power < -1)
									Lc_Power = -1;
							}
							if (rand() % 100 > 90)
							{
								if (Lc_Power > 0 && leftpower[leftsel-1] > 4)
									Lc_Power = 0;
								if (Lc_Power < 0 && leftpower[leftsel-1] < 3)
									Lc_Power = 0;
								if (leftpower[leftsel-1] < 2)
									Lc_Power = 1;
							}
							
							for (x = 0; x < MAX_INSTALLATIONS; x++)
								if (leftsystem[x] == 8 && leftdmg[x] > 0)
									leftsel = x + 1;
							
							if (leftsystem[leftsel-1] == 8)
							{
								if (!leftyinair[leftsel-1])
									Lc_Power = 1;
								if (rand() % leftx[leftsel-1] > 100)
									Lc_Power = 1;
								if (rand() % 240 > leftx[leftsel-1])
									L_Right = true;
								else if ((rand() % 20) + 300 < leftx[leftsel-1])
									L_Left = true;
								else if (rand() % 30 == 1)
									Lc_Angle = (rand() % 3) - 1;
								if (leftx[leftsel-1] > 295 && leftlastmove[leftsel-1] > 1)
								{
									L_Left = true;
									L_Right = false;
								}
								if (leftsystem[leftsel-1] != 8 || leftlastmove[leftsel-1] > 3 || (leftx[leftsel-1] > 160 && leftlastmove[leftsel-1] > -3))
								{
									if (rand() % (int)round(lefty[leftsel-1]) < 150 && leftymov[leftsel-1] < 0.01f && (leftx[leftsel-1] < 160 || leftlastmove[leftsel-1] < 2))
										L_Fire = true;
									NoL_Down = (5 - abs(leftlastmove[leftsel-1])) * (5 - abs(leftlastmove[leftsel-1])) + 3;
									Lc_Power = 1;
								} else
									L_Fire = false;
								
								z = 0;
								for (x = 0; x < MAX_INSTALLATIONS; x++)
									if (abs(rightx[x] - leftx[leftsel-1]) < 8)
									{
										if (rightsystem[x] == 4)
											L_Fire = false;
										else {
											L_Left = false;
											L_Right = false;
											if (leftlastmove[leftsel-1] < -1)
												leftlastmove[leftsel-1]++;
											else if (leftlastmove[leftsel-1] > 1)
												leftlastmove[leftsel-1]--;
										}
									}
							} else {
								Lc_Fire = 1;
							}
							
							if (rand() % 200 > 198)
							{
								L_Change = true;
								Lc_Angle = 0;
								Lc_Power = 0;
								Lc_Fire = 0;
							}
							
							if (rand() % 100 > 98 || leftshottype[leftsel-1] == 1)
								L_Weapon = true;
							
							if (Lc_Angle > 0)
								L_Left = true;
							if (Lc_Angle < 0)
								L_Right = true;
							if (Lc_Power > 0)
								L_Up = true;
							if (Lc_Power < 0 && NoL_Down == 0)
								L_Down = true;
							if (Lc_Fire > 0)
								L_Fire = true;
							
							if (leftymov[leftsel-1] < -0.1f && leftsystem[leftsel-1] == 8)
								L_Fire = false;
							break;
					}
					
					if (leftsystem[leftsel-1] == 8)
					{
						tempx = leftx[leftsel-1] + round(0.1f * leftlastmove[leftsel-1] * leftlastmove[leftsel-1] * leftlastmove[leftsel-1]) + 5;
						tempy = round(lefty[leftsel-1]) + 1;
					} else {
						tempx = round(leftx[leftsel-1] + 6 + cos(leftangle[leftsel-1]) * (leftpower[leftsel-1] * 8 + 7));
						tempy = round(lefty[leftsel-1] - 7 - sin(leftangle[leftsel-1]) * (leftpower[leftsel-1] * 8 + 7));
					}
					JE_pix(tempx, tempy,  14);
					JE_pix(tempx + 3, tempy, 3);
					JE_pix(tempx - 3, tempy, 3);
					JE_pix(tempx, tempy + 2, 3);
					JE_pix(tempx, tempy - 2, 3);
					if (rightsystem[rightsel-1] == 8)
					{  /*Heli*/
						tempx = rightx[rightsel-1] + round(0.1f * rightlastmove[rightsel-1] * rightlastmove[rightsel-1] * rightlastmove[rightsel-1]) + 5;
						tempy = round(righty[rightsel-1]) + 1;
					} else {
						tempx = round(rightx[rightsel-1] + 6 - cos(rightangle[rightsel-1]) * (rightpower[rightsel-1] * 8 + 7));
						tempy = round(righty[rightsel-1] - 7 - sin(rightangle[rightsel-1]) * (rightpower[rightsel-1] * 8 + 7));
					}
					JE_pix(tempx, tempy,  14);
					JE_pix(tempx + 3, tempy, 3);
					JE_pix(tempx - 3, tempy, 3);
					JE_pix(tempx, tempy + 2, 3);
					JE_pix(tempx, tempy - 2, 3);
					
					JE_bar( 5, 3, 14, 8, 241);
					JE_rectangle( 4, 2, 15, 9, 242);
					JE_rectangle( 3, 1, 16, 10, 240);
					JE_bar(18, 3, 140, 8, 241);
					JE_rectangle(17, 2, 143, 9, 242);
					JE_rectangle(16, 1, 144, 10, 240);
					JE_drawShape2(  4, 0, 190 + leftshottype[leftsel-1], eShapes1);
					JE_outText( 20, 3, weaponNames[leftshottype[leftsel-1]-1], 15, 2);
					sprintf(tempstr, "dmg~%d~", leftdmg[leftsel-1]);
					JE_outText( 75, 3, tempstr, 15, 0);
					sprintf(tempstr, "pts~%d~", leftscore);
					JE_outText(110, 3, tempstr, 15, 0);
					JE_bar(175, 3, 184, 8, 241);
					JE_rectangle(174, 2, 185, 9, 242);
					JE_rectangle(173, 1, 186, 10, 240);
					JE_bar(188, 3, 310, 8, 241);
					JE_rectangle(187, 2, 312, 9, 242);
					JE_rectangle(186, 1, 313, 10, 240);
					JE_drawShape2(174, 0, 190 + rightshottype[rightsel-1], eShapes1);
					JE_outText(190, 3, weaponNames[rightshottype[rightsel-1]-1], 15, 2);
					sprintf(tempstr, "dmg~%d~", rightdmg[rightsel-1]);
					JE_outText(245, 3, tempstr, 15, 0);
					sprintf(tempstr, "pts~%d~", rightscore);
					JE_outText(280, 3, tempstr, 15, 0);
					
					JE_showVGA();
					if (firsttime)
					{
						JE_fadeColor(25);
						firsttime = false;
					}
					
					service_SDL_events(true);
					
					if (leftavail > 0)
					{
						/*LEFT PLAYER INPUT*/
						if (systemangle[leftsystem[leftsel-1]-1])
						{
							/*Leftanglechange*/
							if (keysactive[SDLK_c] || L_Left)
							{
								leftangle[leftsel-1] += 0.01f;
								if (leftangle[leftsel-1] > M_PI / 2)
									leftangle[leftsel-1] = M_PI / 2 - 0.01f;
							}
							/*Rightanglechange*/
							if (keysactive[SDLK_v] || L_Right)
							{
								leftangle[leftsel-1] -= 0.01;
								if (leftangle[leftsel-1] < 0)
									leftangle[leftsel-1] = 0;
							}
						} else if (leftsystem[leftsel-1] == 8) {
							if ((keysactive[SDLK_c] || L_Left) && leftx[leftsel-1] > 5)
								if (JE_stabilityCheck(leftx[rightsel-1] - 5, round(lefty[leftsel-1]))) /** NOTE: BUG! **/
								{
									if (leftlastmove[leftsel-1] > -5)
										leftlastmove[leftsel-1]--;
									leftx[leftsel-1]--;
									if (JE_stabilityCheck(leftx[leftsel-1], round(lefty[leftsel-1])))
										leftyinair[leftsel-1] = true;
								}
							if ((keysactive[SDLK_v] || L_Right) && leftx[leftsel-1] < 305)
								if (JE_stabilityCheck(leftx[leftsel-1] + 5, round(lefty[leftsel-1])))
								{
									if (leftlastmove[leftsel-1] < 5)
										leftlastmove[leftsel-1]++;
									leftx[leftsel-1]++;
									if (JE_stabilityCheck(leftx[leftsel-1], round(lefty[leftsel-1])))
										leftyinair[leftsel-1] = true;
								}
						}
						
						/*Leftincreasepower*/
						if (keysactive[SDLK_a] || L_Up)
						{
							if (leftsystem[leftsel-1] == 8)
							{ /*HELI*/
								leftyinair[leftsel-1] = true;
								leftymov[leftsel-1] -= 0.1f;
							} else if (leftsystem[leftsel-1] != 7 || leftyinair[leftsel-1]) {
								leftpower[leftsel-1] += 0.05f;
								if (leftpower[leftsel-1] > 5)
									leftpower[leftsel-1] = 5;
							} else {
								leftymov[leftsel-1] = -3;
								leftyinair[leftsel-1] = true;
								Lc_Power = 0;
							}
						}
						/*Leftdecreasepower*/
						if (keysactive[SDLK_z] || L_Down)
						{
							if (leftsystem[leftsel-1] == 8 && leftyinair[leftsel-1])
							{ /*HELI*/
								leftymov [leftsel-1] += 0.1f;
							} else {
								leftpower[leftsel-1] -= 0.05f;
								if (leftpower[leftsel-1] < 1)
									leftpower[leftsel-1] = 1;
							}
						}
						/*Leftupweapon*/
						if (keysactive[SDLK_LCTRL])
						{
							keysactive[SDLK_LCTRL] = false;
							leftshottype[leftsel-1]++;
							if (leftshottype[leftsel-1] > SHOT_TYPES)
								leftshottype[leftsel-1] = 1;
							
							while (weaponsystems[leftsystem[leftsel-1]-1][leftshottype[leftsel-1]-1] == 0)
							{
								leftshottype[leftsel-1]++;
								if (leftshottype[leftsel-1] > SHOT_TYPES)
									leftshottype[leftsel-1] = 1;
							}
						}
						/*Leftdownweapon*/
						if (keysactive[SDLK_SPACE] || L_Weapon)
						{
							keysactive[SDLK_SPACE] = false;
							leftshottype[leftsel-1]--;
							if (leftshottype[leftsel-1] < 1)
								leftshottype[leftsel-1] = SHOT_TYPES;
							
							while (weaponsystems[leftsystem[leftsel-1]-1][leftshottype[leftsel-1]-1] == 0)
							{
								leftshottype[leftsel-1]--;
								if (leftshottype[leftsel-1] < 1)
									leftshottype[leftsel-1] = SHOT_TYPES;
							}
						}
						
						/*Leftchange*/
						if (keysactive[SDLK_LALT] || L_Change)
						{
							keysactive[SDLK_LALT] = false;
							leftsel++;
							if (leftsel > MAX_INSTALLATIONS)
								leftsel = 1;
						}
						
						/*Newshot*/
						if (leftshotdelay > 0)
							leftshotdelay--;
						if ((keysactive[SDLK_LSHIFT] || keysactive[SDLK_x] || L_Fire) && (leftshotdelay == 0))
						{
							
							leftshotdelay = shotdelay[leftshottype[leftsel-1]-1];
							
							if (shotdirt[leftshottype[leftsel-1]-1] > 20)
							{
								z = 0;
								for (x = 0; x < SHOT_MAX; x++)
									if (shotavail[x])
										z = x + 1;
								
								if (z > 0 && (leftsystem[leftsel-1] != 8 || leftyinair[leftsel-1]))
								{
									soundQueue[0] = shotsound[leftshottype[leftsel-1]-1];
									
									if (leftsystem[leftsel-1] == 8)
									{
										shotrec[z-1].x = leftx[leftsel-1] + leftlastmove[leftsel-1] * 2 + 5;
										shotrec[z-1].y = lefty[leftsel-1] + 1;
										shotrec[z-1].ymov = 0.5f + leftymov[leftsel-1] * 0.1f;
										shotrec[z-1].xmov = 0.02f * leftlastmove[leftsel-1] * leftlastmove[leftsel-1] * leftlastmove[leftsel-1];
										if ((keysactive[SDLK_a] || L_Up) && righty[rightsel-1] < 30) /** NOTE: BUG? **/
										{
											shotrec[z-1].ymov = 0.1;
											if (shotrec[z-1].xmov < 0)
												shotrec[z-1].xmov += 0.1f;
											else if (shotrec[z-1].xmov > 0)
												shotrec[z-1].xmov -= 0.1f;
											shotrec[z-1].y = righty[rightsel-1]; /** NOTE: BUG? **/
										}
									} else {
										shotrec[z-1].x = leftx[leftsel-1] + 6 + cos(leftangle[leftsel-1]) * 10;
										shotrec[z-1].y = lefty[leftsel-1] - 7 - sin(leftangle[leftsel-1]) * 10;
										shotrec[z-1].ymov = -sin(leftangle[leftsel-1]) * leftpower[leftsel-1];
										shotrec[z-1].xmov =  cos(leftangle[leftsel-1]) * leftpower[leftsel-1];
									}
									
									shotrec[z-1].shottype = leftshottype[leftsel-1];
									
									shotavail[z-1] = false;
									
									shotrec[z-1].shotdur = shotfuse[shotrec[z-1].shottype-1];
									
									shotrec[z-1].trail1c = 0;
									shotrec[z-1].trail2c = 0;
								}
							} else {
								switch (shotdirt[leftshottype[leftsel-1]-1])
								{
									case 1:
										for (x = 0; x < SHOT_MAX; x++)
											if (!shotavail[x])
												if (shotrec[x].x > leftx[leftsel-1])
												{
													shotrec[x].xmov += leftpower[leftsel-1] * 0.1f;
												}
										for (x = 0; x < MAX_INSTALLATIONS; x++)
											if (rightsystem[x] == 8 && rightyinair[x] && rightx[x] < 318)
											{
												rightx[x] += 2;
											}
											leftani[leftsel-1] = 1;
										break;
								}
							}
						}
					}
					
					/*RIGHT PLAYER INPUT*/
					if (rightavail > 0)
					{
						if (systemangle[rightsystem[rightsel-1]-1])
						{
							/*Rightanglechange*/
							if (keysactive[SDLK_KP6])
							{
								rightangle[rightsel-1] += 0.01f;
								if (rightangle[rightsel-1] > M_PI / 2)
									rightangle[rightsel-1] = M_PI / 2 - 0.01f;
							}
							/*Rightanglechange*/
							if (keysactive[SDLK_KP4])
							{
								rightangle[rightsel-1] -= 0.01;
								if (rightangle[rightsel-1] < 0)
									rightangle[rightsel-1] = 0;
							}
						} else if (rightsystem[rightsel-1] == 8) { /*Helicopter*/
							if (keysactive[SDLK_KP4] && rightx[rightsel-1] > 5)
								if (JE_stabilityCheck(rightx[rightsel-1] - 5, round(righty[rightsel-1])))
								{
									if (rightlastmove[rightsel-1] > -5)
										rightlastmove[rightsel-1]--;
									rightx[rightsel-1]--;
									if (JE_stabilityCheck(rightx[rightsel-1], round(righty[rightsel-1])))
										rightyinair[rightsel-1] = true;
								}
							if (keysactive[SDLK_KP6] && rightx[rightsel-1] < 305)
								if (JE_stabilityCheck(rightx[rightsel-1] + 5, round(righty[rightsel-1])))
								{
									if (rightlastmove[rightsel-1] < 5)
										rightlastmove[rightsel-1]++;
									rightx[rightsel-1]++;
									if (JE_stabilityCheck(rightx[rightsel-1], round(righty[rightsel-1])))
										rightyinair[rightsel-1] = true;
								}
						}
						
						/*Rightincreasepower*/
						if (keysactive[SDLK_KP8])
						{
							if (rightsystem[rightsel-1] == 8)
							{ /*HELI*/
								rightyinair[rightsel-1] = true;
								rightymov[rightsel-1] -= 0.1f;
							} else if (rightsystem[rightsel-1] != 7 || rightyinair[rightsel-1]) {
								rightpower[rightsel-1] += 0.05f;
								if (rightpower[rightsel-1] > 5)
									rightpower[rightsel-1] = 5;
							} else {
								rightymov[rightsel-1] = -3;
								rightyinair[rightsel-1] = true;
							}
						}
						/*Rightdecreasepower*/
						if (keysactive[SDLK_KP2])
						{
							if (rightsystem[rightsel-1] == 8 && rightyinair[rightsel-1])
							{ /*HELI*/
								rightymov[rightsel-1] += 0.1f;
							} else {
								rightpower[rightsel-1] -= 0.05;
								if (rightpower[rightsel-1] < 1)
									rightpower[rightsel-1] = 1;
							}
						}
						/*Rightupweapon*/
						if (keysactive[SDLK_KP9])
						{
							keysactive[SDLK_KP9] = false;
							rightshottype[rightsel-1]++;
							if (rightshottype[rightsel-1] > SHOT_TYPES)
								rightshottype[rightsel-1] = 1;
							
							while (weaponsystems[rightsystem[rightsel-1]-1][rightshottype[rightsel-1]-1] == 0)
							{
								rightshottype[rightsel-1]++;
								if (rightshottype[rightsel-1] > SHOT_TYPES)
									rightshottype[rightsel-1] = 1;
							}
						}
						/*Rightdownweapon*/
						if (keysactive[SDLK_KP3])
						{
							keysactive[SDLK_KP3] = false;
							rightshottype[rightsel-1]--;
							if (rightshottype[rightsel-1] < 1)
								rightshottype[rightsel-1] = SHOT_TYPES;
							
							while (weaponsystems[rightsystem[rightsel-1]-1][rightshottype[rightsel-1]-1] == 0)
							{
								rightshottype[rightsel-1]--;
								if (rightshottype[rightsel-1] < 1)
									rightshottype[rightsel-1] = SHOT_TYPES;
							}
						}
						
						/*Rightchange*/
						if (keysactive[SDLK_KP5])
						{
							keysactive[SDLK_KP5] = false;
							rightsel++;
							if (rightsel > MAX_INSTALLATIONS)
								rightsel = 1;
						}
						
						/*Newshot*/
						if (rightshotdelay > 0)
							rightshotdelay--;
						if ((keysactive[SDLK_KP0] || keysactive[SDLK_KP_ENTER]) && rightshotdelay == 0)
						{
							
							rightshotdelay = shotdelay[rightshottype[rightsel-1]-1];
							
							z = 0;
							for (x = 0; x < SHOT_MAX; x++)
								if (shotavail[x])
									z = x + 1;
							
							if (shotdirt[rightshottype[rightsel-1]-1] > 20)
							{
								if (z > 0 && (rightsystem[rightsel-1] != 8 || rightyinair[rightsel-1]))
								{
									soundQueue[1] = shotsound[rightshottype[rightsel-1]-1];
									
									if (rightsystem[rightsel-1] == 8)
									{
										shotrec[z-1].x = rightx[rightsel-1] + rightlastmove[rightsel-1] * 2 + 5;
										shotrec[z-1].y = righty[rightsel-1] + 1;
										shotrec[z-1].ymov = 0.5f;
										shotrec[z-1].xmov = 0.02f * rightlastmove[rightsel-1] * rightlastmove[rightsel-1] * rightlastmove[rightsel-1];
										if (keysactive[SDLK_KP8] && righty[rightsel-1] < 30)
										{
											shotrec[z-1].ymov = 0.1f;
											if (shotrec[z-1].xmov < 0)
												shotrec[z-1].xmov += 0.1f;
											else if (shotrec[z-1].xmov > 0)
												shotrec[z-1].xmov -= 0.1f;
											shotrec[z-1].y = righty[rightsel-1];
										}
									} else {
										shotrec[z-1].x = rightx [rightsel-1] + 6 - cos(rightangle[rightsel-1]) * 10;
										shotrec[z-1].y = righty [rightsel-1] - 7 - sin(rightangle[rightsel-1]) * 10;
										shotrec[z-1].ymov = -sin(rightangle[rightsel-1]) * rightpower[rightsel-1];
										shotrec[z-1].xmov = -cos(rightangle[rightsel-1]) * rightpower[rightsel-1];
									}
									
									if (rightsystem[rightsel-1] == 7)
									{
										shotrec[z-1].x = rightx[rightsel-1] + 2;
										if (rightyinair[rightsel-1])
										{
											shotrec[z-1].ymov = 1;
											shotrec[z-1].y = righty[rightsel-1] + 2;
										} else {
											shotrec[z-1].ymov = -2;
											shotrec[z-1].y = righty[rightsel-1] - 12;
										}
									}
									
									shotrec[z-1].shottype = rightshottype[rightsel-1];
									
									shotavail[z-1] = false;
									
									shotrec[z-1].shotdur = shotfuse[shotrec[z-1].shottype-1];
									
									shotrec[z-1].trail1c = 0;
									shotrec[z-1].trail2c = 0;
								}
							} else {
								switch (shotdirt[rightshottype[rightsel-1]-1])
								{
									case 1:
										for (x = 0; x < SHOT_MAX; x++)
											if (!shotavail[x])
												if (shotrec[z-1].x < rightx[rightsel-1])
												{
													shotrec[z-1].xmov -= rightpower[rightsel-1] * 0.1f;
												}
										for (x = 0; x < MAX_INSTALLATIONS; x++)
											if (leftsystem[x] == 8 && leftyinair[x] && leftx[x] > 1)
											{
												leftx[x] -= 2;
											}
										rightani[rightsel-1] = 1;
										break;
								}
							}
							
						}
					}
					
					if (!died)
					{
						if (leftavail == 0)
						{
							rightscore += Lmodescore[mode-1];
							died = true;
							soundQueue[7] = 30;
							enddelay = 80;
						}
						if (rightavail == 0)
						{
							leftscore += Rmodescore[mode-1];
							died = true;
							soundQueue[7] = 30;
							enddelay = 80;
						}
					}
					
					if (soundEffects > 0 && soundActive)
					{
						
						temp = 0;
						for (temp2 = 0; temp2 < 8; temp2++)
							if (soundQueue[temp2] > 0)
							{
								temp = soundQueue[temp2];
								if (temp2 == 7)
									temp3 = fxPlayVol;
								else
									temp3 = fxPlayVol / 2;
								JE_multiSamplePlay(digiFx[temp-1], fxSize[temp-1], temp2, temp3);
								soundQueue[temp2] = 0;
							}
					}
					
					if (keysactive[SDLK_F10])
					{
						keysactive[SDLK_F10] = false;
						cpu = !cpu;
						keysactive[SDLK_F10] = false;
					}
					
					if (keysactive[SDLK_p])
					{
						JE_pauseScreen();
						keysactive[SDLK_p] = false;
						keysactive[lastkey_sym] = false;
					}
					
					if (keysactive[SDLK_F1])
					{
						JE_helpScreen();
						keysactive[lastkey_sym] = false;
					}
					
					int delaycount_temp;
					if ((delaycount_temp = target - SDL_GetTicks()) > 0)
						SDL_Delay(delaycount_temp);
						
					if (keysactive[SDLK_ESCAPE])
					{
						quit = true;
						endofgame = true;
						keysactive[SDLK_ESCAPE] = false;
					}
					
					if (keysactive[SDLK_BACKSPACE])
					{
						quit = true;
						keysactive[SDLK_BACKSPACE] = false;
					}
					
					if (enddelay > 0)
						enddelay--;
				} while (!quit && !(died && enddelay == 0));
				
				quit = false;
				died = false;
				JE_fadeBlack(25);
			} while (!endofgame);
		}
		
	} while (!quit);
}

void JE_modeSelect( void )
{
	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->h * VGAScreen2->pitch);
	mode = 1;
	
	firsttime = true;
	quit = false;
	
	do {
		for (int x = 1; x <= DESTRUCT_MODES; x++)
		{
			temp = (x == mode) * 4;
			JE_textShade(JE_fontCenter(destructModeName[x-1], TINY_FONT), 70 + x * 12, destructModeName[x-1], 12, temp, FULL_SHADE);
		}
		JE_showVGA();
		
		if (firsttime)
		{
			JE_fadeColor(15);
			firsttime = false;
		}
		
		newkey = false;
		while (!newkey)
		{
			service_SDL_events(false);
			SDL_Delay(16);
		}
		
		if (keysactive[SDLK_UP])
		{
			mode--;
			if (mode < 1)
				mode = 5;
		}
		if (keysactive[SDLK_DOWN])
		{
			mode++;
			if (mode > 5)
				mode = 1;
		}
		if (keysactive[SDLK_ESCAPE])
			quit = true;
		
	} while (!quit && !keysactive[SDLK_RETURN]);
	
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
	JE_word tempx, tempy, tempx2, tempy2;
	JE_byte arenatype2;
	
	arenatype = (rand() % 4) + 1;
	arenatype2 = (rand() % 4) + 1;
	
	/*  Type                 Type2
	    1 = normal           1 = Normal
	    2 = fuzzy walls
	    3 = ballzies
	    4 = highwalls
	*/
	
	JE_playSong(goodsel[rand() % 14]);
	heightchange = (rand() % 3) - 1;
	
	sinewave = ((float)rand() / RAND_MAX) * M_PI / 50.0f + 0.01f;
	sinewave2 = ((float)rand() / RAND_MAX) * M_PI / 50.0f + 0.01f;
	cosinewave = ((float)rand() / RAND_MAX) * M_PI / 50.0f + 0.01f;
	cosinewave2 = ((float)rand() / RAND_MAX) * M_PI / 50.0f + 0.01f;
	HC1 = 20;
	
	switch (arenatype)
	{
		case 1:
			sinewave = M_PI - ((float)rand() / RAND_MAX) * 0.3f;
			sinewave2 = M_PI - ((float)rand() / RAND_MAX) * 0.3f;
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
		
		dirtheight[x] = newheight;
	}
	
	memset(leftdmg, 0, sizeof(leftdmg));
	memset(rightdmg, 0, sizeof(rightdmg));
	
	tempW = 0;
	leftavail = 0;
	
	for (x = 0; x < basetypes[Lbaselookup[mode-1]-1][1-1]; x++)
	{
		leftx[x] = (rand() % 120) + 10;
		lefty[x] = JE_placementPosition(leftx[x] - 1, 14);
		leftsystem[x] = basetypes[Lbaselookup[mode-1]-1][(rand() % 10) + 2-1];
		if (leftsystem[x] == 4)
		{
			if (tempW == MAX_INSTALLATIONS - 1)
			{
				leftsystem[x] = 1;
			} else {
				lefty[x] = 30 + (rand() % 40);
				tempW++;
			}
		}
		leftdmg[x] = basedamage[leftsystem[x]-1];
		if (leftsystem[x] != 4)
			leftavail++;
	}
	
	tempW = 0;
	rightavail = 0;
	
	for (x = 0; x < basetypes[Rbaselookup[mode-1]-1][1-1]; x++)
	{
		rightx[x] = 320 - ((rand() % 120) + 22);
		righty[x] = JE_placementPosition(rightx[x] - 1, 14);
		rightymov[x] = 0;
		rightyinair[x] = false;
		rightsystem[x] = basetypes[Rbaselookup[mode-1]-1][(rand() % 10) + 2-1];
		if (rightsystem[x] == 4)
		{
			if (tempW == MAX_INSTALLATIONS - 1)
			{
				rightsystem[x] = 1;
			} else {
				righty[x] = 30 + (rand() % 40);
				tempW++;
			}
		}
		rightdmg[x] = basedamage[rightsystem[x]-1];
		if (rightsystem[x] != 4)
			rightavail++;
	}
	
	for (z = 0; z < 20; z++)
		wallexist[z] = false;
	
	if (havewalls)
	{
		tempW = 20;
		do {
			
			temp = (rand() % 5) + 1;
			if (temp > tempW)
				temp = tempW;
			
			do {
				x = (rand() % 300) + 10;
				
				found = true;
				
				for (z = 0; z < 4; z++)
					if ((x > leftx[z] - 12) && (x < leftx[z] + 13))
						found = false;
				for (z = 0; z < 4; z++)
					if ((x > rightx[z] - 12) && (x < rightx[z] + 13))
						found = false;
				
			} while (!found);
			
			for (z = 1; z <= temp; z++)
			{
				wallexist[tempW - z + 1-1] = true;
				wallsx[tempW - z + 1-1] = x;
				
				wallsy[tempW - z + 1-1] = JE_placementPosition(x, 12) - 14 * z;
			}
			
			tempW -= temp;
			
		} while (tempW != 0);
	}
	
	for (x = 1; x <= 318; x++)
		JE_rectangle(x, dirtheight[x], x, 199, 25);
	
	if (arenatype == 3)
	{ /*RINGIES!!!!*/
		int rings = rand() % 6 + 1;
		for (x = 1; x <= rings; x++)
		{
			tempx = (rand() % 320);
			tempy = (rand() % 160) + 20;
			y = (rand() % 40) + 10;  /*Size*/
			
			for (z = 1; z <= y * y * 2; z++)
			{
				tempr = ((float)rand() / RAND_MAX) * (M_PI * 2);
				tempy2 = tempy + round(cos(tempr) * (((float)rand() / RAND_MAX) * 0.1f + 0.9f) * y);
				tempx2 = tempx + round(sin(tempr) * (((float)rand() / RAND_MAX) * 0.1f + 0.9f) * y);
				if ((tempy2 > 12) && (tempy2 < 200) && (tempx2 > 0) && (tempx2 < 319))
					((Uint8 *)VGAScreen->pixels)[tempx2 + tempy2 * VGAScreen->w] = 25;
			}
		}
	}
	if (arenatype2 == 2)
	{ /*HOLES*/
		int holes = (rand() % 6) + 1;
		for (x = 1; x <= holes; x++)
		{
			tempx = (rand() % 320);
			tempy = (rand() % 160) + 20;
			y = (rand() % 40) + 10;  /*Size*/
			
			for (z = 1; z < y * y * 2; z++)
			{
				tempr = ((float)rand() / RAND_MAX) * (M_PI * 2);
				tempy2 = tempy + round(cos(tempr) * (((float)rand() / RAND_MAX) * 0.1f + 0.9f) * y);
				tempx2 = tempx + round(sin(tempr) * (((float)rand() / RAND_MAX) * 0.1f + 0.9f) * y);
				if ((tempy2 > 12) && (tempy2 < 200) && (tempx2 > 0) && (tempx2 < 319))
					((Uint8 *)VGAScreen->pixels)[tempx2 + tempy2 * VGAScreen->w] = 0;
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
	s += 12 * VGAScreen->w;
	
	for (int y = 12; y < VGAScreen->h; y++)
	{
		for (int x = 0; x < VGAScreen->w; x++)
		{
			if (*s == 0)
			{
				int temp = 0;
				if (*(s - VGAScreen->w) == 25)
					temp += 1;
				if (y < VGAScreen2->h - 1)
					if (*(s + VGAScreen->w) == 25)
						temp += 3;
				if (x > 0)
					if (*(s - 1) == 25)
						temp += 2;
				if (x < VGAScreen2->w - 1)
					if (*(s + 1) == 25)
						temp += 2;
				if (temp)
					*s = temp + 16;
			}
			s++;
		}
	}
}

void JE_tempScreenChecking( void ) /*and copy to vgascreen*/
{
	Uint8 *s = VGAScreen->pixels;
	s += 12 * VGAScreen->w;
	
	Uint8 *temps = destructTempScreen->pixels;
	temps += 12 * destructTempScreen->w;
	
	for (int y = 12; y < VGAScreen->h; y++)
	{
		for (int x = 0; x < VGAScreen->w; x++)
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

void JE_makeExplosion( JE_word tempx, JE_word tempy, JE_byte shottype )
{
	JE_word tempw = 0;
	JE_byte temp;
	
	for (temp = 0; temp < EXPLO_MAX; temp++)
		if (explosionavail[temp])
			tempw = temp + 1;
	
	if (tempw > 0)
	{
		explosionavail[tempw-1] = false;
		
		temp = explosize[shottype-1];
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
		
		explorec[tempw-1].x = tempx;
		explorec[tempw-1].y = tempy;
		explorec[tempw-1].explowidth = 2;
		
		if (shottype > 0)
		{
			explorec[tempw-1].explomax = explosize[shottype-1];
			explorec[tempw-1].explofill = explodensity[shottype-1];
			explorec[tempw-1].explocolor = shotdirt[shottype-1];
		} else {
			explorec[tempw-1].explomax = (rand() % 40) + 10;
			explorec[tempw-1].explofill = (rand() % 60) + 20;
			explorec[tempw-1].explocolor = 252;
		}
	}
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
