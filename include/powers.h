#ifndef POWERS_H
#define POWERS_H

#include "entity.h"

enum sol_target_shape_e {
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

struct sol_power_instance_s;
struct sol_power_s;

typedef struct sol_power_actions_s {
    int  (*can_activate) (struct sol_power_instance_s *source, const int16_t power_level);
    int  (*pay)          (struct sol_power_instance_s *source, const int16_t power_level);
    void (*apply)        (struct sol_power_instance_s *source, sol_entity_t *target);
    int  (*affect_power) (struct sol_power_instance_s *target);
    int  (*update)       (struct sol_power_instance_s *source, struct sol_power_s *power);
} sol_power_actions_t;

// A power is a spell, psionic, innate special ability, or charge on an item.
// This should be stored in a general list/array/hashtable for access at any time.
typedef struct sol_power_s {
    char            *name;
    const char      *const_name;
    char            *description;
    int32_t         range;
    int32_t         aoe;
    enum            sol_target_shape_e shape;
    uint16_t           icon_id;
    animate_sprite_t   icon;
    animate_sprite_t   cast;
    animate_sprite_t   thrown;
    animate_sprite_t   hit;
    uint16_t        cast_sound, thrown_sound, hit_sound;
    int8_t          level;
    sol_power_actions_t actions;
    // Add animation sequence info here.
} sol_power_t;

// This (power/spell entity) is an instance of the spell on a tile in the region. EX: Grease
// During creation a COPY of the entity or item is made, but only one per group.
typedef struct sol_power_instance_s {
    sol_entity_t   *entity;
    sol_item_t *item;
    sol_power_t    *stats;
    struct sol_power_instance_s *next, *prev;
} sol_power_instance_t;

typedef struct sol_power_list_s {
    sol_power_instance_t *head;
} sol_power_list_t;


// List of all the spell nodes on a region.
typedef struct sol_power_overlay_s {
    sol_power_list_t *power_list[MAP_ROWS][MAP_COLUMNS];
} sol_power_overlay_t;

// Okay before I forget.
// Spells create a spell entity on point of impact.
// If AOE, then many children entities are created that point to the parent/master spell entity.
// This allows each tile/square its own trigger and the parent/master can clean up the children when the duration is over.

extern sol_status_t sol_power_list_create(sol_power_list_t **p);
extern sol_status_t sol_power_list_free(sol_power_list_t *pl);
extern sol_status_t sol_power_list_add(sol_power_list_t *pl, sol_power_t *pw);

extern sol_status_t sol_powers_init();
extern sol_status_t sol_powers_cleanup();
extern sol_status_t sol_powers_set_cast(sol_power_t *powers, const uint32_t id);
extern sol_status_t sol_powers_set_icon(sol_power_t *powers, const uint32_t id);
extern sol_status_t sol_powers_set_thrown(sol_power_t *powers, const uint32_t id);
extern sol_status_t sol_powers_set_hit(sol_power_t *powers, const uint32_t id);
extern sol_status_t sol_powers_load(sol_power_t *powers);
extern sol_status_t sol_power_free(sol_power_t *pw);
extern sol_status_t sol_power_create(sol_power_t **p);
extern sol_status_t sol_power_get_icon(sol_power_t *pw, animate_sprite_t **a);
extern sol_status_t sol_power_get_target_type(sol_power_t *power, enum sol_target_shape_e *e);
extern sol_status_t sol_power_select_by_game(const size_t ds1, const size_t ds2, const size_t dso, size_t *what);
extern sol_status_t sol_power_spin_read_description(const uint16_t id, char **msg);

#endif
