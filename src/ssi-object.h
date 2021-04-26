#ifndef SSI_OBJECT_H
#define SSI_OBJECT_H

#include <stdint.h>
#include "ssi-item.h"
#include "ds-scmd.h"

#define COMBAT_NAME_SIZE (18)

struct animate_s;

typedef struct _ds_stats_t {
    uint8_t str;
    uint8_t dex;
    uint8_t con;
    uint8_t intel;
    uint8_t wis;
    uint8_t cha;
} ds_stats_t;

typedef struct _ds_saving_throw_t {
    uint8_t paral;
    uint8_t wand;
    uint8_t petr;
    uint8_t breath;
    uint8_t spell;
} ds_saving_throw_t;

typedef struct _ds_character_s {
    uint32_t current_xp; // confirmed
    uint32_t high_xp;    // probable: repsents how many more exp to go to next level?
    uint16_t base_hp;    // confirmed, natural top hp
    uint16_t high_hp;    // probable: maybe highest hp?
    uint16_t base_psp;   // confirmed
    uint16_t id;         // ? seems to be 1000 or 2000
    uint8_t data1[2];    // ? second byte is always 128 (maybe a negative?)
    uint16_t legal_class;// confirmed
    uint8_t data2[4];    //always 0....
    uint8_t race;        // confirmed
    uint8_t gender;      // confirmed
    uint8_t alignment;   // confirmed
    ds_stats_t stats;    // confirmed
    int8_t real_class[3]; // confirmed
    uint8_t level[3];      // confirmed
    int8_t base_ac;      //confirmed
    uint8_t base_move;      //confirmed
    uint8_t magic_resistance;      //confirmed
    uint8_t num_blows;     //confirmed, blow type?
    uint8_t num_attacks[3]; //confirmed, natural number of attacks
    uint8_t num_dice[3];    //confirmed, #dice for natural attacks
    uint8_t num_sides[3];   //confirmed, #sides for natural attacks
    uint8_t num_bonuses[3]; //confirmed, bonuses for natural attacks
    ds_saving_throw_t saving_throw; //confirmed
    uint8_t allegiance;    // ? always 1
    uint8_t size;          // ? always 0
    uint8_t spell_group;   // ? always 0
    uint8_t high_level[3]; // confirmed, highest possible level (for level drain.)
    uint16_t sound_fx;     // ? always 0
    uint16_t attack_sound;     // ? always 0
    uint8_t psi_group;     // ? always 0
    uint8_t palette;       // confirmed, 2 which is hardcoded
} __attribute__ ((__packed__)) ds_character_t;

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

typedef struct _mini_t {
    int16_t id;
    int16_t next;
    uint8_t priority;
    char name[16];
    uint8_t flags;
    uint8_t data;
} mini_t;

enum {
    RACE_MONSTER,
    RACE_HUMAN,
    RACE_DWARF,
    RACE_ELF,
    RACE_HALFELF,
    RACE_HALFGIANT,
    RACE_HALFLING,
    RACE_MUL,
    RACE_THRIKREEN,
    RACE_MAX
};

enum {
    REAL_CLASS_NONE,
    REAL_CLASS_AIR_CLERIC,
    REAL_CLASS_EARTH_CLERIC,
    REAL_CLASS_FIRE_CLERIC,
    REAL_CLASS_WATER_CLERIC,
    REAL_CLASS_AIR_DRUID,
    REAL_CLASS_EARTH_DRUID,
    REAL_CLASS_FIRE_DRUID,
    REAL_CLASS_WATER_DRUID,
    REAL_CLASS_FIGHTER,
    REAL_CLASS_GLADIATOR,
    REAL_CLASS_PRESERVER,
    REAL_CLASS_PSIONICIST,
    REAL_CLASS_AIR_RANGER,
    REAL_CLASS_EARTH_RANGER,
    REAL_CLASS_FIRE_RANGER,
    REAL_CLASS_WATER_RANGER,
    REAL_CLASS_THIEF,
    REAL_CLASS_DEFILER,
    REAL_CLASS_TEMPLAR,
    REAL_CLASS_MAX,
};

enum {
    GENDER_NONE,
    GENDER_MALE,
    GENDER_FEMALE,
};

#endif
