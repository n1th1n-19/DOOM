# DOOM in the browser

A WebAssembly build of the 1997 DOOM source in [`../linuxdoom-1.10`](../linuxdoom-1.10).
It runs in any modern browser, renders the world at high resolution (including
widescreen), and adds an in-game menu for changing the rules while you play.

The game data is not part of this repository. You supply a WAD file (the free
shareware `doom1.wad` works).

## Contents

- [Quick start](#quick-start)
- [Deploying to Vercel](#deploying-to-vercel)
- [Controls](#controls)
- [Page options](#page-options)
- [Options → Gameplay](#options--gameplay)
- [Playing your own WADs](#playing-your-own-wads)
- [How it works](#how-it-works)
- [Testing](#testing)
- [Limitations](#limitations)

## Quick start

You need [Emscripten](https://emscripten.org/) to build and Python 3 for a static
web server. No other C toolchain is required.

```bash
# 1. Install Emscripten (once)
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
(cd ~/emsdk && ./emsdk install latest && ./emsdk activate latest)

# 2. Build
source ~/emsdk/emsdk_env.sh
cd web
cp /path/to/doom1.wad .        # not in the repo; *.wad is git-ignored
make -f Makefile.web -j8       # writes build/doom.{js,wasm,data}

# 3. Serve and play
python3 -m http.server 8000    # open http://localhost:8000/
```

Header changes rebuild every object file, because structs such as `visplane_t`
are shared across the renderer.

## Deploying to Vercel

The site is static, so there is nothing to build on Vercel itself. `web/dist/` is
the finished site and **is committed**, so a Git-connected Vercel project can serve
it directly.

Git-connected project, in the Vercel project settings:

| Setting | Value |
| --- | --- |
| Root Directory | `web/dist` |
| Framework Preset | Other |
| Build Command | empty |
| Output Directory | empty (the default) |

Without that Root Directory, Vercel serves the repo root, `build/doom.js` is
missing, and the page fails with a 404 plus a "MIME type text/plain" error.

CLI deploy instead:

```bash
cd web/dist
npx vercel deploy --prod
```

After changing any source, rebuild and commit the result:

```bash
make -f Makefile.web dist    # refreshes dist/ (index.html, vercel.json, build/doom.{js,wasm,data})
git add web/dist && git commit
```

- `doom.data` contains the WAD, so the shareware `doom1.wad` is what gets
  published. Only deploy a WAD you are allowed to redistribute.
- [`vercel.json`](vercel.json) sets caching for `build/` and basic security headers.
  Vercel serves `.wasm` as `application/wasm` and compresses the files itself.
- Any other static host works the same way: serve the contents of `dist/`.

## Controls

| Action | Keys |
| --- | --- |
| Move | `W` `A` `S` `D` (arrow Up / Down also move) |
| Turn | Mouse (click the game to capture it), or arrow Left / Right |
| Look up / down | Mouse, `PgUp` / `PgDn`; `Home` re-centers |
| Fire | Left mouse button, or `Ctrl` |
| Use / open doors | `E` or `Space` (`Space` jumps instead when Jump is on) |
| Run | `Shift` (or turn on Always run) |
| Strafe modifier | `Alt` |
| Weapons | `1`–`7`, or the mouse wheel |
| Menu | `Esc` |

On touch devices an on-screen control pad appears; the **Touch controls** button
toggles it elsewhere. The toolbar under the game also has an FPS counter, a
smooth/pixelated scaling toggle, and a fullscreen button.

## Page options

Add these to the URL, for example `index.html?h=1080&debug`.

| Parameter | Meaning |
| --- | --- |
| `h=<rows>` | Render height, 240–1200 (default 720). |
| `w=<cols>` | Render width, up to 1920 (default: the window's aspect, between 4:3 and 8:3). |
| `debug` | Shows the game's console output on the page and logs every 100th frame. |
| `autostart` | Skips the start screen. |

## Options → Gameplay

Open it from `Esc` → Options → Gameplay. Use Left / Right (or Enter) to change a
row.

| Setting | Range | Effect |
| --- | --- | --- |
| Move speed | 25–400 % | Scales player thrust. |
| Gravity | 25–400 % | Fall speed and jump arc. |
| Friction | 25–400 % | How quickly momentum decays; low is icy. |
| Monster health | 25–800 % | Applied when a monster spawns. |
| Monster speed | 25–300 % | Walking speed (projectiles are unchanged). |
| Monster damage | 0–500 % | Damage monsters deal to you. |
| Your damage | 25–1000 % | Damage you deal to monsters, including splash. |
| Infinite ammo | on / off | Refills ammo every tic. |
| God mode | on / off | Same rules as the `iddqd` cheat for damage under 1000. |
| Monsters respawn | on / off | The nightmare respawn rule at any skill. |
| Jump (Space) | on / off | Adds a jump on `Space`. |
| Always run | on / off | Inverts the run key. |
| Mouse look | on / off | Mouse Y looks up and down instead of moving. |

All rows except *Always run* and *Mouse look* are **locked while a demo plays or
records and in netgames**, so recorded demos stay in sync. Settings are saved and
restored between sessions.

## Playing your own WADs

Drop one or more `.wad` files anywhere on the start screen before pressing Play.

- An **IWAD** is renamed to the file name DOOM looks for (`doom1.wad`, `doom.wad`,
  `doomu.wad` or `doom2.wad`), chosen from the maps inside it.
- A **PWAD** is added with `-file`.

Dropped files live in memory only. Config and savegames are stored in the
browser's IndexedDB and survive reloads.

## How it works

The engine is unchanged apart from the platform layer and some targeted edits.

| Area | Where |
| --- | --- |
| Video and input (SDL2) | [`i_video_sdl.c`](i_video_sdl.c) |
| Final frame: hi-res world + 320x200 UI | [`compose.c`](compose.c) |
| Sound: the original mixer, queued into SDL audio | [`../linuxdoom-1.10/i_sound.c`](../linuxdoom-1.10/i_sound.c) |
| Single-player network stub | [`i_net_stub.c`](i_net_stub.c) |
| Emscripten calls (main loop, quit, error, config save) | [`web_glue.c`](web_glue.c) |
| Gameplay settings | [`../linuxdoom-1.10/m_gameplay.c`](../linuxdoom-1.10/m_gameplay.c) |
| Page, start screen, touch controls | [`index.html`](index.html) |
| Build | [`Makefile.web`](Makefile.web) |

**Rendering.** The world is drawn into a separate buffer (`renderbuf`) of the
window's size. The 2D parts (HUD, status bar, menus, intermission) still draw into
the original 320x200 buffer; palette index 255 there means "transparent", so they
show on top of the 3D view. The view uses a 90° horizontal field of view on a 4:3
window and widens for wider windows. Visplane rows are 16-bit (the original 8-bit
rows capped the view at 255 rows).

**Main loop.** `D_DoomLoop` was split into `D_DoomFrame()`, which the browser's
frame loop calls. The screen-wipe animation and a few busy-waits are skipped
because they would freeze the tab.

**Determinism.** `geff` is the settings struct the game reads. It equals vanilla
values whenever a demo plays or records or a netgame runs.

## Testing

`make -f Makefile.web test` builds a headless binary that runs under Node and can
dump a frame as an image.

```bash
# Demo sync: must print "timed 5026 gametics"
node build/doomtest.js -timedemo demo1

# Screenshot of E1M1 at 1280x720 (frame 150), as a PPM file
DOOM_DUMP=150 DOOM_OUT=/tmp/frame.ppm node build/doomtest.js -warp 1 1 -width 1280 -height 720
```

Test hooks, set as environment variables:

| Variable | Effect |
| --- | --- |
| `DOOM_DUMP=<frame>` and `DOOM_OUT=<file>` | Write that frame as a PPM and exit. |
| `DOOM_LOOK=<rows>` | Shear the view once after the level starts. |
| `DOOM_KEYS=27,175,13` | Press these key codes, one every 4 tics (for opening menus). |

## Limitations

- No music. MUS lumps load but nothing decodes them; the Music Volume slider does
  nothing.
- No network play.
- Shareware has no super shotgun sprites, so that weapon stays commercial-only.
- The browser build is tested headlessly; sound, touch controls and mouse-wheel
  weapon switching have not been tried in a real browser.
