#ifndef RULES_H
#define RULES_H

#include <stdint.h>

#include "ssi-object.h"
#include "entity.h"
#include "wizard.h"
#include "combat.h"

void    dnd2e_randomize_stats_pc(sol_entity_t *pc);
void    dnd2e_set_starting_level(sol_entity_t *pc);
void    dnd2e_loop_creation_stats(sol_entity_t *pc);
void    dnd2e_set_exp(sol_entity_t *pc, const uint32_t amt);
void    dnd2e_award_exp(sol_entity_t *pc, const uint32_t amt);
int     dnd2e_character_is_valid(const sol_entity_t *pc);
int     dnd2e_psin_is_valid(ds_character_t *pc, sol_psin_t *psi);
int16_t dnd2e_calc_ac(sol_entity_t *entity);
int32_t dnd2e_exp_to_next_level_up(sol_entity_t *pc);
int16_t dnd2e_get_ac_pc(sol_entity_t *pc);
int16_t dnd2e_get_move(sol_entity_t *pc);
int16_t dnd2e_get_thac0(sol_entity_t *pc, int slot);
int16_t dnd2e_get_attack_num(const sol_entity_t *pc, const sol_item_t *item);
int16_t dnd2e_get_attack_die_pc(const sol_entity_t *pc, const sol_item_t *item);
int16_t dnd2e_get_attack_sides_pc(const sol_entity_t *pc, const sol_item_t *item);
int16_t dnd2e_get_attack_mod_pc(const sol_entity_t *pc, const sol_item_t *item);
int     dnd2e_roll_initiative(sol_entity_t *entity);
int     dnd2e_roll_sub_roll();

//Combat:
extern int16_t      dnd2e_can_melee_again(sol_entity_t *source, const int attack_num, const int round);
extern sol_attack_t sol_dnd2e_melee_attack(sol_entity_t *source, sol_entity_t *target, const int round);
extern sol_attack_t sol_dnd2e_range_attack(sol_entity_t *source, sol_entity_t *target, const int round);
extern int16_t      dnd2e_dice_roll(const uint16_t num, const uint16_t sides);
#endif
