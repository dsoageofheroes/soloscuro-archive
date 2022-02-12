#ifndef SOL_ARBITER_H
#define SOL_ARBITER_H

#include "entity-animation.h"
#include "region.h"

extern int sol_arbiter_hits(entity_animation_node_t *ea);
extern int sol_arbiter_enter_combat(sol_region_t *reg, const uint16_t x, const uint16_t y);
extern int sol_arbiter_is_in_combat(sol_region_t *reg);

#endif
