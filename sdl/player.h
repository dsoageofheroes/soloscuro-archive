#ifndef PLAYER_H
#define PLAYER_H

#include "../src/combat.h"
#include <stdint.h>

void player_move(const uint8_t direction);
void player_unmove(const uint8_t direction);
void player_render_portrait(const int slot);
void player_update();
void player_set_delay(const int amt);
void player_set_move(const int amt);
entity_t* player_get_entity(const int slot);
extern void player_condense();

//enum{PLAYER_UP = 0x01, PLAYER_DOWN = 0x02, PLAYER_LEFT = 0x04, PLAYER_RIGHT = 0x08};

#endif
