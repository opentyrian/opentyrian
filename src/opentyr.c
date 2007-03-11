#include "opentyr.h"
#include "vga256d.h"

#include "SDL.h"

#include <stdio.h>

int main( int argc, char *argv[] )
{
	/* TODO: DetectCFG */
	/* TODO: scanforepisodes */

	printf("\nWelcome to... >> OpenTyrian v0.1 <<\n\n");

	/* TODO: InitKeyboard */

	/* TODO: Not sure what these are about?
	 * recordfilenum = 1;
	 * playdemonum = 0; 
	 * playdemo = false; */

	/* TODO: LoadConfiguration */

	/* TODO: Tyrian originally checked availible memory here. */

	/* here ends line 92771 of TYRIAN2.PAS
	 * TODO: Finish it and stuff. */

    SDL_Init( 0 );

    JE_initvga256();

    JE_SetPalette(1, 0xFF, 0xFF, 0x0);

    SDL_LockSurface(VGAScreenSeg);

    JE_rectangle(1, 1, 320-2, 200-2, 1);
    JE_bar(150,90, 170, 110, 1);
    JE_circle(160, 100, 32, 1);
    JE_line(0,0, 319, 199, 1);

    SDL_UnlockSurface(VGAScreenSeg);

    JE_ShowVGARetrace();

    SDL_SaveBMP(VGAScreenSeg, "sshot.bmp");
    SDL_Delay(3000);

    JE_closevga256();

    SDL_Quit();
	
	return 0;
}
