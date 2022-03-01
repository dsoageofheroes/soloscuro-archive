#ifndef SSI_ITEM_H
#define SSI_ITEM_H

#include <stdint.h>

#include "item.h"

#define DS1_WEAPON_MELEE     (1<<0)
#define DS1_WEAPON_MISSILE   (1<<1)
#define DS1_WEAPON_SHIELD    (1<<2)
#define DS1_WEAPON_USE_AMMO  (1<<3)
#define DS1_WEAPON_THROWN    (1<<4)
#define DS1_ARMOR_FLAG       (1<<7)

enum {
    SLOT_ARM, SLOT_AMMO, SLOT_MISSILE, SLOT_HAND0, SLOT_FINGER0, SLOT_WAIST,
    SLOT_LEGS, SLOT_HEAD, SLOT_NECK, SLOT_CHEST, SLOT_HAND1, SLOT_FINGER1,
    SLOT_CLOAK, SLOT_FOOT,
    SLOT_END,
    // These are for combat
    SLOT_INNATE0,
    SLOT_INNATE1,
    SLOT_INNATE2,
};

typedef struct ssi_ds_inventory_s {
    ds1_item_t arm;
    ds1_item_t ammo;
    ds1_item_t missile;
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
} ssi_ds_inventory_t;

extern void               ssi_item_init();
extern int                ssi_item_load(ds1_item_t *item, int32_t id);
extern const char        *ssi_item_name(const int32_t name_idx);
extern const ds_item1r_t *ssi_get_item1r(const int32_t item_idx);
extern int32_t            ssi_item_get_bmp_id(ds1_item_t *item);
extern void               ssi_item_close();
extern int                ssi_item_allowed_in_slot(ds1_item_t *item, const int slot);

#endif
