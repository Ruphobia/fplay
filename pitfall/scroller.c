#include "scroller.h"
#include <stdlib.h>
#include <time.h>

#define SCALE_FACTOR 4

// Player sprites (8x8, 4 frames: stand, walk1, walk2, jump)
static const Uint8 player_sprites[4][8] = {
    // Standing
    {0b00110000, 0b00110000, 0b01111000, 0b00110000, 0b00110000, 0b01000100, 0b01000100, 0b00111000},
    // Walk 1
    {0b00110000, 0b00110000, 0b01111000, 0b00110000, 0b00110000, 0b01000100, 0b00101000, 0b00010000},
    // Walk 2
    {0b00110000, 0b00110000, 0b01111000, 0b00110000, 0b00110000, 0b00010100, 0b00101000, 0b01000000},
    // Jumping
    {0b00110000, 0b00110000, 0b01111000, 0b00110000, 0b01000100, 0b00101000, 0b00010000, 0b00101000}
};

// Global pixel buffer
static Uint32 *pixels;
static int pitch;

void draw_sprite(int x, int y, const Uint8 *sprite, Uint32 color) {
    SDL_LockTexture(NULL, NULL, (void **)&pixels, &pitch);
    int bytes_per_row = pitch / sizeof(Uint32);

    for (int row = 0; row < 8; row++) {
        int scaled_y = y * SCALE_FACTOR + row * SCALE_FACTOR;
        if (scaled_y < 0 || scaled_y >= SCREEN_HEIGHT) continue;
        Uint8 bits = sprite[row];
        for (int col = 0; col < 8; col++) {
            int scaled_x = x * SCALE_FACTOR + col * SCALE_FACTOR;
            if (scaled_x < 0 || scaled_x >= SCREEN_WIDTH) continue;
            if (bits & (1 << (7 - col))) {
                // Draw a 4x4 block for each pixel
                for (int dy = 0; dy < SCALE_FACTOR; dy++) {
                    for (int dx = 0; dx < SCALE_FACTOR; dx++) {
                        int final_y = scaled_y + dy;
                        int final_x = scaled_x + dx;
                        if (final_y >= 0 && final_y < SCREEN_HEIGHT && final_x >= 0 && final_x < SCREEN_WIDTH) {
                            pixels[final_y * bytes_per_row + final_x] = color;
                        }
                    }
                }
            }
        }
    }
    SDL_UnlockTexture(NULL);
}

void init_game(GameState *game, SDL_Texture *tex) {
    game->texture = tex;
    game->player.x = SCREEN_WIDTH / (2 * SCALE_FACTOR) - PLAYER_SIZE / 2; // Center player in scaled space
    game->player.y = SCREEN_HEIGHT / SCALE_FACTOR - GROUND_HEIGHT - PLAYER_SIZE;
    game->player.vel_x = 0.0f;
    game->player.vel_y = 0.0f;
    game->player.jumping = 0;
    game->player.frame = 0;
    game->world_offset = 0;

    srand(time(NULL));
    for (int i = 0; i < MAX_PITS; i++) {
        game->pits[i].x = 800 + i * (SCREEN_WIDTH / 2 + rand() % 200); // Random spacing
        game->pits[i].width = 50 + rand() % 50; // Random width 50-100
    }
}

int update_game(GameState *game, const Uint8 *keys) {
    const float ACCEL = 0.2f;
    const float MAX_SPEED = 3.0f;
    const float GRAVITY = 0.3f;
    const float JUMP_VEL = -6.0f;

    // Horizontal movement
    if (keys[SDL_SCANCODE_LEFT]) {
        game->player.vel_x -= ACCEL;
        if (game->player.vel_x < -MAX_SPEED) game->player.vel_x = -MAX_SPEED;
    } else if (keys[SDL_SCANCODE_RIGHT]) {
        game->player.vel_x += ACCEL;
        if (game->player.vel_x > MAX_SPEED) game->player.vel_x = MAX_SPEED;
    } else {
        game->player.vel_x *= 0.8f; // Friction
    }

    // Jumping
    if (keys[SDL_SCANCODE_SPACE] && !game->player.jumping) {
        game->player.vel_y = JUMP_VEL;
        game->player.jumping = 1;
    }

    // Physics
    game->player.vel_y += GRAVITY;
    game->player.y += game->player.vel_y;

    // Scroll world (reversed direction)
    game->world_offset += (int)game->player.vel_x;

    // Ground collision and pit detection
    int ground_y = SCREEN_HEIGHT / SCALE_FACTOR - GROUND_HEIGHT;
    for (int i = 0; i < MAX_PITS; i++) {
        int pit_x = game->pits[i].x - game->world_offset;
        if (game->player.x + PLAYER_SIZE > pit_x && game->player.x < pit_x + game->pits[i].width) {
            ground_y = SCREEN_HEIGHT / SCALE_FACTOR; // Pit = no ground
            break;
        } else {
            ground_y = SCREEN_HEIGHT / SCALE_FACTOR - GROUND_HEIGHT;
        }
    }

    if (game->player.y + PLAYER_SIZE > ground_y) {
        if (ground_y == SCREEN_HEIGHT / SCALE_FACTOR) { // In a pit
            if (game->player.y > SCREEN_HEIGHT / SCALE_FACTOR) { // Fallen off screen
                return 0; // Game over
            }
        } else { // On ground
            game->player.y = ground_y - PLAYER_SIZE;
            game->player.vel_y = 0.0f;
            game->player.jumping = 0;
        }
    }

    // Animation
    static int frame_timer = 0;
    frame_timer++;
    if (game->player.jumping) {
        game->player.frame = 3; // Jump sprite
    } else if (game->player.vel_x != 0 && (frame_timer % 10) == 0) {
        game->player.frame = (game->player.frame % 2) + 1; // Walk cycle
    } else if (game->player.vel_x == 0) {
        game->player.frame = 0; // Stand
    }

    return 1; // Game continues
}

void draw_game(GameState *game, SDL_Renderer *renderer) {
    SDL_LockTexture(game->texture, NULL, (void **)&pixels, &pitch);
    int bytes_per_row = pitch / sizeof(Uint32);

    // Clear screen
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        pixels[i] = 0x000000FF; // Black background
    }

    // Draw ground and pits
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        int world_x = x / SCALE_FACTOR + game->world_offset;
        int ground_y = SCREEN_HEIGHT - GROUND_HEIGHT * SCALE_FACTOR;
        for (int i = 0; i < MAX_PITS; i++) {
            int pit_x = (game->pits[i].x - game->world_offset) * SCALE_FACTOR;
            int pit_width = game->pits[i].width * SCALE_FACTOR;
            if (x >= pit_x && x < pit_x + pit_width) {
                ground_y = SCREEN_HEIGHT; // Pit
                break;
            } else {
                ground_y = SCREEN_HEIGHT - GROUND_HEIGHT * SCALE_FACTOR;
            }
        }
        for (int y = ground_y; y < SCREEN_HEIGHT; y++) {
            pixels[y * bytes_per_row + x] = 0x00FF00FF; // Green ground
        }
    }

    // Draw player
    draw_sprite((int)game->player.x, (int)game->player.y, player_sprites[game->player.frame], 0xFFFFFFFF);

    SDL_UnlockTexture(game->texture);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, game->texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}