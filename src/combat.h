#ifndef DS_COMBAT_H
#define DS_COMBAT_H

#include "ds-object.h"
#include "wizard.h"
#include "entity.h"
#include "region.h"
#include "entity-list.h"
#include "entity-animation.h"
#include "ds-combat.h"

#define COMBAT_ERROR    (9999)

enum combat_scmd_t {
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
    COMBAT_POWER_THROW_STATIC,
};

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

enum combat_turn_t {
    NO_COMBAT,
    NONPLAYER_TURN,
    PLAYER1_TURN,
    PLAYER2_TURN,
    PLAYER3_TURN,
    PLAYER4_TURN,
};

#define MAX_COMBAT_ACTIONS (100)
typedef struct entity_action_list_s {
    ds1_combat_t *combat;
    entity_action_t actions[MAX_COMBAT_ACTIONS];
} entity_action_list_t;

void combat_init(combat_region_t *cr);
void combat_free(combat_region_t *rc);
const enum combat_turn_t combat_player_turn();
extern int combat_initiate(region_t *reg, const uint16_t x, const uint16_t y);
scmd_t* combat_get_scmd(const enum combat_scmd_t type);
entity_t* combat_get_current(combat_region_t *cr);
extern void combat_set_hunt(combat_region_t *cr, const uint32_t combat_id);
extern void combat_player_action(const entity_action_t action);
extern int combat_activate_power(power_t *pw, entity_t *source, entity_t *target, const int32_t x, const int32_t y);

// pre-processor ordering.
extern void combat_is_defeated(region_t *reg, entity_t *dude);

#endif
