#ifndef PLAYER2_H
#define PLAYER2_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

void player2_init(SDL_Texture *texture);
void player2_update_and_render(float delta_time, SDL_Texture *texture, const int *top_terrain, const int *bottom_terrain, int scroll_offset);
int player2_is_dead();

#endif // PLAYER2_H