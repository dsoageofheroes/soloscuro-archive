#ifndef SCREEN_MAIN_H
#define SCREEN_MAIN_H

#include <SDL2/SDL.h>
#include "../window-manager.h"

void main_render(void *data);
int main_handle_mouse_movement(const uint32_t x, const uint32_t y);
int main_handle_mouse_click(const uint32_t button, const uint32_t x, const uint32_t y);
void main_free();

extern wops_t main_window;

#endif
