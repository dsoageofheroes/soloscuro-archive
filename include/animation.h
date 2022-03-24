#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdint.h>
#include "ssi-scmd.h"

#define SPRITE_ERROR (0xFFFF)

struct entity_s; // forward declaration
typedef struct animate_sprite_s {
    uint16_t spr;
    uint16_t flags;
    uint16_t delay;
    uint16_t pos;
    uint16_t x, y, w, h;
    uint16_t destx, desty;
    uint32_t bmp_id;
    int16_t xoffset, yoffset;      // offset for (x, y)
    float movex, movey, left_over; // see animate_tick for left_over
    struct entity_s *entity;
    scmd_t *scmd;
} animate_sprite_t;

struct entity_s;
struct entity_action_s;
struct entity_list_node_s;
struct entity_list_s;

extern void animation_shift_entity(struct entity_list_s *list, struct entity_list_node_s *en);
extern void animate_sprite_tick(struct entity_action_s *action, struct entity_s *entity );

#endif
