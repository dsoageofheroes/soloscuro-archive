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

enum{PLAYER_UP = 0x01, PLAYER_DOWN = 0x02, PLAYER_LEFT = 0x04, PLAYER_RIGHT = 0x08};

#endif
