#ifndef ITEM_H
#define  ITEM_H

#include <stdint.h>

typedef enum item_type_e {
    CONSUMABLE,
    WEAPON,
    ARMOR
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

typedef struct item_s { 
    int16_t          ds_id;
    char             *name;
    item_type_t      type;
    uint16_t         quantity;
    uint16_t         value;
    uint16_t         charges;
    uint16_t         legal_class;
    uint8_t          placement; // where on the user
    uint8_t          weight;
    uint8_t          material;
    int8_t           ac;
    item_attack_t    attack;
    effect_node_t    *effect;
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

#endif
