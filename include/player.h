#ifndef DS_PLAYER_H
#define DS_PLAYER_H

#include "entity.h"

#define MAX_PCS (4)
extern sol_status_t sol_player_move(const uint8_t direction);
extern sol_status_t sol_player_unmove(const uint8_t direction);
extern sol_status_t sol_player_render_portrait(const int slot);
extern sol_status_t sol_player_update();
extern sol_status_t sol_player_set_delay(const int amt);
extern sol_status_t sol_player_condense();

// New Interface
extern sol_status_t sol_player_init();
extern sol_status_t sol_player_set(const int slot, sol_entity_t *dude);
extern sol_status_t sol_player_set_active(const int slot);
extern sol_status_t sol_player_cleanup();
extern sol_status_t sol_player_free(const int slot);
extern sol_status_t sol_player_set_ai(const int slot, const int ai);
extern sol_status_t sol_player_center(const int i, const int x, const int y, const int w, const int h);
extern sol_status_t sol_player_center_portrait(const int i, const int x, const int y, const int w, const int h);
extern sol_status_t sol_player_render(const int slot);
extern sol_status_t sol_player_load(const int slot);
extern sol_status_t sol_player_load_zoom(const int slot, const float zoom);
extern sol_status_t sol_player_load_graphics(const int slot);
extern sol_status_t sol_player_close();
extern sol_status_t sol_player_get(const int slot, sol_entity_t **e);
extern sol_status_t sol_player_get_active( sol_entity_t **e);
extern sol_status_t sol_player_get_inventory(const int slot, sol_inventory_t **i);
extern sol_status_t sol_player_get_sprite(const int slot, sol_sprite_t *sprite);
extern sol_status_t sol_player_exists(const int slot);
extern sol_status_t sol_player_get_slot(entity_t *entity, int *slot);
extern sol_status_t sol_player_ai(const int slot);

enum{PLAYER_UP = 0x01, PLAYER_DOWN = 0x02, PLAYER_LEFT = 0x04, PLAYER_RIGHT = 0x08};

#endif
