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

static int ux[MAXWIDTH];

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
}
