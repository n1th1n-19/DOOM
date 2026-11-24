// SDL2 video + input for the browser build. Replaces i_video.c (X11/XShm).
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "d_main.h"
#include "d_event.h"
#include "doomdef.h"
#include "compose.h"

static SDL_Window*   window;
static SDL_Renderer* renderer;
static SDL_Texture*  texture;
static Uint32        palette32[256];
static Uint32        pixels32[MAXWIDTH*MAXHEIGHT];
static int           mouse_buttons;

static int TranslateKey (SDL_Keycode k)
{
    switch (k)
    {
      case SDLK_LEFT:       return KEY_LEFTARROW;
      case SDLK_RIGHT:      return KEY_RIGHTARROW;
      case SDLK_DOWN:       return KEY_DOWNARROW;
      case SDLK_UP:         return KEY_UPARROW;
      case SDLK_ESCAPE:     return KEY_ESCAPE;
      case SDLK_RETURN:
      case SDLK_KP_ENTER:   return KEY_ENTER;
      case SDLK_TAB:        return KEY_TAB;
      case SDLK_BACKSPACE:
      case SDLK_DELETE:     return KEY_BACKSPACE;
      case SDLK_PAUSE:      return KEY_PAUSE;
      case SDLK_KP_PLUS:
      case SDLK_EQUALS:     return KEY_EQUALS;
      case SDLK_KP_MINUS:
      case SDLK_MINUS:      return KEY_MINUS;
      case SDLK_LSHIFT:
      case SDLK_RSHIFT:     return KEY_RSHIFT;
      case SDLK_LCTRL:
      case SDLK_RCTRL:      return KEY_RCTRL;
      case SDLK_LALT:
      case SDLK_RALT:       return KEY_RALT;
      case SDLK_F11:        return KEY_F11;
      case SDLK_F12:        return KEY_F12;
    }
    if (k >= SDLK_F1 && k <= SDLK_F10)
	return KEY_F1 + (k - SDLK_F1);
    if (k >= SDLK_SPACE && k < 127)       // printable ASCII, already lowercase
	return k;
    return 0;                              // unmapped: ignore
}

static void PostKey (evtype_t type, SDL_Keycode sym)
{
    event_t event;
    int key = TranslateKey (sym);
    if (!key)
	return;
    event.type = type;
    event.data1 = key;
    D_PostEvent (&event);
}

void I_ShutdownGraphics (void)
{
    if (texture)  SDL_DestroyTexture (texture);
    if (renderer) SDL_DestroyRenderer (renderer);
    if (window)   SDL_DestroyWindow (window);
    texture = NULL; renderer = NULL; window = NULL;
}

void I_StartFrame (void) { }

void I_StartTic (void)
{
    SDL_Event e;
    event_t   event;

    if (!window)
	return;

    while (SDL_PollEvent (&e))
    {
	switch (e.type)
	{
	  case SDL_KEYDOWN:
	    if (!e.key.repeat)
		PostKey (ev_keydown, e.key.keysym.sym);
	    break;
	  case SDL_KEYUP:
	    PostKey (ev_keyup, e.key.keysym.sym);
	    break;
	  case SDL_MOUSEBUTTONDOWN:
	  case SDL_MOUSEBUTTONUP:
	  {
	    int bit = e.button.button == SDL_BUTTON_LEFT   ? 1 :
		      e.button.button == SDL_BUTTON_MIDDLE ? 2 :
		      e.button.button == SDL_BUTTON_RIGHT  ? 4 : 0;
	    if (e.type == SDL_MOUSEBUTTONDOWN)
	    {
		// pointer lock needs a user gesture, so grab on first click
		SDL_SetRelativeMouseMode (SDL_TRUE);
		mouse_buttons |= bit;
	    }
	    else
		mouse_buttons &= ~bit;
	    event.type = ev_mouse;
	    event.data1 = mouse_buttons;
	    event.data2 = event.data3 = 0;
	    D_PostEvent (&event);
	    break;
	  }
	  case SDL_MOUSEMOTION:
	    if (e.motion.xrel || e.motion.yrel)
	    {
		event.type = ev_mouse;
		event.data1 = mouse_buttons;
		event.data2 = e.motion.xrel << 2;
		event.data3 = -e.motion.yrel << 2;   // DOOM: up is positive
		D_PostEvent (&event);
	    }
	    break;
	  case SDL_QUIT:
	    I_Quit ();
	    break;
	}
    }
}

void I_UpdateNoBlit (void) { }

void I_FinishUpdate (void)
{
    Compose (pixels32, palette32);

    SDL_UpdateTexture (texture, NULL, pixels32, winwidth*sizeof(Uint32));
    SDL_RenderClear (renderer);
    SDL_RenderCopy (renderer, texture, NULL, NULL);
    SDL_RenderPresent (renderer);
}

void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}

void I_SetPalette (byte* palette)
{
    int i;
    for (i = 0; i < 256; i++)
    {
	int r = gammatable[usegamma][*palette++];
	int g = gammatable[usegamma][*palette++];
	int b = gammatable[usegamma][*palette++];
	palette32[i] = 0xff000000u | (r<<16) | (g<<8) | b;
    }
}

void I_InitGraphics (void)
{
    static int firsttime = 1;
    if (!firsttime)
	return;
    firsttime = 0;

    if (SDL_Init (SDL_INIT_VIDEO) < 0)
	I_Error ("SDL_Init failed: %s", SDL_GetError ());

    SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "0");
    window = SDL_CreateWindow ("DOOM", SDL_WINDOWPOS_CENTERED,
			       SDL_WINDOWPOS_CENTERED, winwidth, winheight, 0);
    if (!window)
	I_Error ("SDL_CreateWindow failed: %s", SDL_GetError ());
    renderer = SDL_CreateRenderer (window, -1, 0);
    if (!renderer)
	I_Error ("SDL_CreateRenderer failed: %s", SDL_GetError ());
    texture = SDL_CreateTexture (renderer, SDL_PIXELFORMAT_ARGB8888,
				 SDL_TEXTUREACCESS_STREAMING,
				 winwidth, winheight);
    if (!texture)
	I_Error ("SDL_CreateTexture failed: %s", SDL_GetError ());
}
