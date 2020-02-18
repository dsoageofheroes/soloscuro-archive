#ifndef SCREEN_MAIN_H
#define SCREEN_MAIN_H

#include <SDL2/SDL.h>

void main_init(SDL_Renderer *renderer);
void main_render(void *data, SDL_Renderer *renderer);
int main_handle_mouse_movement(const uint32_t x, const uint32_t y);
int main_handle_mouse_click(const uint32_t x, const uint32_t y);
void main_free();

#endif
