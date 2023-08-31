# Makefile to build x48ng without autotools

# possible values: x11, sdl1
GUI ?= x11

# possible values: yes, no
WITH_DEBUGGER ?= yes

VERSION_MAJOR = 0
VERSION_MINOR = 12
PATCHLEVEL = 1
COMPILE_VERSION = 0

CC = gcc

OPTIM=2
CFLAGS = -g -O$(OPTIM) -I./src/ -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DPATCHLEVEL=$(PATCHLEVEL) -DCOMPILE_VERSION=$(COMPILE_VERSION)
LIBS = -lm

OS_TYPE=unix
ifeq ($(GUI), x11)
	CFLAGS += $(shell pkg-config --cflags x11 xext) -D_GNU_SOURCE=1 -DGUI_IS_X11=1
	LIBS += $(shell pkg-config --libs x11 xext)
endif
ifeq ($(GUI), sdl1)
	CFLAGS += $(shell pkg-config --cflags SDL_gfx sdl12_compat) -DGUI_IS_SDL1=1
	LIBS += $(shell pkg-config --libs SDL_gfx sdl12_compat)
endif
ifeq ($(GUI), baremetal)
	CC = riscv64-unknown-elf-gcc
	CFLAGS += -march=rv32e -mabi=ilp32e --specs=picolibc.specs -DGUI_IS_BAREMETAL=1
	LIBS += -Wl,--print-memory-usage
	LIBS +=-T baremetal.ld
	OS_TYPE=baremetal
# There is no readline in baremetal
	WITH_DEBUGGER=no
endif

FULL_WARNINGS = no
ifeq ($(FULL_WARNINGS), yes)
	CFLAGS += -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Wconversion -Wdouble-promotion -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap
endif

DOTOS = src/main_$(OS_TYPE).o \
	src/hp48_device.o \
	src/hp48_emulate.o \
	src/hp48_init_$(OS_TYPE).o \
	src/hp48_serial_$(OS_TYPE).o \
	src/hp48emu_actions.o \
	src/hp48emu_memory.o \
	src/hp48emu_register.o \
	src/romio.o \
	src/timer_$(OS_TYPE).o \
	src/x48_errors.o \
	src/x48_resources.o \
	src/x48_$(OS_TYPE).o

ifeq ($(WITH_DEBUGGER), yes)
	DOTOS += src/x48_debugger.o \
		 src/x48_debugger_disasm.o \
		 src/x48_debugger_rpl.o
	CFLAGS += $(shell pkg-config --cflags readline) -DWITH_DEBUGGER=1
	LIBS += $(shell pkg-config --libs readline)
endif

.PHONY: all clean clean-all pretty-code install

all: dist/x48ng
ifneq ($(GUI), baremetal)
all: dist/mkcard dist/checkrom dist/dump2rom
endif

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

clean-all: clean
	rm -f dist/mkcard dist/checkrom dist/dump2rom dist/x48ng

# Formatting
pretty-code:
	clang-format -i src/*.c src/*.h

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

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/applications
	sed "s|@PREFIX@|$(PREFIX)|g" dist/x48ng.desktop > $(DESTDIR)$(PREFIX)/share/applications/x48ng.desktop

	install -m 755 -d -- $(DESTDIR)/etc/X11/app-defaults
	install -c -m 644 dist/X48NG.ad $(DESTDIR)/etc/X11/app-defaults/X48NG
