#ifndef DS_OBJECT_H
#define DS_OBJECT_H

#include <stdint.h>
#include "ds-scmd.h"

#define DS_OBJECT_ID 0
#define DS_OBJECT_QTY 1
#define DS_OBJECT_NEXT 2
#define DS_OBJECT_VALUE 3
#define DS_OBJECT_PACKX 4
#define DS_OBJECT_ITEMX 5
#define DS_OBJECT_ICON 6
#define DS_OBJECT_CHARGES 7
#define DS_OBJECT_SPECIAL 8
#define DS_OBJECT_PRIORITY 9
#define DS_OBJECT_SLOT 10
#define DS_OBJECT_NAMEX 11
#define DS_OBJECT_PLUS 12
#define DS_OBJECT_CUR_HP 13
#define DS_OBJECT_CUR_PSI 14
#define DS_OBJECT_CHARX 15
#define DS_OBJECT_READYTEMX 16
#define DS_OBJECT_WEAPONX 17
#define DS_OBJECT_SPECIAL_ATTACK 18
#define DS_OBJECT_SPECIAL_DEFENSE 19
#define DS_OBJECT_CUR_AC 20
#define DS_OBJECT_CUR_MOVE 21
#define DS_OBJECT_STATUS 22
#define DS_OBJECT_CUR_ALLEG 23
#define DS_OBJECT_THACO 24
#define DS_OBJECT_FLAGS 25
#define DS_OBJECT_CUR_STR 26
#define DS_OBJECT_CUR_DEX 27
#define DS_OBJECT_CUR_CON 28
#define DS_OBJECT_CURNT 29
#define DS_OBJECT_CUR_WIS 30
#define DS_OBJECT_CUR_CHA 31
#define DS_OBJECT_DIRECTION 32
#define DS_OBJECT_DATA 33
#define DS_OBJECT_NAME 34
#define DS_OBJECT_CUR_EXP 35
#define DS_OBJECT_HIGH_EXP 36
#define DS_OBJECT_BASE_HP 37
#define DS_OBJECT_HIGH_HP 38
#define DS_OBJECT_BASE_PSI 39
#define DS_OBJECT_LEGAL_CLASS 40
#define DS_OBJECT_RACE 41
#define DS_OBJECT_GENDER 42
#define DS_OBJECT_ALIGNMENT 43
#define DS_OBJECT_BASE_STR 44
#define DS_OBJECT_BASE_DEX 45
#define DS_OBJECT_BASE_CON 46
#define DS_OBJECT_BASENT 47
#define DS_OBJECT_BASE_WIS 48
#define DS_OBJECT_BASE_CHA 49
#define DS_OBJECT_CLASS 50
#define DS_OBJECT_CLASS_2 51
#define DS_OBJECT_CLASS_3 52
#define DS_OBJECT_LEVEL 53
#define DS_OBJECT_LEVEL_2 54
#define DS_OBJECT_LEVEL_3 55
#define DS_OBJECT_BASE_AC 56
#define DS_OBJECT_BASE_MOVE 57
#define DS_OBJECT_MAGIC_RESISTANCE 58
#define DS_OBJECT_NUM_BLOWS 59
#define DS_OBJECT_NUMATTACK 60
#define DS_OBJECT_NUM_ATT2 61
#define DS_OBJECT_NUM_ATT3 62
#define DS_OBJECT_NUMDICE 63
#define DS_OBJECT_NUM_DICE2 64
#define DS_OBJECT_NUM_DICE3 65
#define DS_OBJECT_NUMSIDES 66
#define DS_OBJECT_NUM_SIDE2 67
#define DS_OBJECT_NUM_SIDE3 68
#define DS_OBJECT_NUMADDS 69
#define DS_OBJECT_NUM_ADDS2 70
#define DS_OBJECT_NUM_ADDS3 71
#define DS_OBJECT_SAVING_THROW 72
#define DS_OBJECT_WAND_SAV 73
#define DS_OBJECT_PETRI_SAV 74
#define DS_OBJECT_BREATH_SAV 75
#define DS_OBJECT_SPELL_SAV 76
#define DS_OBJECT_ALLEGIANCE 77
#define DS_OBJECT_SIZE 78
#define DS_OBJECT_SPELL_GROUP 79
#define DS_OBJECT_HIGH_LEVEL 80
#define DS_OBJECT_HIGH_LEVEL2 81
#define DS_OBJECT_HIGH_LEVEL3 82
#define DS_OBJECT_SOUND_FX 83
#define DS_OBJECT_ATTACK_SND 84
#define DS_OBJECT_PSI_GROUP 85
#define DS_OBJECT_PALETTE 86
#define DS_OBJECT_WEAPON 87
#define DS_OBJECT_DAMAGE_TYPE 88
#define DS_OBJECT_WEIGHT 89
#define DS_OBJECT_ITEM_HP 90
#define DS_OBJECT_MATERIAL 91
#define DS_OBJECT_PLACEMENT 92
#define DS_OBJECT_RANGE 93
#define DS_OBJECT_NUM_ATTACKS 94
#define DS_OBJECT_SIDES 95
#define DS_OBJECT_DICE 96
#define DS_OBJECT_ADDS 97

#define COMBAT_NAME_SIZE (18)
#define MAX_REGION_OBJS  (1<<10)

struct animate_s;

typedef struct region_object_s {
    uint8_t flags;      // flags
    uint16_t entry_id;  // object entry table index / map id?
    int16_t bmpx;       // bitmap's x coordinate
    int16_t bmpy;       // bitmap's y coordinate
    int8_t xoffset;     // bitmap offset x
    int16_t yoffset;    // bitmap offset y
    uint16_t mapx;      // object's x position in the region
    uint16_t mapy;      // object's y position in the region
    int8_t mapz;        // object's z position in the region
    uint8_t ht_idx;     // height table index
    uint16_t gt_idx;    // graph table index
    uint8_t bmp_idx;    // current bmp for the script
    uint8_t bmp_width;  // bitmap width
    uint8_t bmp_height; // bitmap height
    uint8_t cdelay;     // init to zero, modified by script handler
    int16_t st_idx;     // script table index
    uint8_t sc_idx;     // script command index
    uint16_t btc_idx;   // bitmap table chunk index
    int16_t disk_idx;   // disk index
    int32_t game_time;  // game time for animating
    scmd_t *scmd;       // the script
    struct animate_s *anim;     // animation script, to be defined later...
} region_object_t;

typedef struct _region_list_s {
    int pos;
    region_object_t objs[MAX_REGION_OBJS];
} region_list_t;

region_object_t* __region_list_get_next(region_list_t *rl, int *i); // private, do not modify/use!

// PERFORMANCE FIXME: should only go through needed objects, possibly quad-tree.
#define region_list_for_each(rl, obj) \
    int __RL_I = 0; \
    for (obj = __region_list_get_next(rl, &__RL_I); obj != NULL;\
        __RL_I++, obj = __region_list_get_next(rl, &__RL_I))

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
    uint8_t base_ac;      //confirmed
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

typedef struct _item1r_t {
    uint8_t weapon;
    uint8_t damage_type;
    uint8_t weight;
    uint8_t base_hp;
    uint8_t material;
    uint8_t placement;
    uint8_t range;
    uint8_t num_attacks;
    uint8_t sides;
    uint8_t dice;
    int8_t bonus; // positive = bonus, negative = cursed
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

enum {
    RACE_MONSTER,
    RACE_HUMAN,
    RACE_DWARF,
    RACE_ELF,
    RACE_HALFELF,
    RACE_HALFGIANT,
    RACE_HALFLING,
    RACE_MUL,
    RACE_TRIKEEN,
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

void dsl_object_init();
void dsl_object_cleanup();

ds_character_t* dsl_get_character(const int id);
int dsl_valid_character_id(const int id);

region_list_t* region_list_create();
void region_list_free(region_list_t *rl);
void region_list_load_objs(region_list_t *rl, const int gff_file, const int map_id);
region_object_t* region_list_create_from_objex(region_list_t *rl, const int id, const int32_t x, const int32_t y);
#define region_list_get_object(rl, i) (rl->objs + i)

#endif
