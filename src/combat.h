#ifndef DS_COMBAT_H
#define DS_COMBAT_H

#include "ds-object.h"

#define MAX_COMBAT_OBJS (200)
#define COMBAT_ERROR    (999999)

enum combat_scmd_t {
    COMBAT_SCMD_STAND_DOWN,
    COMBAT_SCMD_STAND_UP,
    COMBAT_SCMD_STAND_RIGHT,
    COMBAT_SCMD_STAND_LEFT,
    COMBAT_SCMD_MOVE_DOWN,
    COMBAT_SCMD_MOVE_UP,
    COMBAT_SCMD_MOVE_RIGHT,
    COMBAT_SCMD_MOVE_LEFT,
};

// Represents all combatants in the entire region.
typedef struct combat_region_s {
    ds1_combat_t combats[MAX_COMBAT_OBJS];
    uint32_t pos;
    uint8_t in_combat_mode;
} combat_region_t;

void combat_init(combat_region_t *cr);
void combat_free(combat_region_t *rc);
uint32_t combat_add(combat_region_t *rc, ds1_combat_t *combat);
scmd_t* combat_get_scmd(const enum combat_scmd_t type);

#endif
