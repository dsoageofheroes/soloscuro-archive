#ifndef SOL_BACKGROUND_H
#define SOL_BACKGROUND_H

#include "region.h"

extern void sol_background_load_region(sol_region_t *_region);
extern void sol_background_apply_alpha(const uint8_t alpha);
extern void sol_background_render();
extern void sol_background_free();

#endif
