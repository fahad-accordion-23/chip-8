#include "chip-8.h"
#include <unistd.h>

int main(void) {
    Chip_8 machine = { 0 };

    CHIP8_init(&machine);
    CHIP8_load(&machine, "./ROMs/IBM Logo.ch8");

    while (1) {
        CHIP8_tick(&machine);
        CHIP8_render(&machine);

        usleep(16 * 1000);
    }

    return 0;
}

