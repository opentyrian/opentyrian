#include "opentyr.h"
#include "newshape.h"
#include "vga256d.h"

/*************/
VGA256D_EXTERNS
/*************/

JE_boolean LoadOverride = FALSE;

SDL_Surface *tempscreenseg = NULL;

void JE_NewLoadShapesB( JE_byte table, FILE *f )
{
    short tempw;
    short z;

    fread(&tempw, 2, 1, f);
    maxshape[table] = tempw;

    if(!LoadOverride) {
        min = 1;
        max = maxshape[table];
    }

    if(min > 1) {
        for(z = 0; z < min-1; z++) {
            fread(&shapexist[table][z], 1, 1, f);

            if(shapexist[table][z]) {
                fread(&shapex   [table][z], 2, 1, f);
                fread(&shapey   [table][z], 2, 1, f);
                fread(&shapesize[table][z], 2, 1, f);

                (*shapearray)[table][z] = malloc(shapex[table][z]*shapey[table][z]);

                fread((*shapearray)[table][z], shapesize[table][z], 1, f);

                free((*shapearray)[table][z]);
            }
        }
    }

    for(z = min-1; z < max; z++) {
        tempw = z-min+1;
        fread(&shapexist[table][tempw], 1, 1, f);

        if(shapexist[table][tempw]) {
            fread(&shapex   [table][tempw], 2, 1, f);
            fread(&shapey   [table][tempw], 2, 1, f);
            fread(&shapesize[table][tempw], 2, 1, f);

            (*shapearray)[table][tempw] = malloc(shapex[table][tempw]*shapey[table][tempw]);

            fread((*shapearray)[table][tempw], shapesize[table][tempw], 1, f);
        }
    }
}

void JE_NewDrawCShape( JE_byte *shape, JE_word xsize, JE_word ysize )
{
    JE_word x = 2, y = 2;
    JE_word xloop = 0, yloop = 0;
    JE_byte *p; /* shape pointer */
    unsigned char *s;   /* screen pointer, 8-bit specific */

    s = (unsigned char *)tempscreenseg->pixels;
    s += y * tempscreenseg->w + x;

    for(p = shape; yloop < ysize; p++) {
        switch(*p) {
            case 255:   /* p transparent pixels */
                p++;
                s += *p; xloop += *p;
                break;
            case 254:   /* next y */
                s -= xloop; xloop = 0;
                s += tempscreenseg->w; yloop++;
                break;
            case 253:   /* 1 transparent pixel */
                s++; xloop++;
                break;
            default:    /* set a pixel */
                *s = *p;
                s++; xloop++;
                break;
        }
        if(xloop == xsize) {
            s -= xloop; xloop = 0;
            s += tempscreenseg->w; yloop++;
        }
    }

    tempscreenseg = VGAScreenSeg;
}

void JE_NewDrawCShapeNum( JE_byte table, JE_byte shape, JE_word x, JE_word y )
{
    JE_word xsize, ysize;
    JE_word xloop = 0, yloop = 0;
    JE_byte *p; /* shape pointer */
    unsigned char *s;   /* screen pointer, 8-bit specific */

    if((shape > maxshape[table]) || (!shapexist[table][shape]) || (shape == 255)) {
        exit(99);   /* pascalism */
    }

    xsize = shapex[table][shape]; ysize = shapey[table][shape];

    s = (unsigned char *)tempscreenseg->pixels;
    s += y * tempscreenseg->w + x;

    for(p = (*shapearray)[table][shape]; yloop < ysize; p++) {
        switch(*p) {
            case 255:   /* p transparent pixels */
                p++;
                s += *p; xloop += *p;
                break;
            case 254:   /* next y */
                s -= xloop; xloop = 0;
                s += tempscreenseg->w; yloop++;
                break;
            case 253:   /* 1 transparent pixel */
                s++; xloop++;
                break;
            default:    /* set a pixel */
                *s = *p;
                s++; xloop++;
                break;
        }
        if(xloop == xsize) {
            s -= xloop; xloop = 0;
            s += tempscreenseg->w; yloop++;
        }
    }

    tempscreenseg = VGAScreenSeg;
}

void JE_NewPurgeShapes( JE_byte table )
{
    JE_word x;

    if(maxshape[table] > 0) {
        for(x = 0; x < maxshape[table]; x++) {
            if(shapexist[table][x]) {
                free((*shapearray)[table][x]);
                shapexist[table][x] = FALSE;
            }
        }
    }
}

void JE_DrawShapeTypeOne( JE_word x, JE_word y, JE_byte *shape )
{
    JE_word xloop = 0, yloop = 0;
    JE_byte *p = shape; /* shape pointer */
    unsigned char *s;   /* screen pointer, 8-bit specific */

    s = (unsigned char *)VGAScreenSeg->pixels;
    s += y * VGAScreenSeg->w + x;

    for(yloop = 0; yloop < 28; yloop++) {
        for(xloop = 0; xloop < 24; xloop++) {
            *s = *p;
            s++; p++;
        }
        s -= 24;
        s += VGAScreenSeg->w;
    }
}

void JE_GrabShapeTypeOne( JE_word x, JE_word y, JE_byte *shape )
{
    JE_word xloop = 0, yloop = 0;
    JE_byte *p = shape; /* shape pointer */
    unsigned char *s;   /* screen pointer, 8-bit specific */

    s = (unsigned char *)VGAScreenSeg->pixels;
    s += y * VGAScreenSeg->w + x;

    for(yloop = 0; yloop < 28; yloop++) {
        for(xloop = 0; xloop < 24; xloop++) {
            *p = *s;
            s++; p++;
        }
        s -= 24;
        s += VGAScreenSeg->w;
    }
}

void newshape_init( void )
{
    tempscreenseg = VGAScreenSeg;
    for(x = 0; x < maxtable; x++) {
        maxshape[x] = 0;
    }
    shapearray = malloc(sizeof(JE_shapearraytype));
}
