#ifndef SOL_ALIGNMENT_H
#define SOL_ALIGNMENT_H

#include "status.h"
#include "entity.h"

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

sol_status_t sol_dnd2e_alignment_allowed(const uint8_t alignment, const class_t classes[3], int target_is_pc);

#endif
