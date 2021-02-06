#ifndef DS_PLAYER_H
#define DS_PLAYER_H

#include "dsl-object.h"

void ds_player_init();
int ds_player_exists(const int slot);
int ds_player_load_character_charsave(const int slot, const int res_id);
ds1_combat_t* ds_player_get_combat(const int slot);
ds_character_t* ds_player_get_char(const int slot);

#endif
