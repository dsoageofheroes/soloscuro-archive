#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include "ds-scmd.h"
#include "ds-item.h"
#include "gff-map.h"
#include "spells.h"

typedef uint16_t psi_group_t;

typedef struct saving_throws_s {
    uint8_t paralysis;
    uint8_t wand;
    uint8_t petrify;
    uint8_t breath;
    uint8_t spell;
} saving_throws_t;

typedef struct innate_attack_s {
    uint8_t number;
    uint8_t num_dice;
    uint8_t sides;
    uint8_t bonus;
    uint32_t special;
} innate_attack_t;

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
    uint8_t magic_resistance;
    uint32_t special_defense;
    innate_attack_t attacks[3];
    saving_throws_t saves;
} stats_t;

typedef struct sprite_info_s {
    int16_t bmp_id;     // Which bmp this is.
    int16_t xoffset;    // bitmap offset x
    int16_t yoffset;    // bitmap offset y
    uint16_t flags;     // sprite/scmd flags
    scmd_t *scmd;       // the animation script
    void *data;         // used for special data the UI needs (IE: SDL.)
} sprite_info_t;

typedef struct effect_node_s {
    uint32_t effect;
    struct effect_node_s *next;
} effect_node_t;

typedef struct class_s {
    uint32_t current_xp;
    uint32_t high_xp;    // for level drain.
    int8_t class;
    uint8_t level;
    uint8_t high_level; // for level drain.
} class_t;

typedef struct entity_s {
    char *name;
    uint8_t size;
    uint8_t race;
    uint8_t gender;
    uint8_t alignment;
    int8_t allegiance;
    int8_t combat_flags;
    uint16_t region;
    uint16_t mapx;      // object's x position in the region
    uint16_t mapy;      // object's y position in the region
    int16_t mapz;       // object's z position in the region
    uint16_t sound_fx;
    uint16_t attack_sound;
    class_t class[3];
    stats_t stats;
    sprite_info_t sprite;
    psi_group_t psi;
    ds_inventory_t *inv;
    spell_list_t *spells;
    psionic_list_t *psionics;
    effect_node_t *effects;
} entity_t;

// For the lolz
typedef entity_t dude_t;

extern entity_t* entity_create_from_objex(const int id);
extern entity_t* entity_create_from_etab(gff_map_object_t *entry_table, uint32_t id);
extern void entity_free(entity_t *entity);

#endif
