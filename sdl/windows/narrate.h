#ifndef SCRREN_NARRATE_H
#define SCRREN_NARRATE_H

#include <SDL2/SDL.h>
#include "window-manager.h"

void narrate_init(const uint32_t x, const uint32_t y);
void narrate_render(void *data);
int narrate_handle_mouse_movement(const uint32_t x, const uint32_t y);
int narrate_handle_mouse_click(const uint32_t button, const uint32_t x, const uint32_t y);
void narrate_clear();
void narrate_free();
int narrate_is_open();

extern wops_t narrate_window;

#endif
