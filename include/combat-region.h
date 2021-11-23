#ifndef COMBAT_REGION_H
#define COMBAT_REGION_H

#include "entity-list.h"

#define MAX_COMBAT_OBJS (200)
struct entity_list_s;

// Represents all combatants in the entire region.
typedef struct combat_region_s {
    struct entity_list_s *combatants;
    uint8_t hunt[MAX_COMBAT_OBJS];
    uint32_t pos;
    uint8_t in_combat_mode;
} combat_region_t;

#endif
