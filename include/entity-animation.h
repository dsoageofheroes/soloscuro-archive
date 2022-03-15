#ifndef ENTITY_ANIMAION_H
#define ENTITY_ANIMAION_H

#include <stdint.h>
#include "ssi-scmd.h"

struct sol_region_s;

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
    EA_BIG_RED_DAMAGE,
    EA_GREEN_DAMAGE,
    EA_MAGIC_DAMAGE,
    EA_BROWN_DAMAGE,
    EA_DAMAGE_APPLY,
    EA_POWER_CAST,
    EA_POWER_THROW,
    EA_POWER_HIT,
    EA_POWER_APPLY,
    EA_SCMD,
    EA_ACTIVATE,
    EA_WAIT_ON_ENTITY,
    EA_END,
};

struct entity_s;
struct power_s;
struct region_s;

typedef struct entity_action_s {
    struct entity_s *source;
    struct entity_s *target;
    struct power_s  *power;
    enum entity_action_e action;
    int32_t amt, start_amt, ticks, scmd_pos;
    int32_t speed, damage;
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
extern scmd_t* entity_animation_face_direction(scmd_t *current_scmd, const enum entity_action_e action);
extern int entity_animation_region_execute(struct sol_region_s *reg);
extern int entity_animation_has_more();

entity_animation_list_t* entity_animation_list_create();
void                     entity_animation_list_free(entity_animation_list_t *list);
void                     entity_animation_list_add(entity_animation_list_t *list, enum entity_action_e action,
        struct entity_s *source, struct entity_s *target, struct power_s *power, const int32_t amt);
extern void              entity_animation_list_add_speed(entity_animation_list_t *list, enum entity_action_e action,
        struct entity_s *source, struct entity_s *target, struct power_s *power, const int32_t amt, const int32_t speed,
        const int32_t damage);
extern void              entity_animation_list_add_effect(entity_animation_list_t *list, enum entity_action_e action,
        struct entity_s *source, struct entity_s *target, struct power_s *power, const int32_t amt, const int damage);
extern int               entity_animation_list_execute(entity_animation_list_t *list, struct sol_region_s *reg);
extern int               entity_animation_list_empty(entity_animation_list_t *list);
extern int               entity_animation_list_remove_current(entity_animation_list_t *list);
extern void              sol_animation_render(const entity_action_t *ea);
extern void              entity_animation_update(struct entity_s *entity, const int16_t xdiff, const int16_t ydiff);

struct entity_s;
extern int entity_animation_execute(struct entity_s *entity);
extern scmd_t* entity_animation_get_scmd(struct entity_s *entity, const int xdiff, const int ydiff,
        const enum entity_action_e action);
#endif
