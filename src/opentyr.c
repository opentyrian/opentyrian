#include "opentyr.h"
#include "vga256d.h"

#include "SDL.h"

#include <stdio.h>

int main( int argc, char *argv[] )
{
    JE_char a = '!';

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

    JE_SetPalette(1, 0xFF, 0xFF,  0x0);
    JE_SetPalette(2, 0x00, 0x00, 0xAA);
    JE_SetPalette(3,  0x0, 0xFF,  0x0);

    SDL_LockSurface(VGAScreenSeg);

    JE_rectangle(1, 1, 318, 198, 1);
    JE_bar(150,90, 170, 110, 2);
    JE_circle(160, 100, 32, 3);
    JE_line(1,1, 318, 198, 1);

    SDL_UnlockSurface(VGAScreenSeg);

    JE_ShowVGARetrace();
    SDL_SaveBMP(VGAScreenSeg, "sshot.bmp");

    JE_getk(&a);
    printf("Key pressed: %d\n", a);

    JE_closevga256();

	return 0;
}
