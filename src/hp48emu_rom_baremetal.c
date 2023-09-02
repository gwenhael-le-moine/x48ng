
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "hp48.h"
#include "hp48emu.h"
#include "romio.h"
#include "x48.h"
#include "x48_resources.h"

long read_nibble_from_rom(long addr) {
	long b = saturn.rom[addr / 2];
	long n;
	if (addr % 2) {
		n = b & 0x0F;
	} else {
		n = (b >> 4) & 0x0F;
	}

	printf("%x %x %x\n", addr, b, n);
}
