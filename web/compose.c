// Combines the hi-res 3D view with the 320x200 UI buffer (screens[0]).
//
// UI pixels equal to UI_KEY are "transparent" over the 3D area, so the HUD,
// menu and pause graphics (drawn into the UI buffer) show on top of the world.
// The UI is 4:3 and centered; original pixels were 1.2x taller than wide, so
// it is stretched to the full window height.
#include <string.h>

#include "doomdef.h"
#include "doomstat.h"
#include "v_video.h"
#include "compose.h"
#include "m_gameplay.h"

extern boolean menuactive;
extern boolean paused;

static int ux[MAXWIDTH];

static void Rect (uint32_t* dst, int W, int H, int x0, int y0, int x1, int y1, uint32_t c)
{
    int x, y;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > W) x1 = W;
    if (y1 > H) y1 = H;
    for (y = y0; y < y1; y++)
	for (x = x0; x < x1; x++)
	    dst[y*W+x] = c;
}

// Four short arms around the view center, white with a black outline.
static void Crosshair (uint32_t* dst, int W, int H)
{
    int cx = W/2, cy = renderheight/2;
    int t   = H/360 > 1 ? H/360 : 1;		// thickness
    int gap = 2*t;
    int arm = H/70 > 4 ? H/70 : 4;
    int pass;

    for (pass = 0; pass < 2; pass++)
    {
	int o = pass == 0 ? 1 : 0;		// outline grows each rect by o
	uint32_t c = pass == 0 ? 0xff000000u : 0xffffffffu;
	Rect (dst, W, H, cx-gap-arm-o, cy-t/2-o, cx-gap+o,     cy-t/2+t+o, c);	// left
	Rect (dst, W, H, cx+gap-o,     cy-t/2-o, cx+gap+arm+o, cy-t/2+t+o, c);	// right
	Rect (dst, W, H, cx-t/2-o, cy-gap-arm-o, cx-t/2+t+o, cy-gap+o,     c);	// up
	Rect (dst, W, H, cx-t/2-o, cy+gap-o,     cx-t/2+t+o, cy+gap+arm+o, c);	// down
    }
}

void Compose (uint32_t* dst, const uint32_t* pal)
{
    int W = winwidth, H = winheight;
    int uiw = W < H*4/3 ? W : H*4/3;
    int ox = (W-uiw)/2;
    int x, y;
    const byte* ui = screens[0];
    uint32_t black = pal[0];

    for (x = 0; x < W; x++)
	ux[x] = (x < ox || x >= ox+uiw) ? -1 : (x-ox)*SCREENWIDTH/uiw;

    for (y = 0; y < H; y++)
    {
	const byte* uirow = ui + (y*SCREENHEIGHT/H)*SCREENWIDTH;
	uint32_t* out = dst + y*W;

	if (render3d && y < renderheight)
	{
	    const byte* world = renderbuf + y*renderwidth;
	    for (x = 0; x < W; x++)
	    {
		int u = ux[x] < 0 ? UI_KEY : uirow[ux[x]];
		out[x] = pal[u == UI_KEY ? world[x] : u];
	    }
	}
	else
	{
	    for (x = 0; x < W; x++)
		out[x] = ux[x] < 0 ? black : pal[uirow[ux[x]]];
	}
    }

    if (render3d && gp.crosshair && !menuactive && !paused)
	Crosshair (dst, W, H);
}
