#include <SDL2/SDL.h>
#include "cave.h"
#include <stdio.h>
#include <stdlib.h>

// External globals from main.c
extern Uint32 *pixels;
extern int pitch;

// Terrain buffer and scroll offset
int top_terrain[TERRAIN_WIDTH];
int bottom_terrain[TERRAIN_WIDTH];
static float scroll_offset = 0.0f;
static const float SCROLL_SPEED = 25.0f; // Pixels per second

// Draw cave terrain
static void draw_cave(SDL_Texture *texture) {
    SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);
    int bytes_per_row = pitch / sizeof(Uint32);

    // Clear screen
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            pixels[y * bytes_per_row + x] = 0x000000FF; // Black space
        }
    }

    // Draw terrain with offset
    int offset = (int)scroll_offset;
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        int terrain_x = (x + offset) % TERRAIN_WIDTH;
        for (int y = 0; y < top_terrain[terrain_x]; y++) {
            pixels[y * bytes_per_row + x] = 0x808080FF; // Gray cave top
        }
        for (int y = bottom_terrain[terrain_x]; y < SCREEN_HEIGHT; y++) {
            pixels[y * bytes_per_row + x] = 0x808080FF; // Gray cave bottom
        }
    }
    SDL_UnlockTexture(texture);
}

// Generate procedural terrain with seamless loop and wider cave
static void generate_terrain() {
    int start_top = SCREEN_HEIGHT / 3;
    int start_bottom = SCREEN_HEIGHT - SCREEN_HEIGHT / 3;
    int last_top = start_top;
    int last_bottom = start_bottom;

    for (int x = 0; x < TERRAIN_WIDTH; x++) {
        last_top += (rand() % 21) - 10;
        last_bottom += (rand() % 21) - 10;

        if (x % 200 < 40) {
            int gap = (rand() % 2 == 0) ? 150 + (rand() % 51) : 300 + (rand() % 101);
            last_bottom = last_top + gap;
        }

        if (last_top < 50) last_top = 50;
        if (last_top > SCREEN_HEIGHT / 2 - 50) last_top = SCREEN_HEIGHT / 2 - 50;
        if (last_bottom < SCREEN_HEIGHT / 2 + 50) last_bottom = SCREEN_HEIGHT / 2 + 50;
        if (last_bottom > SCREEN_HEIGHT - 50) last_bottom = SCREEN_HEIGHT - 50;
        if (last_bottom - last_top < 150) last_bottom = last_top + 150;

        top_terrain[x] = last_top;
        bottom_terrain[x] = last_bottom;

        if (x == TERRAIN_WIDTH - 1) {
            top_terrain[x] = start_top;
            bottom_terrain[x] = start_bottom;
        }
    }

    for (int x = TERRAIN_WIDTH - 50; x < TERRAIN_WIDTH; x++) {
        float t = (float)(x - (TERRAIN_WIDTH - 50)) / 50.0f;
        top_terrain[x] = (int)((1.0f - t) * top_terrain[x] + t * start_top);
        bottom_terrain[x] = (int)((1.0f - t) * bottom_terrain[x] + t * start_bottom);
    }
}

void cave_init(SDL_Texture *texture) {
    generate_terrain();
    draw_cave(texture);
}

void cave_update_and_render(float delta_time, SDL_Texture *texture) {
    scroll_offset += SCROLL_SPEED * delta_time;
    if (scroll_offset >= TERRAIN_WIDTH) {
        scroll_offset -= TERRAIN_WIDTH;
    }
    draw_cave(texture);
}

float cave_get_scroll_offset(void) {
    return scroll_offset;
}