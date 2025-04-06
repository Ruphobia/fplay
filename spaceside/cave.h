#ifndef CAVE_H
#define CAVE_H

#include <SDL2/SDL.h>

// Screen dimensions and terrain width
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TERRAIN_WIDTH (SCREEN_WIDTH * 10)

void cave_init(SDL_Texture *texture);
void cave_update_and_render(float delta_time, SDL_Texture *texture);

// Accessors for terrain and offset
extern int top_terrain[TERRAIN_WIDTH];
extern int bottom_terrain[TERRAIN_WIDTH];
float cave_get_scroll_offset(void);

#endif // CAVE_H