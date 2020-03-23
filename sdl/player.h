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
void player_render(SDL_Renderer *rend);
void player_move(const uint8_t direction);

enum{PLAYER_UP, PLAYER_DOWN, PLAYER_LEFT, PLAYER_RIGHT};

#endif
