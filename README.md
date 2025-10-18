# x48ng -- HP 48 emulator

This is my fork of x48-0.6.4 where I deviate from the original code and do my own thing. (See ./README_0.6.4 for the original README)

**x48ng** is part of my little collection of HP calculators' emulators' that I took over maintenance :

- [x48ng](https://codeberg.org/gwh/x48ng) ← You are here
- [x50ng](https://codeberg.org/gwh/x50ng)
- [saturnng](https://codeberg.org/gwh/saturnng)
- [hpemung](https://codeberg.org/gwh/hpemung)

[ The reference public source repository is https://codeberg.org/gwh/x48ng ]

## Usage

`x48ng --help`

You can use the script `setup-x48ng-home.sh` or simply run `x48ng --rom=<romfilename>`

## Screenshots

### `--tui-small` version (ncurses, 4 pixels per character)

![screenshot of x48ng --tui-small running in a terminal](./tui-small-screenshot.png?raw=true "screenshot of x48ng --tui-small running in a terminal")

### `--tui-tiny --mono` version (ncurses, 8 pixels per character)

![screenshot of x48ng --tui-tiny --mono running in a terminal](./tui-tiny-screenshot.png?raw=true "screenshot of x48ng --tui-tiny --mono running in a terminal")

### `--tui` version (ncurses)

![screenshot of x48ng --tui running in a terminal](./tui-screenshot.png?raw=true "screenshot of x48ng --tui running in a terminal")

### `--sdl` version

![screenshot of x48ng --sdl](./sdl-screenshot.png?raw=true "screenshot of x48ng --sdl")

### manual setup

1. Create `$XDG_CONFIG_HOME/x48ng` (usually `~/.config/x48ng/`)
2. Copy `/usr/share/x48ng/ROMs/gxrom-r` (or any other rom) to `$XDG_CONFIG_HOME/x48ng/rom`
3. Run `x48ng --print-config > $XDG_CONFIG_HOME/x48ng/config.lua`
4. Run `x48ng`

### Ncurses UI (`--tui`)

I had to be a bit 'creative' mapping keys there:
```
┌─[ Help ]─────────────────────────────────────────────────────────┐
│Special keys:                                                     │
│ F1: Help, F7: Quit                                               │
│Calculator keys:                                                  │
│ all alpha-numerical keys                                         │
│ F2: Left-Shift, F3: Right-Shift, F4: Alpha, F5: On, F6: Enter    │
└──────────────────────────────────────────────────────────────────┘
```

There's also additional _hidden_ shortcuts that might work and/or come handy:

- *Left-Shift*: `[` or `PgUp`
- *Right-Shift*: `]` or `PgDn`
- *Alpha*: `;` or `Ins`
- *On*: `Esc` or `Home`
- *Enter*: `,`
- quit: `|` or `Shift + End` or `F10`

## What have I done:

0. renamed it to x48ng to avoid confusion
1. merged in a SDL1 version I found @ https://code.google.com/archive/p/x48-sdl/source/default/source
2. removed the code supporting Solaris, HP-UX, etc.
3. removed the autotools-based build system and wrote a simple Makefile instead
4. added a x48ng.desktop file and an icon
5. refactoring as a way to explore/understand the codebase
6. drop Xresources
7. link to lua to use it as a config file reader
8. ported the SDL1 GUI to SDL2 to SDL3

## Bugs to fix

See and report at https://codeberg.org/gwh/x48ng/issues or https://github.com/gwenhael-le-moine/x48ng/issues

## What more I would like to do:

### long(er) term

- GUI in gtk(4)?

## Compilation

The `Makefile` will try to autodetect if necessary dependencies for x11 and sdl front-ends are met and enable/disable x11 and sdl front-ends accordingly.

You can force disable sdl front-end by compiling with `make WITH_SDL=no`.

Ncurses front-end is always built-in.

### Dependencies (see .github/workflows/c-cpp.yml for debian packages names)

- Lua
- readline

for Ncurses:

- ncursesw

for SDL version:

- SDL3
- SDL3_gfx

## Installation

0. Run `make get-roms` unless you have a good reason not to. (ROMs licenses are unclear so they're not bundled by default.)
1. Run `sudo make install PREFIX=/usr` (see the Makefile to see what variables your can override.)
2. Run `x48ng-launcher`. On first launch or if no ~/.config/x48ng/ exists it will be created and populated.
   By default it sets up a HP 48GX with ROM `R` with a 128KB card in port 1 and a 4MB card in port 2. You can change the ROM by setting the ROM environment variable (`ROM=sxrom-j x48ng-launcher` for example.)

## Development

- `make FULL_WARNINGS=yes` to compile with all warnings
- `make clean` and `make mrproper` to clean between compilation runs
- `make pretty-code` to format the code using `clang-format` and the provided `.clang-format`

## Friends and inspiration

- https://github.com/gwenhael-le-moine/x48 (original x48 slightly touched but not too much)
- https://github.com/shagr4th/droid48 (x48 ported to Android)
- https://hp.giesselink.com/emu48.htm (The Emu48)
- https://github.com/dgis/emu48android (Emu48 ported to Android)
- https://github.com/gwenhael-le-moine/jsEmu48 (another HP 48 emulator)
- [x50ng](https://codeberg.org/gwh/x50ng)
- [saturnng](https://codeberg.org/gwh/saturnng)
- [hpemung](https://codeberg.org/gwh/hpemung)
