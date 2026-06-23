#include "chip-8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void CHIP8_init(Chip_8 *machine) {
    memset(machine->display, 0, sizeof(machine->display));
    memset(machine->memory, 0, sizeof(machine->memory));
    memset(machine->V, 0, sizeof(machine->V));

    machine->DT = 0;
    machine->ST = 0;
    machine->I  = 0;
    machine->SP = 0;
    machine->PC = 0;

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

    memcpy(&machine->memory[0x0], sprites, sizeof(sprites));
}

void CHIP8_load(Chip_8 *machine, const char *file_path) {
    FILE *file = fopen(file_path, "rb");

    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", file_path);
        exit(1);
    }

    fread(&machine->memory[0x200], 1, MEMORY_SIZE - 0x200, file);
    fclose(file);

    machine->PC = 0x200;
}

void CHIP8_tick(Chip_8 *machine) {
    /* FETCH */
    uint16_t instruction = machine->memory[machine->PC];
    instruction <<= 8;
    instruction |= machine->memory[machine->PC + 1];
    machine->PC += 2;

    /* pre-extracted parts of the instruction for convenience */
    uint16_t type = (instruction & 0xF000u) >> 12;
    uint8_t  x    = (instruction & 0x0F00u) >> 8;
    uint8_t  y    = (instruction & 0x00F0u) >> 4;
    uint8_t  n    = (instruction & 0x000Fu);
    uint8_t  kk   = (instruction & 0x00FFu);
    uint16_t nnn  = (instruction & 0x0FFFu);


    /* DECODE & EXECUTE */
    switch (type) {
        case 0x0: {
            // 00E0 - CLS
            if (kk == 0xE0) {
                memset(machine->display, 0, sizeof(machine->display));
            }

            // 00EE - RET
            else if (kk == 0xEE) {
                machine->PC = machine->stack[machine->SP];
                machine->SP -= 1;
            }

            // 0nnn - SYS addr
            else ;

            break;
        }

        // 1nnn - JP addr
        case 0x1: {
            machine->PC = nnn;

            break;
        }

        // 2nnn - CALL addr
        case 0x2: {
            machine->SP += 1;
            machine->stack[machine->SP] = machine->PC;
            machine->PC = nnn;

            break;
        };

        // 3xkk - SE Vx, byte
        case 0x3: {
            if (machine->V[x] == kk)
                machine->PC += 2;

            break;
        }

        // 4xkk - SNE Vx, byte
        case 0x4: {
            if (machine->V[x] != kk)
                machine->PC += 2;

            break;
        }

        // 5xy0 - SE Vx, Vy
        case 0x5: {
            if (n != 0)
                goto invalid_instruction;

            if (machine->V[x] == machine->V[y])
                machine->PC += 2;

            break;
        }

        // 6xkk - LD Vx, byte
        case 0x6: {
            machine->V[x] = kk;

            break;
        }

        // 7xnn - ADD Vx, byte
        case 0x7: {
            machine->V[x] = machine->V[x] + kk;

            break;
        }

        case 0x8: {
            switch (n) {
                // 8xy0 - LD Vx, Vy
                case 0x0: {
                    machine->V[x] = machine->V[y];

                    break;
                }

                // 8xy1 - OR Vx, Vy
                case 0x1: {
                    machine->V[x] = machine->V[x] | machine->V[y];

                    break;
                }

                // 8xy2 - AND Vx, Vy
                case 0x2: {
                    machine->V[x] = machine->V[x] & machine->V[y];

                    break;
                }

                // 8xy3 - XOR Vx, Vy
                case 0x3: {
                    machine->V[x] = machine->V[x] ^ machine->V[y];

                    break;
                }

                // 8xy4 - ADD Vx, Vy
                case 0x4: {
                    uint16_t result = machine->V[x] + machine->V[y];
                    machine->V[x]   = (uint8_t) result;
                    machine->V[0xF] = (result > 255) ? 1 : 0;

                    break;
                }

                // 8xy5 - SUB Vx, Vy
                case 0x5: {
                    machine->V[0xF] = (machine->V[x] > machine->V[y]) ? 1 : 0;
                    machine->V[x]   = machine->V[x] - machine->V[y];

                    break;
                }

                // 8xy6 - SHR Vx {, Vy}
                case 0x6: {
                    machine->V[0xF] = (machine->V[x] & 1) ? 1 : 0;
                    machine->V[x]   = machine->V[x] >> 1;
                }

                // 8xy7 - SUBN Vx, Vy
                case 0x7: {
                    machine->V[0xF] = (machine->V[x] > machine->V[y]) ? 1 : 0;
                    machine->V[x]   = machine->V[x] - machine->V[y];
                }

                // 8xyE - SHL Vx {, Vy}
                case 0xE: {
                    machine->V[0xF] = (machine->V[x] & 0x80) ? 1 : 0;
                    machine->V[x]   = machine->V[x] << 1;
                }

                default:
                    goto invalid_instruction;
            }

            break;
        }

        // 9xy0 - SNE Vx, Vy
        case 0x9: {
            if (n != 0x0)
                goto invalid_instruction;

            if (machine->V[x] != machine->V[y])
                machine->PC += 2;

            break;
        }

        // Annn - LD I, addr
        case 0xA: {
            machine->I = nnn;

            break;
        }

        // Bnnn - JP V0, addr
        case 0xB: {
            machine->PC = machine->V[0] + nnn;

            break;
        }

        // Cxkk - RND Vx, byte
        case 0xC: {
            uint8_t rand_int = (uint8_t) (rand() % 256) & kk;
            machine->V[x]    = rand_int;

            break;
        }

        // Dxyn - DRW Vx, Vy, nibble
        case 0xD: {
            uint8_t cx = machine->V[x];
            uint8_t cy = machine->V[y];

            cx &= DISPLAY_WIDTH - 1;    // mod by 63
            cy &= DISPLAY_HEIGHT - 1;   // mod by 31

            for (int i = 0; i < n; i++) {
                uint8_t byte = machine->memory[machine->I + i];

                uint8_t here = (byte >> (cx & 7));
                uint8_t there = (byte << (8 - (cx & 7)));

                if (cy + i >= DISPLAY_HEIGHT)
                    break;

                int location_l = (cy + i) * WIDTH_BYTES + (cx / 8);
                int location_r = (cy + i) * WIDTH_BYTES + ((cx + 8) / 8);
                machine->display[location_l] ^= here;
                machine->display[location_r] ^= there;

            }
            printf("\n");

            break;
        }

        case 0xE: {
            switch (kk) {
                // Ex9E - SKP Vx
                case 0x9E: {
                    // TODO: implement key down stuff

                    break;
                }

                // ExA1 - SKNP Vx
                case 0xA1: {
                    // TODO: implement if key up stuff

                    break;
                }

                default:
                    goto invalid_instruction;
            }

            break;
        }

        case 0xF: {
            switch (kk) {
                // Fx07 - LD Vx, DT
                case 0x07: {
                    machine->V[x] = machine->DT;

                    break;
                }

                case 0x0A: {
                    // TODO: implement wait for key press

                    break;
                }

                // Fx15 - LD DT, Vx
                case 0x15: {
                    machine->DT = machine->V[x];

                    break;
                }

                // Fx18 - LD ST, Vx
                case 0x18: {
                    machine->ST = machine->V[x];

                    break;
                }

                // Fx1E - ADD I, Vx
                case 0x1E: {
                    machine->I = machine->I + machine->V[x];

                    break;
                }

                // Fx29 - LD F, Vx
                case 0x29: {
                    uint8_t ch = machine->V[x];

                    switch (ch) {
                        case '0': machine->I =  0; break;
                        case '1': machine->I =  6; break;
                        case '2': machine->I = 12; break;
                        case '3': machine->I = 18; break;
                        case '4': machine->I = 24; break;
                        case '5': machine->I = 30; break;
                        case '6': machine->I = 36; break;
                        case '7': machine->I = 42; break;
                        case '8': machine->I = 48; break;
                        case '9': machine->I = 54; break;
                        case 'A': machine->I = 60; break;
                        case 'B': machine->I = 66; break;
                        case 'C': machine->I = 72; break;
                        case 'D': machine->I = 78; break;
                        case 'E': machine->I = 84; break;
                        case 'F': machine->I = 90; break;
                    }

                    break;
                }

                // Fx33 - LD B, Vx
                case 0x33: {
                    uint8_t num = machine->V[x];

                    machine->memory[machine->I + 2] = num % 10;
                    num /= 10;

                    machine->memory[machine->I + 1] = num % 10;
                    num /= 10;

                    machine->memory[machine->I + 0] = num % 10;
                    num /= 10;

                    break;
                }

                // Fx55 - LD [I], Vx
                case 0x55: {
                    for (int i = 0; i <= x; i++)
                        machine->memory[machine->I + i] = machine->V[i];

                    // TODO: allow to change behaviour
                    // older interpreters used to increment register I

                    break;
                }

                // Fx65 - LD Vx, [I]
                case 0x65: {
                    for (int i = 0; i <= x; i++)
                        machine->V[i] = machine->memory[machine->I + i];

                    // TODO: allow to change behaviour
                    // older interpreters used to increment register I

                    break;
                }

                default:
                    goto invalid_instruction;
            }

        break;
        }

        default:
            goto invalid_instruction;
    }

    return;

invalid_instruction:
    fprintf(stderr, "Invalid instruction: %04X\n"
                    "type: %01X\n"
                    "x: %01X\n"
                    "y: %01X\n"
                    "n: %01X\n"
                    "kk: %02X\n"
                    "nnn: %03X\n"
                    "Exiting...\n", instruction, type, x, y, n, kk, nnn);
    exit(1);
}

void CHIP8_decrement_timers(Chip_8 *machine) {
    machine->DT -= 1;
    machine->ST -= 1;
}

