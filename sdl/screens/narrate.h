#ifndef SCRREN_NARRATE_H
#define SCRREN_NARRATE_H

#include <SDL2/SDL.h>
#include "../screen-manager.h"

void print_line_len(SDL_Renderer *renderer, const char *text, size_t x, size_t y, const uint32_t len);

void narrate_init(SDL_Renderer *renderer, const uint32_t x, const uint32_t y, const float zoom);
void narrate_render(void *data, SDL_Renderer *renderer);
int narrate_handle_mouse_movement(const uint32_t x, const uint32_t y);
int narrate_handle_mouse_click(const uint32_t button, const uint32_t x, const uint32_t y);
void narrate_free();

extern sops_t narrate_screen;

#endif
