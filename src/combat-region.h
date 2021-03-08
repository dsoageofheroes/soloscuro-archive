#ifndef COMBAT_REGION_H
#define COMBAT_REGION_H

#include "entity-list.h"

#define MAX_COMBAT_OBJS (200)

// Represents all combatants in the entire region.
typedef struct combat_region_s {
    entity_list_t *combatants;
    uint8_t hunt[MAX_COMBAT_OBJS];
    uint32_t pos;
    uint8_t in_combat_mode;
} combat_region_t;

#endif
