#include "opentyr.h"
#include "vga256d.h"

#include "SDL.h"
#include <stdio.h>
#include <string.h>

void JE_initvga256( void )
{
    if (!(VGAScreenSeg = SDL_SetVideoMode(320,200,8, SDL_SWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF))) {
        printf("Display initialization failed: %s\n", SDL_GetError());
        exit(1);
    }
}

void JE_InitVGA256X( void )
{
    JE_initvga256();
}

void JE_closevga256( void )
{
    /* We don't have any cleanup to do */
}

void JE_clr256( void )
{
    memset(VGAScreenSeg->pixels, 0, VGAScreenSeg->pitch * VGAScreenSeg->h);
}

void JE_ShowVGA( void )
{
    SDL_Flip(VGAScreenSeg);
}

void JE_ShowVGARetrace( void )
{
    SDL_Flip(VGAScreenSeg);
}

void JE_GetVGA( void )
{
    SDL_Flip(VGAScreenSeg); /* TODO: YKS: This is probably not what we should do, but I don't see a way of doing it either. */
}

/* OnScreen TODO */

/* OffScreen TODO */

void JE_disable_refresh( void )
{
    /* This would normally blank the screen, but since it's hard to implement and not used I'll leave it as a no-op. */
}

void JE_enable_refresh( void )
{
    /* Same as JE_disable_refresh */
}

void JE_WaitRetrace( void )
{
    /* nop */
}

void JE_WaitPartialRetrace( void )
{
}

void JE_WaitNotRetrace( void )
{
}



INLINE void JE_pix( JE_word x, JE_word y, JE_byte c )
{
    JE_pix2(x,y,c);
}

INLINE void JE_pix2( JE_word x, JE_word y, JE_byte c )
{
    /* Bad things happen if we don't clip */
    if (x < 320 && y < 200) {
        char *vga = VGAScreenSeg->pixels;
        vga[y*320+x] = c;
    }
}

INLINE void JE_pixcool( JE_word x, JE_word y, JE_byte c )
{
    JE_pix3(x,y,c);
}

void JE_pix3( JE_word x, JE_word y, JE_byte c )
{
    /* Originally impemented as several direct accesses */
    JE_pix2(x,y,c);
    JE_pix2(x-1,y,c);
    JE_pix2(x+1,y,c);
    JE_pix2(x,y-1,c);
    JE_pix2(x,y+1,c);
}

INLINE void JE_pixabs( JE_word x, JE_byte c )
{
    if (x < 320*200) {
        char *vga = VGAScreenSeg->pixels;
        vga[x] = c;
    }
}

INLINE void JE_getpix( JE_word x, JE_word y, JE_byte *c )
{
    /* Bad things happen if we don't clip */
    if (x < 320 && y < 200) {
        char *vga = VGAScreenSeg->pixels;
        *c = vga[y*320+x];
    }
}

INLINE JE_byte JE_getpixel( JE_word x, JE_word y )
{
    /* Bad things happen if we don't clip */
    if (x < 320 && y < 200) {
        char *vga = VGAScreenSeg->pixels;
        return vga[y*320+x];
    }

    return 0;
}

void JE_rectangle( JE_word a, JE_word b, JE_word c, JE_word d, JE_word e ) /* x1, y1, x2, y2, color */
{
    if (a < 320 && c < 320 && b < 200 && d < 200) {
        char *vga = VGAScreenSeg->pixels;
        int i;

        /* Top line */
        memset(vga+(b*320+a), e, c-a+1);

        /* Bottom line */
        memset(vga+(d*320+a), e, c-a+1);

        /* Left line */
        for (i=(b+1)*320+a; i < (d*320+a); i += 320) {
            vga[i] = e;
        }
        
        /* Right line */
        for (i=(b+1)*320+c; i < (d*320+c); i += 320) {
            vga[i] = e;
        }
    } else {
        printf("!!! WARNING: Rectangle clipped: %d %d %d %d %d\n", a,b,c,d,e);
    }
}
