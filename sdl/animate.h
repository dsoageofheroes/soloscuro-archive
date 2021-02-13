#ifndef ANIMATE_H
#define ANIMATE_H

#include <stdint.h>
#include <SDL2/SDL.h>
#include "../src/ds-object.h"
#include "../src/ds-scmd.h"

typedef struct animate_sprite_s {
    scmd_t *scmd;
    uint16_t spr;
    uint16_t delay;
    uint16_t pos;
    uint16_t x, y;
    uint16_t destx, desty;
    float move, left_over; // see animate_tick for left_over
} animate_sprite_t;

#include "map.h"

struct map_s;

void animate_init();
void animate_clear();
void animate_close();

// NEW INTERFACE:
void animate_list_render(SDL_Renderer *renderer);
void animate_list_add(animate_sprite_t *anim, const int zpos);
#endif
