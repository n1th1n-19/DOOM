# DOOM source code

The DOOM sources as released by id Software on December 23, 1997, under the GPL.
This repository also contains a **WebAssembly build** that runs the game in a
browser.

## Layout

| Directory | What it is |
| --- | --- |
| [`linuxdoom-1.10/`](linuxdoom-1.10) | The game: renderer, play simulation, menus, platform layer. |
| [`sndserv/`](sndserv) | The Linux sound-server process used by the original build. |
| [`ipx/`](ipx) | DOS IPX network driver and setup program. |
| [`sersrc/`](sersrc) | DOS serial and modem driver and setup program. |
| [`web/`](web) | The browser build. See [`web/README.md`](web/README.md). |

Original text files are kept as released: [`README.TXT`](README.TXT),
[`LICENSE.TXT`](LICENSE.TXT) and the notes in `linuxdoom-1.10/` (`README.*`,
`TODO`, `FILES`).

## Play it in the browser

```bash
source ~/emsdk/emsdk_env.sh          # Emscripten, see web/README.md to install
cd web
cp /path/to/doom1.wad .              # the free shareware WAD
make -f Makefile.web -j8
python3 -m http.server 8000          # open http://localhost:8000/
```

What the browser build adds to the original game:

- High-resolution and widescreen rendering, up to 1920x1200.
- Mouse look and WASD controls.
- **Options → Gameplay**: change speed, gravity, monster stats, damage, jump and
  more while playing. Locked in demos and netgames so demos stay in sync.
- Drag-and-drop WADs, and saves that persist in the browser.

## Game data

The code needs a WAD file with the game's levels and graphics, which is **not**
included. The shareware `doom1.wad` is free to download and use. Commercial WADs
come from a copy of DOOM you own.

## Original Linux build

The original build targets Linux with X11 and uses the separate sound server:

```bash
cd linuxdoom-1.10 && make    # see the Makefile; modern systems need porting work
```

This path was not touched by the browser port and has not been rebuilt here.

## License

GPL. See [`LICENSE.TXT`](LICENSE.TXT).
