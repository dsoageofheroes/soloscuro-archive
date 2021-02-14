#ifndef ANIMATE_H
#define ANIMATE_H

#include <stdint.h>
#include <SDL2/SDL.h>
#include "../src/ds-object.h"
#include "../src/ds-scmd.h"
#include "map.h"

typedef struct animate_sprite_s {
    scmd_t *scmd;
    uint16_t spr;
    uint16_t delay;
    uint16_t pos;
    uint16_t x, y;
    uint16_t destx, desty;
    region_object_t *obj;
    float move, left_over; // see animate_tick for left_over
} animate_sprite_t;

typedef struct animate_sprite_node_s {
    animate_sprite_t *anim;
    struct animate_sprite_node_s *next;
    struct animate_sprite_node_s *prev;
} animate_sprite_node_t;

void animate_init();
void animate_clear();
void animate_close();

// NEW INTERFACE:
void animate_list_render(SDL_Renderer *renderer);
animate_sprite_node_t *animate_list_add(animate_sprite_t *anim, const int zpos);
void animate_list_remove(animate_sprite_node_t *node, const int zpos);
void animate_shift_node(animate_sprite_node_t *an, const int zpos);

#endif
