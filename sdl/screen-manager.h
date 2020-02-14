#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <SDL2/SDL.h>

void screen_init(SDL_Renderer *renderer);
void screen_render(SDL_Renderer *renderer, const uint32_t xmappos, const uint32_t ymappos);
void screen_free();

#endif
