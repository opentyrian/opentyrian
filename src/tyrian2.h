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
#ifndef TYRIAN2_H
#define TYRIAN2_H

#include "opentyr.h"

#include "varz.h"
#include "helptext.h"


#define CUBE_WIDTH 35
#define LINE_WIDTH 150

extern JE_word statDmg[2];

void JE_scaleInPicture( void );

void JE_createNewEventEnemy( JE_byte enemytypeofs, JE_word enemyoffset );

void JE_genItemMenu( JE_byte itemnum );

/*void JE_drawBackground3( void );*/

void JE_weaponViewFrame( JE_byte testshotnum );

void JE_doNetwork( void );

void JE_makeEnemy( struct JE_SingleEnemyType *enemy );

void JE_weaponSimUpdate( void );

void JE_funkyScreen( void );

void JE_eventJump( JE_word jump );

void JE_menuFunction( JE_byte select );

void JE_drawScore( void );

void JE_doStatBar( void );

JE_boolean JE_quitRequest( void );

void JE_whoa( void );

void JE_drawMainMenuHelpText( void );

void JE_doFunkyScreen( void );

JE_integer JE_partWay( JE_integer start, JE_integer finish, JE_byte dots, JE_byte dist );

void JE_computeDots( void );

void JE_barX ( JE_word x1, JE_word y1, JE_word x2, JE_word y2, JE_byte col );

void JE_initWeaponView( void );

void JE_scaleBitmap( SDL_Surface *bitmap, JE_word x, JE_word y, JE_word x1, JE_word y1, JE_word x2, JE_word y2 );

typedef JE_byte JE_MenuChoiceType[MAX_MENU];

void JE_newEnemy( int enemyOffset );
void JE_drawEnemy( int enemyOffset );
void JE_starShowVGA( void );

void JE_main( void );
void JE_loadMap( void );
void JE_titleScreen( JE_boolean animate );
void JE_readTextSync( void );
void JE_displayText( void );

JE_boolean JE_searchFor( JE_byte PLType );
void JE_eventSystem( void );

JE_longint JE_cashLeft( void );
void JE_loadCubes( void );
void JE_drawItem( JE_byte itemType, JE_word itemNum, JE_word x, JE_word y );
void JE_itemScreen( void );

void JE_drawMenuHeader( void );
void JE_drawMenuChoices( void );
void JE_updateNavScreen( void );

void JE_drawNavLines( JE_boolean dark );
void JE_drawLines( JE_boolean dark );
void JE_drawDots( void );
void JE_drawPlanet( JE_byte planetNum );

#endif /* TYRIAN2_H */

// kate: tab-width 4; vim: set noet:
