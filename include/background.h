#ifndef SOL_BACKGROUND_H
#define SOL_BACKGROUND_H

#include "region.h"

extern sol_status_t sol_background_load_region(sol_region_t *_region);
extern sol_status_t sol_background_render_box(const int32_t startx, const int32_t starty);
extern sol_status_t sol_background_apply_alpha(const uint8_t alpha);
extern sol_status_t sol_background_render();
extern sol_status_t sol_background_free();

#endif
