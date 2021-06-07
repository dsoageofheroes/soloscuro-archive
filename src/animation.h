#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdint.h>
#include "ssi-object.h"
#include "ssi-scmd.h"
#include "entity.h"

typedef struct animate_sprite_s {
    scmd_t *scmd;
    uint16_t spr;
    uint16_t delay;
    uint16_t pos;
    uint16_t x, y, w, h;
    uint16_t destx, desty;
    entity_t *entity;
    float movex, movey, left_over; // see animate_tick for left_over
} animate_sprite_t;

typedef struct animate_sprite_node_s {
    animate_sprite_t *anim;
    struct animate_sprite_node_s *next;
    struct animate_sprite_node_s *prev;
} animate_sprite_node_t;

typedef animate_sprite_node_t animation_node_t;

typedef struct animation_list_s {
    animation_node_t *head;
} animation_list_t;

animate_sprite_node_t* animate_sprite_node_create();
void animation_node_free(animation_node_t *node);
void animation_shift_node(animate_sprite_node_t *an);

animation_list_t* animation_list_create();
void animation_list_free(animation_list_t *al);
animation_node_t* animation_list_add(animation_list_t *al, animate_sprite_t *anim);
animation_node_t* animation_list_remove(animation_list_t *al, animation_node_t *node);

#endif
