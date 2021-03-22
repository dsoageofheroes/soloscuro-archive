#ifndef POWERS_H
#define POWERS_H

#include "entity.h"

enum power_shape {
    POWER_SINGLE, // a point, aoe is ignore
    POWER_CIRCLE, // EX: Fireball
    POWER_CONE,   // EX: Cone of Cold
    POWER_RECTANGLE, // aoe is two 8-bit ints, width times height
    POWER_MULTI,  // aoe is number of charges.
};

struct power_instance_s;

typedef struct power_actions_s {
    int (*can_activate) (struct power_instance_s *source);
    int (*pay)          (struct power_instance_s *source);
    int (*apply)        (struct power_instance_s *source, entity_t *target);
    int (*still_active) (struct power_instance_s *source, const int rounds_past);
    int (*affect_power) (struct power_instance_s *target);
} power_actions_t;

// A power is a spell, psionic, innate special ability, or charge on an item.
// This should be stored in a general list/array/hashtable for access at any time.
typedef struct power_s {
    char            *name;
    char            *description;
    uint16_t        range;
    uint16_t        aoe;
    enum            power_shape shape;
    sprite_info_t   icon;
    sprite_info_t   thrown;
    sprite_info_t   hit;
    uint16_t        sound_id;
    power_actions_t actions;
    // Add animation sequence info here.
} power_t;

// This (power/spell entity) is an instance of the spell on a tile in the region. EX: Grease
// During creation a COPY of the entity or item is made, but only one per group.
typedef struct power_instance_s {
    entity_t *entity;
    item_t   *item;
    power_t  *stats;
    struct power_instance_s *next, *prev;
} power_instance_t;

typedef struct power_instance_list_s {
    power_instance_t *head;
} power_instance_list_t;

// List of all the spell nodes on a region.
typedef struct power_overlay_s {
    power_instance_list_t *power_list[MAP_ROWS][MAP_COLUMNS];
} power_overlay_t;

// Okay before I forget.
// Spells create a spell entity on point of impact.
// If AOE, then many children entities are created that point to the parent/master spell entity.
// This allows each tile/square its own trigger and the parent/master can clean up the children when the duration is over.

#endif
