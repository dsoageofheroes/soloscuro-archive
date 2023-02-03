#ifndef COMBAT_STATUS_H
#define COMBAT_STATUS_H

#include "window-manager.h"
#include "combat.h"

typedef struct combat_status_s {
    char name[40];
    uint32_t current_hp, max_hp;
    uint8_t status;
    uint16_t move;
} combat_status_t;

extern sol_wops_t combat_status_window;

extern sol_status_t sol_combat_action(const entity_action_t *ca);
extern sol_status_t sol_combat_clear_damage();
extern sol_status_t sol_combat_status_get(combat_status_t **cs);

#endif
