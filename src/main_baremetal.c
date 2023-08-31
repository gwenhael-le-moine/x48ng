#include <stdio.h>

#include "hp48.h"
#include "x48.h"
#include "x48_resources.h"

static int
sample_putc(char c, FILE *file)
{
    (void) file;		/* Not used in this function */
    return c;
}

static int
sample_getc(FILE *file)
{
	return EOF;
}

static FILE __stdio = FDEV_SETUP_STREAM(sample_putc,
					sample_getc,
					NULL,
					_FDEV_SETUP_RW);

FILE *const __iob[3] = { &__stdio, &__stdio, &__stdio };

#if _PICOLIBC_MINOR__ >= 7
FILE *const stdin = __iob[0];
FILE *const stdout = __iob[1];
FILE *const stderr = __iob[2];
#endif

int main() {
    init_emulator();
    serial_init();
    init_display();
    emulate();
}
