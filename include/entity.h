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

typedef struct ability_set_s {
    unsigned int hunt             : 1;
    unsigned int attackable       : 1;
    unsigned int talkable         : 1;
    unsigned int must_go          : 1; // given order to go to x,y
    psi_abilities_t psi;
    //struct spell_abilities_s spells; // NOT shown
    //struct class_abilities_s class;  // Not shown
    union {
        struct {
            uint16_t x, y;
        } pos;
    } args;
} ability_set_t;

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
    int8_t direction; // Left, Right, Up, Down.
    uint16_t region;
    uint16_t mapx;      // object's x position in the region
    uint16_t mapy;      // object's y position in the region
    int16_t mapz;       // object's z position in the region
    uint16_t sound_fx;
    uint16_t attack_sound;
    uint16_t combat_status;
    stats_t stats;
    class_t class[3];
    animate_sprite_t anim;
    ability_set_t abilities;
    struct entity_animation_list_s actions;
    effect_node_t *effects; // anything currently affecting the entity.
    item_t *inv; // NULL means that there is no inventory on this entity (IE: some monsters.)
    //spell_list_t *spells;
    //psionic_list_t *psionics;
} entity_t;

// For the lolz
typedef entity_t dude_t;

extern entity_t* sol_entity_create_default_human();
extern entity_t* sol_entity_create(const int add_inventory);
extern entity_t* entity_create_from_objex(const int id);
extern entity_t* entity_create_from_etab(gff_map_object_t *entry_table, uint32_t id);
extern entity_t* entity_create_clone(entity_t *clone);
extern entity_t* entity_create_fake(const int mapx, const int mapy);
extern int       entity_is_fake(entity_t *entity);
extern void      entity_free(entity_t *entity);
extern void      sol_entity_gui_free(entity_t *entity);
extern void      entity_load_from_gff(entity_t *entity, const int gff_idx, const int player, const int res_id);
extern void      entity_copy_item(entity_t *entity, item_t *item, const size_t slot);
extern void      entity_clear_item(entity_t *entity, const size_t slot);
extern void      entity_load_from_object(entity_t *entity, const char *data);
extern uint32_t  entity_get_total_exp(entity_t *entity);
extern int       entity_attempt_move(dude_t *dude, const int xdiff, const int ydiff, const int speed);
extern int       entity_has_class(const entity_t *entity, const uint16_t class);
extern int       entity_get_level(entity_t *entity, const int class);
extern int       entity_has_wizard_slot(entity_t *entity, const int slot);
extern int       entity_has_priest_slot(entity_t *entity, const int slot);
extern int       entity_take_wizard_slot(entity_t *entity, const int slot);
extern int       entity_take_priest_slot(entity_t *entity, const int slot);
extern int       entity_get_wizard_level(entity_t *entity);
extern int       entity_get_priest_level(entity_t *entity);
extern int       entity_get_ranger_level(entity_t *entity);
extern int16_t   entity_distance(const entity_t *source, const entity_t *dest);
extern int       sol_entity_go(entity_t *dude, const uint16_t x, uint16_t y);

#endif
