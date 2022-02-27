#ifndef SOL_CLASS_H
#define SOL_CLASS_H

#include <stdint.h>

#include "entity.h"

extern int     sol_dnd2e_is_class_allowed(const uint8_t race, const class_t classes[3]);
extern int     sol_dnd2e_next_level_exp(const int8_t class, const int8_t clevel);
extern int     sol_dnd2e_class_total_hit_die(const int8_t class, const int8_t level);
extern int     sol_dnd2e_class_level(const uint8_t class, const uint32_t xp);
extern int32_t sol_dnd2e_class_exp_to_next_level(entity_t *pc);
extern int32_t sol_dnd2e_class_thac0(entity_t *pc);
extern void    sol_dnd2e_class_apply_stats(entity_t *pc, int class);
extern int16_t sol_dnd2e_class_attack_num(const entity_t *pc, const item_t *item);
extern int     sol_dnd2e_class_max_hp(int class, int level, int con_mod);
extern int     sol_dnd2e_class_hp_die(int class);

#endif
