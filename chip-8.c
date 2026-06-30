#include "chip-8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Helper macros for instruction decoding */

#define GET_TYPE(instruction) ((instruction & 0xF000u) >> 12)
#define GET_X(instruction)    ((instruction & 0x0F00u) >> 8)
#define GET_Y(instruction)    ((instruction & 0x00F0u) >> 4)
#define GET_N(instruction)    (instruction & 0x000Fu)
#define GET_KK(instruction)   (instruction & 0x00FFu)
#define GET_NNN(instruction)  (instruction & 0x0FFFu)

#define GET_BYTE GET_KK
#define GET_ADDR GET_NNN

void decode_type_0(Chip_8 *machine, uint16_t instruction);
void decode_type_1(Chip_8 *machine, uint16_t instruction);
void decode_type_2(Chip_8 *machine, uint16_t instruction);
void decode_type_3(Chip_8 *machine, uint16_t instruction);
void decode_type_4(Chip_8 *machine, uint16_t instruction);
void decode_type_5(Chip_8 *machine, uint16_t instruction);
void decode_type_6(Chip_8 *machine, uint16_t instruction);
void decode_type_7(Chip_8 *machine, uint16_t instruction);
void decode_type_8(Chip_8 *machine, uint16_t instruction);
void decode_type_9(Chip_8 *machine, uint16_t instruction);
void decode_type_A(Chip_8 *machine, uint16_t instruction);
void decode_type_B(Chip_8 *machine, uint16_t instruction);
void decode_type_C(Chip_8 *machine, uint16_t instruction);
void decode_type_D(Chip_8 *machine, uint16_t instruction);
void decode_type_E(Chip_8 *machine, uint16_t instruction);
void decode_type_F(Chip_8 *machine, uint16_t instruction);

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

    memset(machine->key_state, 0, sizeof(machine->key_state));

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

    /* DECODE & EXECUTE */
    switch (GET_TYPE(instruction)) {
        case 0x0:
            decode_type_0(machine, instruction);
            break;

        case 0x1:
            decode_type_1(machine, instruction);
            break;

        case 0x2:
            decode_type_2(machine, instruction);
            break;

        case 0x3:
            decode_type_3(machine, instruction);
            break;

        case 0x4:
            decode_type_4(machine, instruction);
            break;

        case 0x5:
            decode_type_5(machine, instruction);
            break;

        case 0x6:
            decode_type_6(machine, instruction);
            break;

        case 0x7:
            decode_type_7(machine, instruction);
            break;

        case 0x8:
            decode_type_8(machine, instruction);
            break;

        case 0x9:
            decode_type_9(machine, instruction);
            break;

        case 0xA:
            decode_type_A(machine, instruction);
            break;

        case 0xB:
            decode_type_B(machine, instruction);
            break;

        case 0xC:
            decode_type_C(machine, instruction);
            break;

        case 0xD:
            decode_type_D(machine, instruction);
            break;

        case 0xE:
            decode_type_E(machine, instruction);
            break;

        case 0xF:
            decode_type_F(machine, instruction);
            break;
    }
}

void decode_type_0(Chip_8 *machine, uint16_t instruction) {
    uint16_t nnn = GET_NNN(instruction);

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
}

void decode_type_1(Chip_8 *machine, uint16_t instruction) {
    uint16_t addr = GET_ADDR(instruction);

    // 1nnn - JP addr
    machine->PC = addr;
}

void decode_type_2(Chip_8 *machine, uint16_t instruction) {
    uint16_t addr = GET_ADDR(instruction);

    // 2nnn - CALL addr
    machine->SP += 1;
    machine->stack[machine->SP] = machine->PC;
    machine->PC = addr;
}

void decode_type_3(Chip_8 *machine, uint16_t instruction) {
    uint8_t x    = GET_X(instruction);
    uint8_t byte = GET_BYTE(instruction);

    // 3xkk - SE Vx, byte
    if (machine->V[x] == byte)
        machine->PC += 2;
}

void decode_type_4(Chip_8 *machine, uint16_t instruction) {
    uint8_t x    = GET_X(instruction);
    uint8_t byte = GET_BYTE(instruction);

    // 4xkk - SNE Vx, byte
    if (machine->V[x] != byte)
        machine->PC += 2;
}

void decode_type_5(Chip_8 *machine, uint16_t instruction) {
    uint8_t n = GET_N(instruction);
    uint8_t x = GET_X(instruction);
    uint8_t y = GET_Y(instruction);

    // 5xy0 - SE Vx, Vy
    if (n == 0) {
        if (machine->V[x] == machine->V[y])
            machine->PC += 2;
    }
}

void decode_type_6(Chip_8 *machine, uint16_t instruction) {
    uint8_t x    = GET_X(instruction);
    uint8_t byte = GET_BYTE(instruction);

    // 6xkk - LD Vx, byte
    machine->V[x] = byte;
}

void decode_type_7(Chip_8 *machine, uint16_t instruction) {
    uint8_t x    = GET_X(instruction);
    uint8_t byte = GET_BYTE(instruction);

    // 7xnn - ADD Vx, byte
    machine->V[x] = machine->V[x] + byte;
}

void decode_type_8(Chip_8 *machine, uint16_t instruction) {
    uint8_t x = GET_X(instruction);
    uint8_t y = GET_Y(instruction);
    uint8_t n = GET_N(instruction);

    uint8_t  VF;
    uint16_t result;

    switch (n) {
        // 8xy0 - LD Vx, Vy
        case 0x0:
            machine->V[x] = machine->V[y];
            break;

        // 8xy1 - OR Vx, Vy
        case 0x1:
            machine->V[x] = machine->V[x] | machine->V[y];

            if (machine->vf_reset)
                machine->V[0xF] = 0;

            break;

        // 8xy2 - AND Vx, Vy
        case 0x2:
            machine->V[x] = machine->V[x] & machine->V[y];

            if (machine->vf_reset)
                machine->V[0xF] = 0;

            break;

        // 8xy3 - XOR Vx, Vy
        case 0x3:
            machine->V[x] = machine->V[x] ^ machine->V[y];

            if (machine->vf_reset)
                machine->V[0xF] = 0;

            break;

        // 8xy4 - ADD Vx, Vy
        case 0x4:
            result = machine->V[x] + machine->V[y];

            machine->V[x]   = (uint8_t) result;
            machine->V[0xF] = (result > 255) ? 1 : 0;

            break;

        // 8xy5 - SUB Vx, Vy
        case 0x5:
            VF = (machine->V[x] >= machine->V[y]) ? 1 : 0;

            machine->V[x]   = machine->V[x] - machine->V[y];
            machine->V[0xF] = VF;

            break;

        // 8xy6 - SHR Vx {, Vy}
        case 0x6:
            VF = (machine->V[x] & 1) ? 1 : 0;

            machine->V[x]   = machine->V[x] >> 1;
            machine->V[0xF] = VF;

            break;

        // 8xy7 - SUBN Vx, Vy
        case 0x7:
            VF = (machine->V[y] >= machine->V[x]) ? 1 : 0;

            machine->V[x]   = machine->V[y] - machine->V[x];
            machine->V[0xF] = VF;

            break;

        // 8xyE - SHL Vx {, Vy}
        case 0xE:
            VF = (machine->V[x] & 0x80) ? 1 : 0;

            machine->V[x]   = machine->V[x] << 1;
            machine->V[0xF] = VF;

            break;
    }
}

void decode_type_9(Chip_8 *machine, uint16_t instruction) {
    uint8_t x = GET_X(instruction);
    uint8_t y = GET_Y(instruction);
    uint8_t n = GET_N(instruction);

    // 9xy0 - SNE Vx, Vy
    if (n == 0) {
        if (machine->V[x] != machine->V[y])
            machine->PC += 2;
    }
}

void decode_type_A(Chip_8 *machine, uint16_t instruction) {
    uint16_t addr = GET_ADDR(instruction);

    // Annn - LD I, addr
    machine->I = addr;
}

void decode_type_B(Chip_8 *machine, uint16_t instruction) {
    uint16_t addr = GET_ADDR(instruction);

    // Bnnn - JP V0, addr
    machine->PC = machine->V[0] + addr;
}

void decode_type_C(Chip_8 *machine, uint16_t instruction) {
    uint8_t x    = GET_X(instruction);
    uint8_t byte = GET_BYTE(instruction);

    // Cxkk - RND Vx, byte
    machine->V[x] = (uint8_t) (rand() % 256) & byte;
}

void decode_type_D(Chip_8 *machine, uint16_t instruction) {
    uint8_t x = GET_X(instruction);
    uint8_t y = GET_Y(instruction);
    uint8_t n = GET_N(instruction);

    // Dxyn - DRW Vx, Vy, nibble
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
}

void decode_type_E(Chip_8 *machine, uint16_t instruction) {
    uint8_t x  = GET_X(instruction);
    uint8_t kk = GET_KK(instruction);

    uint8_t key = machine->V[x];
    bool key_pressed = machine->key_state[key];

    switch (kk) {
        // Ex9E - SKP Vx
        case 0x9E:
            if (key_pressed)
                machine->PC += 2;

            break;

        // ExA1 - SKNP Vx
        case 0xA1:
            if (!key_pressed)
                machine->PC += 2;

            break;
    }
}

void decode_type_F(Chip_8 *machine, uint16_t instruction) {
    uint8_t x  = GET_X(instruction);
    uint8_t kk = GET_KK(instruction);

    uint8_t num;

    switch (kk) {
        // Fx07 - LD Vx, DT
        case 0x07:
            machine->V[x] = machine->DT;
            break;

        // Fx0A - LD Vx, K
        case 0x0A:
        {
            int key = -1;

            for (uint8_t i = 0; i <= 0xF; i++) {
                if (machine->key_state[i]) {
                    key = i;
                    break;
                }
            }

            if (key != -1)
                machine->V[x] = (uint8_t) key;
            else
                machine->PC -= 2;

            break;
        }

        // Fx15 - LD DT, Vx
        case 0x15:
            machine->DT = machine->V[x];
            break;

        // Fx18 - LD ST, Vx
        case 0x18:
            machine->ST = machine->V[x];
            break;

        // Fx1E - ADD I, Vx
        case 0x1E:
            machine->I = machine->I + machine->V[x];
            break;

        // Fx29 - LD F, Vx
        case 0x29:
            switch (machine->V[x]) {
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

        // Fx33 - LD B, Vx
        case 0x33:
            num = machine->V[x];

            machine->memory[machine->I + 2] = num % 10;
            num /= 10;

            machine->memory[machine->I + 1] = num % 10;
            num /= 10;

            machine->memory[machine->I + 0] = num % 10;
            num /= 10;

            break;

        // Fx55 - LD [I], Vx
        case 0x55:
            for (int i = 0; i <= x; i++) {
                if (machine->increment_index) {
                    machine->memory[machine->I] = machine->V[i];
                    machine->I += 1;
                } else {
                    machine->memory[machine->I + i] = machine->V[i];
                }
            }

            break;

        // Fx65 - LD Vx, [I]
        case 0x65:
            for (int i = 0; i <= x; i++) {
                if (machine->increment_index) {
                    machine->V[i] = machine->memory[machine->I];
                    machine->I += 1;
                } else {
                    machine->V[i] = machine->memory[machine->I + i];
                }
            }

            break;
    }
}


void CHIP8_decrement_timers(Chip_8 *machine) {
    if (machine->DT > 0)
        machine->DT -= 1;

    if (machine->ST > 0)
        machine->ST -= 1;
}

