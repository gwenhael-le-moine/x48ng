# Makefile to build x48ng without autotools
#
# The cc-option function and the C{,PP}FLAGS logic were copied from the
# fsverity-utils project.
# https://git.kernel.org/pub/scm/fs/fsverity/fsverity-utils.git/
# The governing license can be found in the LICENSE file or at
# https://opensource.org/license/MIT.

TARGETS = dist/x48ng dist/x48ng-checkrom dist/x48ng-dump2rom

VERSION_MAJOR = 0
VERSION_MINOR = 50
PATCHLEVEL = 0

PREFIX ?= /usr
DOCDIR ?= $(PREFIX)/doc/x48ng
MANDIR ?= $(PREFIX)/man

LUA_VERSION ?= lua
PKG_CONFIG ?= pkg-config

OPTIM ?= 2
FULL_WARNINGS ?= no
WITH_SDL ?= yes

MAKEFLAGS +=-j$(NUM_CORES) -l$(NUM_CORES)

cc-option = $(shell if $(CC) $(1) -c -x c /dev/null -o /dev/null > /dev/null 2>&1; \
		  then echo $(1); fi)

### lua
LUA_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags $(LUA_VERSION))
LUA_LIBS = $(shell "$(PKG_CONFIG)" --libs $(LUA_VERSION))

### debugger
DEBUG_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags readline)
DEBUG_LIBS = $(shell "$(PKG_CONFIG)" --libs readline)

### Text UI
NCURSES_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags ncursesw) -DNCURSES_WIDECHAR=1
NCURSES_LIBS = $(shell "$(PKG_CONFIG)" --libs ncursesw)

### SDL UI
ifeq ($(WITH_SDL), yes)
	SDL_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags sdl2) -DHAS_SDL=1
	SDL_LIBS = $(shell "$(PKG_CONFIG)" --libs sdl2)

	SDL_SRC = src/ui_sdl.c
endif

LIBS = -lm \
	$(LUA_LIBS) \
	$(DEBUG_LIBS) \
	$(NCURSES_LIBS) \
	$(SDL_LIBS)

HEADERS = src/debugger.h \
	src/emulator_core.h \
	src/emulator_for_debugger.h \
	src/emulator_inner.h \
	src/options.h \
	src/romio.h \
	src/ui.h \
	src/ui_bitmaps_big_font.h \
	src/ui_bitmaps_misc.h \
	src/ui_bitmaps_small_font.h \
	src/ui_inner.h

SRC = src/emu_serial.c \
	src/emu_emulate.c \
	src/emu_init.c \
	src/emu_keyboard.c \
	src/emu_memory.c \
	src/emu_register.c \
	src/emu_timer.c \
	src/debugger.c \
	src/options.c \
	src/romio.c \
	src/ui_text.c \
	src/ui.c \
	$(SDL_SRC) \
	src/main.c
OBJS = $(SRC:.c=.o)

ifeq ($(FULL_WARNINGS), no)
EXTRA_WARNING_CFLAGS := -Wno-unused-function \
	-Wno-redundant-decls \
	$(call cc-option,-Wno-maybe-uninitialized) \
	$(call cc-option,-Wno-discarded-qualifiers) \
	$(call cc-option,-Wno-uninitialized) \
	$(call cc-option,-Wno-ignored-qualifiers)
else
EXTRA_WARNING_CFLAGS := -Wunused-function \
	-Wredundant-decls \
	-fsanitize=thread \
	$(call cc-option,-Wunused-variable)
endif

override CFLAGS := -std=c11 \
	-Wall -Wextra -Wpedantic \
	-Wformat=2 -Wshadow \
	-Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
	-Wnested-externs -Wmissing-include-dirs \
	-Wdouble-promotion \
	-Wno-sign-conversion \
	-Wno-unused-variable \
	-Wno-unused-parameter \
	-Wno-conversion \
	-Wno-format-nonliteral \
	$(call cc-option,-Wjump-misses-init) \
	$(call cc-option,-Wlogical-op) \
	$(call cc-option,-Wno-unknown-warning-option) \
	$(EXTRA_WARNING_CFLAGS) \
	$(SDL_CFLAGS) \
	$(LUA_CFLAGS) \
	$(NCURSES_CFLAGS) \
	$(DEBUG_CFLAGS) \
	-O$(OPTIM) \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	-I./src/ -D_GNU_SOURCE=1 \
	$(CFLAGS)

# depfiles = $(objects:.o=.d)

# # Have the compiler output dependency files with make targets for each
# # of the object files. The `MT` option specifies the dependency file
# # itself as a target, so that it's regenerated when it should be.
# %.dep.mk: %.c
#	$(CC) -M -MP -MT '$(<:.c=.o) $@' $(CPPFLAGS) $< > $@

# # Include each of those dependency files; Make will run the rule above
# # to generate each dependency file (if it needs to).
# -include $(depfiles)

.PHONY: all clean clean-all pretty-code mrproper install uninstall

all: $(TARGETS)

dist/x48ng-dump2rom: src/legacy_tools/dump2rom.o
dist/x48ng-checkrom: src/legacy_tools/checkrom.o src/romio.o
dist/x48ng: $(OBJS) $(HEADERS)

# Binaries
$(TARGETS):
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

# Cleaning
clean:
	rm -f src/*.o src/legacy_tools/*.o src/*.dep.mk src/legacy_tools/*.dep.mk

mrproper: clean
	rm -f $(TARGETS)
	make -C dist/ROMs mrproper

clean-all: mrproper

# Formatting
pretty-code:
	clang-format -i src/*.c src/*.h src/legacy_tools/*.c

# Installing
get-roms:
	make -C dist/ROMs

dist/config.lua: dist/x48ng
	$^ --print-config > $@

install: all dist/config.lua
	install -m 755 -d -- $(DESTDIR)$(PREFIX)/bin
	install -c -m 755 dist/x48ng $(DESTDIR)$(PREFIX)/bin/x48ng

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/x48ng
	install -c -m 644 dist/hplogo.png $(DESTDIR)$(PREFIX)/share/x48ng/hplogo.png
	cp -R dist/ROMs/ $(DESTDIR)$(PREFIX)/share/x48ng/
	install -c -m 755 dist/setup-x48ng-home.sh $(DESTDIR)$(PREFIX)/share/x48ng/setup-x48ng-home.sh
	chmod 755 $(DESTDIR)$(PREFIX)/share/x48ng/setup-x48ng-home.sh

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/libexec
	install -c -m 755 dist/x48ng-dump2rom $(DESTDIR)$(PREFIX)/libexec/x48ng-dump2rom
	install -c -m 755 dist/x48ng-checkrom $(DESTDIR)$(PREFIX)/libexec/x48ng-checkrom

	install -m 755 -d -- $(DESTDIR)$(MANDIR)/man1
	sed "s|@VERSION@|$(VERSION_MAJOR).$(VERSION_MINOR).$(PATCHLEVEL)|g" dist/x48ng.man.1 > $(DESTDIR)$(MANDIR)/man1/x48ng.1

	install -m 755 -d -- $(DESTDIR)$(DOCDIR)
	cp -R AUTHORS LICENSE README* doc* romdump/ $(DESTDIR)$(DOCDIR)
	install -c -m 644 dist/config.lua $(DESTDIR)$(DOCDIR)/config.lua

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/applications
	sed "s|@PREFIX@|$(PREFIX)|g" dist/x48ng.desktop > $(DESTDIR)$(PREFIX)/share/applications/x48ng.desktop

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/x48ng
	rm -f $(DESTDIR)$(PREFIX)/libexec/x48ng-dump2rom
	rm -f $(DESTDIR)$(PREFIX)/libexec/x48ng-checkrom
	rm -f $(DESTDIR)$(MANDIR)/man1/x48ng.1
	rm -f $(DESTDIR)$(PREFIX)/share/applications/x48ng.desktop

	rm -fr $(DESTDIR)$(PREFIX)/share/x48ng/
	rm -fr $(DESTDIR)$(DOCDIR)
