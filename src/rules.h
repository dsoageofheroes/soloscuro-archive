#ifndef RULES_H
#define RULES_H

#include <stdint.h>

#include "dsl-object.h"

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

int dnd2e_is_class_allowed(const uint8_t race, const int8_t classes[3]);
void dnd2e_randomize_stats_pc(ds_character_t *pc);
void dnd2e_fix_stats_pc(ds_character_t *pc);
void dnd2e_set_exp(ds_character_t *pc, const uint32_t amt);
void dnd2e_award_exp(ds_character_t *pc, const uint32_t amt);
int32_t dnd2e_exp_to_next_level_up(ds_character_t *pc);
int16_t dnd2e_get_ac_pc(ds_character_t *pc);
int16_t dnd2e_get_damage_pc(ds_character_t *pc);
int16_t dnd2e_get_attack_num_pc(ds_character_t *pc);
int16_t dnd2e_get_attack_die_pc(ds_character_t *pc);
int16_t dnd2e_get_attack_mod_pc(ds_character_t *pc);

#endif