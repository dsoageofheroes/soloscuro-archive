#ifndef SCREEN_MAIN_H
#define SCREEN_MAIN_H

#include <SDL2/SDL.h>
#include "../screen-manager.h"

void main_render(void *data, SDL_Renderer *renderer);
int main_handle_mouse_movement(const uint32_t x, const uint32_t y);
int main_handle_mouse_click(const uint32_t button, const uint32_t x, const uint32_t y);
void main_free();

extern sops_t main_screen;

#endif
