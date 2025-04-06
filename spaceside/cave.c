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

// Fuel pod array
static FuelPod fuel_pods[MAX_FUEL_PODS];
static int fuel_pod_count = 0;

// Draw cave terrain and fuel pods
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

    // Draw fuel pods
    for (int i = 0; i < fuel_pod_count; i++) {
        if (fuel_pods[i].active) {
            int pod_x = (int)(fuel_pods[i].x - scroll_offset);
            int pod_y = (int)fuel_pods[i].y;
            if (pod_x >= 0 && pod_x < SCREEN_WIDTH - 4) {
                for (int y = pod_y; y < pod_y + 4; y++) {
                    for (int x = pod_x; x < pod_x + 4; x++) {
                        if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
                            pixels[y * bytes_per_row + x] = 0xFFFF00FF; // Yellow fuel pod
                        }
                    }
                }
            }
        }
    }
    SDL_UnlockTexture(texture);
}

// Generate procedural terrain with seamless loop and more variance
static void generate_terrain() {
    int start_top = SCREEN_HEIGHT / 3;
    int start_bottom = SCREEN_HEIGHT - SCREEN_HEIGHT / 3;
    int last_top = start_top;
    int last_bottom = start_bottom;

    for (int x = 0; x < TERRAIN_WIDTH; x++) {
        // Increased variance
        last_top += (rand() % 31) - 15; // -15 to +15
        last_bottom += (rand() % 31) - 15;

        // Choke points and open areas (every ~150 pixels)
        if (x % 150 < 30) {
            int gap = (rand() % 2 == 0) ? 100 + (rand() % 51) : 300 + (rand() % 201); // 100-150 or 300-500
            last_bottom = last_top + gap;
        }

        // Bounds for wider cave
        if (last_top < 50) last_top = 50;
        if (last_top > SCREEN_HEIGHT / 2 - 50) last_top = SCREEN_HEIGHT / 2 - 50;
        if (last_bottom < SCREEN_HEIGHT / 2 + 50) last_bottom = SCREEN_HEIGHT / 2 + 50;
        if (last_bottom > SCREEN_HEIGHT - 50) last_bottom = SCREEN_HEIGHT - 50;
        if (last_bottom - last_top < 100) last_bottom = last_top + 100; // Min gap widened

        top_terrain[x] = last_top;
        bottom_terrain[x] = last_bottom;

        // Spawn fuel pods every ~500 pixels
        if (x % 500 == 0 && fuel_pod_count < MAX_FUEL_PODS) {
            fuel_pods[fuel_pod_count].x = (float)x;
            fuel_pods[fuel_pod_count].y = (float)(last_top + (last_bottom - last_top) / 2); // Middle of gap
            fuel_pods[fuel_pod_count].active = 1;
            fuel_pod_count++;
        }

        // Seamless loop
        if (x == TERRAIN_WIDTH - 1) {
            top_terrain[x] = start_top;
            bottom_terrain[x] = start_bottom;
        }
    }

    // Smooth transition for loop
    for (int x = TERRAIN_WIDTH - 50; x < TERRAIN_WIDTH; x++) {
        float t = (float)(x - (TERRAIN_WIDTH - 50)) / 50.0f;
        top_terrain[x] = (int)((1.0f - t) * top_terrain[x] + t * start_top);
        bottom_terrain[x] = (int)((1.0f - t) * bottom_terrain[x] + t * start_bottom);
    }
}

void cave_init(SDL_Texture *texture) {
    fuel_pod_count = 0; // Reset fuel pods
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

int cave_get_fuel_pod_count(void) {
    return fuel_pod_count;
}

FuelPod* cave_get_fuel_pods(void) {
    return fuel_pods;
}

void cave_consume_fuel_pod(int index) {
    if (index >= 0 && index < fuel_pod_count && fuel_pods[index].active) {
        fuel_pods[index].active = 0;
    }
}