#include "chip-8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void init(Chip_8 *chip_8) {
    memset(chip_8->display, 0, sizeof(chip_8->display));
    memset(chip_8->memory, 0, sizeof(chip_8->memory));
    memset(chip_8->V, 0, sizeof(chip_8->V));

    chip_8->DT = 0;
    chip_8->ST = 0;
    chip_8->I  = 0;
    chip_8->SP = 0;
    chip_8->PC = 0;

    /* load fonts */
    uint8_t sprites[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80, // F
    };

    memcpy(&chip_8->memory[0x0], sprites, sizeof(sprites));
}

void load_program(Chip_8 *chip_8, const char *file_path) {
    FILE *file = fopen(file_path, "rb");

    if (!file) {
        fprintf(stderr, "Failed to open file: %s", file_path);
        exit(1);
    }

    fread(&chip_8->memory[0x200], 1, MEMORY_SIZE - 0x200, file);
    fclose(file);

    chip_8->PC = 0x200;
}

void start(Chip_8 *chip_8) {
    while (1) {
        uint16_t instruction = chip_8->memory[chip_8->PC];
        instruction = instruction << 8;
        instruction = instruction | chip_8->memory[chip_8->PC + 1];
        chip_8->PC += 2;

        uint16_t type = (instruction & 0xF000) >> 12;
        uint8_t x    = (instruction & 0x0F00) >> 8;
        uint8_t y    = (instruction & 0x00F0) >> 4;
        uint8_t n    = (instruction & 0x000F);
        uint8_t kk   = (uint8_t) (instruction & 0x00FF);
        uint16_t nnn  = (instruction & 0x0FFF);

        switch (type) {
            case 0x0:

            // 00E0 - CLS
            if (kk == 0xE0) {
                // printf("CLS\n");
                memset(chip_8->display, 0, sizeof(chip_8->display));
            }

            // 00EE - RET
            else if (kk == 0xEE)
                ;// printf("RET\n");

            // 0nnn - SYS addr
            else
                ;// printf("SYS addr\n");

            break;

            // 1nnn - JP addr
            case 0x1:
            // printf("JP addr\n");
            chip_8->PC = nnn;

            break;

            case 0x2:
            // printf("CALL addr\n");
            break;

            case 0x3:
            // printf("SE Vx, byte\n");
            break;

            case 0x4:
            // printf("SNE Vx, byte\n");
            break;

            case 0x5:
            // printf("SE Vx, Vy\n");
            break;

            // 6xkk - LD Vx, byte
            case 0x6:
            // printf("LD Vx, byte\n");
            chip_8->V[x] = kk;

            break;

            // 7xnn - ADD Vx, byte
            case 0x7:
            // printf("ADD Vx, byte\n");
            chip_8->V[x] = chip_8->V[x] + kk;

            break;

            case 0x8:
            {
            }
            break;

            case 0x9:
            break;

            // Annn - LD I, addr
            case 0xA:
            // printf("LD I, addr\n");
            chip_8->I = nnn;

            break;

            case 0xB:
            break;

            case 0xC:
            break;

            // Dxyn - DRW Vx, Vy, nibble
            case 0xD:
            // printf("DRW Vx, Vy, nibble\n");
            {
                uint8_t cx = chip_8->V[x];
                uint8_t cy = chip_8->V[y];

                cx &= DISPLAY_WIDTH - 1;    // mod by 63
                cy &= DISPLAY_HEIGHT - 1;   // mod by 31

                for (int i = 0; i < n; i++) {
                    uint8_t byte = chip_8->memory[chip_8->I + i];

                    uint8_t here = (byte >> (cx & 7));
                    uint8_t there = (byte << (8 - (cx & 7)));

                    if (cy + i >= DISPLAY_HEIGHT)
                        break;

                    int location_l = (cy + i) * WIDTH_BYTES + (cx / 8);
                    int location_r = (cy + i) * WIDTH_BYTES + ((cx + 8) / 8);
                    chip_8->display[location_l] ^= here;
                    chip_8->display[location_r] ^= there;

                }
                printf("\n");

            }

            break;

            case 0xE:
            {
            }
            break;

            case 0xF:
            {
            }
            break;

            default:
            fprintf(stderr, "Invalid instruction: %d. Exitting...\n", instruction);
            exit(1);
        }

        // temporary rendering method
        printf("\x1B[2J\x1B[H");

        for (int i = 0; i < HEIGHT_BYTES; i++) {
            for (int j = 0; j < WIDTH_BYTES; j++) {

                for (int l = sizeof(uint8_t) * 8 - 1; l >= 0; l--) {
                    int a = (chip_8->display[i * WIDTH_BYTES + j] >> l) & 1 ;

                    if (a == 0)
                        printf(" ");
                    else
                        printf("#");
                }

            }
            printf("\n");
        }

        usleep(16 * 1000);
    }
}
