#ifndef SCROLLER_H
#define SCROLLER_H

#include <SDL2/SDL.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GROUND_HEIGHT 100
#define PLAYER_SIZE 8
#define MAX_PITS 10

// Player structure
typedef struct {
    float x, y;         // Position (y for jumping)
    float vel_x, vel_y; // Velocity
    int jumping;        // Jump state
    int frame;          // Animation frame
} Player;

// Pit structure
typedef struct {
    int x;              // Pit position in world space
    int width;          // Pit width
} Pit;

// Game state
typedef struct {
    Player player;
    Pit pits[MAX_PITS];
    int world_offset;   // Scrolling offset
    SDL_Texture *texture;
} GameState;

void init_game(GameState *game, SDL_Texture *texture);
int update_game(GameState *game, const Uint8 *keys); // Changed from void to int
void draw_game(GameState *game, SDL_Renderer *renderer);

#endif