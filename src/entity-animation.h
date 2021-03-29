#ifndef ENTITY_ANIMAION_H
#define ENTITY_ANIMAION_H

#include "entity.h"
#include "combat.h"

typedef struct entity_animation_node_s {
    entity_action_t ca;
    struct entity_animation_node_s *next;
} entity_animation_node_t;

extern void entity_animation_add(enum entity_action_e action, entity_t *source, entity_t *target, const int32_t amt);
extern scmd_t* entity_animation_get_scmd(scmd_t *current_scmd, const int xdiff, const int ydiff,
        const enum entity_action_e action);
extern scmd_t* entity_animation_face_direction(scmd_t *current_scmd, const enum entity_action_e action);
extern int entity_animation_execute(region_t *reg);
extern int entity_animation_has_more();

#endif
