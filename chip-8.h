#ifndef CHIP_8
#define CHIP_8

#include <stdint.h>
#include <stdbool.h>

#define DISPLAY_WIDTH  64
#define DISPLAY_HEIGHT 32
#define WIDTH_BYTES    (DISPLAY_WIDTH / 8)
#define HEIGHT_BYTES   (DISPLAY_HEIGHT)
#define MEMORY_SIZE    4096
#define STACK_SIZE     16

typedef struct {
    /* INTERNALS */
    uint8_t display[WIDTH_BYTES * HEIGHT_BYTES];
    uint8_t memory[MEMORY_SIZE];

    uint16_t stack[STACK_SIZE];

    uint8_t V[16];

    uint8_t DT;
    uint8_t ST;

    uint16_t I;

    uint8_t SP;
    uint16_t PC;

    /* CONFIGURATION RELATED */
    bool vf_reset;
    bool increment_index;

} Chip_8;

/* Initialize the Chip_8 struct */
void CHIP8_init(Chip_8 *machine);

/* Set the vf_reset quirk */
void CHIP8_set_vf_reset(Chip_8 *machine, bool value);

/* Set the increment_index quirk */
void CHIP8_set_increment_index(Chip_8 *machine, bool value);

/* Load a program to run */
void CHIP8_load(Chip_8 *machine, const char *file_path);

/* Perform one fetch, execute, and decode cycle */
void CHIP8_tick(Chip_8 *machine);

/* Decrement the timers
 * Call 60 times per second
 */
void CHIP8_decrement_timers(Chip_8 *machine);

#endif // !CHIP_8
