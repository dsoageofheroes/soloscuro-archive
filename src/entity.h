#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include <stdlib.h>
#include "ds-scmd.h"
#include "ds-item.h"
#include "gff-map.h"
#include "psionic.h"
#include "item.h"

typedef struct ability_set_s {
    unsigned int hunt             : 1;
    psi_abilities_t psi;
    //struct spell_abilities_s spells; // NOT shown
    //struct class_abilities_s class;  // Not shown
} ability_set_t;

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
    int8_t move;
    int8_t base_move;
    int8_t base_thac0;
    uint8_t magic_resistance;
    uint32_t special_defense;
    innate_attack_t attacks[3];
    saving_throws_t saves;
} stats_t;

typedef struct class_s {
    uint32_t current_xp;
    uint32_t high_xp;    // for level drain.
    int8_t class;
    int8_t level;
    uint8_t high_level; // for level drain.
    psi_abilities_t psi;
} class_t;

typedef struct entity_s {
    char *name;
    int16_t ds_id;     // This is the darksun/GPL id
    uint8_t size;
    uint8_t race;
    uint8_t gender;
    uint8_t alignment;
    int8_t allegiance;
    int8_t object_flags;
    uint16_t region;
    uint16_t mapx;      // object's x position in the region
    uint16_t mapy;      // object's y position in the region
    int16_t mapz;       // object's z position in the region
    uint16_t sound_fx;
    uint16_t attack_sound;
    uint16_t combat_status;
    stats_t stats;
    class_t class[3];
    sprite_info_t sprite;
    ability_set_t abilities;
    effect_node_t *effects; // anything currently affecting the entity.
    item_t *inv; // NULL means that there is no inventory on this entity (IE: some monsters.)
    //spell_list_t *spells;
    //psionic_list_t *psionics;
} entity_t;

// For the lolz
typedef entity_t dude_t;

extern entity_t* entity_create_from_objex(const int id);
extern entity_t* entity_create_from_etab(gff_map_object_t *entry_table, uint32_t id);
extern entity_t* entity_create_clone(entity_t *clone);
extern void entity_free(entity_t *entity);
extern void entity_load_from_gff(entity_t *entity, const int gff_idx, const int player, const int res_id);
extern void entity_copy_item(entity_t *entity, item_t *item, const size_t slot);
extern void entity_clear_item(entity_t *entity, const size_t slot);
extern void entity_load_from_object(entity_t *entity, const char *data);
extern uint32_t entity_get_total_exp(entity_t *entity);

extern int entity_has_wizard_slot(entity_t *entity, const int slot);
extern int entity_has_priest_slot(entity_t *entity, const int slot);
extern int entity_take_wizard_slot(entity_t *entity, const int slot);
extern int entity_take_priest_slot(entity_t *entity, const int slot);
extern int entity_get_wizard_level(entity_t *entity);
extern int entity_get_priest_level(entity_t *entity);

#endif
