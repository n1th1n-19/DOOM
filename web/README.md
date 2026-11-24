# DOOM in the browser

WebAssembly build of the 1997 DOOM source (`../linuxdoom-1.10`).

## Build and run

```
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
(cd ~/emsdk && ./emsdk install latest && ./emsdk activate latest)
source ~/emsdk/emsdk_env.sh

cp /path/to/doom1.wad .          # the free shareware IWAD (not in the repo)
make -f Makefile.web -j8
python3 -m http.server 8000      # then open http://localhost:8000/
```

Page options: `?h=1080` render height, `?w=` render width (default: window aspect,
4:3 up to 8:3), `?debug` shows the game console on the page.
Drop any IWAD/PWAD on the start screen to play it instead.

## What differs from the original

- Platform layer: `i_video_sdl.c` (SDL2 video/input), sound uses the original mixer
  queued into SDL audio (`../linuxdoom-1.10/i_sound.c`), `i_net_stub.c` (single player).
- The 3D view renders hi-res into `renderbuf`; the 320x200 UI buffer is composited
  over it (`compose.c`, palette index 255 = transparent). Widescreen is Hor+.
- Look up/down by y-shear (mouse, PgUp/PgDn/Home); mouse wheel switches weapons.
- Options → Gameplay edits rules at runtime (`m_gameplay.c`); locked in demos and netgames.
- Config and savegames persist in IndexedDB.

## Tests

`make -f Makefile.web test` builds a headless node binary. Examples:

```
node build/doomtest.js -timedemo demo1          # must print: timed 5026 gametics
DOOM_DUMP=150 DOOM_OUT=/tmp/f.ppm node build/doomtest.js -warp 1 1 -width 1280 -height 720
```
Test hooks: `DOOM_LOOK=<rows>`, `DOOM_KEYS=27,175,13` (key codes, one every 4 tics).

Not done: music (needs a MUS synth), real netplay.
