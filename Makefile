# Makefile to build x48ng without autotools

PREFIX = /usr
DOCDIR = $(PREFIX)/doc/x48ng
MANDIR = $(PREFIX)/man

VERSION_MAJOR = 0
VERSION_MINOR = 33
PATCHLEVEL = 0

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
dist/ROMs/sxrom-a:
	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-a.zip" --output - | funzip > "dist/ROMs/sxrom-a"
dist/ROMs/sxrom-b:
	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-b.zip" --output - | funzip > "dist/ROMs/sxrom-b"
dist/ROMs/sxrom-c:
	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-c.zip" --output - | funzip > "dist/ROMs/sxrom-c"
dist/ROMs/sxrom-d:
	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-d.zip" --output - | funzip > "dist/ROMs/sxrom-d"
dist/ROMs/sxrom-e:
	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-e.zip" --output - | funzip > "dist/ROMs/sxrom-e"
dist/ROMs/sxrom-j:
	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-j.zip" --output - | funzip > "dist/ROMs/sxrom-j"
dist/ROMs/gxrom-l:
	curl "https://www.hpcalc.org/hp48/pc/emulators/gxrom-l.zip" --output - | funzip > "dist/ROMs/gxrom-l"
dist/ROMs/gxrom-m:
	curl "https://www.hpcalc.org/hp48/pc/emulators/gxrom-m.zip" --output - | funzip > "dist/ROMs/gxrom-m"
dist/ROMs/gxrom-p:
	curl "https://www.hpcalc.org/hp48/pc/emulators/gxrom-p.zip" --output - | funzip > "dist/ROMs/gxrom-p"
dist/ROMs/gxrom-r:
	curl "https://www.hpcalc.org/hp48/pc/emulators/gxrom-r.zip" --output - | funzip > "dist/ROMs/gxrom-r"

get-roms: dist/ROMs/sxrom-a dist/ROMs/sxrom-b dist/ROMs/sxrom-c dist/ROMs/sxrom-d dist/ROMs/sxrom-e dist/ROMs/sxrom-j dist/ROMs/gxrom-l dist/ROMs/gxrom-m dist/ROMs/gxrom-p dist/ROMs/gxrom-r

install: all get-roms
	install -m 755 -d -- $(DESTDIR)$(PREFIX)/bin
	install -c -m 755 dist/x48ng $(DESTDIR)$(PREFIX)/bin/x48ng

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/x48ng
	install -c -m 755 dist/mkcard $(DESTDIR)$(PREFIX)/share/x48ng/mkcard
	install -c -m 755 dist/dump2rom $(DESTDIR)$(PREFIX)/share/x48ng/dump2rom
	install -c -m 755 dist/checkrom $(DESTDIR)$(PREFIX)/share/x48ng/checkrom
	install -c -m 644 dist/hplogo.png $(DESTDIR)$(PREFIX)/share/x48ng/hplogo.png
	cp -R dist/ROMs/ $(DESTDIR)$(PREFIX)/share/x48ng/
	sed "s|@PREFIX@|$(PREFIX)|g" dist/setup-x48ng-home.sh > $(DESTDIR)$(PREFIX)/share/x48ng/setup-x48ng-home.sh
	chmod 755 $(DESTDIR)$(PREFIX)/share/x48ng/setup-x48ng-home.sh

	install -m 755 -d -- $(DESTDIR)$(MANDIR)/man1
	sed "s|@VERSION@|$(VERSION_MAJOR).$(VERSION_MINOR).$(PATCHLEVEL)|g" dist/x48ng.man.1 > $(DESTDIR)$(MANDIR)/man1/x48ng.1
	gzip -9  $(DESTDIR)$(MANDIR)/man1/x48ng.1

	install -m 755 -d -- $(DESTDIR)$(DOCDIR)
	cp -R AUTHORS LICENSE README* doc* romdump/ $(DESTDIR)$(DOCDIR)
	dist/x48ng --print-config > dist/config.lua
	install -c -m 644 dist/config.lua $(DESTDIR)$(DOCDIR)/config.lua

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/applications
	sed "s|@PREFIX@|$(PREFIX)|g" dist/x48ng.desktop > $(DESTDIR)$(PREFIX)/share/applications/x48ng.desktop
