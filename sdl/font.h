#ifndef FONT_H
#define FONT_H

#include <SDL2/SDL.h>
#include "window-manager.h"
#include "../src/port.h"

extern void font_init(SDL_Renderer *renderer);
extern void font_free();
uint32_t font_pixel_width(sol_font_t font, const char *text, const uint32_t len);
uint16_t font_char_width(sol_font_t font, const int c);
uint32_t font_pixel_height(sol_font_t font);
void font_render_ttf(const char *msg, uint16_t x, uint16_t y, uint32_t color);
void print_line_len(SDL_Renderer *renderer, sol_font_t font, const char *text, size_t x, size_t y, const uint32_t len);
void font_render_center(SDL_Renderer *rend, sol_font_t font, const char *str, const SDL_Rect loc);

#endif
