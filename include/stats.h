#ifndef SOL_STATS_H
#define SOL_STATS_H

#include <stdint.h>

typedef struct sol_combat_round_stats_s {
    int8_t move;
    int8_t initiative;
    int8_t attack_num;
    int8_t has_cast;
    int8_t has_melee;
    int8_t has_ranged;
} sol_combat_round_stats_t;

typedef struct sol_innate_attack_s {
    uint8_t number;
    uint8_t num_dice;
    uint8_t sides;
    uint8_t bonus;
    uint32_t special;
} sol_innate_attack_t;

typedef struct sol_saving_throws_s {
    uint8_t paralysis;
    uint8_t wand;
    uint8_t petrify;
    uint8_t breath;
    uint8_t spell;
} sol_saving_throws_t;

typedef struct sol_slots_s {
    uint16_t amt, max;
} sol_slots_t;

typedef struct sol_stats_s {
    uint8_t str;
    uint8_t dex;
    uint8_t con;
    uint8_t intel;
    uint8_t wis;
    uint8_t cha;
    int16_t hp;
    int16_t high_hp;
    int16_t psp;
    int16_t high_psp;
    int8_t base_ac; // before applying any modifiers.
    int8_t base_move;
    int8_t base_thac0;
    sol_slots_t wizard[10];
    sol_slots_t priest[10];
    sol_combat_round_stats_t combat;
    uint8_t magic_resistance;
    uint32_t special_defense;
    sol_innate_attack_t attacks[3];
    sol_saving_throws_t saves;
} sol_stats_t;

// Keeping the old interface for now...
extern int8_t sol_dnd2e_melee_damage_mod(const sol_stats_t *stats);
extern int8_t sol_dnd2e_melee_hit_mod(const sol_stats_t *stats);
extern int8_t sol_dnd2e_range_damage_mod(const sol_stats_t *stats);
extern int8_t sol_dnd2e_range_hit_mod(const sol_stats_t *stats);
extern int8_t sol_dnd2e_reaction_mod(const sol_stats_t *stats);
extern int8_t sol_dnd2e_ac_mod(const sol_stats_t *stats);
extern int8_t sol_dnd2e_hp_mod(const sol_stats_t *stats);
extern int8_t sol_dnd2e_shock_mod(const sol_stats_t *stats);
extern int8_t sol_dnd2e_res_mod(const sol_stats_t *stats);
extern int8_t sol_dnd2e_poison_mod(const sol_stats_t *stats);
extern int8_t sol_dnd2e_regen_mod(const sol_stats_t *stats);
extern int8_t sol_dnd2e_stats_valid(const sol_stats_t *stats);

#endif
