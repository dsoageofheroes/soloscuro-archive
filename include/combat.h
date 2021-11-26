#ifndef DS_COMBAT_H
#define DS_COMBAT_H

#include "ssi-object.h"
#include "wizard.h"
#include "entity.h"
#include "region.h"
#include "entity-list.h"
#include "entity-animation.h"

#define COMBAT_ERROR    (9999)

typedef enum combat_scmd_e {
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
    COMBAT_POWER_THROW_STATIC_U,
    COMBAT_POWER_THROW_STATIC_UUR,
    COMBAT_POWER_THROW_STATIC_UR,
    COMBAT_POWER_THROW_STATIC_URR,
    COMBAT_POWER_THROW_STATIC_R,
    COMBAT_POWER_THROW_STATIC_RRD,
    COMBAT_POWER_THROW_STATIC_RD,
    COMBAT_POWER_THROW_STATIC_RDD,
    COMBAT_POWER_THROW_STATIC_D,
    COMBAT_POWER_THROW_STATIC_DDL,
    COMBAT_POWER_THROW_STATIC_DL,
    COMBAT_POWER_THROW_STATIC_DLL,
    COMBAT_POWER_THROW_STATIC_L,
    COMBAT_POWER_THROW_STATIC_LLU,
    COMBAT_POWER_THROW_STATIC_LU,
    COMBAT_POWER_THROW_STATIC_LUU,
    COMBAT_POWER_THROW_ANIM_U,
    COMBAT_POWER_THROW_ANIM_UUR,
    COMBAT_POWER_THROW_ANIM_UR,
    COMBAT_POWER_THROW_ANIM_URR,
    COMBAT_POWER_THROW_ANIM_R,
    COMBAT_POWER_THROW_ANIM_RRD,
    COMBAT_POWER_THROW_ANIM_RD,
    COMBAT_POWER_THROW_ANIM_RDD,
    COMBAT_POWER_THROW_ANIM_D,
    COMBAT_POWER_THROW_ANIM_DDL,
    COMBAT_POWER_THROW_ANIM_DL,
    COMBAT_POWER_THROW_ANIM_DLL,
    COMBAT_POWER_THROW_ANIM_L,
    COMBAT_POWER_THROW_ANIM_LLU,
    COMBAT_POWER_THROW_ANIM_LU,
    COMBAT_POWER_THROW_ANIM_LUU,
} combat_scmd_t;

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

typedef enum combat_turn_e {
    NO_COMBAT,
    NONPLAYER_TURN,
    PLAYER1_TURN,
    PLAYER2_TURN,
    PLAYER3_TURN,
    PLAYER4_TURN,
} combat_turn_t;

#define MAX_COMBAT_ACTIONS (100)
typedef struct entity_action_list_s {
    ds1_combat_t *combat;
    entity_action_t actions[MAX_COMBAT_ACTIONS];
} entity_action_list_t;

void          sol_combat_init(combat_region_t *cr);
void          sol_combat_free(combat_region_t *rc);
combat_turn_t sol_combat_player_turn();
extern int    sol_combat_initiate(sol_region_t *reg, const uint16_t x, const uint16_t y);
scmd_t*       sol_combat_get_scmd(const combat_scmd_t type);
entity_t*     sol_combat_get_current(combat_region_t *cr);
extern void   sol_combat_set_hunt(combat_region_t *cr, const uint32_t combat_id);
extern void   sol_combat_player_action(const entity_action_t action);
extern int    sol_combat_activate_power(power_t *pw, entity_t *source, entity_t *target, const int32_t x, const int32_t y);
extern void   sol_combat_enter_combat(); // currently implemented in src/ui/map.c

// pre-processor ordering.
extern void   sol_combat_is_defeated(sol_region_t *reg, entity_t *dude);

#endif
