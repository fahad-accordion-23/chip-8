#include "chip-8.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 320

#define TIMER_TICK_RATE 60   // Hz
#define CPU_TICK_RATE   700  // Hz
#define FRAME_RATE      60   // Hz

#define TIME_PER_TIMER_TICK (1.0f / TIMER_TICK_RATE)
#define TIME_PER_CPU_TICK   (1.0f / CPU_TICK_RATE)
#define TIME_PER_FRAME      (1.0f / FRAME_RATE)

#define MIN_CPU_TICKS_PER_FRAME (CPU_TICK_RATE / FRAME_RATE)
#define MAX_CPU_TICKS_PER_FRAME (MIN_CPU_TICKS_PER_FRAME * 2)

int get_key_from_scancode(SDL_Scancode scancode);
void update_pixel_buffer(Chip_8 *machine, uint32_t *pixel_buffer);

int main(int argc, char **argv) {
    const char *file_path;

    if (argc >= 2)
        file_path = argv[1];
    else {
        fprintf(stderr, "Please provide a file path to the program you wish to run!\n");
        return 1;
    }

    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *texture;
    SDL_Event    event;

    uint32_t *pixel_buffer = malloc(sizeof(uint32_t) * 64 * 32);
    Chip_8 machine;

    if (!SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to initialize SDL subsystems: %s",
                     SDL_GetError());
        return 3;
    }

    if (!SDL_CreateWindowAndRenderer("CHIP-8 Interpreter",
                                     WINDOW_WIDTH,
                                     WINDOW_HEIGHT,
                                     0,
                                     &window,
                                     &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to create window and renderer: %s",
                     SDL_GetError());
        return 3;
    }

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_RGBA32,
                                SDL_TEXTUREACCESS_STREAMING,
                                64,
                                32);

    if (texture == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to create texture: %s",
                     SDL_GetError());
        return 3;
    }

    if (!SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_PIXELART)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to set scale mode: %s",
                     SDL_GetError());
        return 3;
    }

    CHIP8_init(&machine);
    CHIP8_load(&machine, file_path);
    CHIP8_set_vf_reset(&machine, true);
    CHIP8_set_increment_index(&machine, true);

    Uint64 last_time = SDL_GetPerformanceCounter();

    float timer_accumulator = 0.0f;
    float cpu_accumulator   = 0.0f;
    float frame_accumulator = 0.0f;

    while (1) {

        Uint64 now          = SDL_GetPerformanceCounter();
        Uint64 elapsed_time = now - last_time;
               last_time    = now;

        float dt = (float) elapsed_time / (float) SDL_GetPerformanceFrequency();

        timer_accumulator += dt;
        cpu_accumulator   += dt;
        frame_accumulator += dt;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                goto quit;
            }

            else if (event.type == SDL_EVENT_KEY_DOWN) {
                int key = get_key_from_scancode(event.key.scancode);

                if (key != -1) {
                    machine.key_state[key] = true;
                }
            }

            else if (event.type == SDL_EVENT_KEY_UP) {
                int key = get_key_from_scancode(event.key.scancode);

                if (key != -1) {
                    machine.key_state[key] = false;
                }
            }
        }

        int ticks_processed = 0;
        while (cpu_accumulator >= TIME_PER_CPU_TICK && ticks_processed <= MAX_CPU_TICKS_PER_FRAME) {
            
            CHIP8_tick(&machine);

            cpu_accumulator -= TIME_PER_CPU_TICK;
            ticks_processed++;
        }

        if (timer_accumulator >= TIME_PER_TIMER_TICK) {

            CHIP8_decrement_timers(&machine);

            timer_accumulator = 0.0f;  // timer ticks maybe skipped
        }

        if (frame_accumulator >= TIME_PER_FRAME) {

            update_pixel_buffer(&machine, pixel_buffer);

            SDL_UpdateTexture(texture, NULL, pixel_buffer, 64 * sizeof(uint32_t));

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderClear(renderer);
            SDL_RenderTexture(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);

            frame_accumulator = 0.0f;  // frame skipping
        }
    }

quit:
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    SDL_Quit();

    return 0;
}

int get_key_from_scancode(SDL_Scancode scancode) {
    switch (scancode) {
        case SDL_SCANCODE_X: return 0x0;
        case SDL_SCANCODE_1: return 0x1;
        case SDL_SCANCODE_2: return 0x2;
        case SDL_SCANCODE_3: return 0x3;
        case SDL_SCANCODE_Q: return 0x4;
        case SDL_SCANCODE_W: return 0x5;
        case SDL_SCANCODE_E: return 0x6;
        case SDL_SCANCODE_A: return 0x7;
        case SDL_SCANCODE_S: return 0x8;
        case SDL_SCANCODE_D: return 0x9;
        case SDL_SCANCODE_Z: return 0xA;
        case SDL_SCANCODE_C: return 0xB;
        case SDL_SCANCODE_4: return 0xC;
        case SDL_SCANCODE_R: return 0xD;
        case SDL_SCANCODE_F: return 0xE;
        case SDL_SCANCODE_V: return 0xF;

        default: return -1;
    }
}

void update_pixel_buffer(Chip_8 *machine, uint32_t *pixel_buffer) {
    for (int j = 0; j < 32; j++) {
       for (int i = 0; i < 64 / 8; i++) {

           for (int n = 7; n >= 0; n--) {
               int bit = (machine->display[j * 8 + i] >> n) & 1;

               pixel_buffer[j * 64 + i * 8 + (7 - n)] = bit ? 0xFFFFFFFF : 0x0;
           }
       }
   }
}
