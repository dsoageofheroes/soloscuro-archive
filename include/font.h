#ifndef FONT_H
#define FONT_H

#include "window-manager.h"

extern sol_status_t sol_font_free();
extern sol_status_t sol_font_pixel_width(sol_font_t font, const char *text, const uint32_t len, uint32_t *width);
extern sol_status_t sol_font_pixel_height(sol_font_t font, uint32_t *height);

#endif
