#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdint.h>

#define SPRITE_ERROR (0xFFFF)

struct entity_s; // forward declaration
struct scmd_s; // forward declaration

typedef struct animate_sprite_s {
    uint16_t spr;
    uint16_t flags;
    uint16_t delay;
    uint16_t pos;
    uint16_t x, y, w, h;
    uint16_t destx, desty;
    uint32_t bmp_id;
    float movex, movey, left_over; // see animate_tick for left_over
    struct entity_s *entity;
    struct scmd_s *scmd;
} animate_sprite_t;

typedef struct animate_sprite_node_s {
    animate_sprite_t *anim;
    struct animate_sprite_node_s *next;
    struct animate_sprite_node_s *prev;
} animate_sprite_node_t;

typedef struct sprite_info_s {
    int16_t bmp_id;     // Which bmp this is.
    int16_t xoffset;    // bitmap offset x
    int16_t yoffset;    // bitmap offset y
    uint16_t flags;     // sprite/scmd flags
    //animate_sprite_t anim; // Data is directly stored here. Any list data is stored later.
    animate_sprite_node_t *data;         // used for special data the UI needs (IE: SDL.)
} sprite_info_t;

typedef animate_sprite_node_t animation_node_t;

typedef struct animation_list_s {
    animation_node_t *head;
} animation_list_t;

struct entity_s;
struct entity_action_s;
struct entity_list_node_s;
struct entity_list_s;

animate_sprite_node_t* animate_sprite_node_create();
void                   animation_node_free(animation_node_t *node);
void                   animation_shift_node(animate_sprite_node_t *an);
extern void            animation_shift_entity(struct entity_list_s *list, struct entity_list_node_s *en);

void                   animate_sprite_tick(struct entity_action_s *action, struct entity_s *entity );
animation_list_t*      animation_list_create();
void                   animation_list_free(animation_list_t *al);
animation_node_t*      animation_list_add(animation_list_t *al, animate_sprite_t *anim);
animation_node_t*      animation_list_remove(animation_list_t *al, animation_node_t *node);
animation_node_t*      animation_list_find(animation_list_t *al, animate_sprite_t *anim);

#define animation_list_for_each(el, obj) \
    obj = (el && el->head) ? el->head->anim : NULL; \
    for (animation_node_t *__el_rover = (el->head); __el_rover;\
        __el_rover = __el_rover->next, obj = __el_rover ? __el_rover->anim : NULL)

#include "item.h"

#endif
