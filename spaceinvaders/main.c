#include <SDL2/SDL.h>
#include <stdio.h>

// Global screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Global pixel buffer and pitch
Uint32 *pixels;
int pitch;

// Invader sprite (8x8)
const Uint8 invader_sprite[8] = {
    0b00100100, //   *  *    (antennae)
    0b00011000, //    **     (eyes)
    0b01111110, //  ******
    0b11111111, // ********  (body)
    0b11011011, // ** ** **
    0b11011011, // ** ** **
    0b01100110, //  **  **   (legs)
    0b00000000  //           (empty row)
};

// Ship sprite (8x8)
const Uint8 ship_sprite[8] = {
    0b00001000, //     *   
    0b00011000, //    **   
    0b00111100, //   ****  
    0b01111110, //  ****** 
    0b11111111, // ********
    0b11111111, // ********
    0b01111110, //  ****** 
    0b00111100  //   ****  
};

// Missile sprite (4x4)
const Uint8 missile_sprite[4] = {
    0b01100000, //  ** 
    0b11110000, // ****
    0b11110000, // ****
    0b01100000  //  ** 
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

// Move invader
void move_invader(int *inv_x, int *inv_vel, int *inv_y, SDL_Texture *texture) {
    draw_sprite(*inv_x, *inv_y, invader_sprite, 8, 8, 0x000000FF, texture);
    *inv_x += *inv_vel;
    if (*inv_x + 8 > SCREEN_WIDTH) {
        *inv_x = SCREEN_WIDTH - 8;
        *inv_vel = -2;
        *inv_y += 8;
    } else if (*inv_x < 0) {
        *inv_x = 0;
        *inv_vel = 2;
        *inv_y += 8;
    }
    draw_sprite(*inv_x, *inv_y, invader_sprite, 8, 8, 0x00FF00FF, texture);
}

// Draw score
void draw_score(int score, SDL_Texture *texture) {
    int x = 10, y = 10;
    char score_str[10];
    snprintf(score_str, 10, "%d", score);
    for (int i = 0; score_str[i]; i++) {
        int digit = score_str[i] - '0';
        draw_sprite(x + i * 6, y, digit_sprites[digit], 5, 5, 0xFFFFFFFF, texture);
    }
}

// Struct for missiles
typedef struct {
    int x, y;
    int active;
    int friendly;
} Missile;

// Check collision between point and rectangle
int check_collision(int x, int y, int rect_x, int rect_y, int rect_w, int rect_h) {
    return (x >= rect_x && x < rect_x + rect_w && y >= rect_y && y < rect_y + rect_h);
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Space Invaders",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
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
        SDL_Quit();
        return 1;
    }

    // Clear screen
    SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        pixels[i] = 0x000000FF;
    }
    SDL_UnlockTexture(texture);

    // Game state
    int running = 1;
    int ship_x = SCREEN_WIDTH / 2 - 4;
    int ship_y = SCREEN_HEIGHT - 16;
    int ship_vel = 0;
    int ship_alive = 1;
    int inv_x[5] = {100, 200, 300, 400, 500};
    int inv_vel[5] = {2, 2, 2, 2, 2};
    int inv_y[5] = {50, 50, 50, 50, 50};
    int inv_alive[5] = {1, 1, 1, 1, 1};
    Missile missiles[100] = {0};
    int score = 0;
    SDL_Event event;
    Uint32 last_alien_shot = 0;

    while (running) {
        // Input
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN && ship_alive) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        ship_vel = -5;
                        break;
                    case SDLK_RIGHT:
                        ship_vel = 5;
                        break;
                    case SDLK_SPACE:
                        for (int i = 0; i < 100; i++) {
                            if (!missiles[i].active) {
                                missiles[i] = (Missile){ship_x + 2, ship_y - 4, 1, 1};
                                break;
                            }
                        }
                        break;
                }
            }
        }

        // Update ship
        if (ship_alive) {
            draw_sprite(ship_x, ship_y, ship_sprite, 8, 8, 0x000000FF, texture);
            ship_x += ship_vel;
            if (ship_x < 0) ship_x = 0;
            if (ship_x + 8 > SCREEN_WIDTH) ship_x = SCREEN_WIDTH - 8;
        }

        // Update invaders
        int invaders_left = 0;
        for (int i = 0; i < 5; i++) {
            if (inv_alive[i]) {
                move_invader(&inv_x[i], &inv_vel[i], &inv_y[i], texture);
                invaders_left++;
                if (inv_y[i] >= SCREEN_HEIGHT - 8) {
                    ship_alive = 0;
                }
            }
        }

        // Alien shooting (every 2 sec)
        Uint32 now = SDL_GetTicks();
        if (now - last_alien_shot > 2000 && invaders_left > 0) {
            for (int i = 0; i < 5; i++) {
                if (inv_alive[i]) {
                    for (int j = 0; j < 100; j++) {
                        if (!missiles[j].active) {
                            missiles[j] = (Missile){inv_x[i] + 2, inv_y[i] + 8, 1, 0};
                            break;
                        }
                    }
                    break;
                }
            }
            last_alien_shot = now;
        }

        // Update missiles with path collision
        for (int i = 0; i < 100; i++) {
            if (missiles[i].active) {
                int old_y = missiles[i].y;
                draw_sprite(missiles[i].x, old_y, missile_sprite, 4, 4, 0x000000FF, texture);
                missiles[i].y += missiles[i].friendly ? -50 : 50;

                // Check collision along path
                int steps = 50; // Bullet moves 50 pixels
                int step_y = (missiles[i].y - old_y) / steps;
                int hit = 0;
                for (int s = 0; s <= steps && !hit; s++) {
                    int check_y = old_y + s * step_y;
                    if (missiles[i].friendly) {
                        for (int j = 0; j < 5 && !hit; j++) {
                            if (inv_alive[j] && check_collision(missiles[i].x, check_y, inv_x[j], inv_y[j], 8, 8)) {
                                inv_alive[j] = 0;
                                missiles[i].active = 0;
                                draw_sprite(inv_x[j], inv_y[j], invader_sprite, 8, 8, 0x000000FF, texture);
                                score += 10;
                                hit = 1;
                            }
                        }
                    } else if (ship_alive && check_collision(missiles[i].x, check_y, ship_x, ship_y, 8, 8)) {
                        ship_alive = 0;
                        draw_sprite(ship_x, ship_y, ship_sprite, 8, 8, 0x000000FF, texture);
                        hit = 1;
                    }
                }

                if (missiles[i].y < 0 || missiles[i].y >= SCREEN_HEIGHT) {
                    missiles[i].active = 0;
                } else if (!hit) {
                    Uint32 color = missiles[i].friendly ? 0xFFFFFF00 : 0xFF0000FF;
                    draw_sprite(missiles[i].x, missiles[i].y, missile_sprite, 4, 4, color, texture);
                }
            }
        }

        // Compact missiles
        int new_count = 0;
        for (int i = 0; i < 100; i++) {
            if (missiles[i].active) {
                missiles[new_count] = missiles[i];
                new_count++;
            }
        }
        for (int i = new_count; i < 100; i++) {
            missiles[i].active = 0;
        }

        // Draw ship if alive
        if (ship_alive) {
            draw_sprite(ship_x, ship_y, ship_sprite, 8, 8, 0x0000FFFF, texture);
        }

        // Draw score
        draw_score(score, texture);

        // Win/lose
        if (invaders_left == 0) {
            printf("You Win! Score: %d\n", score);
            running = 0;
        } else if (!ship_alive) {
            printf("Game Over! Score: %d\n", score);
            running = 0;
        }

        // Render
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}