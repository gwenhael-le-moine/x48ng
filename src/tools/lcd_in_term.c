/*
 * Emulate a LCD in an ANSI terminal such as a xterm
 */

#include <stdio.h>

struct pixel {
    int x;
    int y;
    int on;
};

struct pixel get_next_pixel_cmd() {
    struct pixel p = { 0 };

    if (scanf("%d %d %d", &p.x, &p.y, &p.on) == EOF) {
        p.on = -1;
    }

    return p;
}

void cls() {
    printf("\e[1J");
}

void printxy(int x, int y, char c) {
    printf("\e[%d;%dH %c", x, y, c);
}

int main() {

    cls();

    while (1) {
        struct pixel p = get_next_pixel_cmd();
        if (p.on == -1) break;

        printxy(p.x, p.y, p.on ? 'X' : ' ');
        fflush(stdout);
    }
}
