#include "chip-8.h"

int main(void) {
    Chip_8 chip_8 = { 0 };

    init(&chip_8);
    load_program(&chip_8, "./games/IBM Logo.ch8");
    start(&chip_8);

    return 0;
}

