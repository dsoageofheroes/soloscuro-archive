#ifndef DS_ITEM_H
#define DS_ITEM_H

#include <stdint.h>

typedef struct _ds1_item_t { // Not confirmed at all...
    int16_t  id; // 0, confirmed (but is negative...), is the OJFF entry
    uint16_t quantity; // confirmed, 0 mean no need.
    int16_t  next;  // 4, for some internal book keeping.
    uint16_t value; // 6, confirmed
    int16_t  pack_index;
    int16_t  item_index; // Correct, maps into it1r.
    int16_t  icon;
    uint16_t charges;
    uint8_t  data0;
    uint8_t  slot;     // confirmed
    uint8_t  name_idx; //confirmed
    int8_t   bonus;
    uint16_t priority;
    int8_t   special;
} __attribute__ ((__packed__)) ds1_item_t;

typedef struct ds_item1r_s {
    uint8_t weapon_type;
    uint8_t data0; // always 0, probably structure alignment byte.
    uint16_t damage_type;
    uint8_t weight;
    uint16_t data1;
    uint8_t base_hp;
    uint8_t material;
    uint8_t placement;
    uint8_t range;// Need to confirm
    uint8_t num_attacks;
    uint8_t sides;
    uint8_t dice;
    int8_t mod;
    uint8_t flags;
    uint16_t legal_class;
    uint8_t data2; // padding?
    int8_t base_AC;
} __attribute__ ((__packed__)) ds_item1r_t;

typedef struct _item_name_t {
    char name[25];
} item_name_t;

enum {
    SLOT_ARM, SLOT_AMMO, SLOT_MISSILE, SLOT_HAND0, SLOT_FINGER0, SLOT_WAIST,
    SLOT_LEGS, SLOT_HEAD, SLOT_NECK, SLOT_CHEST, SLOT_HAND1, SLOT_FINGER1,
    SLOT_CLOAK, SLOT_FOOT
};

typedef struct ds_inventory_s {
    ds1_item_t arm;
    ds1_item_t ammo;
    ds1_item_t missle;
    ds1_item_t hand0;
    ds1_item_t finger0;
    ds1_item_t waist;
    ds1_item_t legs;
    ds1_item_t head;
    ds1_item_t neck;
    ds1_item_t chest;
    ds1_item_t hand1;
    ds1_item_t finger1;
    ds1_item_t cloak;
    ds1_item_t foot;
    ds1_item_t bp[12];
} ds_inventory_t;

void ds_item_init();
int ds_item_load(ds1_item_t *item, int32_t id);
const char *ds_item_name(const int32_t name_idx);
const ds_item1r_t *ds_get_item1r(const int32_t item_idx);
int32_t ds_item_get_bmp_id(ds1_item_t *item);
void ds_item_close();
int ds_item_allowed_in_slot(ds1_item_t *item, const int slot);
//int ds_item_read(const int32_t id, 

#endif
