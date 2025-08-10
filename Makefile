# Makefile to build x48ng without autotools
#
# The cc-option function and the C{,PP}FLAGS logic were copied from the
# fsverity-utils project.
# https://git.kernel.org/pub/scm/fs/fsverity/fsverity-utils.git/
# The governing license can be found in the LICENSE file or at
# https://opensource.org/license/MIT.

TARGETS = dist/x48ng dist/x48ng-checkrom dist/x48ng-dump2rom

PREFIX = /usr
DOCDIR = $(PREFIX)/doc/x48ng
MANDIR = $(PREFIX)/man

CFLAGS ?= -g -O2
FULL_WARNINGS = no
LUA_VERSION ?= lua
PKG_CONFIG ?= pkg-config
WITH_X11 ?= yes
WITH_SDL ?= yes
WITH_SDL2 ?= yes

VERSION_MAJOR = 0
VERSION_MINOR = 41
PATCHLEVEL = 1

DOTOS = src/emu_serial.o \
	src/emu_emulate.o \
	src/emu_init.o \
	src/emu_keyboard.o \
	src/emu_memory.o \
	src/emu_register.o \
	src/emu_timer.o \
	src/debugger.o \
	src/config.o \
	src/romio.o \
	src/ui_text.o \
	src/ui.o \
	src/main.o

MAKEFLAGS +=-j$(NUM_CORES) -l$(NUM_CORES)

cc-option = $(shell if $(CC) $(1) -c -x c /dev/null -o /dev/null > /dev/null 2>&1; \
	      then echo $(1); fi)

ifeq ($(FULL_WARNINGS), no)
EXTRA_WARNING_FLAGS := -Wno-unused-function \
	-Wno-redundant-decls \
	$(call cc-option,-Wno-maybe-uninitialized) \
	$(call cc-option,-Wno-discarded-qualifiers) \
	$(call cc-option,-Wno-uninitialized) \
	$(call cc-option,-Wno-ignored-qualifiers)
endif

ifeq ($(FULL_WARNINGS), yes)
EXTRA_WARNING_FLAGS := -Wunused-function \
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
	$(EXTRA_WARNING_FLAGS) \
	$(CFLAGS)

override CPPFLAGS := -I./src/ -D_GNU_SOURCE=1 \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	$(CPPFLAGS)

LIBS = -lm

### lua
override CFLAGS += $(shell "$(PKG_CONFIG)" --cflags $(LUA_VERSION))
LIBS += $(shell "$(PKG_CONFIG)" --libs $(LUA_VERSION))

### debugger
override CFLAGS += $(shell "$(PKG_CONFIG)" --cflags readline)
LIBS += $(shell "$(PKG_CONFIG)" --libs readline)

### Text UI
override CFLAGS += $(shell "$(PKG_CONFIG)" --cflags ncursesw) -DNCURSES_WIDECHAR=1
LIBS += $(shell "$(PKG_CONFIG)" --libs ncursesw)

### X11 UI
ifeq ($(WITH_X11), yes)
	X11CFLAGS = $(shell "$(PKG_CONFIG)" --cflags x11 xext) -D_GNU_SOURCE=1
	X11LIBS = $(shell "$(PKG_CONFIG)" --libs x11 xext)

	override CFLAGS += $(X11CFLAGS) -DHAS_X11=1
	LIBS += $(X11LIBS)
	DOTOS += src/ui_x11.o
endif

### SDL2 UI
ifeq ($(WITH_SDL), yes)
	WITH_SDL2 = yes
endif
ifeq ($(WITH_SDL2), yes)
	SDLCFLAGS = $(shell "$(PKG_CONFIG)" --cflags sdl2)
	SDLLIBS = $(shell "$(PKG_CONFIG)" --libs sdl2)

	override CFLAGS += $(SDLCFLAGS) -DHAS_SDL2=1
	LIBS += $(SDLLIBS)
	DOTOS += src/ui_sdl2.o
endif

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
dist/x48ng: $(DOTOS)

# Binaries
$(TARGETS):
	$(CC) $^ -o $@ $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LIBS)

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
