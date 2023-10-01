# x48ng -- HP48 CPU emulator

(I'm not very good at writing, see ./README_0.6.4 for the original README)

This is my fork of x48-0.6.4 where I deviate from the original code and do my own thing.

## Screenshots

### `--tui` version (ncurses)

![screenshot of x48ng --tui running in a terminal](./tui-0.28.0-screenshot.png?raw=true "screenshot of x48ng --tui running in a terminal")

### `--x11` version

![screenshot of x48ng --x11](./x11-0.28.0-screenshot.png?raw=true "screenshot of x48ng --x11")

### `--sdl` version

![screenshot of x48ng --sdl](./sdl-0.28.0-screenshot.png?raw=true "screenshot of x48ng --sdl")

## Usage

`./dist/x48ng --help`

You can use the script `./dist/setup-x48ng-home.sh` or simply run `./dist/x48ng --rom=<romfilename>`

### manual setup

1. Create `~/.config/x48ng/`
2. Copy `/usr/share/x48ng/ROMs/gxrom-r` (or any other rom) to `~/.config/x48ng/rom`
3. Run `x48ng --print-config > ~/.config/x48ng/config.lua`
4. Run `x48ng`

### Ncurses UI (`--tui`)

I had to be a bit 'creative' mapping keys there:

- `On` is either `F4` or `Esc` or `Home` or `\`
- `Left Shift` is either `F5` or `PgUp` or `[`
- `Right Shift` is either `F6` or `PgDn` or `]`
- `Alpha` is either `F8` or `Ins` or `;`

_To quit `x48ng --tui` use `F10` or `Shift+End` or `|`_

## What have I done:

0. renamed it to x48ng to avoid confusion
1. merged in a SDL1 version I found @ https://code.google.com/archive/p/x48-sdl/source/default/source
2. removed the code supporting Solaris, HP-UX, etc.
3. removed the autotools-based build system and wrote a simple Makefile instead
4. added a x48ng.desktop file and an icon
5. refactoring as a way to explore/understand the codebase

## Bugs to fix

See https://github.com/gwenhael-le-moine/x48ng/issues

## What more I would like to do:

### short term

- port from sdl1.2 to sdl2

### long(er) term

- GUI in gtk(4)?
- support the HP49g ROM?

## Compilation

The `Makefile` will try to autodetect if necessary dependencies for x11 and sdl front-ends are met and enable/disable x11 and sdl front-ends accordingly.

You can force disable x11 front-end by compiling with `make WITH_X11=no`.

You can force disable sdl front-end by compiling with `make WITH_SDL=no`.

Ncurses front-end is always built-in.

### Dependencies (see .github/workflows/c-cpp.yml for debian packages names)

- readline

for SDL version:

- SDL_gfx1
- SDL 1.2 or sdl12_compat

for x11 version:

- x11

for Ncurses:

- ncursesw

## Installation

1. Run `sudo make install PREFIX=/usr DOCDIR=/usr/doc/x48ng MANDIR=/usr/man DESTDIR=/tmp/package` filling in your own values for PREFIX, DOCDIR, MANDIR and DESTDIR.
2. once installed run `/usr/share/x48ng/setup-x48ng-home.sh` to setup your ~/.x48ng/. It sets up a HP 48GX with a 128KB card in port 1 and a 4MB card in port 2
3. run `x48ng` and enjoy

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
