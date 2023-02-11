#ifndef ITEM_H
#define ITEM_H

#include <stdint.h>
#include "ssi-scmd.h"
#include "animation.h"

enum ssi_door_status_e {
    SSI_DOOR_OPEN   = 0x00,
    SSI_DOOR_CLOSED = 0x01,
    SSI_DOOR_LOCKED = 0x02,
    SSI_DOOR_GPL    = 0x04,
    SSI_DOOR_SECRET = 0x08,
};

// First the DS1 items structs
typedef struct _ds1_item_t { // Not confirmed at all...
    int16_t  id; // 0, confirmed (but is negative...), is the OJFF entry
    uint16_t quantity; // confirmed, 0 mean no need.
    int16_t  next;  // 4, for some internal book keeping.
    uint16_t value; // 6, confirmed
    int16_t  pack_index;
    int16_t  item_index; // Correct, maps into it1r.
    int16_t  icon;
    uint16_t charges;
    uint8_t  special;  // confirmed
    uint8_t  slot;     // confirmed
    uint8_t  name_idx; // confirmed
    int8_t   bonus;
    uint16_t priority;
    int8_t   data0;
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
    int8_t base_AC;
    uint8_t data2; // padding?
} __attribute__ ((__packed__)) ds_item1r_t;

typedef struct _item_name_t {
    char name[25];
} item_name_t;

//End of DS1 item structs

// Regular items.

typedef enum item_type_e {
    ITEM_CONSUMABLE,
    ITEM_MELEE,
    ITEM_MISSILE_THROWN,
    ITEM_MISSILE_USE_AMMO,
    ITEM_ARMOR
} item_type_t;

typedef struct effect_node_s {
    uint32_t psi;
    uint32_t spell;
    uint32_t other;
    uint32_t action; // IE: on attack, on equip, on use
    struct effect_node_s *next;
} effect_node_t;

typedef struct item_attack_s {
    uint8_t  number;
    uint8_t  num_dice;
    uint8_t  sides;
    uint8_t  bonus;
    uint8_t  range;
    uint16_t damage_type;
} item_attack_t;

#define ITEM_NAME_MAX (32)

typedef struct item_s { 
    int16_t           ds_id;
    char              name[ITEM_NAME_MAX];
    item_type_t       type;
    uint16_t          quantity;
    uint16_t          value;
    uint16_t          charges;
    uint16_t          legal_class;
    int16_t           item_index;
    uint8_t           placement; // where on the user
    uint8_t           weight;
    uint8_t           material;
    int8_t            ac;
    item_attack_t     attack;
    animate_sprite_t  anim;
    effect_node_t     *effect;
} item_t;

typedef struct inventory_s {
    item_t arm;
    item_t ammo;
    item_t missile;
    item_t hand0;
    item_t finger0;
    item_t waist;
    item_t legs;
    item_t head;
    item_t neck;
    item_t chest;
    item_t hand1;
    item_t finger1;
    item_t cloak;
    item_t foot;
    item_t bp[12];
} inventory_t;

#define ITEM_SLOT_MAX (sizeof(inventory_t) / sizeof(item_t))

extern item_t*           sol_inventory_create();
extern int               item_allowed_in_slot(item_t *item, const int slot);
extern void              item_free(item_t *item);
extern void              item_free_except_graphics(item_t *item);
extern item_t*           item_dup(item_t *item);
extern void              item_free_inventory(item_t *item);
extern animate_sprite_t* item_icon(item_t *item);
extern sol_status_t      sol_item_get_wizard_level(item_t *item, uint8_t *level);
extern sol_status_t      sol_item_get_priest_level(item_t *item, uint8_t *level);
extern item_t*           sol_item_get(inventory_t *inv, const int8_t slot);

// DS1 specific functions
extern void item_convert_from_ds1(item_t *item, const ds1_item_t *ds1_item);

struct sol_entity_s;
extern void item_set_starting(struct sol_entity_s *pc);
extern void sol_give_ds1_item(struct sol_entity_s *pc, const int slot, const int item_index, const int id);

#endif
