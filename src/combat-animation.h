#ifndef COMBAT_ANIMAION_H
#define COMBAT_ANIMAION_H

#include "entity.h"
#include "combat.h"

typedef struct combat_animation_node_s {
    combat_action_t ca;
    struct combat_animation_node_s *next;
} combat_animation_node_t;

extern void combat_animation_add(enum combat_action_e action, entity_t *source, entity_t *target, const int32_t amt);
extern scmd_t* combat_animation_get_scmd(scmd_t *current_scmd, const int xdiff, const int ydiff,
        const enum combat_action_e action);
extern scmd_t* combat_animation_face_direction(scmd_t *current_scmd, const enum combat_action_e action);
extern int combat_animation_execute(region_t *reg);
extern int combat_animation_has_more();

#endif
