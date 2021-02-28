#ifndef DS_COMBAT_H
#define DS_COMBAT_H

#include "ds-object.h"

#define MAX_COMBAT_OBJS (200)
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
};

typedef struct _ds_combat_t {
    int16_t hp; // At byte pos 0, confirmed
    int16_t psp; // 2, confirmed
    int16_t char_index; // 4, unconfirmed but looks right.
    int16_t id;  // 6, yes, but is id *-1
    int16_t ready_item_index; // 8, to be cleared.
    int16_t weapon_index; // 10, to be cleared
    int16_t pack_index;   // 12, to be cleared
    uint8_t data_block[8]; // just to shift down 8 bytes.
    uint8_t special_attack; // 22, looks probable.
    uint8_t special_defense; // 23, looks probable.
    int16_t icon; // doesn't look right
    int8_t  ac;   // 26, confirmed
    uint8_t move; // 27, confirmed
    uint8_t status;
    uint8_t allegiance;
    uint8_t data;
    int8_t  thac0; // 31, confirmed
    uint8_t priority;
    uint8_t flags;
    ds_stats_t stats; // 34, confirmed
    // WARNING: This is actually 16, but we do 18 as a buffer.
    char    name[COMBAT_NAME_SIZE]; // 40, confirmed
//} ds1_combat_t;
} __attribute__ ((__packed__)) ds1_combat_t;

// Represents all combatants in the entire region.
typedef struct combat_region_s {
    ds1_combat_t combats[MAX_COMBAT_OBJS];
    region_object_t *robjs[MAX_COMBAT_OBJS];
    uint8_t hunt[MAX_COMBAT_OBJS];
    uint32_t pos;
    uint8_t in_combat_mode;
} combat_region_t;

enum combat_turn_t {
    NO_COMBAT,
    NONPLAYER_TURN,
    PLAYER1_TURN,
    PLAYER2_TURN,
    PLAYER3_TURN,
    PLAYER4_TURN,
};

enum combat_action_e {
    CA_NONE,
    CA_WALK_LEFT,
    CA_WALK_RIGHT,
    CA_WALK_UP,
    CA_WALK_DOWN,
    CA_WALK_UPLEFT,
    CA_WALK_UPRIGHT,
    CA_WALK_DOWNLEFT,
    CA_WALK_DOWNRIGHT,
    CA_MELEE,
    CA_MISSILE,
    CA_PSIONIC,
    CA_SPELL,
};

typedef struct combat_action_s {
    enum combat_action_e action;
    region_object_t *target;
} combat_action_t;

#define MAX_COMBAT_ACTION (100)
typedef struct combat_action_list_s {
    region_object_t *robj;
    ds1_combat_t *combat;
    combat_action_t actions[MAX_COMBAT_ACTION];
} combat_action_list_t;

void combat_init(combat_region_t *cr);
void combat_free(combat_region_t *rc);
const enum combat_turn_t combat_player_turn();
uint32_t combat_add(combat_region_t *rc, region_object_t *robj, ds1_combat_t *combat);
scmd_t* combat_get_scmd(const enum combat_scmd_t type);
ds1_combat_t* combat_get_combat( combat_region_t* cr, const uint32_t combat_id);
void combat_set_hunt(combat_region_t *cr, const uint32_t combat_id);

#endif
