#ifndef RULES_H
#define RULES_H

#include <stdint.h>

#include "ds-object.h"
#include "entity.h"
#include "spells.h"

enum {
    LAWFUL_GOOD,
    LAWFUL_NEUTRAL,
    LAWFUL_EVIL,
    NEUTRAL_GOOD,
    TRUE_NEUTRAL,
    NEUTRAL_EVIL,
    CHAOTIC_GOOD,
    CHAOTIC_NEUTRAL,
    CHAOTIC_EVIL,
};

enum {
    ANY_ALIGNMENT        = 0,
    ONLY_LAWFUL_ETHICS   = 1 << 0,
    ONLY_NEUTRAL_ETHICS  = 1 << 1,
    ONLY_CHAOTIC_ETHICS  = 1 << 2,
    ONLY_GOOD_MORALS     = 1 << 3,
    ONLY_NEUTRAL_MORALS  = 1 << 4,
    ONLY_EVIL_MORALS     = 1 << 5,
    ONLY_LAWFUL_GOOD     = 1 << 6,
    ONLY_LAWFUL_NEUTRAL  = 1 << 7,
    ONLY_LAWFUL_EVIL     = 1 << 8,
    ONLY_NEUTRAL_GOOD    = 1 << 9,
    ONLY_TRUE_NEUTRAL    = 1 << 10,
    ONLY_NEUTRAL_EVIL    = 1 << 11,
    ONLY_CHAOTIC_GOOD    = 1 << 12,
    ONLY_CHAOTIC_NEUTRAL = 1 << 13,
    ONLY_CHAOTIC_EVIL    = 1 << 14,
    NOT_LAWFUL_ETHICS    = 1 << 15,
    NOT_NEUTRAL_ETHICS   = 1 << 16,
    NOT_CHAOTIC_ETHICS   = 1 << 17,
    NOT_GOOD_MORALS      = 1 << 18,
    NOT_NEUTRAL_MORALS   = 1 << 19,
    NOT_EVIL_MORALS      = 1 << 20,
    NOT_LAWFUL_GOOD      = 1 << 21,
    NOT_LAWFUL_NEUTRAL   = 1 << 22,
    NOT_LAWFUL_EVIL      = 1 << 23,
    NOT_NEUTRAL_GOOD     = 1 << 24,
    NOT_TRUE_NEUTRAL     = 1 << 25,
    NOT_NEUTRAL_EVIL     = 1 << 26,
    NOT_CHAOTIC_GOOD     = 1 << 27,
    NOT_CHAOTIC_NEUTRAL  = 1 << 28,
    NOT_CHAOTIC_EVIL     = 1 << 29,
};

int dnd2e_is_class_allowed(const uint8_t race, const int8_t classes[3]);
int dnd2e_is_alignment_allowed(const uint8_t alignment, const int8_t classes[3], int target_is_pc);
void dnd2e_randomize_stats_pc(ds_character_t *pc);
void dnd2e_fix_stats_pc(ds_character_t *pc);
void dnd2e_set_exp(ds_character_t *pc, const uint32_t amt);
void dnd2e_award_exp(ds_character_t *pc, const uint32_t amt);
int dnd2e_character_is_valid(ds_character_t *pc);
int dnd2e_psin_is_valid(ds_character_t *pc, psin_t *psi);
int32_t dnd2e_exp_to_next_level_up(ds_character_t *pc);
int16_t dnd2e_get_ac_pc(ds_character_t *pc, ds_inventory_t *inv);
int16_t dnd2e_get_move_pc(ds_character_t *pc);
int16_t dnd2e_get_thac0_pc(ds_character_t *pc);
int16_t dnd2e_get_damage_pc(ds_character_t *pc);
int16_t dnd2e_get_attack_num_pc(const entity_t *pc, const ds1_item_t *item);
int16_t dnd2e_get_attack_die_pc(const entity_t *pc, const ds1_item_t *item);
int16_t dnd2e_get_attack_sides_pc(const entity_t *pc, const ds1_item_t *item);
int16_t dnd2e_get_attack_mod_pc(const entity_t *pc, const ds1_item_t *item);
int dnd2e_roll_initiative(entity_t *entity);
int dnd2e_roll_sub_roll();

#endif
