#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

// External globals from main.c
extern Uint32 *pixels;
extern int pitch;

// Terrain buffer (double SCREEN_WIDTH for smooth scrolling)
#define TERRAIN_WIDTH (SCREEN_WIDTH * 2)
static int top_terrain[TERRAIN_WIDTH];
static int bottom_terrain[TERRAIN_WIDTH];
static float scroll_offset = 0.0f;
static const float SCROLL_SPEED = 100.0f; // Pixels per second

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

    // Draw terrain
    int offset = (int)scroll_offset % SCREEN_WIDTH;
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

// Generate procedural terrain
static void generate_terrain() {
    int last_top = SCREEN_HEIGHT / 4;
    int last_bottom = SCREEN_HEIGHT - SCREEN_HEIGHT / 4;
    for (int x = 0; x < TERRAIN_WIDTH; x++) {
        // Simple random walk with bounds
        last_top += (rand() % 11) - 5; // -5 to +5 variation
        last_bottom += (rand() % 11) - 5;
        if (last_top < 50) last_top = 50; // Min height
        if (last_top > SCREEN_HEIGHT / 3) last_top = SCREEN_HEIGHT / 3; // Max height
        if (last_bottom < SCREEN_HEIGHT * 2 / 3) last_bottom = SCREEN_HEIGHT * 2 / 3; // Min bottom
        if (last_bottom > SCREEN_HEIGHT - 50) last_bottom = SCREEN_HEIGHT - 50; // Max bottom
        top_terrain[x] = last_top;
        bottom_terrain[x] = last_bottom;
    }
}

void cave_init(SDL_Texture *texture) {
    generate_terrain();
    draw_cave(texture);
}

void cave_update_and_render(float delta_time, SDL_Texture *texture) {
    scroll_offset += SCROLL_SPEED * delta_time; // Move right to left
    if (scroll_offset >= SCREEN_WIDTH) {
        scroll_offset -= SCREEN_WIDTH; // Reset for seamless loop
    }
    draw_cave(texture);
}