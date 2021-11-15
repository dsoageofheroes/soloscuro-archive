#ifndef FONT_H
#define FONT_H

#include <SDL2/SDL.h>
#include "window-manager.h"

typedef uint16_t font_t;

enum {
    FONT_YELLOW,
    FONT_GREY,
    FONT_GREYLIGHT,
    FONT_BLACK,
    FONT_BLACKDARK,
    FONT_RED,
    FONT_REDDARK,
    FONT_BLUE,
    NUM_FONTS
};

extern void font_init(SDL_Renderer *renderer);
extern void font_free();
uint32_t font_pixel_width(font_t font, const char *text, const uint32_t len);
uint16_t font_char_width(font_t font, const int c);
uint32_t font_pixel_height(font_t font);
void font_render_ttf(const char *msg, uint16_t x, uint16_t y, uint32_t color);
void print_line_len(SDL_Renderer *renderer, font_t font, const char *text, size_t x, size_t y, const uint32_t len);
void font_render_center(SDL_Renderer *rend, font_t font, const char *str, const SDL_Rect loc);

#endif
