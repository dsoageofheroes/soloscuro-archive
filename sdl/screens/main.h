#ifndef SCREEN_MAIN_H
#define SCREEN_MAIN_H

#include <SDL2/SDL.h>

void main_init(SDL_Renderer *renderer);
void main_render(void *data, SDL_Renderer *renderer);
void main_free();

#endif
