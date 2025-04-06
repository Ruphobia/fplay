#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <math.h>

// Global screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Global pixel buffer and pitch
Uint32 *pixels;
int pitch;

// Lander sprite (8x8)
const Uint8 lander_sprite[8] = {
    0b00011000, //    **   
    0b00111100, //   ****  
    0b01111110, //  ****** 
    0b11111111, // ********
    0b11011011, // ** ** **
    0b10011001, // *  **  *
    0b01000010, //  *    * 
    0b00100100  //   *  *  
};

// Flame sprite (4x4, below lander when thrusting up)
const Uint8 flame_sprite[4] = {
    0b01100000, //  ** 
    0b11110000, // ****
    0b01100000, //  ** 
    0b00100000  //  *  
};

// Digit sprites (5x5, 0-9)
const Uint8 digit_sprites[10][5] = {
    {0b11110, 0b10010, 0b10010, 0b10010, 0b11110}, // 0
    {0b00100, 0b01100, 0b00100, 0b00100, 0b01110}, // 1
    {0b11110, 0b00010, 0b11110, 0b10000, 0b11110}, // 2
    {0b11110, 0b00010, 0b11110, 0b00010, 0b11110}, // 3
    {0b10010, 0b10010, 0b11110, 0b00010, 0b00010}, // 4
    {0b11110, 0b10000, 0b11110, 0b00010, 0b11110}, // 5
    {0b11110, 0b10000, 0b11110, 0b10010, 0b11110}, // 6
    {0b11110, 0b00010, 0b00100, 0b01000, 0b10000}, // 7
    {0b11110, 0b10010, 0b11110, 0b10010, 0b11110}, // 8
    {0b11110, 0b10010, 0b11110, 0b00010, 0b11110}  // 9
};

// Draw sprite
void draw_sprite(int x, int y, const Uint8 *sprite, int width, int height, Uint32 color, SDL_Texture *texture) {
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

// Draw terrain
void draw_terrain(int *terrain, SDL_Texture *texture) {
    SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);
    int bytes_per_row = pitch / sizeof(Uint32);

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        for (int y = terrain[x]; y < SCREEN_HEIGHT; y++) {
            pixels[y * bytes_per_row + x] = 0x808080FF; // Gray terrain
        }
    }
    SDL_UnlockTexture(texture);
}

// Draw score/fuel text
void draw_text(int x, int y, const char *text, SDL_Texture *texture) {
    for (int i = 0; text[i]; i++) {
        int digit = text[i] - '0';
        if (digit >= 0 && digit <= 9) {
            draw_sprite(x + i * 6, y, digit_sprites[digit], 5, 5, 0xFFFFFFFF, texture);
        }
    }
}

// Draw fuel gauge (20x100, left side)
void draw_fuel_gauge(float fuel, SDL_Texture *texture) {
    SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);
    int bytes_per_row = pitch / sizeof(Uint32);
    int gauge_x = 10; // Left side
    int gauge_y = 50; // Below score
    int gauge_width = 20;
    int gauge_height = 100;
    int fuel_height = (int)(fuel * gauge_height / 100.0f); // Scale fuel to 0-100

    // Clear gauge area
    for (int y = gauge_y; y < gauge_y + gauge_height; y++) {
        for (int x = gauge_x - 1; x < gauge_x + gauge_width + 1; x++) {
            pixels[y * bytes_per_row + x] = 0x000000FF; // Black background
        }
    }

    // Draw border
    for (int x = gauge_x - 1; x < gauge_x + gauge_width + 1; x++) {
        pixels[gauge_y * bytes_per_row + x] = 0xFFFFFFFF; // Top
        pixels[(gauge_y + gauge_height) * bytes_per_row + x] = 0xFFFFFFFF; // Bottom
    }
    for (int y = gauge_y; y < gauge_y + gauge_height; y++) {
        pixels[y * bytes_per_row + gauge_x - 1] = 0xFFFFFFFF; // Left
        pixels[y * bytes_per_row + gauge_x + gauge_width] = 0xFFFFFFFF; // Right
    }

    // Draw fuel bar (bottom-up)
    for (int y = gauge_y + gauge_height - fuel_height; y < gauge_y + gauge_height; y++) {
        for (int x = gauge_x; x < gauge_x + gauge_width; x++) {
            pixels[y * bytes_per_row + x] = 0x00FF00FF; // Green fuel
        }
    }
    SDL_UnlockTexture(texture);
}

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

    SDL_Window *window = SDL_CreateWindow("Lunar Lander",
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

    // Load sound effects
    Mix_Chunk *thruster_sound = Mix_LoadWAV("thruster.wav");
    Mix_Chunk *crash_sound = Mix_LoadWAV("crash.wav");
    Mix_Chunk *land_sound = Mix_LoadWAV("land.wav");
    if (!thruster_sound || !crash_sound || !land_sound) {
        printf("Failed to load sound: %s\n", Mix_GetError());
        if (thruster_sound) Mix_FreeChunk(thruster_sound);
        if (crash_sound) Mix_FreeChunk(crash_sound);
        if (land_sound) Mix_FreeChunk(land_sound);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    // Clear screen
    SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        pixels[i] = 0x000000FF; // Black sky
    }
    SDL_UnlockTexture(texture);

    // Terrain (jagged with flat spot at 300-340)
    int terrain[SCREEN_WIDTH];
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        if (x >= 300 && x <= 340) {
            terrain[x] = SCREEN_HEIGHT - 50; // Flat landing pad
        } else {
            terrain[x] = SCREEN_HEIGHT - 50 - (rand() % 30); // Jagged elsewhere
        }
    }
    draw_terrain(terrain, texture);

    // Game state
    int running = 1;
    float lander_x = SCREEN_WIDTH / 2.0f;
    float lander_y = 50.0f;
    float last_x = lander_x, last_y = lander_y;
    float last_flame_x = lander_x + 2, last_flame_y = lander_y + 8;
    float vel_x = 0.0f, vel_y = 0.0f;
    float fuel = 100.0f;
    int score = 0;
    int landed = 0;
    int frame_count = 0;
    int sound_playing = 0;
    SDL_Event event;
    const float GRAVITY = 0.1f;
    const float THRUST = 0.2f;
    const float MAX_LANDING_SPEED = 1.0f;

    while (running) {
        // Handle quit event
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Input (polling keys)
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        int thrusting = 0;
        if (!landed) {
            if (state[SDL_SCANCODE_LEFT]) {
                vel_x -= THRUST; // Thrust left, no fuel cost
                thrusting = 1;
            }
            if (state[SDL_SCANCODE_RIGHT]) {
                vel_x += THRUST; // Thrust right, no fuel cost
                thrusting = 1;
            }
            if (state[SDL_SCANCODE_SPACE] && fuel > 0) {
                vel_y -= THRUST; // Thrust up, consumes fuel
                fuel -= 0.8f; // 4x faster depletion (was 0.2f)
                thrusting = 1;
            }
        }

        // Sound control (thruster)
        if (thrusting && !sound_playing && fuel > 0) {
            Mix_PlayChannel(1, thruster_sound, -1); // Channel 1, looped
            sound_playing = 1;
        } else if (!thrusting && sound_playing) {
            Mix_HaltChannel(1);
            sound_playing = 0;
        }

        // Physics
        if (!landed) {
            vel_y += GRAVITY; // Gravity
            last_x = lander_x;
            last_y = lander_y;
            last_flame_x = lander_x + 2;
            last_flame_y = lander_y + 8;
            lander_x += vel_x;
            lander_y += vel_y;

            // Bounds
            if (lander_x < 0) lander_x = 0;
            if (lander_x + 8 > SCREEN_WIDTH) lander_x = SCREEN_WIDTH - 8;

            // Landing/crash check
            int lander_bottom = (int)lander_y + 8;
            int lander_left = (int)lander_x;
            int lander_right = lander_left + 8;
            if (lander_bottom >= terrain[lander_left] || lander_bottom >= terrain[lander_right]) {
                landed = 1;
                lander_y = terrain[lander_left] - 8; // Snap to surface
                if (vel_y > MAX_LANDING_SPEED || lander_left < 300 || lander_right > 340) {
                    Mix_PlayChannel(2, crash_sound, 0); // Crash sound
                    printf("Crashed! Score: %d\n", score);
                    SDL_Delay(1000); // Pause to hear crash
                    running = 0;
                } else {
                    Mix_PlayChannel(2, land_sound, 0); // Land sound
                    printf("Landed! Score: %d\n", score += 50);
                    SDL_Delay(1000); // Pause to hear landing
                    running = 0;
                }
            }
        }

        // Draw
        draw_sprite((int)last_x, (int)last_y, lander_sprite, 8, 8, 0x000000FF, texture); // Erase lander
        draw_sprite((int)last_flame_x, (int)last_flame_y, flame_sprite, 4, 4, 0x000000FF, texture); // Erase flame
        if (!landed) {
            draw_sprite((int)lander_x, (int)lander_y, lander_sprite, 8, 8, 0xFFFF00FF, texture); // Yellow lander
            // Blinking flame when thrusting up
            if (state[SDL_SCANCODE_SPACE] && fuel > 0 && (frame_count % 8) < 4) {
                draw_sprite((int)lander_x + 2, (int)lander_y + 8, flame_sprite, 4, 4, 0xFF8000FF, texture); // Orange flame
            }
        } else {
            draw_sprite((int)lander_x, (int)lander_y, lander_sprite, 8, 8, vel_y > MAX_LANDING_SPEED ? 0xFF0000FF : 0x00FF00FF, texture); // Red if crashed, green if safe
        }
        draw_terrain(terrain, texture); // Redraw terrain

        // Draw HUD
        char score_str[10];
        snprintf(score_str, 10, "%d", score);
        draw_text(10, 10, score_str, texture); // Score text
        draw_fuel_gauge(fuel, texture); // Fuel gauge

        // Render
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        frame_count++;
        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    Mix_FreeChunk(thruster_sound);
    Mix_FreeChunk(crash_sound);
    Mix_FreeChunk(land_sound);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}