#ifndef SOL_ARBITER_H
#define SOL_ARBITER_H

#include "entity-animation.h"
#include "region.h"
#include "combat.h"

extern sol_status_t sol_arbiter_hits(sol_entity_animation_node_t *ea);
extern sol_status_t sol_arbiter_enter_combat(sol_region_t *reg, const uint16_t x, const uint16_t y);
extern sol_status_t sol_arbiter_combat_region(sol_region_t *reg, sol_combat_region_t **cr);
extern sol_status_t sol_arbiter_entity_attack(sol_entity_t *source, sol_entity_t *target, int round, enum sol_entity_action_e action, sol_attack_t *attack);
extern sol_status_t sol_arbiter_next_round(sol_combat_region_t* cr);
extern sol_status_t sol_arbiter_combat_check(sol_region_t* reg);
extern sol_status_t sol_arbiter_in_combat(sol_dude_t *dude);

#endif
