#ifndef SCRREN_NARRATE_H
#define SCRREN_NARRATE_H

#include <SDL2/SDL.h>
#include "../screen-manager.h"

void narrate_init(SDL_Renderer *renderer);
void narrate_render(void *data, SDL_Renderer *renderer);
int narrate_handle_mouse_movement(const uint32_t x, const uint32_t y);
int narrate_handle_mouse_click(const uint32_t x, const uint32_t y);
void narrate_free();

extern sops_t narrate_screen;

#endif
