
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

char read_nibble_from_rom(long addr) {
	return saturn.rom[addr];
}
