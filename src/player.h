#ifndef DS_PLAYER_H
#define DS_PLAYER_H

#include "entity.h"

#define MAX_PCS (4)

// New Interface
void             sol_player_init();
extern int       player_exists(const int slot);
extern entity_t* player_get(const int slot);
extern void      player_set(const int slot, entity_t *dude);
extern void      player_set_active(const int slot);
extern void      player_cleanup();
extern entity_t* player_get_active();
extern int       player_get_active_slot();
extern int       player_get_slot(entity_t *entity);
extern void      player_free(const int slot);
extern int       player_ai(const int slot);
extern void      player_set_ai(const int slot, const int ai);
extern void      sol_player_center(const int i, const int x, const int y, const int w, const int h);
extern void      sol_player_center_portrait(const int i, const int x, const int y, const int w, const int h);
extern uint16_t  sol_player_get_sprite(const int slot);
extern void      sol_player_render(const int slot);
extern void      sol_player_load(const int slot, const float zoom);
extern void      sol_player_load_graphics(const int slot);

enum{PLAYER_UP = 0x01, PLAYER_DOWN = 0x02, PLAYER_LEFT = 0x04, PLAYER_RIGHT = 0x08};

#endif
