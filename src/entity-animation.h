#ifndef ENTITY_ANIMAION_H
#define ENTITY_ANIMAION_H

#include <stdint.h>
#include "ds-scmd.h"

enum entity_action_e {
    EA_NONE,
    EA_WALK_LEFT,
    EA_WALK_RIGHT,
    EA_WALK_UP,
    EA_WALK_DOWN,
    EA_WALK_UPLEFT,
    EA_WALK_UPRIGHT,
    EA_WALK_DOWNLEFT,
    EA_WALK_DOWNRIGHT,
    EA_MELEE,
    EA_MISSILE,
    EA_PSIONIC,
    EA_SPELL,
    EA_WAIT,
    EA_GUARD,
    EA_RED_DAMAGE,
    EA_GREEN_DAMAGE,
    EA_DAMAGE_APPLY,
    EA_POWER_CAST,
    EA_POWER_THROW,
    EA_POWER_HIT,
    EA_END,
};

struct entity_s;
struct power_s;
struct region_s;

typedef struct entity_action_s {
    enum entity_action_e action;
    struct entity_s *source;
    struct entity_s *target;
    struct power_s  *power;
    int32_t amt;
} entity_action_t;

typedef struct entity_animation_node_s {
    entity_action_t ca;
    struct entity_animation_node_s *next;
} entity_animation_node_t;

typedef struct entity_animation_list_s {
    entity_animation_node_t *head;
    entity_animation_node_t *last_to_execute;
} entity_animation_list_t;

extern void entity_animation_add(enum entity_action_e action, struct entity_s *source, struct entity_s *target,
        struct power_s *power, const int32_t amt);
extern scmd_t* entity_animation_get_scmd(scmd_t *current_scmd, const int xdiff, const int ydiff,
        const enum entity_action_e action);
extern scmd_t* entity_animation_face_direction(scmd_t *current_scmd, const enum entity_action_e action);
extern int entity_animation_execute(struct region_s *reg);
extern int entity_animation_has_more();

entity_animation_list_t* entity_animation_list_create();
void entity_animation_list_free(entity_animation_list_t *list);
void entity_animation_list_add(entity_animation_list_t *list, enum entity_action_e action,
        struct entity_s *source, struct entity_s *target, struct power_s *power, const int32_t amt);
extern int entity_animation_list_execute(entity_animation_list_t *list, struct region_s *reg);

#endif
