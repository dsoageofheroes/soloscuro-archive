#ifndef ENTITY_ANIMAION_H
#define ENTITY_ANIMAION_H

#include <stdint.h>
#include "ssi-scmd.h"
#include "status.h"

struct sol_region_s;

enum sol_entity_action_e {
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
    EA_DOOR_OPEN,
    EA_DOOR_CLOSE,
    EA_END,
};

struct sol_entity_s;
struct sol_power_s;
struct region_s;

typedef struct sol_entity_action_s {
    struct sol_entity_s *source;
    struct sol_entity_s *target;
    struct sol_power_s  *power;
    enum sol_entity_action_e action;
    int32_t amt, start_amt, ticks, scmd_pos;
    int32_t speed, damage;
} sol_entity_action_t;

typedef struct sol_entity_animation_node_s {
    sol_entity_action_t ca;
    struct sol_entity_animation_node_s *next;
} sol_entity_animation_node_t;

typedef struct sol_entity_animation_list_s {
    sol_entity_animation_node_t *head;
    sol_entity_animation_node_t *last_to_execute;
} sol_entity_animation_list_t;

extern sol_status_t sol_entity_animation_list_remove_references(sol_entity_animation_list_t *list, struct sol_entity_s *dead);
extern sol_status_t sol_animation_render(const sol_entity_action_t *ea);
extern sol_status_t sol_entity_animation_update(struct sol_entity_s *entity, const int16_t xdiff, const int16_t ydiff);
extern sol_status_t sol_entity_animation_list_empty(sol_entity_animation_list_t *list);
extern sol_status_t sol_entity_animation_list_remove_current(sol_entity_animation_list_t *list);
extern sol_status_t sol_entity_animation_execute(struct sol_entity_s *entity);
extern sol_status_t sol_entity_animation_region_execute(struct sol_region_s *reg);
extern sol_status_t sol_entity_animation_list_free(sol_entity_animation_list_t *list);
extern sol_status_t sol_entity_animation_list_add(sol_entity_animation_list_t *list, enum sol_entity_action_e action,
        struct sol_entity_s *source, struct sol_entity_s *target, struct sol_power_s *power, const int32_t amt);
extern sol_status_t sol_entity_animation_list_add_speed(sol_entity_animation_list_t *list, enum sol_entity_action_e action,
        struct sol_entity_s *source, struct sol_entity_s *target, struct sol_power_s *power, const int32_t amt, const int32_t speed,
        const int32_t damage);
extern sol_status_t sol_entity_animation_list_add_effect(sol_entity_animation_list_t *list, enum sol_entity_action_e action,
        struct sol_entity_s *source, struct sol_entity_s *target, struct sol_power_s *power, const int32_t amt, const int damage);
extern sol_status_t sol_entity_animation_add(enum sol_entity_action_e action, struct sol_entity_s *source, struct sol_entity_s *target,
        struct sol_power_s *power, const int32_t amt);
extern sol_status_t sol_entity_animation_face_direction(scmd_t *current_scmd, const enum sol_entity_action_e action, scmd_t **ret);
extern sol_status_t sol_entity_animation_get_scmd(struct sol_entity_s *entity, const int xdiff, const int ydiff,
        const enum sol_entity_action_e action, scmd_t **ret);

#endif
