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
#include "r_main.h"
#include "p_local.h"
#include "p_mobj.h"
#include "d_event.h"
#include "compose.h"

static byte pal[768];
static int  frame;

void I_ShutdownGraphics (void) { }
void I_StartFrame (void) { }
void I_StartTic (void)
{
    // test hook: DOOM_LOOK=<rows> shears the view once, after the level started
    static int n;
    if (++n == 30 && getenv ("DOOM_LOOK"))
	R_AddLook (atoi (getenv ("DOOM_LOOK")));

    // test hook: DOOM_STATS=1 prints monster counts at tic 100
    if (n == 100 && getenv ("DOOM_STATS"))
    {
	thinker_t* th; int alive = 0, dead = 0, shown = 0;
	for (th = thinkercap.next ; th != &thinkercap ; th = th->next)
	{
	    mobj_t* mo;
	    if (th->function.acp1 != (actionf_p1)P_MobjThinker) continue;
	    mo = (mobj_t*)th;
	    if (!(mo->flags & MF_COUNTKILL) && !(mo->flags & MF_CORPSE)) continue;
	    if (mo->health > 0 && (mo->flags & MF_COUNTKILL)) alive++; else dead++;
	    if (shown++ < 8) fprintf (stderr, "mobj type %d health %d flags %x\n", mo->type, mo->health, mo->flags);
	}
	{
	    mobj_t* pl = players[consoleplayer].mo; int best = 1<<30;
	    for (th = thinkercap.next ; th != &thinkercap ; th = th->next)
	    {
		mobj_t* mo;
		if (th->function.acp1 != (actionf_p1)P_MobjThinker) continue;
		mo = (mobj_t*)th;
		if ((mo->flags & MF_COUNTKILL) && mo->health > 0)
		{
		    int d = P_AproxDistance (mo->x - pl->x, mo->y - pl->y) >> FRACBITS;
		    if (d < best) best = d;
		}
	    }
	    fprintf (stderr, "nearest living monster: %d units (player radius 16)\n", best);
	}
	fprintf (stderr, "STATS alive=%d dead/corpse=%d skill=%d\n", alive, dead, gameskill);
    }


    // test hook: DOOM_KEYS=27,173,13 presses these keys, one every 4 tics
    //  (from tic 40), e.g. to open menus
    {
	char* k = getenv ("DOOM_KEYS");
	if (k && n >= 40 && (n % 4) == 0)
	{
	    int idx = (n-40)/4, key = 0, i;
	    for (i = 0; k && i <= idx; i++)
	    {
		key = atoi (k);
		k = strchr (k, ',');
		if (k) k++;
		if (i < idx && !k)
		    return;
	    }
	    if (i == idx+1 && key)
	    {
		event_t e;
		e.type = ev_keydown; e.data1 = key; D_PostEvent (&e);
		e.type = ev_keyup;   D_PostEvent (&e);
	    }
	}
    }
}
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
