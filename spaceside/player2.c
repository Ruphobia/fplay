#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "player2.h"
#include "cave.h"
#include <stdio.h>
#include <math.h>

// External globals from main.c
extern Uint32 *pixels;
extern int pitch;

// Player 2 sprite (8x8)
static const Uint8 ship_sprite[8] = {
    0b00011000, //    **   
    0b00111100, //   ****  
    0b01111110, //  ****** 
    0b11111111, // ********
    0b11011011, // ** ** **
    0b10011001, // *  **  *
    0b01000010, //  *    * 
    0b00100100  //   *  *  
};

// Flame sprite (4x4)
static const Uint8 flame_sprite[4] = {
    0b01100000, //  ** 
    0b11110000, // ****
    0b01100000, //  ** 
    0b00100000  //  *  
};

// Player 2 state
static float x, y, last_x, last_y, last_flame_x, last_flame_y;
static float vel_x, vel_y;
static float fuel = 100.0f;
static int dead = 0;
static int frame_count = 0;
static int sound_playing = 0;
static Mix_Chunk *thruster_sound;
static Mix_Chunk *crash_sound;
static const float GRAVITY = 0.1f;
static const float THRUST = 0.2f;

// Draw sprite
static void draw_sprite(int x, int y, const Uint8 *sprite, int width, int height, Uint32 color, SDL_Texture *texture) {
    SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);
    int bytes_per_row = pitch / sizeof(Uint32);

    for (int row = 0; row < height; row++) {
        if (y + row < 0 || y + row >= SCREEN_HEIGHT) continue;
        Uint8 bits = sprite[row];
        for (int col = 0; col < width; col++) {
            if (x + col < 0 || x + col >= SCREEN_WIDTH) continue;
            if (bits & (1 << (7 - col))) {
                pixels[(y + row) * bytes_per_row + (x + col)] = color;
            }
        }
    }
    SDL_UnlockTexture(texture);
}

// Draw fuel gauge (20x100, right side)
static void draw_fuel_gauge(SDL_Texture *texture) {
    SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);
    int bytes_per_row = pitch / sizeof(Uint32);
    int gauge_x = SCREEN_WIDTH - 30; // Right side
    int gauge_y = 50;
    int gauge_width = 20;
    int gauge_height = 100;
    int fuel_height = (int)(fuel * gauge_height / 100.0f);

    // Clear gauge area
    for (int gy = gauge_y; gy < gauge_y + gauge_height; gy++) {
        for (int gx = gauge_x - 1; gx < gauge_x + gauge_width + 1; gx++) {
            pixels[gy * bytes_per_row + gx] = 0x000000FF;
        }
    }

    // Draw border
    for (int gx = gauge_x - 1; gx < gauge_x + gauge_width + 1; gx++) {
        pixels[gauge_y * bytes_per_row + gx] = 0xFFFFFFFF;
        pixels[(gauge_y + gauge_height) * bytes_per_row + gx] = 0xFFFFFFFF;
    }
    for (int gy = gauge_y; gy < gauge_y + gauge_height; gy++) {
        pixels[gy * bytes_per_row + gauge_x - 1] = 0xFFFFFFFF;
        pixels[gy * bytes_per_row + gauge_x + gauge_width] = 0xFFFFFFFF;
    }

    // Draw fuel bar
    for (int gy = gauge_y + gauge_height - fuel_height; gy < gauge_y + gauge_height; gy++) {
        for (int gx = gauge_x; gx < gauge_x + gauge_width; gx++) {
            pixels[gy * bytes_per_row + gx] = 0x0000FFFF; // Blue
        }
    }
    SDL_UnlockTexture(texture);
}

void player2_init(SDL_Texture *texture) {
    x = SCREEN_WIDTH * 3 / 4.0f; // Right side
    y = SCREEN_HEIGHT / 2.0f;
    last_x = x;
    last_y = y;
    last_flame_x = x + 2;
    last_flame_y = y + 8;
    vel_x = 0.0f;
    vel_y = 0.0f;
    fuel = 100.0f;
    dead = 0;
    frame_count = 0;
    sound_playing = 0;

    thruster_sound = Mix_LoadWAV("thruster.wav");
    crash_sound = Mix_LoadWAV("crash.wav");
    if (!thruster_sound) printf("Player 2: Failed to load thruster sound: %s\n", Mix_GetError());
    if (!crash_sound) printf("Player 2: Failed to load crash sound: %s\n", Mix_GetError());
}

void player2_update_and_render(float delta_time, SDL_Texture *texture, const int *top_terrain, const int *bottom_terrain, int scroll_offset) {
    if (dead) return;

    // Input
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    int thrusting = 0;
    if (state[SDL_SCANCODE_LEFT]) {
        vel_x -= THRUST; // Left
        thrusting = 1;
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        vel_x += THRUST; // Right
        thrusting = 1;
    }
    if (state[SDL_SCANCODE_UP] && fuel > 0) {
        vel_y -= THRUST; // Up
        fuel -= 0.02f; // Fuel burn per frame
        thrusting = 1;
    }

    // Sound
    if (thrusting && !sound_playing && fuel > 0 && thruster_sound) {
        Mix_PlayChannel(3, thruster_sound, -1); // Channel 3 for Player 2 thruster
        sound_playing = 1;
    } else if (!thrusting && sound_playing) {
        Mix_HaltChannel(3);
        sound_playing = 0;
    }

    // Physics
    vel_y += GRAVITY; // Gravity per frame
    last_x = x;
    last_y = y;
    last_flame_x = x + 2;
    last_flame_y = y + 8;
    x += vel_x * delta_time;
    y += vel_y * delta_time;

    // Screen bounds
    if (x < 0) x = 0;
    if (x + 8 > SCREEN_WIDTH) x = SCREEN_WIDTH - 8;

    // Collision with cave
    int terrain_x = ((int)x + scroll_offset) % TERRAIN_WIDTH;
    if ((int)y < top_terrain[terrain_x] || (int)y + 8 > bottom_terrain[terrain_x]) {
        dead = 1;
        if (crash_sound) {
            Mix_PlayChannel(4, crash_sound, 0); // Channel 4 for Player 2 crash
        } else {
            printf("Player 2: Crash sound not loaded!\n");
        }
        return;
    }

    // Fuel pod collision
    FuelPod *pods = cave_get_fuel_pods();
    int pod_count = cave_get_fuel_pod_count();
    for (int i = 0; i < pod_count; i++) {
        if (pods[i].active) {
            int pod_x = (int)(pods[i].x - scroll_offset);
            int pod_y = (int)pods[i].y;
            if (x + 8 > pod_x && x < pod_x + 4 && y + 8 > pod_y && y < pod_y + 4) {
                fuel += 25.0f; // Add 25% fuel
                if (fuel > 100.0f) fuel = 100.0f; // Cap at 100
                cave_consume_fuel_pod(i); // Consume pod
            }
        }
    }

    // Draw
    draw_sprite((int)last_x, (int)last_y, ship_sprite, 8, 8, 0x000000FF, texture); // Erase
    draw_sprite((int)last_flame_x, (int)last_flame_y, flame_sprite, 4, 4, 0x000000FF, texture);
    draw_sprite((int)x, (int)y, ship_sprite, 8, 8, 0x00FFFFFF, texture); // Cyan ship
    if (state[SDL_SCANCODE_UP] && fuel > 0 && (frame_count % 8) < 4) {
        draw_sprite((int)x + 2, (int)y + 8, flame_sprite, 4, 4, 0xFF8000FF, texture); // Flame
    }
    draw_fuel_gauge(texture);

    frame_count++;
}

int player2_is_dead() {
    return dead;
}