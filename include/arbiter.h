#ifndef SOL_ARBITER_H
#define SOL_ARBITER_H

#include "entity-animation.h"
#include "region.h"
#include "combat.h"

extern int              sol_arbiter_hits(entity_animation_node_t *ea);
extern int              sol_arbiter_enter_combat(sol_region_t *reg, const uint16_t x, const uint16_t y);
extern combat_region_t* sol_arbiter_combat_region(sol_region_t *reg);
extern sol_attack_t     sol_arbiter_entity_attack(entity_t *source, entity_t *target, int round, enum entity_action_e action);
extern void             sol_arbiter_next_round(combat_region_t* cr);

#endif
