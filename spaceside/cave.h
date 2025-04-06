#ifndef CAVE_H
#define CAVE_H

#include <SDL2/SDL.h>

// Screen dimensions and terrain width
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TERRAIN_WIDTH (SCREEN_WIDTH * 10)
#define MAX_FUEL_PODS 20

void cave_init(SDL_Texture *texture);
void cave_update_and_render(float delta_time, SDL_Texture *texture);

// Accessors for terrain and offset
extern int top_terrain[TERRAIN_WIDTH];
extern int bottom_terrain[TERRAIN_WIDTH];
float cave_get_scroll_offset(void);

// Fuel pod structure and accessors
typedef struct {
    float x, y;     // Position
    int active;     // 1 if active, 0 if consumed
} FuelPod;

int cave_get_fuel_pod_count(void);
FuelPod* cave_get_fuel_pods(void);
void cave_consume_fuel_pod(int index);

#endif // CAVE_H