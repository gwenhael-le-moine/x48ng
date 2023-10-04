# Makefile to build x48ng without autotools

VERSION_MAJOR = 0
VERSION_MINOR = 29
PATCHLEVEL = 91

MAKEFLAGS +=-j$(NUM_CORES) -l$(NUM_CORES)

CC ?= gcc

WITH_X11 ?= yes
WITH_SDL ?= yes

OPTIM ?= 2

CFLAGS = -g -O$(OPTIM) -I./src/ -D_GNU_SOURCE=1 -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DPATCHLEVEL=$(PATCHLEVEL)
LIBS = -lm

### lua
CFLAGS += $(shell pkg-config --cflags lua)
LIBS += $(shell pkg-config --libs lua)

### debugger
CFLAGS += $(shell pkg-config --cflags readline)
LIBS += $(shell pkg-config --libs readline)

FULL_WARNINGS = no
ifeq ($(FULL_WARNINGS), yes)
	CFLAGS += -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Wconversion -Wdouble-promotion -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap
endif

DOTOS = src/emu_serial.o \
	src/emu_emulate.o \
	src/emu_init.o \
	src/emu_actions.o \
	src/emu_memory.o \
	src/emu_register.o \
	src/emu_timer.o \
	src/debugger.o \
	src/runtime_options.o \
	src/romio.o \
	src/ui_text.o \
	src/ui.o \
	src/main.o

### X11 UI
ifeq ($(WITH_X11), yes)
X11CFLAGS = $(shell pkg-config --cflags x11 xext) -D_GNU_SOURCE=1
X11LIBS = $(shell pkg-config --libs x11 xext)

CFLAGS += $(X11CFLAGS) -DHAS_X11=1
LIBS += $(X11LIBS)
DOTOS += src/ui_x11.o
endif

### SDL UI
ifeq ($(WITH_SDL), yes)
SDLCFLAGS = $(shell pkg-config --cflags SDL_gfx sdl12_compat)
SDLLIBS = $(shell pkg-config --libs SDL_gfx sdl12_compat)

CFLAGS += $(SDLCFLAGS) -DHAS_SDL=1
LIBS += $(SDLLIBS)
DOTOS += src/ui_sdl.o
endif

### Text UI
CFLAGS += $(shell pkg-config --cflags ncursesw) -DNCURSES_WIDECHAR=1
LIBS += $(shell pkg-config --libs ncursesw)

.PHONY: all clean clean-all pretty-code install mrproper

all: dist/mkcard dist/checkrom dist/dump2rom dist/x48ng

# Binaries
dist/mkcard: src/tools/mkcard.o
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

dist/dump2rom: src/tools/dump2rom.o
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

dist/checkrom: src/tools/checkrom.o src/romio.o
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

dist/x48ng: $(DOTOS)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

# Cleaning
clean:
	rm -f src/*.o src/tools/*.o

mrproper: clean
	rm -f dist/mkcard dist/checkrom dist/dump2rom dist/x48ng

clean-all: mrproper

# Formatting
pretty-code:
	clang-format -i src/*.c src/*.h src/tools/*.c

# Installing
PREFIX = /usr
DOCDIR = $(PREFIX)/doc/x48ng
MANDIR = $(PREFIX)/man
install: all
	install -m 755 -d -- $(DESTDIR)$(PREFIX)/bin
	install -c -m 755 dist/x48ng $(DESTDIR)$(PREFIX)/bin/x48ng

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/x48ng
	install -c -m 755 dist/mkcard $(DESTDIR)$(PREFIX)/share/x48ng/mkcard
	install -c -m 755 dist/dump2rom $(DESTDIR)$(PREFIX)/share/x48ng/dump2rom
	install -c -m 755 dist/checkrom $(DESTDIR)$(PREFIX)/share/x48ng/checkrom
	install -c -m 644 dist/hplogo.png $(DESTDIR)$(PREFIX)/share/x48ng/hplogo.png
	cp -R dist/ROMs/ $(DESTDIR)$(PREFIX)/share/x48ng/
	find $(DESTDIR)$(PREFIX)/share/x48ng/ROMs/ -name "*.bz2" -exec bunzip2 {} \;
	sed "s|@PREFIX@|$(PREFIX)|g" dist/setup-x48ng-home.sh > $(DESTDIR)$(PREFIX)/share/x48ng/setup-x48ng-home.sh
	chmod 755 $(DESTDIR)$(PREFIX)/share/x48ng/setup-x48ng-home.sh

	install -m 755 -d -- $(DESTDIR)$(MANDIR)/man1
	sed "s|@VERSION@|$(VERSION_MAJOR).$(VERSION_MINOR).$(PATCHLEVEL)|g" dist/x48ng.man.1 > $(DESTDIR)$(MANDIR)/man1/x48ng.1
	gzip -9  $(DESTDIR)$(MANDIR)/man1/x48ng.1

	install -m 755 -d -- $(DESTDIR)$(DOCDIR)
	cp -R AUTHORS LICENSE README* doc* romdump/ $(DESTDIR)$(DOCDIR)
	./dist/x48ng --print-config > ./dist/config.lua
	install -c -m 644 dist/config.lua $(DESTDIR)$(DOCDIR)/config.lua

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/applications
	sed "s|@PREFIX@|$(PREFIX)|g" dist/x48ng.desktop > $(DESTDIR)$(PREFIX)/share/applications/x48ng.desktop
