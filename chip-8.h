#ifndef CHIP_8
#define CHIP_8

#include <stdint.h>

#define DISPLAY_WIDTH  64
#define DISPLAY_HEIGHT 32
#define MEMORY_SIZE    4096
#define STACK_SIZE     16

typedef struct {
    uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    uint8_t memory[MEMORY_SIZE];

    uint16_t stack[STACK_SIZE];

    uint8_t V[16];

    uint8_t DT;
    uint8_t ST;

    uint16_t I;

    uint8_t SP;
    uint16_t PC;
} Chip_8;

void init(Chip_8 *chip_8);
void load_program(Chip_8 *chip_8, const char *file_path);
void start(Chip_8 *chip_8);

#endif // !CHIP_8
