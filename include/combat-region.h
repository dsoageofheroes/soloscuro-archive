#ifndef COMBAT_REGION_H
#define COMBAT_REGION_H

#include "entity-list.h"

#define MAX_COMBAT_OBJS (200)

typedef struct sol_combat_round_s {
    sol_entity_list_t entities;
    uint16_t num;
} sol_combat_round_t;

// Represents all combatants in the entire region.
typedef struct sol_combat_region_s {
    sol_entity_list_t combatants;
    sol_combat_round_t round;
} sol_combat_region_t;

extern sol_status_t sol_combat_region_init(sol_combat_region_t *cr);
extern sol_status_t sol_combat_check_if_over(sol_combat_region_t *cr);
extern sol_status_t sol_combat_clear(sol_combat_region_t *cr);
extern sol_status_t sol_combat_guard_check(sol_combat_region_t *cr);
extern sol_status_t sol_combat_get_current(sol_combat_region_t *cr, sol_entity_t **e);
extern sol_status_t sol_combat_next_combatant(sol_combat_region_t *cr);
extern sol_status_t sol_combat_attempt_action(sol_combat_region_t *cr, sol_dude_t *dude);
extern sol_status_t sol_combat_get_closest_enemy(sol_combat_region_t *cr, const int x, const int y, sol_entity_t **e);

#endif
