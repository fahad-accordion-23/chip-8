#include "chip-8.h"

int main(void) {
    Chip_8 chip_8 = { 0 };
    load_program(&chip_8, "./games/INVADERS");

    return 0;
}

