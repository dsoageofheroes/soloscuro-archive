#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include <stdlib.h>
#include "ssi-scmd.h"
#include "ssi-item.h"
#include "gff-map.h"
#include "psionic.h"
#include "item.h"
#include "stats.h"
#include "entity-animation.h"

typedef struct sol_ability_set_s {
    unsigned int hunt             : 1; unsigned int attackable       : 1;
    unsigned int talkable         : 1;
    unsigned int must_go          : 1; // given order to go to x,y
    sol_psi_abilities_t psi;
    //struct spell_abilities_s spells; // NOT shown
    //struct class_abilities_s class;  // Not shown
    union {
        struct {
            uint16_t x, y;
        } pos;
    } args;
} sol_ability_set_t;

typedef struct sol_class_s {
    uint32_t current_xp;
    uint32_t high_xp;    // for level drain.
    int8_t class;
    int8_t level;
    uint8_t high_level; // for level drain.
    sol_psi_abilities_t psi;
} sol_class_t;

typedef struct sol_entity_s {
    char *name;
    int16_t ds_id;     // This is the darksun/GPL id
    uint8_t size;
    uint8_t race;
    uint8_t gender;
    uint8_t alignment;
    int8_t allegiance;
    int8_t object_flags;
    int8_t map_flags;
    int8_t direction; // Left, Right, Up, Down.
    uint16_t region;
    uint16_t mapx;      // object's x position in the region
    uint16_t mapy;      // object's y position in the region
    int16_t mapz;       // object's z position in the region
    uint16_t sound_fx;
    uint16_t attack_sound;
    uint16_t combat_status;
    sol_stats_t stats;
    sol_class_t class[3];
    animate_sprite_t anim;
    sol_ability_set_t abilities;
    struct sol_entity_animation_list_s actions;
    sol_effect_node_t *effects; // anything currently affecting the entity.
    sol_item_t *inv; // NULL means that there is no inventory on this entity (IE: some monsters.)
    uint32_t cp;
    //spell_list_t *spells;
    //psionic_list_t *psionics;
} sol_entity_t;

// For the lolz
typedef sol_entity_t sol_dude_t;

extern sol_status_t sol_entity_create_default_human(sol_entity_t **ret);
extern sol_status_t sol_entity_create(const int add_inventory, sol_entity_t **ret);
extern sol_status_t sol_entity_create_from_objex(const int id, sol_entity_t **ret);
extern sol_status_t sol_entity_create_from_etab(gff_map_object_t *entry_table, uint32_t id, sol_entity_t **ret);
extern sol_status_t sol_entity_create_clone(sol_entity_t *clone, sol_entity_t **ret);
extern sol_status_t sol_entity_create_fake(const int mapx, const int mapy, sol_entity_t **ret);
extern sol_status_t sol_entity_is_fake(sol_entity_t *entity);
extern sol_status_t sol_entity_free(sol_entity_t *entity);
extern sol_status_t sol_entity_gui_free(sol_entity_t *entity);
extern sol_status_t sol_entity_load_from_gff(sol_entity_t *entity, const int gff_idx, const int player, const int res_id);
extern sol_status_t sol_entity_copy_item(sol_entity_t *entity, sol_item_t *item, const size_t slot);
extern sol_status_t sol_entity_clear_item(sol_entity_t *entity, const size_t slot);
extern sol_status_t sol_entity_load_from_object(sol_entity_t *entity, const char *data);
extern sol_status_t sol_entity_get_total_exp(sol_entity_t *entity, int32_t *exp);
extern sol_status_t sol_entity_attempt_move(sol_dude_t *dude, const int xdiff, const int ydiff, const int speed);
extern sol_status_t sol_entity_has_class(const sol_entity_t *entity, const uint16_t class);
extern sol_status_t sol_entity_get_level(sol_entity_t *entity, const int class, uint8_t *level);
extern sol_status_t sol_entity_get_wizard_level(sol_entity_t *entity, uint8_t *level);
extern sol_status_t sol_entity_get_priest_level(sol_entity_t *entity, uint8_t *level);
extern sol_status_t sol_entity_get_ranger_level(sol_entity_t *entity, uint8_t *level);
extern sol_status_t sol_entity_has_wizard_slot(sol_entity_t *entity, const int slot);
extern sol_status_t sol_entity_has_priest_slot(sol_entity_t *entity, const int slot);
extern sol_status_t sol_entity_take_wizard_slot(sol_entity_t *entity, const int slot);
extern sol_status_t sol_entity_take_priest_slot(sol_entity_t *entity, const int slot);
extern sol_status_t sol_entity_distance(const sol_entity_t *source, const sol_entity_t *dest, int16_t *dist);
extern sol_status_t sol_entity_go(sol_entity_t *dude, const uint16_t x, uint16_t y);
extern sol_status_t sol_entity_debug(sol_entity_t *dude);
extern sol_status_t sol_entity_update_scmd(sol_entity_t *entity);

#endif
