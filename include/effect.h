#ifndef _EFFECT_H
#define _EFFECT_H

#include <stdint.h>
#include "entity.h"
#include "powers.h"

enum {
    DURATION_ROUNDS            = 0x0001,
    DURATION_UNTIL_ATTACKS     = 0x0002,
    DURATION_UNTIL_IS_ATTACKED = 0x0004,
};

enum {
    PREVENT_MELEE    = 0x0001,
    PREVENT_MISSILE  = 0x0002,
    PREVENT_WIZARD   = 0x0004,
    PREVENT_PRIEST   = 0x0008,
    PREVENT_PSIONIC  = 0x0010,
};

typedef enum sol_effect_type_e {
    EFFECT_POISON     = 0x0001,
    EFFECT_FIRE       = 0x0002,
    EFFECT_COLD       = 0x0004,
    EFFECT_BLUNT      = 0x0008,
    EFFECT_CUTTING    = 0x0010,
    EFFECT_POINTED    = 0x0020,
    EFFECT_ACID       = 0x0040,
    EFFECT_ELECTRIC   = 0x0080,
    EFFECT_DRAINING   = 0x0100,
    EFFECT_MAGIC      = 0x0200,
    EFFECT_MENTAL     = 0x0400,
    EFFECT_DEATH      = 0x0800,
    EFFECT_PLUS1      = 0x1000,
    EFFECT_PLUS2      = 0x2000,
    EFFECT_METAL      = 0x4000,
    EFFECT_OVER2      = 0x8000, // More than a +2
    EFFECT_MELEE      = 0x10000,
    EFFECT_MISSILE    = 0x20000,
} sol_effect_type_t;

enum sol_stat_e {
    STAT_STR,
    STAT_DEX,
    STAT_CON,
    STAT_INT,
    STAT_WIS,
    STAT_CHA,
    STAT_CURRENT_HP,
    STAT_HIGH_HP,
    STAT_CURRENT_PSP,
    STAT_HIGH_PSP,
    STAT_BASE_AC, // before applying mods
    STAT_MOVE,
    STAT_BASE_MOVE,
    STAT_BASE_THAC0,
    STAT_MAGIC_RESISTANCE,
    STAT_SAVE_PARALYSIS,
    STAT_SAVE_WAND,
    STAT_SAVE_PETRIFY,
    STAT_SAVE_BREATH,
    STAT_SAVE_SPELL,
    STAT_AC,
    STAT_THAC0,
    STAT_DAMAGE_BASE,
    STAT_DAMAGE_PLUS,
    STAT_SAVE,
    STAT_INTIATIVE,
    STAT_DAMAGE_TAKEN,
    STAT_NUMBER_OF_ATTACKS,
};

typedef struct sol_effect_s {
    sol_power_t *pw; // the power that generated this effect.
    uint16_t duration_type; // indefinite/rounds
    uint16_t duration_left;
    uint16_t preventions; // does this prevent something (Example: attacking.)
    uint16_t fx;// Special Effect (invisibility, shimmer, etc...)
    int32_t (*affect)                (const sol_entity_t *target, enum sol_stat_e stat, const uint64_t effect_type, const int32_t amt);
    // Some spell have an end of effect trigger. (EX: Spirit Armor: need to save vs spell or suffer 2d3 damage.)
    void    (*end_of_effect_trigger) (sol_entity_t *source);
} sol_effect_t;

extern sol_status_t sol_effect_apply_damage(sol_entity_t *source, sol_entity_t *target, const int32_t damage, sol_effect_type_t type);
extern sol_status_t sol_effect_apply_condition(sol_entity_t *source, sol_entity_t *target, uint16_t condition);

#endif
