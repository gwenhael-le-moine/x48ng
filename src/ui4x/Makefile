# Makefile

FULL_WARNINGS = no
WITH_GTK = yes
WITH_SDL = yes
WITH_SDL2 = no

TARGETS = ui4x
VERSION_MAJOR = 0
VERSION_MINOR = 0
PATCHLEVEL = 0

PKG_CONFIG ?= pkg-config

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
	-fsanitize-trap \
	$(call cc-option,-Wunused-variable)
endif

### Text UI
NCURSES_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags ncursesw) -DNCURSES_WIDECHAR=1 -DHAS_NCURSES=1
NCURSES_LIBS = $(shell "$(PKG_CONFIG)" --libs ncursesw)
NCURSES_SRC = src/ncurses.c
NCURSES_HEADERS = src/ncurses.h

### SDL UI
ifeq ($(WITH_SDL), yes)
	SDL_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags sdl3) -DHAS_SDL=1
	SDL_LIBS = $(shell "$(PKG_CONFIG)" --libs sdl3)
	SDL_SRC = src/sdl.c
	SDL_HEADERS = src/sdl.h
endif

ifeq ($(WITH_SDL2), yes)
	SDL_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags sdl2) -DHAS_SDL=1 -DHAS_SDL2=1
	SDL_LIBS = $(shell "$(PKG_CONFIG)" --libs sdl2)
	SDL_SRC = src/sdl.c
	SDL_HEADERS = src/sdl.h
endif

ifeq ($(WITH_GTK), yes)
	GTK_CFLAGS = -DHAS_GTK=1 $(shell "$(PKG_CONFIG)" --cflags gtk4)
	GTK_LIBS = $(shell "$(PKG_CONFIG)" --libs gtk4)
	GTK_SRC = src/gtk.c
	GTK_HEADERS = src/gtk.h
endif

override CFLAGS := -g -pg -std=c2x \
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
	$(SDL_CFLAGS) \
	$(GTK_CFLAGS) \
	$(NCURSES_CFLAGS) \
	$(CFLAGS)

override CPPFLAGS := -I./example/ -I./src/ -D_GNU_SOURCE=1 \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	$(CPPFLAGS)

LIBS = -lm \
	$(SDL_LIBS) \
	$(GTK_LIBS) \
	$(NCURSES_LIBS)

HEADERS = example/options.h \
	example/emulator_api.h \
	src/api.h \
	src/bitmaps_misc.h \
	src/inner.h \
	$(SDL_HEADERS) \
	$(GTK_HEADERS) \
	$(NCURSES_HEADERS)


SRC = $(NCURSES_SRC) \
	$(GTK_SRC) \
	$(SDL_SRC) \
	src/48gx.c \
	src/48sx.c \
	src/49g.c \
	src/40g.c \
	src/50g.c \
	src/api.c \
	src/bitmaps_misc.c \
	src/fonts.c \
	example/emulator_api.c \
	example/options.c \
	example/main.c
OBJS = $(SRC:.c=.o)

.PHONY: all clean clean-all pretty-code install mrproper

all: $(TARGETS)

ui4x: $(OBJS)

# Binaries
$(TARGETS):
	$(CC) $^ -o $@ $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LIBS)

# Cleaning
clean:
	rm -f $(OBJS)

mrproper: clean
	rm -f $(TARGETS)

clean-all: mrproper

# Formatting
pretty-code:
	clang-format -i example/*.c example/*.h src/*.c src/*.h
