#ifndef COMBAT_STATUS_H
#define COMBAT_STATUS_H

#include <SDL2/SDL.h>
#include "../screen-manager.h"
#include "../../src/combat.h"

typedef struct combat_status_s {
    char name[40];
    uint32_t current_hp, max_hp;
    uint8_t status;
    uint16_t move;
} combat_status_t;

combat_status_t* combat_status_get();

extern sops_t combat_status_screen;

#endif
