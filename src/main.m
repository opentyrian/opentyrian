/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2008  The OpenTyrian Development Team
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
#include "opentyr.h"

#import <Cocoa/Cocoa.h>
#import <SDL/SDL.h>
#import <SDL_net/SDL_net.h>

// SDL_main.h (included by SDL.h) redefines main to SDL_main. As we are
// implementing the "real" main here we must undef this preprocessor symbol.
#undef main

@interface TyrianStartupClass : NSObject
@end

@implementation TyrianStartupClass

- (void)awakeFromNib
{
	// Copied and modified from SDLMain.m from the "SDL Application" template
    
	NSArray *argv_objc = [[NSProcessInfo processInfo] arguments];
	int argc = [argv_objc count];
	char ** argv;
	BOOL finderLaunch;
	
    if ( argc >= 2 && strncmp ([[argv_objc objectAtIndex:1] cString], "-psn", 4) == 0 ) {
        argv = (char **) SDL_malloc(sizeof (char *) * 2);
        argv[0] = (char *)[[argv_objc objectAtIndex:0] cString];
        argv[1] = NULL;
        argc = 1;
        finderLaunch = YES;
    } else {
        int i;
        argv = (char **) SDL_malloc(sizeof (char *) * (argc+1));
        for (i = 0; i < argc; i++) {
            argv[i] = (char *)[[argv_objc objectAtIndex:i] cString];
		}
        finderLaunch = NO;
    }
	
	NSLog(@"%@",argv_objc);
	
	SDL_main(argc, argv);
}
@end

const char* tyrian_game_folder()
{
    return [[[[[NSBundle mainBundle] resourcePath]
                stringByAppendingPathComponent:@"data"]
                stringByAppendingString:@"/"]
            cStringUsingEncoding:NSASCIIStringEncoding];
}

const char* get_user_directory()
{
    // Get Application Support folder
    NSArray* paths =
        NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory,
                                            NSUserDomainMask, YES);
    if ([paths count] > 0)
    {
        NSString* path = [
                            [paths objectAtIndex:0]
                            stringByAppendingPathComponent:@"OpenTyrian"
                         ];
        
        // Create OpenTyrian if it doesn't already exist
        [[NSFileManager defaultManager]
            createDirectoryAtPath:path attributes:nil];
        
        // The return value is expected to end with a /
        return [[path stringByAppendingString:@"/"] cStringUsingEncoding:NSASCIIStringEncoding];
    }
    
    return "";
}

int main(int argc, char** argv)
{	
    return NSApplicationMain(argc, (const char **) argv);
}
