# x48ng -- HP48 CPU emulator

(I'm not very good at writing, see ./README_0.6.4 for the original README)

This is my fork of x48-0.6.4 where I deviate from the original code and do my own thing.

## What have I done:

0. renamed it to x48ng to avoid confusion
1. merged in a SDL1 version I found @ https://code.google.com/archive/p/x48-sdl/source/default/source
2. removed the code supporting Solaris, HP-UX, etc.
3. removed the autotools-based build system and wrote a simple Makefile instead
4. added a x48ng.desktop file and an icon
5. make including the debugger an option at compilation

## Bugs to fix

See https://github.com/gwenhael-le-moine/x48ng/issues

## What more I would like to do:

1. clean-up further.
2. split the core emulator in a lib and have the GUI use that to cleanly separate the two.
3. have a more modern GUI in gtk4
4. can something be merged from droid48? [ https://github.com/shagr4th/droid48 ]
99. support the HP49g ROM?

## Compilation

### GUI

By default the X11 version is built. It is the most complete UI-wise.

To build the X11 version run `make GUI=x11`

To build the SDL1 version run `make GUI=sdl1`

### Debugger

Debugger is not built by default.
To build the debugger compile with `make WITH_DEBUGGER=yes`

## Installation

1. Run `sudo make install PREFIX=/usr DOCDIR=/usr/doc/x48ng MANDIR=/usr/man DESTDIR=/tmp/package` filling in your own values for PREFIX, DOCDIR, MANDIR and DESTDIR.
2. once installed run `/usr/share/x48ng/setup-x48ng-home.sh` to setup your ~/.x48ng/. It sets up a HP 48GX with a 128KB card in port 1 and a 4MB card in port 2
3. run `x48ng` and enjoy

## Development

- `make FULL_WARNINGS=yes` to compile with all warnings
- `make clean` and `make clean-all` to clean between compilation runs
- `make pretty-code` to format the code using `clang-format` and the provided `.clang-format`
