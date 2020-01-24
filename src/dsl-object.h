#ifndef DSL_OBJECT_H
#define DSL_OBJECT_H

#include <stdint.h>

#define DSL_OBJECT_ID 0
#define DSL_OBJECT_QTY 1
#define DSL_OBJECT_NEXT 2
#define DSL_OBJECT_VALUE 3
#define DSL_OBJECT_PACKX 4
#define DSL_OBJECT_ITEMX 5
#define DSL_OBJECT_ICON 6
#define DSL_OBJECT_CHARGES 7
#define DSL_OBJECT_SPECIAL 8
#define DSL_OBJECT_PRIORITY 9
#define DSL_OBJECT_SLOT 10
#define DSL_OBJECT_NAMEX 11
#define DSL_OBJECT_PLUS 12
#define DSL_OBJECT_CUR_HP 13
#define DSL_OBJECT_CUR_PSI 14
#define DSL_OBJECT_CHARX 15
#define DSL_OBJECT_READYTEMX 16
#define DSL_OBJECT_WEAPONX 17
#define DSL_OBJECT_SPECIAL_ATTACK 18
#define DSL_OBJECT_SPECIAL_DEFENSE 19
#define DSL_OBJECT_CUR_AC 20
#define DSL_OBJECT_CUR_MOVE 21
#define DSL_OBJECT_STATUS 22
#define DSL_OBJECT_CUR_ALLEG 23
#define DSL_OBJECT_THACO 24
#define DSL_OBJECT_FLAGS 25
#define DSL_OBJECT_CUR_STR 26
#define DSL_OBJECT_CUR_DEX 27
#define DSL_OBJECT_CUR_CON 28
#define DSL_OBJECT_CURNT 29
#define DSL_OBJECT_CUR_WIS 30
#define DSL_OBJECT_CUR_CHA 31
#define DSL_OBJECT_DIRECTION 32
#define DSL_OBJECT_DATA 33
#define DSL_OBJECT_NAME 34
#define DSL_OBJECT_CUR_EXP 35
#define DSL_OBJECT_HIGH_EXP 36
#define DSL_OBJECT_BASE_HP 37
#define DSL_OBJECT_HIGH_HP 38
#define DSL_OBJECT_BASE_PSI 39
#define DSL_OBJECT_LEGAL_CLASS 40
#define DSL_OBJECT_RACE 41
#define DSL_OBJECT_GENDER 42
#define DSL_OBJECT_ALIGNMENT 43
#define DSL_OBJECT_BASE_STR 44
#define DSL_OBJECT_BASE_DEX 45
#define DSL_OBJECT_BASE_CON 46
#define DSL_OBJECT_BASENT 47
#define DSL_OBJECT_BASE_WIS 48
#define DSL_OBJECT_BASE_CHA 49
#define DSL_OBJECT_CLASS 50
#define DSL_OBJECT_CLASS_2 51
#define DSL_OBJECT_CLASS_3 52
#define DSL_OBJECT_LEVEL 53
#define DSL_OBJECT_LEVEL_2 54
#define DSL_OBJECT_LEVEL_3 55
#define DSL_OBJECT_BASE_AC 56
#define DSL_OBJECT_BASE_MOVE 57
#define DSL_OBJECT_MAGIC_RESISTANCE 58
#define DSL_OBJECT_NUM_BLOWS 59
#define DSL_OBJECT_NUMATTACK 60
#define DSL_OBJECT_NUM_ATT2 61
#define DSL_OBJECT_NUM_ATT3 62
#define DSL_OBJECT_NUMDICE 63
#define DSL_OBJECT_NUM_DICE2 64
#define DSL_OBJECT_NUM_DICE3 65
#define DSL_OBJECT_NUMSIDES 66
#define DSL_OBJECT_NUM_SIDE2 67
#define DSL_OBJECT_NUM_SIDE3 68
#define DSL_OBJECT_NUMADDS 69
#define DSL_OBJECT_NUM_ADDS2 70
#define DSL_OBJECT_NUM_ADDS3 71
#define DSL_OBJECT_SAVING_THROW 72
#define DSL_OBJECT_WAND_SAV 73
#define DSL_OBJECT_PETRI_SAV 74
#define DSL_OBJECT_BREATH_SAV 75
#define DSL_OBJECT_SPELL_SAV 76
#define DSL_OBJECT_ALLEGIANCE 77
#define DSL_OBJECT_SIZE 78
#define DSL_OBJECT_SPELL_GROUP 79
#define DSL_OBJECT_HIGH_LEVEL 80
#define DSL_OBJECT_HIGH_LEVEL2 81
#define DSL_OBJECT_HIGH_LEVEL3 82
#define DSL_OBJECT_SOUND_FX 83
#define DSL_OBJECT_ATTACK_SND 84
#define DSL_OBJECT_PSI_GROUP 85
#define DSL_OBJECT_PALETTE 86
#define DSL_OBJECT_WEAPON 87
#define DSL_OBJECT_DAMAGE_TYPE 88
#define DSL_OBJECT_WEIGHT 89
#define DSL_OBJECT_ITEM_HP 90
#define DSL_OBJECT_MATERIAL 91
#define DSL_OBJECT_PLACEMENT 92
#define DSL_OBJECT_RANGE 93
#define DSL_OBJECT_NUM_ATTACKS 94
#define DSL_OBJECT_SIDES 95
#define DSL_OBJECT_DICE 96
#define DSL_OBJECT_ADDS 97

#define COMBAT_NAME_SIZE (18)

typedef struct _ds1_combat_t {
    int16_t hp; // At byte pos 0, confirmed
    int16_t psi; // 2, confirmed
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
    uint8_t stats[6]; // 34, confirmed
    //uint8_t direction;
    // WARNING: This is actually 16, but we do 18 as a buffer.
    char    name[COMBAT_NAME_SIZE]; // 40, confirmed
} ds1_combat_t;

typedef struct _ds1_item_t { // Not confirmed at all...
    int16_t  id; // 0, confirmed (but is negative...)
    uint16_t quantity; // ?
    int16_t  next;  // ?
    uint16_t value; // 6, look correct.
    int16_t  pack_index;
    int16_t  item_index;
    int16_t  icon;
    uint16_t charges;
    uint16_t special;
    uint8_t  priority;
    int8_t   slot;
    uint16_t name_index;
    int8_t   adds;
} ds1_item_t;

typedef struct _ds_stats_t {
    uint8_t STR;
    uint8_t DEX;
    uint8_t CON;
    uint8_t INT;
    uint8_t WIS;
    uint8_t CHA;
} _ds_stats_t;

typedef struct _ds_saving_throw_t {
    uint8_t paral;
    uint8_t wand;
    uint8_t petr;
    uint8_t breath;
    uint8_t spell;
} _ds_saving_throw_t;

typedef struct _ds1_character_t {
    uint32_t current_xp;
    uint32_t high_xp;
    uint16_t base_hp;
    uint16_t high_hp;
    uint16_t base_psi;
    uint16_t id;
    uint16_t legal_class;
    uint8_t race;
    uint8_t gender;
    uint8_t alignment;
    _ds_stats_t stats;
    uint8_t class[3];
    uint8_t level[3];
    uint8_t base_ac;
    uint8_t base_move;
    uint8_t magic_resistance;
    uint8_t num_blows;
    uint8_t num_attacks[3];
    uint8_t num_dice[3];
    uint8_t num_sides[3];
    uint8_t num_adds[3];
    _ds_saving_throw_t saving_throw;
    uint8_t allegiance;
    uint8_t size;
    uint8_t spell_group;
    uint8_t high_level[3];
    uint16_t sound_fx;
    uint16_t attack_sound;
    uint8_t psi_group;
    uint8_t palette;
} ds1_character_t;

typedef struct _item1r_t {
    uint8_t weapon;
    uint8_t damage_type;
    uint8_t wieght;
    uint8_t base_hp;
    uint8_t material;
    uint8_t placement;
    uint8_t range;
    uint8_t num_attacks;
    uint8_t sides;
    uint8_t dice;
    int8_t adds;
    uint8_t flags;
    uint16_t legal_class;
    int8_t base_ac;
} item1r_t;

typedef struct _mini_t {
    int16_t id;
    int16_t next;
    uint8_t priority;
    char name[16];
    uint8_t flags;
    uint8_t data;
} mini_t;

typedef struct _item_name_t {
    char name[25];
} item_name_t;

void dsl_object_init();
void dsl_object_cleanup();

#endif
