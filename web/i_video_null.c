// Headless video for tests under node: no window, dumps one frame as PPM.
//   DOOM_DUMP=<frame number>  DOOM_OUT=<file.ppm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "d_main.h"
#include "doomdef.h"
#include "compose.h"

static byte pal[768];
static int  frame;

void I_ShutdownGraphics (void) { }
void I_StartFrame (void) { }
void I_StartTic (void) { }
void I_UpdateNoBlit (void) { }
void I_InitGraphics (void) { }

void I_SetPalette (byte* palette)
{
    int i;
    for (i = 0; i < 768; i++)
	pal[i] = gammatable[usegamma][palette[i]];
}

void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}

void I_FinishUpdate (void)
{
    char* dump = getenv ("DOOM_DUMP");
    char* out  = getenv ("DOOM_OUT");
    frame++;
    if (dump && out && frame == atoi (dump))
    {
	static uint32_t pix[MAXWIDTH*MAXHEIGHT], pal32[256];
	FILE* f = fopen (out, "wb");
	int i;
	for (i = 0; i < 256; i++)
	    pal32[i] = 0xff000000u | (pal[i*3]<<16) | (pal[i*3+1]<<8) | pal[i*3+2];
	Compose (pix, pal32);
	fprintf (f, "P6\n%d %d\n255\n", winwidth, winheight);
	for (i = 0; i < winwidth*winheight; i++)
	{
	    unsigned char rgb[3] = { pix[i]>>16, pix[i]>>8, pix[i] };
	    fwrite (rgb, 1, 3, f);
	}
	fclose (f);
	I_Quit ();
    }
}
