#ifndef MAP_H
#define MAP_H

#include "region.h"
#include "window-manager.h"

typedef struct map_s {
    sol_region_t *region;
} map_t;

void map_load_region(sol_region_t *reg);
void map_render(void *data);
int map_handle_mouse(const uint32_t x, const uint32_t y);
int map_handle_mouse_click(const uint32_t button, const uint32_t x, const uint32_t y);
void map_apply_alpha(const uint8_t alpha);
extern void map_highlight_tile(const int tilex, const int tiley, const int frame);

int cmap_is_block(const int row, const int column);
int cmap_is_actor(const int row, const int column);
int cmap_is_danger(const int row, const int column);

extern sol_wops_t map_window;

#endif
