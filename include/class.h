#ifndef SOL_CLASS_H
#define SOL_CLASS_H

#include <stdint.h>

#include "entity.h"

extern sol_status_t sol_dnd2e_is_class_allowed(const uint8_t race, const sol_class_t classes[3]);
extern sol_status_t sol_dnd2e_next_level_exp(const int8_t class, const int8_t clevel, int8_t *nlevel);
extern sol_status_t sol_dnd2e_class_total_hit_die(const int8_t class, const int8_t level, uint8_t *hit_die);
extern sol_status_t sol_dnd2e_class_level(const uint8_t class, const uint32_t xp, uint8_t *class_level);
extern sol_status_t sol_dnd2e_class_exp_to_next_level(sol_entity_t *pc, int32_t *next_exp);
extern sol_status_t sol_dnd2e_class_thac0(sol_entity_t *pc, int32_t *thac0_ret);
extern sol_status_t sol_dnd2e_class_update_max_spell_slots(sol_entity_t *pc);
extern sol_status_t sol_dnd2e_class_apply_stats(sol_entity_t *pc, int class);
extern sol_status_t sol_dnd2e_class_attack_num(const sol_entity_t *pc, const sol_item_t *item, int16_t *attack_num);
extern sol_status_t sol_dnd2e_class_max_hp(int class, int level, int con_mod, int32_t *max_hp);
extern sol_status_t sol_dnd2e_class_hp_die(int class, int8_t *ht);

#endif
