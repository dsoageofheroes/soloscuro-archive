#ifndef DS1_COMBAT_H
#define DS1_COMBAT_H

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

#endif
