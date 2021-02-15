#ifndef PLAYER_H
#define PLAYER_H

#include "animate.h"
#include <stdint.h>

typedef struct inventory_sprites_s {
    uint16_t arm;
    uint16_t ammo;
    uint16_t missle;
    uint16_t hand0;
    uint16_t finger0;
    uint16_t waist;
    uint16_t legs;
    uint16_t head;
    uint16_t neck;
    uint16_t chest;
    uint16_t hand1;
    uint16_t finger1;
    uint16_t cloak;
    uint16_t foot;
    uint16_t bp[12];
} inventory_sprites_t;

typedef struct player_s {
    int32_t x, y;
} player_t;

void player_init();
void player_close();
void player_load_graphics(SDL_Renderer *rend);
void player_render(SDL_Renderer *rend, const int slot);
void player_move(const uint8_t direction);
void player_unmove(const uint8_t direction);
void player_load(SDL_Renderer *renderer, const int slot, const float zoom);
int32_t player_getx(const int slot);
int32_t player_gety(const int slot);
void player_center(const int i, const int x, const int y, const int w, const int h);
void player_center_portrait(const int i, const int x, const int y, const int w, const int h);
void player_render_portrait(SDL_Renderer *rend, const int slot);
void player_update();
inventory_sprites_t* player_get_inventory_sprites(const int slot);
void player_add_to_animation_list();
void player_set_delay(const int amt);
void player_set_move(const int amt);

enum{PLAYER_UP = 0x01, PLAYER_DOWN = 0x02, PLAYER_LEFT = 0x04, PLAYER_RIGHT = 0x08};

#endif
