#ifndef FONT_H
#define FONT_H

#include "window-manager.h"

extern void sol_font_free();
extern uint32_t sol_font_pixel_width(sol_font_t font, const char *text, const uint32_t len);
extern uint32_t sol_font_pixel_height(sol_font_t font);

#endif
