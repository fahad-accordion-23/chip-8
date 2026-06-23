#include "chip-8.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 320
#define TIMER_SPEED   60  // 60Hz
#define TICK_SPEED    700 // 700Hz

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

    Uint64 last_time = SDL_GetPerformanceCounter();
    float time_elapsed_since_last_tick = 0.0f;
    float time_elapsed_since_last_decrement = 0.0f;

    while (1) {
        Uint64 elapsed_time = SDL_GetPerformanceCounter() - last_time;
        float dt = (float) elapsed_time / (float) SDL_GetPerformanceFrequency();

        time_elapsed_since_last_tick += dt;
        time_elapsed_since_last_decrement += dt;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                goto quit;
            }
        }

        if (time_elapsed_since_last_tick >= (1.0f / TICK_SPEED)) {
            time_elapsed_since_last_tick = 0.0f;

            CHIP8_tick(&machine);
        }

        if (time_elapsed_since_last_decrement >= (1.0f / TIMER_SPEED)) {
            time_elapsed_since_last_decrement = 0.0f;

            CHIP8_decrement_timers(&machine);
        }

        for (int j = 0; j < 32; j++) {
            for (int i = 0; i < 64 / 8; i++) {

                for (int n = 7; n >= 0; n--) {
                    int bit = (machine.display[j * 8 + i] >> n) & 1;

                    pixel_buffer[j * 64 + i * 8 + (7 - n)] = bit ? 0xFFFFFFFF : 0x0;
                }
            }
        }

        SDL_UpdateTexture(texture, NULL, pixel_buffer, 64 * sizeof(uint32_t));

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

quit:
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    SDL_Quit();

    return 0;
}

