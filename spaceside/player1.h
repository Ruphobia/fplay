#ifndef PLAYER1_H
#define PLAYER1_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

void player1_init(SDL_Texture *texture);
void player1_update_and_render(float delta_time, SDL_Texture *texture, const int *top_terrain, const int *bottom_terrain, int scroll_offset);
int player1_is_dead();

#endif // PLAYER1_H