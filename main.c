#include "chip-8.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    const char *file_path;

    if (argc >= 2)
        file_path = argv[1];
    else {
        fprintf(stderr, "Please provide a file path to the program you wish to run!\n");
        return 1;
    }

    Chip_8 machine = { 0 };

    CHIP8_init(&machine);
    CHIP8_load(&machine, file_path);

    while (1) {
        CHIP8_tick(&machine);
        CHIP8_render(&machine);

        usleep(16 * 1000);
    }

    return 0;
}

