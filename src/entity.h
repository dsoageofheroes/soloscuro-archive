#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include "ds-scmd.h"
#include "ds-item.h"
#include "spells.h"

typedef uint16_t psi_group_t;

typedef struct saving_throws_s {
    uint8_t paral;
    uint8_t wand;
    uint8_t petr;
    uint8_t breath;
    uint8_t spell;
} saving_throws_t;

typedef struct stats_s {
    uint8_t str;
    uint8_t dex;
    uint8_t con;
    uint8_t intel;
    uint8_t wis;
    uint8_t cha;
    int16_t hp;
    int16_t high_hp;
    int16_t psp;
    int16_t high_psp;
    int8_t base_ac; // before applying any modifiers.
    int8_t base_move;
    int8_t base_thac0;
    uint8_t num_blows;
    uint8_t num_attacks[3];
    uint8_t num_dice[3];
    uint8_t num_sides[3];
    uint8_t num_bonuses[3];
    uint8_t special_attack;
    uint8_t special_defense;
    saving_throws_t saves;
} stats_t;

typedef struct map_info_s {
    uint16_t mapx;      // object's x position in the region
    uint16_t mapy;      // object's y position in the region
    int8_t mapz;        // object's z position in the region
    int16_t bmpx;       // bitmap's x coordinate
    int16_t bmpy;       // bitmap's y coordinate
    int8_t xoffset;     // bitmap offset x
    int16_t yoffset;    // bitmap offset y
    scmd_t *scmd;       // the animation script
    void *data;         // used for special data the UI needs (IE: SDL.)
} map_info_t;

typedef struct entity_s {
    char *name;
    uint8_t size;
    uint8_t race;
    uint8_t gender;
    uint8_t alignment;
    uint32_t current_xp;
    uint32_t high_xp;    // for level drain.
    uint8_t magic_resistance;
    int8_t real_class[3];
    uint8_t level[3];
    uint8_t high_level[3]; // for level drain.
    int8_t allegiance;
    int8_t combat_flags;
    uint16_t sound_fx;
    uint16_t attack_sound;
    stats_t stats;
    map_info_t mobile;
    psi_group_t psi;
    ds_inventory_t *inv;
    spell_list_t *spells;
    psionic_list_t *psionics;

} entity_t;

// For the lolz
typedef entity_t dude_t;

#endif
