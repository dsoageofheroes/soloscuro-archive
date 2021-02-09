#ifndef PLAYER_H
#define PLAYER_H

#include "animate.h"
#include <stdint.h>

typedef struct player_s {
    int32_t x, y;
    animate_t *anim;
} player_t;

void player_init();
void player_load_graphics(SDL_Renderer *rend);
void player_render(SDL_Renderer *rend, const int slot);
void player_move(const uint8_t direction);
void player_load(SDL_Renderer *renderer, const int slot, const float zoom);
int32_t player_getx(const int slot);
int32_t player_gety(const int slot);
void player_center(const int i, const int x, const int y, const int w, const int h);
void player_center_portrait(const int i, const int x, const int y, const int w, const int h);
void player_render_portrait(SDL_Renderer *rend, const int slot);
void player_set_loc(const int slot, const int32_t x, const int32_t y);

enum{PLAYER_UP, PLAYER_DOWN, PLAYER_LEFT, PLAYER_RIGHT};

#endif
