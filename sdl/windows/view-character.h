#ifndef VIEW_CHARACTER_H
#define VIEW_CHARACTER_H

#include <SDL2/SDL.h>
#include "../window-manager.h"

void view_character_render(void *data);
int view_character_handle_mouse_movement(const uint32_t x, const uint32_t y);
int view_character_handle_mouse_click(const uint32_t button, const uint32_t x, const uint32_t y);
void view_character_free();

extern wops_t view_character_window;

#endif
