// This is a collection of common structs that are needed to avoid
// collisions during preprocessing.
#ifndef SOL_COMMON_H
#define SOL_COMMON_H

#include <stdint.h>

typedef struct sol_attack_trigger_s {
    uint32_t obj, file, addr;
} sol_attack_trigger_t;

typedef struct sol_noorders_trigger_s {
    uint32_t obj, file, addr;
    uint8_t need_to_run; // flag if we need to run/execute at the next oppurtunity.
    uint8_t trigger_on_tile; // flag if we need to run/execute next tile trigger.
} sol_noorders_trigger_t;

typedef struct sol_use_trigger_s {
    uint32_t obj, file, addr;
} sol_use_trigger_t;

typedef struct sol_look_trigger_s {
    uint32_t obj, file, addr;
} sol_look_trigger_t;

typedef struct sol_talkto_trigger_s {
    uint32_t obj, file, addr;
} sol_talkto_trigger_t;

typedef struct sol_usewith_trigger_s {
    uint32_t obj1, obj2, file, addr;
} sol_usewith_trigger_t;

typedef struct sol_tile_trigger_s {
    uint32_t x, y, file, addr, trip;
} sol_tile_trigger_t;

typedef struct sol_box_trigger_s {
    uint32_t x, y, w, h, file, addr, trip;
} sol_box_trigger_t;

typedef struct sol_los_trigger_s {
    uint32_t obj, file, addr, param;
} sol_los_trigger_t;

typedef struct sol_trigger_node_s {
    union {
        sol_attack_trigger_t attack;
        sol_noorders_trigger_t noorders;
        sol_use_trigger_t use;
        sol_look_trigger_t look;
        sol_talkto_trigger_t talkto;
        sol_usewith_trigger_t usewith;
        sol_tile_trigger_t tile;
        sol_box_trigger_t box;
        sol_los_trigger_t los;
    };
    struct sol_trigger_node_s *next;
} sol_trigger_node_t;

typedef struct sol_triggers_s {
    sol_trigger_node_t *attack_list, *noorders_list, *use_list, *look_list,
                       *talkto_list, *usewith_list, *tile_list, *box_list,
                       *los_list;
} sol_triggers_t;

#endif
