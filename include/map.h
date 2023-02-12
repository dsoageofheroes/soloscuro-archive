#ifndef MAP_H
#define MAP_H

#include "region.h"
#include "window-manager.h"

extern sol_status_t sol_map_load_region(sol_region_t *reg);
extern sol_status_t sol_map_apply_alpha(const uint8_t alpha);
extern sol_status_t sol_map_highlight_tile(const int tilex, const int tiley, const int frame);
extern sol_status_t sol_map_place_entity(entity_t *entity);
extern sol_status_t sol_map_game_over();
extern sol_status_t sol_map_update_active_player(const int prev);
extern sol_status_t sol_map_clear_highlight();
extern sol_status_t sol_map_set_pause(int pause);
extern sol_status_t sol_map_is_paused();

extern sol_wops_t map_window;

#endif
