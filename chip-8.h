#ifndef CHIP_8
#define CHIP_8

#include <stdint.h>

#define DISPLAY_WIDTH  64
#define DISPLAY_HEIGHT 32
#define WIDTH_BYTES    (DISPLAY_WIDTH / 8)
#define HEIGHT_BYTES   (DISPLAY_HEIGHT)
#define MEMORY_SIZE    4096
#define STACK_SIZE     16

typedef struct {
    uint8_t display[WIDTH_BYTES * HEIGHT_BYTES];
    uint8_t memory[MEMORY_SIZE];

    uint16_t stack[STACK_SIZE];

    uint8_t V[16];

    uint8_t DT;
    uint8_t ST;

    uint16_t I;

    uint8_t SP;
    uint16_t PC;
} Chip_8;

void CHIP8_init(Chip_8 *machine);
void CHIP8_load(Chip_8 *machine, const char *file_path);
void CHIP8_tick(Chip_8 *machine);
void CHIP8_render(Chip_8 *machine);

#endif // !CHIP_8
