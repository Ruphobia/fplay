#ifndef CAVE_H
#define CAVE_H

#include <SDL2/SDL.h>

void cave_init(SDL_Texture *texture);
void cave_update_and_render(float delta_time, SDL_Texture *texture);

#endif // CAVE_H