#ifndef PLAYER_H
#define PLAYER_H

#include "animate.h"
#include "../src/combat.h"
#include <stdint.h>

void player_init();
void player_close();
void player_load_graphics(const int slot);
void player_render(SDL_Renderer *rend, const int slot);
void player_move(const uint8_t direction);
void player_unmove(const uint8_t direction);
void player_load(const int slot, const float zoom);
void player_center(const int i, const int x, const int y, const int w, const int h);
void player_center_portrait(const int i, const int x, const int y, const int w, const int h);
void player_render_portrait(SDL_Renderer *rend, const int slot);
void player_update();
void player_set_delay(const int amt);
void player_set_move(const int amt);
uint16_t player_get_sprite(const int slot);
entity_t* player_get_entity(const int slot);
extern void player_condense();

enum{PLAYER_UP = 0x01, PLAYER_DOWN = 0x02, PLAYER_LEFT = 0x04, PLAYER_RIGHT = 0x08};

#endif
