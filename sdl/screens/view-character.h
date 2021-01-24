#ifndef VIEW_CHARACTER_H
#define VIEW_CHARACTER_H

#include <SDL2/SDL.h>
#include "../screen-manager.h"

void view_character_init(SDL_Renderer *renderer, const uint32_t x, const uint32_t y, const float zoom);
void view_character_render(void *data, SDL_Renderer *renderer);
int view_character_handle_mouse_movement(const uint32_t x, const uint32_t y);
int view_character_handle_mouse_click(const uint32_t button, const uint32_t x, const uint32_t y);
void view_character_free();

extern sops_t view_character_screen;

#endif
