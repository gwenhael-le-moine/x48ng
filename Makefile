# Makefile to build x48 without autotools

CC = gcc

CFLAGS = -g -O2 -Wall
LIBS = -lm -lX11 -lXext -lhistory -lreadline

all: mkcard checkrom dump2rom x48

# Binaries
mkcard: src/mkcard.o
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

dump2rom: src/dump2rom.o
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

checkrom: src/checkrom.o src/romio.o
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

x48: src/main.o src/actions.o src/debugger.o src/device.o src/disasm.o src/emulate.o src/errors.o src/init.o src/lcd.o src/memory.o src/register.o src/resources.o src/romio.o src/rpl.o src/serial.o src/timer.o src/x48_x11.o src/options.o src/resources.o
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

# Cleaning
clean:
	rm -f src/*.o

clean-all: clean
	rm -f x48 mkcard checkrom dump2rom

# Formatting
pretty-code:
	clang-format -i src/*.c src/*.h
