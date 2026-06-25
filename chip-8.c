#include "chip-8.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

SDL_Scancode get_scancode_from_key(uint8_t key) {
    switch (key) {
        case 0x0: return SDL_SCANCODE_X;
        case 0x1: return SDL_SCANCODE_1;
        case 0x2: return SDL_SCANCODE_2;
        case 0x3: return SDL_SCANCODE_3;
        case 0x4: return SDL_SCANCODE_Q;
        case 0x5: return SDL_SCANCODE_W;
        case 0x6: return SDL_SCANCODE_E;
        case 0x7: return SDL_SCANCODE_A;
        case 0x8: return SDL_SCANCODE_S;
        case 0x9: return SDL_SCANCODE_D;
        case 0xA: return SDL_SCANCODE_Z;
        case 0xB: return SDL_SCANCODE_C;
        case 0xC: return SDL_SCANCODE_4;
        case 0xD: return SDL_SCANCODE_R;
        case 0xE: return SDL_SCANCODE_F;
        case 0xF: return SDL_SCANCODE_V;

        default : return SDL_SCANCODE_UNKNOWN;
    }
}

void CHIP8_init(Chip_8 *machine) {
    memset(machine->display, 0, sizeof(machine->display));
    memset(machine->memory, 0, sizeof(machine->memory));
    memset(machine->V, 0, sizeof(machine->V));

    machine->DT = 0;
    machine->ST = 0;
    machine->I  = 0;
    machine->SP = 0;
    machine->PC = 0;

    machine->vf_reset = false;
    machine->increment_index = false;

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

void CHIP8_set_vf_reset(Chip_8 *machine, bool value) {
    machine->vf_reset = true;
}

void CHIP8_set_increment_index(Chip_8 *machine, bool value) {
    machine->increment_index = true;
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
            switch (nnn) {
                // 00E0 - CLS
                case 0x0E0:
                    memset(machine->display, 0, sizeof(machine->display));
                    break;

                // 00EE - RET
                case 0x0EE:
                    machine->PC  = machine->stack[machine->SP];
                    machine->SP -= 1;
                    break;

                // 0nnn - SYS addr
                default:
                    break;
            }

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
                    uint8_t Vf = 0;

                    machine->V[x]   = machine->V[x] | machine->V[y];

                    if (machine->vf_reset)
                        machine->V[0xF] = Vf;

                    break;
                }

                // 8xy2 - AND Vx, Vy
                case 0x2: {
                    uint8_t Vf = 0;

                    machine->V[x]   = machine->V[x] & machine->V[y];

                    if (machine->vf_reset)
                        machine->V[0xF] = Vf;

                    break;
                }

                // 8xy3 - XOR Vx, Vy
                case 0x3: {
                    uint8_t Vf = 0;

                    machine->V[x]   = machine->V[x] ^ machine->V[y];

                    if (machine->vf_reset)
                        machine->V[0xF] = Vf;

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
                    uint8_t Vf = (machine->V[x] >= machine->V[y]) ? 1 : 0;

                    machine->V[x]   = machine->V[x] - machine->V[y];
                    machine->V[0xF] = Vf;

                    break;
                }

                // 8xy6 - SHR Vx {, Vy}
                case 0x6: {
                    uint8_t Vf = (machine->V[x] & 1) ? 1 : 0;

                    machine->V[x]   = machine->V[x] >> 1;
                    machine->V[0xF] = Vf;

                    break;
                }

                // 8xy7 - SUBN Vx, Vy
                case 0x7: {
                    uint8_t Vf = (machine->V[y] >= machine->V[x]) ? 1 : 0;

                    machine->V[x]   = machine->V[y] - machine->V[x];
                    machine->V[0xF] = Vf;

                    break;
                }

                // 8xyE - SHL Vx {, Vy}
                case 0xE: {
                    uint8_t Vf = (machine->V[x] & 0x80) ? 1 : 0;

                    machine->V[x]   = machine->V[x] << 1;
                    machine->V[0xF] = Vf;

                    break;
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

            break;
        }

        case 0xE: {
            const bool *key_states = SDL_GetKeyboardState(NULL);
            SDL_Scancode key = get_scancode_from_key(machine->V[x]);
            bool key_pressed = key_states[key];

            switch (kk) {
                // Ex9E - SKP Vx
                case 0x9E: {
                    if (key_pressed) {
                        machine->PC += 2;
                    }

                    break;
                }

                // ExA1 - SKNP Vx
                case 0xA1: {
                    if (!key_pressed) {
                        machine->PC += 2;
                    }

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

                // Fx0A - LD Vx, K
                case 0x0A: {
                    // TODO: clean up & bug fix (issue #01) are related
                    const bool *key_states = SDL_GetKeyboardState(NULL);
                    uint8_t key;

                    if (key_states[SDL_SCANCODE_X]) { key = 0x0; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_1]) { key = 0x1; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_2]) { key = 0x2; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_3]) { key = 0x3; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_Q]) { key = 0x4; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_W]) { key = 0x5; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_E]) { key = 0x6; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_A]) { key = 0x7; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_S]) { key = 0x8; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_D]) { key = 0x9; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_Z]) { key = 0xA; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_C]) { key = 0xB; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_4]) { key = 0xC; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_R]) { key = 0xD; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_F]) { key = 0xE; goto key_pressed; }
                    if (key_states[SDL_SCANCODE_V]) { key = 0xF; goto key_pressed; }

                    machine->PC -= 2;
                    break;

key_pressed:        machine->V[x] = key;
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
                    for (int i = 0; i <= x; i++) {
                        if (machine->increment_index) {
                            machine->memory[machine->I] = machine->V[i];
                            machine->I += 1;
                        } else {
                            machine->memory[machine->I + i] = machine->V[i];
                        }
                    }

                    break;
                }

                // Fx65 - LD Vx, [I]
                case 0x65: {
                    for (int i = 0; i <= x; i++) {
                        if (machine->increment_index) {
                            machine->V[i] = machine->memory[machine->I];
                            machine->I += 1;
                        } else {
                            machine->V[i] = machine->memory[machine->I + i];
                        }
                    }

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

