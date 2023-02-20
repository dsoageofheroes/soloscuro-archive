#ifndef DS_COMBAT_H
#define DS_COMBAT_H

#include "ssi-object.h"
#include "wizard.h"
#include "entity.h"
#include "region.h"
#include "effect.h"
#include "entity-list.h"
#include "entity-animation.h"

#define MAX_COMBAT_ACTIONS (100)

typedef struct sol_attack_s {
    int16_t damage;
    sol_effect_type_t effect;
} sol_attack_t;

typedef enum sol_combat_scmd_e {
    COMBAT_SCMD_STAND_DOWN,
    COMBAT_SCMD_STAND_UP,
    COMBAT_SCMD_STAND_RIGHT,
    COMBAT_SCMD_STAND_LEFT,
    COMBAT_SCMD_MOVE_DOWN,
    COMBAT_SCMD_MOVE_UP,
    COMBAT_SCMD_MOVE_RIGHT,
    COMBAT_SCMD_MOVE_LEFT,
    COMBAT_SCMD_PLAYER_MOVE_DOWN,
    COMBAT_SCMD_PLAYER_MOVE_UP,
    COMBAT_SCMD_PLAYER_MOVE_RIGHT,
    COMBAT_SCMD_PLAYER_MOVE_LEFT,
    COMBAT_SCMD_MELEE_DOWN,
    COMBAT_SCMD_MELEE_UP,
    COMBAT_SCMD_MELEE_RIGHT,
    COMBAT_SCMD_MELEE_LEFT,
    COMBAT_POWER_CAST,
    COMBAT_POWER_THROW,
    COMBAT_POWER_HIT,
} sol_combat_scmd_t;

enum {
    COMBAT_STATUS_OK          = 1,
    COMBAT_STATUS_STUNNED     = 2,
    COMBAT_STATUS_UNCONSCIOUS = 3,
    COMBAT_STATUS_DYING       = 4,
    COMBAT_STATUS_ANIMATED    = 5,
    COMBAT_STATUS_PETRIFIED   = 6,
    COMBAT_STATUS_DEAD        = 7,
    COMBAT_STATUS_GONE        = 8,
    COMBAT_STATUS_MAX
};

extern sol_status_t sol_combat_set_scmd(sol_entity_t *e, const sol_combat_scmd_t scmd);
extern sol_status_t sol_combat_update_scmd_info(sol_entity_t *dude);
extern sol_status_t sol_combat_get_scmd(const sol_combat_scmd_t type, scmd_t **);
extern sol_status_t sol_combat_active(sol_combat_region_t *cr);
extern sol_status_t sol_combat_activate_power(sol_power_t *pw, sol_entity_t *source, sol_entity_t *target, const int32_t x, const int32_t y);
extern sol_status_t sol_combat_add_attack_animation(sol_region_t *reg, sol_dude_t *dude, sol_entity_t *target,
                                              sol_power_t *power, enum sol_entity_action_e action);
extern sol_status_t sol_combat_guard(sol_entity_t *entity);
extern sol_status_t sol_combat_kill_all_enemies();

#endif
