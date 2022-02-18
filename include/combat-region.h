#ifndef COMBAT_REGION_H
#define COMBAT_REGION_H

#include "entity-list.h"

#define MAX_COMBAT_OBJS (200)

typedef struct combat_round_s {
    entity_list_t entities;
    uint16_t num;
    uint8_t attack_count;
    uint8_t cast_count;
} combat_round_t;

// Represents all combatants in the entire region.
typedef struct combat_region_s {
    entity_list_t combatants;
    combat_round_t round;
} combat_region_t;

extern void      sol_combat_region_init(combat_region_t *cr);
extern int       sol_combat_check_if_over(combat_region_t *cr);
extern void      sol_combat_clear(combat_region_t *cr);
extern entity_t* sol_combat_get_current(combat_region_t *cr);
extern void      sol_combat_next_combatant(combat_region_t *cr);
extern int       sol_combat_attempt_action(combat_region_t *cr, dude_t *dude);

#endif
