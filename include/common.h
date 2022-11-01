// This is a collection of common structs that are needed to avoid
// collisions during preprocessing.
#ifndef SOL_COMMON_H
#define SOL_COMMON_H

#include <stdint.h>

typedef struct attack_trigger_s {
    uint32_t obj, file, addr;
} attack_trigger_t;

typedef struct noorders_trigger_s {
    uint32_t obj, file, addr;
    uint8_t need_to_run; // flag if we need to run/execute at the next oppurtunity.
    uint8_t trigger_on_tile; // flag if we need to run/execute next tile trigger.
} noorders_trigger_t;

typedef struct use_trigger_s {
    uint32_t obj, file, addr;
} use_trigger_t;

typedef struct look_trigger_s {
    uint32_t obj, file, addr;
} look_trigger_t;

typedef struct talkto_trigger_s {
    uint32_t obj, file, addr;
} talkto_trigger_t;

typedef struct usewith_trigger_s {
    uint32_t obj1, obj2, file, addr;
} usewith_trigger_t;

typedef struct tile_trigger_s {
    uint32_t x, y, file, addr, trip;
} tile_trigger_t;

typedef struct box_trigger_s {
    uint32_t x, y, w, h, file, addr, trip;
} box_trigger_t;

typedef struct los_trigger_s {
    uint32_t obj, file, addr, param;
} los_trigger_t;

typedef struct trigger_node_s {
    union {
        attack_trigger_t attack;
        noorders_trigger_t noorders;
        use_trigger_t use;
        look_trigger_t look;
        talkto_trigger_t talkto;
        usewith_trigger_t usewith;
        tile_trigger_t tile;
        box_trigger_t box;
        los_trigger_t los;
    };
    struct trigger_node_s *next;
} trigger_node_t;

typedef struct sol_triggers_s {
    trigger_node_t *attack_list, *noorders_list, *use_list, *look_list,
                   *talkto_list, *usewith_list, *tile_list, *box_list,
                   *los_list;
} sol_triggers_t;

#endif
