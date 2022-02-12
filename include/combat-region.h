#ifndef COMBAT_REGION_H
#define COMBAT_REGION_H

#include "entity-list.h"

#define MAX_COMBAT_OBJS (200)
struct entity_list_s;

// Represents all combatants in the entire region.
typedef struct combat_region_s {
    entity_list_t combatants;
} combat_region_t;

extern int    sol_combat_check_if_over(combat_region_t *cr);
extern void   sol_combat_clear(combat_region_t *cr);

#endif
