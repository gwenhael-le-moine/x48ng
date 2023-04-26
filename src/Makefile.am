## Process this file with automake to produce Makefile.in

bin_PROGRAMS = x48 dump2rom mkcard checkrom

x48_SOURCES = actions.c debugger.c device.c \
              disasm.c emulate.c errors.c init.c \
              lcd.c main.c memory.c options.c register.c \
              resources.c romio.c rpl.c serial.c timer.c \
              x48_x11.c \
              annunc.h append.h buttons.h constants.h debugger.h \
              device.h disasm.h errors.h global.h hp48char.h hp48_emu.h \
              hp48.h hp.h icon.h mmu.h options.h resources.h romio.h rpl.h \
              small.h timer.h version.h X48.ad.h x48_x11.h

dump2rom_SOURCES = dump2rom.c

mkcard_SOURCES = mkcard.c

checkrom_SOURCES = checkrom.c romio.c

man1_MANS = x48.man

EXTRA_DIST = $(man1_MANS) X48.ad

AM_CPPFLAGS = $(X_CFLAGS)

x48_LDADD = $(X_LIBS)
