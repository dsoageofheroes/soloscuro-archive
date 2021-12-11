#ifndef POWERS_H
#define POWERS_H

#include "entity.h"

enum target_shape {
    TARGET_NONE,
    TARGET_ALLY,
    TARGET_SELF,
    TARGET_ENEMY,
    TARGET_SINGLE, // check AOE for circle (IE: Fireball)
    TARGET_CONE,   // EX: Cone of Cold
    TARGET_RECTANGLE, // aoe is two 8-bit ints, width times height
    TARGET_ANY,
    TARGET_MULTI,  // aoe is number of charges.
};

struct power_instance_s;
struct power_s;

typedef struct power_actions_s {
    int  (*can_activate) (struct power_instance_s *source, const int16_t power_level);
    int  (*pay)          (struct power_instance_s *source, const int16_t power_level);
    void (*apply)        (struct power_instance_s *source, entity_t *target);
    int  (*affect_power) (struct power_instance_s *target);
    int  (*update)       (struct power_instance_s *source, struct power_s *power);
} power_actions_t;

// A power is a spell, psionic, innate special ability, or charge on an item.
// This should be stored in a general list/array/hashtable for access at any time.
typedef struct power_s {
    char            *name;
    const char      *const_name;
    char            *description;
    int32_t         range;
    int32_t         aoe;
    enum            target_shape shape;
    uint16_t           icon_id;
    animate_sprite_t   icon;
    animate_sprite_t   cast;
    animate_sprite_t   thrown;
    animate_sprite_t   hit;
    uint16_t        cast_sound, thrown_sound, hit_sound;
    int8_t          level;
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

typedef struct power_list_s {
    power_instance_t *head;
} power_list_t;

extern power_list_t* power_list_create();
extern void power_list_free(power_list_t *pl);
extern void power_list_add(power_list_t *pl, power_t *pw);

// List of all the spell nodes on a region.
typedef struct power_overlay_s {
    power_list_t *power_list[MAP_ROWS][MAP_COLUMNS];
} power_overlay_t;

// Okay before I forget.
// Spells create a spell entity on point of impact.
// If AOE, then many children entities are created that point to the parent/master spell entity.
// This allows each tile/square its own trigger and the parent/master can clean up the children when the duration is over.

extern void              powers_init();
extern void              powers_cleanup();
extern power_t*          power_create();
extern animate_sprite_t* power_get_icon(power_t *pw);
extern void              powers_set_cast(power_t *powers, const uint32_t id);
extern void              powers_set_icon(power_t *powers, const uint32_t id);
extern void              powers_set_thrown(power_t *powers, const uint32_t id);
extern void              powers_set_hit(power_t *powers, const uint32_t id);
extern void              powers_load(power_t *powers);
extern void              power_free(power_t *pw);
extern enum target_shape power_get_target_type(power_t *power);
extern size_t            power_select_by_game(const size_t ds1, const size_t ds2, const size_t dso);
extern char*             power_spin_read_description(const uint16_t id);

#endif
