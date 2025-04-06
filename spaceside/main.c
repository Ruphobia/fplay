#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "cave.h"
#include "player1.h"
#include "player2.h"
#include <stdio.h>

// Global pixel buffer and pitch
Uint32 *pixels;
int pitch;

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL Init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer Init failed: %s\n", Mix_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Cave Scroller",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!texture) {
        printf("Texture creation failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    // Clear screen
    SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        pixels[i] = 0x000000FF; // Black space
    }
    SDL_UnlockTexture(texture);

    // Initialize modules
    cave_init(texture);
    player1_init(texture);
    player2_init(texture);

    // Game loop (60 Hz)
    int running = 1;
    SDL_Event event;
    Uint32 last_time = SDL_GetTicks();
    const float TARGET_FRAME_TIME = 1000.0f / 60.0f;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        // Update and render modules
        cave_update_and_render(delta_time, texture);
        player1_update_and_render(delta_time, texture, top_terrain, bottom_terrain, (int)cave_get_scroll_offset());
        player2_update_and_render(delta_time, texture, top_terrain, bottom_terrain, (int)cave_get_scroll_offset());

        // Check for game over
        if (player1_is_dead() && player2_is_dead()) {
            printf("Both players crashed!\n");
            SDL_Delay(1000); // Pause to hear crash sounds
            running = 0;
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        Uint32 frame_time = SDL_GetTicks() - current_time;
        if (frame_time < TARGET_FRAME_TIME) {
            SDL_Delay((Uint32)(TARGET_FRAME_TIME - frame_time));
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}