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
typedef struct ds1_item_s { // Not confirmed at all...
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

//End of DS1 item structs

// Regular items.

typedef enum sol_item_type_e {
    ITEM_CONSUMABLE,
    ITEM_MELEE,
    ITEM_MISSILE_THROWN,
    ITEM_MISSILE_USE_AMMO,
    ITEM_ARMOR
} sol_item_type_t;

typedef struct sol_effect_node_s {
    uint32_t psi;
    uint32_t spell;
    uint32_t other;
    uint32_t action; // IE: on attack, on equip, on use
    struct effect_node_s *next;
} sol_effect_node_t;

typedef struct sol_item_attack_s {
    uint8_t  number;
    uint8_t  num_dice;
    uint8_t  sides;
    uint8_t  bonus;
    uint8_t  range;
    uint16_t damage_type;
} sol_item_attack_t;

#define ITEM_NAME_MAX (32)

typedef struct sol_item_s { 
    int16_t            ds_id;
    char               name[ITEM_NAME_MAX];
    sol_item_type_t    type;
    uint16_t           quantity;
    uint16_t           value;
    uint16_t           charges;
    uint16_t           legal_class;
    int16_t            item_index;
    uint8_t            placement; // where on the user
    uint8_t            weight;
    uint8_t            material;
    int8_t             ac;
    sol_item_attack_t  attack;
    animate_sprite_t   anim;
    sol_effect_node_t *effect;
} sol_item_t;

typedef struct sol_inventory_s {
    sol_item_t arm;
    sol_item_t ammo;
    sol_item_t missile;
    sol_item_t hand0;
    sol_item_t finger0;
    sol_item_t waist;
    sol_item_t legs;
    sol_item_t head;
    sol_item_t neck;
    sol_item_t chest;
    sol_item_t hand1;
    sol_item_t finger1;
    sol_item_t cloak;
    sol_item_t foot;
    sol_item_t bp[12];
} sol_inventory_t;

#define ITEM_SLOT_MAX (sizeof(sol_inventory_t) / sizeof(sol_item_t))

extern sol_status_t sol_item_get_wizard_level(sol_item_t *item, uint8_t *level);
extern sol_status_t sol_item_get_priest_level(sol_item_t *item, uint8_t *level);
extern sol_status_t sol_item_free(sol_item_t *item);
extern sol_status_t sol_item_free_except_graphics(sol_item_t *item);
extern sol_status_t sol_item_free_inventory(sol_item_t *item);
extern sol_status_t sol_item_convert_from_ds1(sol_item_t *item, const ds1_item_t *ds1_item);
extern sol_status_t sol_item_set_starting(struct sol_entity_s *pc);
extern sol_status_t sol_give_ds1_item(struct sol_entity_s *pc, const int slot, const int item_index, const int id);
extern sol_status_t sol_item_allowed_in_slot(sol_item_t *item, const int slot);
extern sol_status_t sol_inventory_create(sol_item_t **d);
extern sol_status_t sol_item_dup(sol_item_t *item, sol_item_t **d);
extern sol_status_t sol_item_get(sol_inventory_t *inv, const int8_t slot, sol_item_t **d);
extern sol_status_t sol_item_icon(sol_item_t *item, animate_sprite_t **);

#endif
