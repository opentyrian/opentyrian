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
#include "animlib.h"
#include "file.h"
#include "keyboard.h"
#include "network.h"
#include "nortsong.h"
#include "palette.h"
#include "video.h"

#include <assert.h>

/*** Structs ***/
/* Presumably all of these structs must line up EXACTLY with what's in the file */
typedef struct anim_FileHeader_s
{
	Uint32 id;              /* Should equal the little endian equiv of "LPF " */
	Uint16 maxlps;          /* Constant value, 256.  Related to nlps (below) */
	Uint16 nlps;            /* Number of 'pages', max 256. */
	Uint32 nRecords;        /* Number of 'records', max 65534 (plus one last-to-first delta, wahtever that is). */
	Uint16 maxRecsPerLp;	/* Number of 'records' permitted by an lp.  256. */
	Uint16 lpfTableOffset;  /* Seek position of lpfTable.  1280.  */

	Uint32 contentType;     /* Should equal the little endian equiv of "ANIM" */
	Uint16 width;           /* Width of course, in pixels */
	Uint16 height;          /* and height */
	Uint8  variant;         /* Constant value, 0. */
	Uint8  version;         /* Typedef.  Frame rate.  0 = 18fps, 1  = 70fps */

	Uint8  hasLastDelta;    /* Bool.  If true, the last frame is a delta from the last-to-first frame.  I don't know what the hell a last-to-first frame is yet. */
	Uint8  lastDeltaValid;  /* Bool.  If false, the mysterious last-to-first dela hasn't been updated or some such and should be ignored. */


	Uint8  pixelType;       /* Typedef. 0 = a 256 color palette (and it's all we support probably) */
	Uint8  CompressionType; /* Typedef. Compression used.  1 = RunSkipDump and is the only valid option */
	Uint8  otherRecsPerFrm; /* Constant 0 for us */
	Uint8  bitmaptype;      /* Typedef.  1 = 320x200, which is all we support. */
	Uint8  recordTypes[32]; /* Unknown; not implemented. */

	Uint32 nFrames;         /* Number of frames. */

	Uint16 framesPerSecond; /* Numbe of frames to play per second.  One wonders what version, above, is for then... */
	Uint8  padding[58];     /* Exactly what it says on the tin. */
	Uint16 emptyWord;       /* More padding */
} anim_FileHeader_t;
typedef struct anim_LargePageHeader_s
{
	Uint16 baseRecord;      /* The first record's number */
	Uint16 nRecords;        /* Number of records.  Two bit flags are used. */
	Uint16 nBytes;	        /* Number of bytes used, excluding headers */
} anim_LargePageHeader_t;


/*** Globals ***/
Uint8 * CurrentPageSeg;
Uint8 CurrentPageBuffer[65536];
Uint16 CurrentPageRecordSizes[256];
anim_LargePageHeader_t CurrentPageHeader;

unsigned int i;
unsigned int MaxFrame;

anim_FileHeader_t FileHeader;
anim_LargePageHeader_t PageHeader[256];

unsigned int Curlpnum;

FILE * InFile;
unsigned int temp;
unsigned int PageFind[150];


/*** Function decs ***/
int JE_playRunSkipDump( Uint8 *, unsigned int );
void JE_closeAnim( void );
void JE_loadAnim( JE_char * );
int JE_renderFrame( unsigned int );
JE_word JE_findPage ( JE_word );
int JE_drawFrame( unsigned int );
int JE_loadPage( unsigned int );

static inline int ExtractFromStream( const Uint8 *, unsigned int *, unsigned int *, unsigned int, unsigned int);
/*** Implementation ***/


/* Loads the given page into memory.
 *
 * Returns  0 on success
 *         -1 if the file header offers conflicting information
 *         -2 if file IO failed
 */
int JE_loadPage( unsigned int pagenumber )
{
	unsigned int i, pageSize;


	if (Curlpnum == pagenumber) { return(0); } /* Already loaded */


	/* Every read or seek operation is wrapped in an if statement that aborts
	 * on failure.  As Yuriks likes to remind me, error handling in these cases
	 * is an ugly beast of C.  But I get the feeling he'd be even less pleased
	 * with some tidy gotos and macros.
	 */
	Curlpnum = pagenumber;
	if (fseek(InFile, 0x0B00 + (pagenumber<<16), SEEK_SET) != 0) { return(-2); }
	if (efread(&CurrentPageHeader, sizeof(CurrentPageHeader), 1, InFile) != 1) { return(-2); } //todo: use as pointer, reassign headers

	if (fseek(InFile, 2, SEEK_CUR) != 0) { return(-2); } /* 2 byte padding or something */
	if (efread(CurrentPageRecordSizes, sizeof(Uint16), CurrentPageHeader.nRecords, InFile) != CurrentPageHeader.nRecords) { return(-2); }
	if (efread(CurrentPageBuffer, 1, CurrentPageHeader.nBytes, InFile) != CurrentPageHeader.nBytes) { return(-2); }

	/* Okay, we've succeeded in all our IO checks.  Now, make sure the
	 * headers aren't lying or damaged or something.
	 */
	pageSize = 0;
	for (i = 0; i < CurrentPageHeader.nRecords; i++)
	{
		pageSize += CurrentPageRecordSizes[i];
	}

	if(pageSize != CurrentPageHeader.nBytes) { return(-1); }

	/* So far, so good */
	return(0);
}

int JE_drawFrame( unsigned int framenumber )
{
	int ret;


	ret = JE_loadPage(framenumber);
	if (ret) { return(ret); }

	ret = JE_renderFrame (framenumber);
	if (ret) { return(ret); }

	return(0);
}

JE_word JE_findPage ( JE_word framenumber )
{
	unsigned int i;


	for (i = 0; i < FileHeader.nlps; i++)
	{
		if (PageHeader[i].baseRecord < framenumber
		 && PageHeader[i].baseRecord + PageHeader[i].nRecords + 1 > framenumber)
		{
			break;
		}
	}

	return(i); /* what happens if not found? */
}

int JE_renderFrame( unsigned int framenumber )
{
	unsigned int i, offset, destframe;


	destframe = framenumber - CurrentPageHeader.baseRecord;

	offset = 0;
	for (i = 0; i < destframe; i++)
	{
		offset += CurrentPageRecordSizes[i];
	}

	return (JE_playRunSkipDump (CurrentPageBuffer + offset, CurrentPageRecordSizes[destframe]));
}

void JE_playAnim( JE_char *animfile, JE_byte startingframe, JE_boolean keyhalt, JE_byte speed )
{
	unsigned int i;


	MaxFrame = speed;
	JE_loadAnim(animfile);


	for (i = 0; i < 150; i++) //should be FileHeader.nRecords?
	{
		PageFind[i] = JE_findPage(i+1);
	}

	/* Blank screen */
	JE_clr256();
	JE_showVGA();

	startingframe--; /* pascal's stupid indexing... */

    for (i = startingframe; i < FileHeader.nRecords; i++)
    {
    	/* Handle boring crap */
    	setjasondelay(MaxFrame);
    	JE_showVGA();

		/* Load required frame, if necessary.  Function returns clean if already loaded, so no checks needed */
		if (JE_loadPage(PageFind[i]) != 0) /* Danger, Will Robinson!  File is bad! */
		{
			break; /* Bail! */
		}

    	if (JE_renderFrame(i) != 0)
    	{
    		break; /* Bail, same as above */
    	}

		/* Return early if user presses a key */
		service_SDL_events(true);
		if (newkey)
		{
			JE_closeAnim();
			return;
		}

		/* Wait until we need the next frame */
		NETWORK_KEEP_ALIVE();
		wait_delay();
    }

	JE_closeAnim();
}

void JE_loadAnim( JE_char *filename )
{
	Curlpnum = 65535;
	CurrentPageSeg = CurrentPageBuffer;

	InFile = fopen(filename, "rb");

	efread(&FileHeader, sizeof(anim_FileHeader_t), 1, InFile);
	fseek(InFile, 260, SEEK_SET);

	/* read in palette */
	efread(colors, 1, 4 * 256, InFile);
	JE_updateColorsFast(colors);

	/* Read in headers */
	fseek(InFile, 1280, SEEK_SET);
	efread(PageHeader, sizeof(anim_LargePageHeader_t), FileHeader.nlps, InFile); /* All 256 ehaders exist, but there's no point in getting the last few */
}

void JE_closeAnim( void )
{
	fclose(InFile);
}

/* RunSkipDump decompresses the video.  There are three operations, run, skip,
 * and dump.  They can be used in either byte or word variations, making six
 * possible actions, and there's a seventh 'stop' action, which looks
 * like 0x80 0x00 0x00.
 *
 * Run is a memset.
 * Dump is a memcpy.
 * Skip leaves the old data intact and simply increments the pointers.
 *
 * returns 0 on success or 1 if decompressing failed.  Failure to decompress
 * indicates a broken or malicious file; playback should terminate.
 */
int JE_playRunSkipDump( Uint8 *incomingBuffer, unsigned int IncomingBufferLength )
{
	unsigned int IncomingBufferPos, OutgoingBufferPos, OutgoingBufferLength;
	unsigned int opcode, value, count;
	Uint8 * outgoingBuffer;

	#define ANI_SHORT_RLE  0x00
	#define ANI_SHORT_SKIP 0x80
	#define ANI_LONG_OP    0x80
	#define ANI_LONG_COPY_OR_RLE  0x8000
	#define ANI_LONG_RLE   0x4000
	#define ANI_STOP       0x0000

	opcode = value = count = 0;
	IncomingBufferPos = OutgoingBufferPos = 0;
	OutgoingBufferLength = VGAScreen->h * VGAScreen->pitch;
	outgoingBuffer = VGAScreen->pixels;

	/* 320x200 is the only supported format.  Assert is here to remind people if our screen size ever changes */
	assert(OutgoingBufferLength == 320 * 200);


	while (1)
	{
		/* Get one byte.  This byte may have flags that tell us more */
		if (ExtractFromStream(incomingBuffer, &opcode, &IncomingBufferPos, IncomingBufferLength, 1) ) { return(-1); }

		/* Divide into 'short' and 'long' */
		if (opcode == ANI_LONG_OP) /* long ops */
		{
			if (ExtractFromStream(incomingBuffer, &opcode, &IncomingBufferPos, IncomingBufferLength, 2) ) { return(-1); }

			if (opcode == ANI_STOP) /* We are done decompressing.  Leave */
			{
				break;
			}
			else if ((opcode & ANI_LONG_COPY_OR_RLE) == false) /* If it's not those two, it's a skip */
			{
				count = opcode;

				/* Sanity check.  Not technically necessary since there is no output */
				if (OutgoingBufferPos + count >= OutgoingBufferLength) { return(-1); }

				/* The actual skip */
				OutgoingBufferPos += count;
			}
			else /* Now things get a bit mroe interesting... */
			{
				opcode &= ~ANI_LONG_COPY_OR_RLE; /* Clear that flag */

				if (opcode & ANI_LONG_RLE) /* RLE */
				{
					count = opcode & ~ANI_LONG_RLE; /* Clear flag */

					/* Extract another byte */
					if (ExtractFromStream(incomingBuffer, &value, &IncomingBufferPos, IncomingBufferLength, 1) ) { return(-1); }

					/* Buffer checks */
					if (OutgoingBufferPos + count >= OutgoingBufferLength) { return(-1); }

					/* The actual run */
					memset(outgoingBuffer + OutgoingBufferPos, value, count);
					OutgoingBufferPos += count;
				}
				else
				{ /* Long copy */
					count = opcode;

					/* Sanity checks */
					if (IncomingBufferPos + count >= IncomingBufferLength) { return(-1); }
					if (OutgoingBufferPos + count >= OutgoingBufferLength) { return(-1); }

					/* Run */
					memcpy(outgoingBuffer + OutgoingBufferPos, incomingBuffer + IncomingBufferPos, count);
					IncomingBufferPos += count;
					OutgoingBufferPos += count;
				}
			}
		} /* End of long ops */
		else /* short ops */
		{
			if (opcode & ANI_SHORT_SKIP) /* Short skip, move pointer only */
			{
				count = opcode & ~ANI_SHORT_SKIP; /* clear flag to get count */

				/* Bound checking.
				 * This is technically optional since any actual runs or dumps
				 * will catch it before we write to our array.
				 */
				if (OutgoingBufferPos + count >= OutgoingBufferLength) { return(-1); }

				/* Skip */
				OutgoingBufferPos += count;
			}
			else if (opcode == ANI_SHORT_RLE) /* Short RLE, memset the destination */
			{
				/* Extract a few more bytes */
				if (ExtractFromStream(incomingBuffer, &count, &IncomingBufferPos, IncomingBufferLength, 1) ) { return(-1); }
				if (ExtractFromStream(incomingBuffer, &value, &IncomingBufferPos, IncomingBufferLength, 1) ) { return(-1); }

				/* Bounds checking */
				if (OutgoingBufferPos + count >= OutgoingBufferLength) { return(-1); }

				/* Run */
				memset(outgoingBuffer + OutgoingBufferPos, value, count);
				OutgoingBufferPos += count;
			}
			else /* Short copy, memcpy from src to dest. */
			{
				count = opcode;

				/* Bounds checking */
				if (IncomingBufferPos + count >= IncomingBufferLength) { return(-1); }
				if (OutgoingBufferPos + count >= OutgoingBufferLength) { return(-1); }

				/* Dump */
				memcpy(outgoingBuffer + OutgoingBufferPos, incomingBuffer + IncomingBufferPos, count);
				IncomingBufferPos += count;
				OutgoingBufferPos += count;
			}
		} /* End of short ops */
	}

	/* And that's that */
	return(0);
}

/* This extraction function serves to help clean up the above function.
 * It 'returns' nonzero on failure; that way we can easily wrap our extracts
 * in if statements.  Since it's inline, we can load it with parameters
 * and not worry. It also handles endian swaps if necessary.
 */
static inline int ExtractFromStream( const Uint8 * IN_stream, unsigned int * OUT_value, unsigned int * stream_pos, unsigned int stream_size, unsigned int extract_size)
{
	assert(sizeof(OUT_value) >= extract_size);

	if(*stream_pos + extract_size >= stream_size) /* Extraction would go beyond buffer */
	{
		return(-1);
	}

	/* This is ugly.  That's why it's here instead of littered all over code.
	 * Todo: see how well this is optimized, since it's inlined and therefore
	 * each inlined version will have a known extract_state value...
	 */
	switch (extract_size)
	{
		case 1:
			*OUT_value = (((Uint8  *)IN_stream)[*stream_pos]);
			break;
		case 2:
			*OUT_value = SDL_SwapLE16(((Uint16 *)IN_stream)[*stream_pos]);
			break;
		case 4:
			*OUT_value = SDL_SwapLE32(((Uint32 *)IN_stream)[*stream_pos]);
			break;
		case 8:
			*OUT_value = SDL_SwapLE64(((Uint64 *)IN_stream)[*stream_pos]);
			break;
		default:
			assert(false); /* Unnatural extraction.  We only work on bytes words or dwords and I see no reason to change that for one function */
	}
	*stream_pos += extract_size;

	return(0);
}


// kate: tab-width 4; vim: set noet:
