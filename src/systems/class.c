#include "rules.h"

enum {
    A_END   = 0x0,     // End of Allowed Classes
    A_CLR   = 0x1,     // Allowed Cleric
    A_DRU   = 0x2,     // Allowed Druid
    A_FGR   = 0x4,     // Allowed Fighter
    A_GLD   = 0x8,     // Allowed Gladiator
    A_PRE   = 0x10,    // Allowed Preserver
    A_PSI   = 0x20,    // Allowed Psionicist
    A_RNG   = 0x40,    // Allowed Ranger
    A_THF   = 0x80,    // Allowed Thief
    A_DLC   = 0x8000,  // Allowed Dual Class
};

static const uint16_t human_classes[] = {
    A_CLR, A_DRU, A_FGR, A_GLD, A_PRE, A_PSI, A_RNG, A_THF,

    A_DLC | A_CLR | A_FGR,            A_DLC | A_CLR | A_GLD,            A_DLC | A_CLR | A_PRE,
    A_DLC | A_CLR | A_PSI,            A_DLC | A_CLR | A_RNG,            A_DLC | A_CLR | A_THF,
    A_DLC | A_CLR | A_FGR | A_PRE,    A_DLC | A_CLR | A_FGR | A_PSI,    A_DLC | A_CLR | A_FGR | A_THF,
    A_DLC | A_CLR | A_GLD | A_PRE,    A_DLC | A_CLR | A_GLD | A_PSI,    A_DLC | A_CLR | A_GLD | A_THF,
    A_DLC | A_CLR | A_PRE | A_PSI,    A_DLC | A_CLR | A_PRE | A_RNG,    A_DLC | A_CLR | A_PRE | A_THF,
    A_DLC | A_CLR | A_PSI | A_RNG,    A_DLC | A_CLR | A_PSI | A_THF,
    A_DLC | A_CLR | A_RNG | A_THF,

    A_DLC | A_DRU | A_FGR,            A_DLC | A_DRU | A_GLD,            A_DLC | A_DRU | A_PRE,
    A_DLC | A_DRU | A_PSI,            A_DLC | A_DRU | A_THF,
    A_DLC | A_DRU | A_FGR | A_PRE,    A_DLC | A_DRU | A_FGR | A_PSI,    A_DLC | A_DRU | A_FGR | A_THF,
    A_DLC | A_DRU | A_GLD | A_PRE,    A_DLC | A_DRU | A_GLD | A_PSI,    A_DLC | A_DRU | A_GLD | A_THF,
    A_DLC | A_DRU | A_PRE | A_PSI,    A_DLC | A_DRU | A_PRE | A_THF,
    A_DLC | A_DRU | A_PSI | A_THF,

    A_DLC | A_FGR | A_PRE,            A_DLC | A_FGR | A_PSI,            A_DLC | A_FGR | A_THF,
    A_DLC | A_FGR | A_PRE | A_PSI,    A_DLC | A_FGR | A_PRE | A_THF,
    A_DLC | A_FGR | A_PSI | A_THF,

    A_DLC | A_GLD | A_PRE,            A_DLC | A_GLD | A_PSI,            A_DLC | A_GLD | A_THF,
    A_DLC | A_GLD | A_PRE | A_PSI,    A_DLC | A_GLD | A_PRE | A_THF,
    A_DLC | A_GLD | A_PSI | A_THF,

    A_DLC | A_PRE | A_PSI,            A_DLC | A_PRE | A_RNG,            A_DLC | A_PRE | A_THF,
    A_DLC | A_PRE | A_PSI | A_RNG,    A_DLC | A_PRE | A_PSI | A_THF,
    A_DLC | A_PRE | A_RNG | A_THF,

    A_DLC | A_PSI | A_RNG,            A_DLC | A_PSI | A_THF,
    A_DLC | A_PSI | A_RNG | A_THF,

    A_DLC | A_RNG | A_THF,

    A_END // END Human
};
static const uint16_t dwarf_classes[] = {
    A_CLR, A_FGR, A_GLD, A_PSI, A_THF,

    A_CLR | A_FGR,                    A_CLR | A_PSI,

    A_FGR | A_PSI,                    A_FGR | A_THF,

    A_PSI | A_THF,

    A_END // END Dwarf
};
static const uint16_t elf_classes[] = {
    A_CLR, A_FGR, A_GLD, A_PRE, A_PSI, A_RNG, A_THF,

    A_CLR | A_FGR,                    A_CLR | A_PRE,                    A_CLR | A_PSI,
    A_CLR | A_RNG,                    A_CLR | A_THF,
    A_CLR | A_FGR | A_PRE,            A_CLR | A_FGR | A_PSI,            A_CLR | A_FGR | A_THF,
    A_CLR | A_PRE | A_PSI,            A_CLR | A_PRE | A_RNG,            A_CLR | A_PRE | A_THF,
    A_CLR | A_PSI | A_RNG,            A_CLR | A_PSI | A_THF,
    A_CLR | A_RNG | A_THF,

    A_FGR | A_PRE,                    A_FGR | A_PSI,                    A_FGR | A_THF,
    A_FGR | A_PRE | A_PSI,            A_FGR | A_PRE | A_THF,
    A_FGR | A_PSI | A_THF,

    A_PRE | A_PSI,                    A_PRE | A_RNG,                    A_PRE | A_THF,
    A_PRE | A_PSI | A_RNG,            A_PRE | A_PSI | A_THF,

    A_PSI | A_RNG,                    A_PSI | A_THF,
    A_PSI | A_RNG | A_THF,

    A_RNG | A_THF,

    A_END // END Elf
};
static const uint16_t half_elf_classes[] = {
    A_CLR, A_DRU, A_FGR, A_GLD, A_PRE, A_PSI, A_RNG, A_THF,

    A_CLR | A_FGR,                    A_CLR | A_PRE,                    A_CLR | A_PSI,
    A_CLR | A_RNG,                    A_CLR | A_THF,
    A_CLR | A_FGR | A_PRE,            A_CLR | A_FGR | A_PSI,            A_CLR | A_FGR | A_THF,
    A_CLR | A_PRE | A_PSI,            A_CLR | A_PRE | A_RNG,            A_CLR | A_PRE | A_THF,
    A_CLR | A_PSI | A_RNG,            A_CLR | A_PSI | A_THF,
    A_CLR | A_RNG | A_THF,

    A_DRU | A_FGR,                    A_DRU | A_PRE,                    A_DRU | A_PSI,
    A_DRU | A_THF,
    A_DRU | A_FGR | A_PRE,            A_DRU | A_FGR | A_PSI,            A_DRU | A_FGR | A_THF,
    A_DRU | A_PRE | A_PSI,            A_DRU | A_PRE | A_THF,
    A_DRU | A_PSI | A_THF,

    A_FGR | A_PRE,                    A_FGR | A_PSI,                    A_FGR | A_THF,
    A_FGR | A_PRE | A_PSI,            A_FGR | A_PRE | A_THF,
    A_FGR | A_PSI | A_THF,

    A_PRE | A_PSI,                    A_PRE | A_RNG,                    A_PRE | A_THF,
    A_PRE | A_PSI | A_RNG,            A_PRE | A_PSI | A_THF,
    A_PRE | A_RNG | A_THF,

    A_PSI | A_RNG,                    A_PSI | A_THF,
    A_PSI | A_RNG | A_THF,

    A_RNG | A_THF,


    A_END // END Half-Elf
};
static const uint16_t half_giant_classes[] = {
    A_CLR, A_FGR, A_GLD, A_PSI, A_RNG,

    A_CLR | A_FGR,                    A_CLR | A_PSI,                    A_CLR | A_RNG,

    A_FGR | A_PSI,

    A_PSI | A_RNG,

    A_END // END Half-Giant
};
static const uint16_t halfling_classes[] = {
    A_CLR, A_DRU, A_FGR, A_GLD, A_PSI, A_RNG, A_THF,

    A_CLR | A_FGR,                    A_CLR | A_PSI,                    A_CLR | A_RNG,
    A_CLR | A_THF,

    A_DRU | A_FGR,                    A_DRU | A_PSI,                    A_DRU | A_RNG,
    A_DRU | A_THF,

    A_FGR | A_PSI,                    A_FGR | A_THF,
    A_FGR | A_PSI | A_THF,

    A_PSI | A_RNG,                    A_PSI | A_THF,
    A_PSI | A_RNG | A_THF,

    A_RNG | A_THF,

    A_END // END Halfling
};
static const uint16_t mul_classes[] = {
    A_CLR, A_DRU, A_FGR, A_GLD, A_PSI, A_RNG, A_THF,

    A_CLR | A_FGR,                    A_CLR | A_PSI,                    A_CLR | A_THF,
    A_CLR | A_FGR | A_THF,

    A_DRU | A_FGR,                    A_DRU | A_PSI,                    A_DRU | A_THF,
    A_DRU | A_FGR | A_THF,

    A_FGR | A_PSI,                    A_FGR | A_THF,

    A_FGR | A_PSI | A_THF,

    A_PSI | A_THF,

    A_END // END Mul
};
static const uint16_t thrikreen_classes[] = {
    A_CLR, A_DRU, A_FGR, A_GLD, A_PSI, A_RNG,

    A_CLR | A_FGR,                    A_CLR | A_PSI,                    A_CLR | A_RNG,
    A_CLR | A_FGR | A_PSI,
    A_CLR | A_PSI | A_RNG,

    A_DRU | A_FGR,                    A_DRU | A_PSI,
    A_DRU | A_FGR | A_PSI,

    A_FGR | A_PSI,

    A_PSI | A_RNG,

    A_END // END Thri-Kreen
};

//        exp    hdlimit      hapr
static const uint32_t fighter_levels[][3] = {
    {       0,         0,        0 }, // 00 - the third parameter is how many half-attacks per round.
    {       0,         1,        2 }, // 01
    {    2000,         2,        2 }, // 02
    {    4000,         3,        2 }, // 03
    {    8000,         4,        2 }, // 04
    {   16000,         5,        2 }, // 05
    {   32000,         6,        2 }, // 06
    {   64000,         7,        3 }, // 07
    {  125000,         8,        3 }, // 08
    {  250000,         9,        3 }, // 09
    {  500000,         9,        3 }, // 10
    {  750000,         9,        3 }, // 11
    { 1000000,         9,        3 }, // 12
    { 1250000,         9,        4 }, // 13
    { 1500000,         9,        4 }, // 14
    { 1750000,         9,        4 }, // 15
    { 2000000,         9,        4 }, // 16
    { 2250000,         9,        4 }, // 17
    { 2500000,         9,        4 }, // 18
    { 2750000,         9,        4 }, // 19
    { 3000000,         9,        4 }, // 20
};

//        exp    hdlimit      hapr
static const uint32_t ranger_levels[][3] = {
    {       0,         0,        0 }, // 00 - the third parameter is how many half-attacks per round.
    {       0,         1,        2 }, // 01
    {    2250,         2,        2 }, // 02
    {    4500,         3,        2 }, // 03
    {    9000,         4,        2 }, // 04
    {   18000,         5,        2 }, // 05
    {   36000,         6,        2 }, // 06
    {   75000,         7,        3 }, // 07
    {  150000,         8,        3 }, // 08
    {  300000,         9,        3 }, // 09
    {  600000,         9,        3 }, // 10
    {  900000,         9,        3 }, // 11
    { 1200000,         9,        3 }, // 12
    { 1500000,         9,        4 }, // 13
    { 1800000,         9,        4 }, // 14
    { 2100000,         9,        4 }, // 15
    { 2400000,         9,        4 }, // 16
    { 2700000,         9,        4 }, // 17
    { 3000000,         9,        4 }, // 18
    { 3300000,         9,        4 }, // 19
    { 3600000,         9,        4 }, // 20
};

//         exp   hdlimit      hapr
static const uint32_t preserver_levels[][3] = {
    {        0,        0,        0 }, // 00 - the third parameter is how many half-attacks per round.
    {        0,        1,        0 }, // 01
    {     2500,        2,        0 }, // 02
    {     5000,        3,        0 }, // 03
    {    10000,        4,        0 }, // 04
    {    20000,        5,        0 }, // 05
    {    40000,        6,        0 }, // 06
    {    60000,        7,        0 }, // 07
    {    90000,        8,        0 }, // 08
    {   135000,        9,        0 }, // 09
    {   250000,       10,        0 }, // 10
    {   375000,       10,        0 }, // 11
    {   750000,       10,        0 }, // 12
    {  1125000,       10,        0 }, // 13
    {  1500000,       10,        0 }, // 14
    {  1875000,       10,        0 }, // 15
    {  2250000,       10,        0 }, // 16
    {  2625000,       10,        0 }, // 17
    {  3000000,       10,        0 }, // 18
    {  3375000,       10,        0 }, // 19
    {  3750000,       10,        0 }, // 20
};

//         exp   hdlimit      hapr
static const uint32_t defiler_levels[][3] = {
    {        0,        0,        0 }, // 00 - the third parameter is how many half-attacks per round.
    {        0,        1,        0 }, // 01
    {     1750,        2,        0 }, // 02
    {     3500,        3,        0 }, // 03
    {     7000,        4,        0 }, // 04
    {    14000,        5,        0 }, // 05
    {    28000,        6,        0 }, // 06
    {    42000,        7,        0 }, // 07
    {    63000,        8,        0 }, // 08
    {    94500,        9,        0 }, // 09
    {   180000,       10,        0 }, // 10
    {   270000,       10,        0 }, // 11
    {   540000,       10,        0 }, // 12
    {   820000,       10,        0 }, // 13
    {  1080000,       10,        0 }, // 14
    {  1350000,        0,        0 }, // 15
    {  1620000,        0,        0 }, // 16
    {  1890000,        0,        0 }, // 17
    {  2160000,        0,        0 }, // 18
    {  2430000,        0,        0 }, // 19
    {  2700000,        0,        0 }, // 20
};

static const int8_t class_mininum[][6] = {
    // REAL_CLASS_NONE,
    { 0, 0, 0, 0, 0, 0 },
    // REAL_CLASS_AIR_CLERIC,
    { 0, 0, 0, 0, 9, 0 },
    // REAL_CLASS_EARTH_CLERIC,
    { 0, 0, 0, 0, 9, 0 },
    // REAL_CLASS_FIRE_CLERIC,
    { 0, 0, 0, 0, 9, 0 },
    // REAL_CLASS_WATER_CLERIC,
    { 0, 0, 0, 0, 9, 0 },
    // REAL_CLASS_AIR_DRUID,
    { 0, 0, 0, 0,12,15 },
    // REAL_CLASS_EARTH_DRUID,
    { 0, 0, 0, 0,12,15 },
    // REAL_CLASS_FIRE_DRUID,
    { 0, 0, 0, 0,12,15 },
    // REAL_CLASS_WATER_DRUID,
    { 0, 0, 0, 0,12,15 },
    // REAL_CLASS_FIGHTER,
    { 9, 0, 0, 0, 0, 0 },
    // REAL_CLASS_GLADIATOR,
    { 13, 12, 15, 0, 0, 0 },
    // REAL_CLASS_PRESERVER,
    { 0, 0, 0, 9, 0, 0 },
    // REAL_CLASS_PSIONICIST,
    { 0, 0,11,12,15, 0 },
    // REAL_CLASS_AIR_RANGER,
    {13,13,14, 0,14, 0 },
    // REAL_CLASS_EARTH_RANGER,
    {13,13,14, 0,14, 0 },
    // REAL_CLASS_FIRE_RANGER,
    {13,13,14, 0,14, 0 },
    // REAL_CLASS_WATER_RANGER,
    {13,13,14, 0,14, 0 },
    // REAL_CLASS_THIEF,
    { 0, 9, 0, 0, 0, 0 },
    // REAL_CLASS_DEFILER,
    { 0, 0, 0, 9, 0, 0 },
    // REAL_CLASS_TEMPLAR,
    { 0, 0, 0,10, 9, 0 },
    // REAL_CLASS_MAX,
};


static int8_t warrior_thac0[] = {
    20, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1
};

static int8_t rogue_thac0[] = {
    20, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11
};

static int8_t priest_thac0[] = {
    20, 20, 20, 20, 18, 18, 18, 16, 16, 16, 14, 14, 14, 12, 12, 12, 10, 10, 10, 8, 8
};

static int8_t wizard_thac0[] = {
    20, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 15, 15, 15, 14, 14
};

static const int8_t hit_die[] = {
     0,  8, 8, 8,  8,  8,  8,  8, 8,
    10, 10, 4, 6, 10, 10, 10, 10, 6
};

static int8_t wizard_spell_slots[][10] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {3, 2, 0, 0, 0, 0, 0, 0, 0, 0},
    {4, 2, 1, 0, 0, 0, 0, 0, 0, 0},
    {4, 2, 2, 0, 0, 0, 0, 0, 0, 0},
    {4, 3, 2, 1, 0, 0, 0, 0, 0, 0},
    {4, 3, 3, 2, 0, 0, 0, 0, 0, 0},
    {4, 3, 3, 2, 1, 0, 0, 0, 0, 0},
    {4, 4, 3, 2, 2, 0, 0, 0, 0, 0},
    {4, 4, 4, 3, 3, 0, 0, 0, 0, 0},
    {4, 4, 4, 4, 4, 1, 0, 0, 0, 0},
    {5, 5, 5, 4, 4, 2, 0, 0, 0, 0},
    {5, 5, 5, 4, 4, 2, 1, 0, 0, 0},
    {5, 5, 5, 5, 5, 2, 1, 0, 0, 0},
    {5, 5, 5, 5, 5, 3, 2, 1, 0, 0},
    {5, 5, 5, 5, 5, 3, 3, 2, 0, 0},
    {5, 5, 5, 5, 5, 3, 3, 2, 1, 0},
    {5, 5, 5, 5, 5, 3, 3, 3, 1, 0},
    {5, 5, 5, 5, 5, 4, 3, 3, 2, 0},
};

static int8_t priest_spell_slots[][10] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {3, 2, 0, 0, 0, 0, 0, 0, 0, 0},
    {3, 3, 1, 0, 0, 0, 0, 0, 0, 0},
    {3, 3, 2, 0, 0, 0, 0, 0, 0, 0},
    {3, 3, 2, 1, 0, 0, 0, 0, 0, 0},
    {3, 3, 3, 2, 0, 0, 0, 0, 0, 0},
    {4, 4, 3, 2, 1, 0, 0, 0, 0, 0},
    {4, 4, 3, 3, 2, 0, 0, 0, 0, 0},
    {5, 4, 4, 3, 2, 1, 0, 0, 0, 0},
    {6, 5, 5, 3, 2, 2, 0, 0, 0, 0},
    {6, 6, 6, 4, 2, 2, 0, 0, 0, 0},
    {6, 6, 6, 5, 3, 2, 1, 0, 0, 0},
    {6, 6, 6, 6, 4, 2, 1, 0, 0, 0},
    {7, 7, 7, 6, 4, 3, 1, 0, 0, 0},
    {7, 7, 7, 7, 5, 3, 2, 0, 0, 0},
    {8, 8, 8, 8, 6, 4, 2, 0, 0, 0},
    {9, 9, 8, 8, 6, 4, 2, 0, 0, 0},
    {9, 9, 9, 8, 7, 5, 2, 0, 0, 0},
};

static int8_t ranger_spell_slots[][10] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {3, 2, 1, 0, 0, 0, 0, 0, 0, 0},
    {4, 2, 2, 0, 0, 0, 0, 0, 0, 0},
    {5, 2, 2, 1, 0, 0, 0, 0, 0, 0},
    {6, 3, 2, 1, 0, 0, 0, 0, 0, 0},
    {7, 3, 2, 2, 0, 0, 0, 0, 0, 0},
    {8, 3, 3, 2, 0, 0, 0, 0, 0, 0},
    {9, 3, 3, 3, 0, 0, 0, 0, 0, 0}, // This is max for rangers.
    {9, 3, 3, 3, 0, 0, 0, 0, 0, 0},
    {9, 3, 3, 3, 0, 0, 0, 0, 0, 0},
    {9, 3, 3, 3, 0, 0, 0, 0, 0, 0},
    {9, 3, 3, 3, 0, 0, 0, 0, 0, 0},
};

#define MAX(a,b) (a > b) ? (a) : (b)

extern void sol_dnd2e_class_update_max_spell_slots(entity_t *pc) {
    if (!pc) { return; }
    int wizard_level = entity_get_wizard_level(pc);
    int priest_level = entity_get_priest_level(pc);
    int ranger_level = entity_get_ranger_level(pc);

    for (int i = 0; i < 10; i++) {
        pc->stats.wizard[i].max = wizard_spell_slots[wizard_level][i];
        pc->stats.wizard[i].amt = MAX(pc->stats.wizard[i].amt, pc->stats.wizard[i].max);
        pc->stats.priest[i].max = priest_spell_slots[priest_level][i];
        if (ranger_spell_slots[ranger_level][i] > pc->stats.priest[i].max) {
            pc->stats.priest[i].max = ranger_spell_slots[ranger_level][i];
        }
        pc->stats.priest[i].amt = MAX(pc->stats.priest[i].amt, pc->stats.priest[i].max);
    }
}

static int convert_to_class_sel(const uint8_t class) {
    switch (class) {
        case REAL_CLASS_AIR_CLERIC:
        case REAL_CLASS_EARTH_CLERIC:
        case REAL_CLASS_FIRE_CLERIC:
        case REAL_CLASS_WATER_CLERIC:
            return A_CLR;
        case REAL_CLASS_AIR_DRUID:
        case REAL_CLASS_EARTH_DRUID:
        case REAL_CLASS_FIRE_DRUID:
        case REAL_CLASS_WATER_DRUID:
            return A_DRU;
        case REAL_CLASS_FIGHTER: return A_FGR;
        case REAL_CLASS_GLADIATOR: return A_GLD;
        case REAL_CLASS_PRESERVER: return A_PRE;
        case REAL_CLASS_PSIONICIST: return A_PSI;
        case REAL_CLASS_AIR_RANGER: return A_RNG;
        case REAL_CLASS_THIEF: return A_THF;
    }

    return 0; // UNKNOWN CLASS
}

int sol_dnd2e_is_class_allowed(const uint8_t race, const class_t classes[3]) {
    uint16_t class = convert_to_class_sel(classes[0].class);
    const uint16_t *allowed = NULL;
    if (classes[0].class == -1) { return 0; }

    //printf("race = %d (%d, %d, %d)\n", race, classes[0], classes[1], classes[2]);
    if (classes[1].class != -1) {
        // NO DUPS.
        if (classes[0].class == classes[1].class || classes[1].class == classes[2].class
            || classes[0].class == classes[2].class) { return 0; }
        class |= convert_to_class_sel(classes[1].class);
        if (classes[2].class != -1) {
            class |= convert_to_class_sel(classes[2].class);
        }
    }

    switch(race) {
        case RACE_HUMAN: allowed = human_classes; break;
        case RACE_DWARF: allowed = dwarf_classes; break;
        case RACE_ELF: allowed = elf_classes; break;
        case RACE_HALFELF: allowed = half_elf_classes; break;
        case RACE_HALFGIANT: allowed = half_giant_classes; break;
        case RACE_HALFLING: allowed = halfling_classes; break;
        case RACE_MUL: allowed = mul_classes; break;
        case RACE_THRIKREEN: allowed = thrikreen_classes; break;
    }

    while (allowed && *allowed) {
        if (*allowed == class) { return 1; }
        allowed++;
    }
    return 0;
}

static const uint32_t* get_xp_table(const uint8_t class, const uint8_t level) {
    switch (class) {
        case REAL_CLASS_AIR_CLERIC:
        case REAL_CLASS_EARTH_CLERIC:
        case REAL_CLASS_FIRE_CLERIC:
        case REAL_CLASS_WATER_CLERIC:
            return (const uint32_t *)ranger_levels[level];
        case REAL_CLASS_AIR_DRUID:
        case REAL_CLASS_EARTH_DRUID:
        case REAL_CLASS_FIRE_DRUID:
        case REAL_CLASS_WATER_DRUID:
            return (const uint32_t *)ranger_levels[level];
        case REAL_CLASS_FIGHTER:
            return (const uint32_t *)fighter_levels[level];
        case REAL_CLASS_GLADIATOR:
            return (const uint32_t *)ranger_levels[level];
        case REAL_CLASS_PRESERVER:
            return (const uint32_t *)preserver_levels[level];
        case REAL_CLASS_PSIONICIST:
            return (const uint32_t *)fighter_levels[level];
        case REAL_CLASS_AIR_RANGER:
        case REAL_CLASS_EARTH_RANGER:
        case REAL_CLASS_FIRE_RANGER:
        case REAL_CLASS_WATER_RANGER:
            return (const uint32_t *)ranger_levels[level];
        case REAL_CLASS_THIEF:
            return (const uint32_t *)fighter_levels[level];
    }
    return NULL;
}

extern int sol_dnd2e_next_level_exp(const int8_t class, const int8_t clevel) {
    const uint32_t *xp_table = get_xp_table(class, clevel);
    if (xp_table == NULL) { return -1; }
    if (clevel < 0 || clevel > 19) { return -1; }

    return xp_table[0];
}

extern int sol_dnd2e_class_total_hit_die(const int8_t class, const int8_t level) {
    const uint32_t *xp_table = get_xp_table(class, level);
    if (xp_table == NULL) { return -1; }
    if (level < 0 || level > 19) { return -1; }

    return xp_table[1];
}

extern int sol_dnd2e_class_level(const uint8_t class, const uint32_t xp) {
    const uint32_t *xp_table = get_xp_table(class, 0);
    if (!xp_table) { return 0; }

    for (int i = 0; i < 19; i++) {
        xp_table = get_xp_table(class, i);
        //printf("xp_table[%d] = %d, xp = %u\n", 3*i, xp_table[3*i], xp);
        //if (xp_table[3*(i+1)] > xp) { return i; }
        if (xp_table[0] > xp) { return i; }
    }

    return 20; // Current Max is 20.
}

extern int32_t sol_dnd2e_class_exp_to_next_level(entity_t *pc) {
    int next_exp = 999999999;
    for (int i = 0; i < 3; i++) {
        if (pc->class[i].level > -1) {
            const uint32_t *xp_table = get_xp_table(pc->class[i].class, pc->class[i].level);
            if (!xp_table) { return next_exp; }
            int next = xp_table[0];
            if (next < next_exp) {
                next_exp = next;
            }
        }
    }
    return next_exp;
}

extern int32_t sol_dnd2e_class_thac0(entity_t *pc) {
    int thac0 = 9999;
    for (int i = 0; i < 3; i++) {
        switch(pc->class[i].class) {
            case REAL_CLASS_FIGHTER:
            case REAL_CLASS_GLADIATOR:
            case REAL_CLASS_AIR_RANGER:
            case REAL_CLASS_EARTH_RANGER:
            case REAL_CLASS_FIRE_RANGER:
            case REAL_CLASS_WATER_RANGER:
                thac0 = thac0 > warrior_thac0[pc->class[i].level]
                    ? warrior_thac0[pc->class[i].level]
                    : thac0;
                break;
            case REAL_CLASS_AIR_CLERIC:
            case REAL_CLASS_EARTH_CLERIC:
            case REAL_CLASS_FIRE_CLERIC:
            case REAL_CLASS_WATER_CLERIC:
            case REAL_CLASS_AIR_DRUID:
            case REAL_CLASS_EARTH_DRUID:
            case REAL_CLASS_FIRE_DRUID:
            case REAL_CLASS_WATER_DRUID:
            case REAL_CLASS_TEMPLAR:
                thac0 = thac0 > priest_thac0[pc->class[i].level]
                    ? priest_thac0[pc->class[i].level]
                    : thac0;
                break;
            case REAL_CLASS_PSIONICIST:
            case REAL_CLASS_THIEF:
                thac0 = thac0 > rogue_thac0[pc->class[i].level]
                    ? rogue_thac0[pc->class[i].level]
                    : thac0;
                break;
            case REAL_CLASS_DEFILER:
            case REAL_CLASS_PRESERVER:
                thac0 = thac0 > wizard_thac0[pc->class[i].level]
                    ? wizard_thac0[pc->class[i].level]
                    : thac0;
                break;
            default: break;
        }
    }

    if (thac0 > 1000) { thac0 = pc->stats.base_thac0; } // not a pc.

    return thac0;
}

extern void sol_dnd2e_class_apply_stats(entity_t *pc, int class) {
    if (!pc || class < 0 || class > REAL_CLASS_MAX) { return; }

    if (pc->stats.str   < class_mininum[class][0]) { pc->stats.str   = class_mininum[class][0]; }
    if (pc->stats.dex   < class_mininum[class][1]) { pc->stats.dex   = class_mininum[class][1]; }
    if (pc->stats.con   < class_mininum[class][2]) { pc->stats.con   = class_mininum[class][2]; }
    if (pc->stats.intel < class_mininum[class][3]) { pc->stats.intel = class_mininum[class][3]; }
    if (pc->stats.wis   < class_mininum[class][4]) { pc->stats.wis   = class_mininum[class][4]; }
    if (pc->stats.cha   < class_mininum[class][5]) { pc->stats.cha   = class_mininum[class][5]; }
}

// return half attacks
extern int16_t sol_dnd2e_class_attack_num(const entity_t *pc, const item_t *item) {
    if (!item || item->type == ITEM_MELEE) { // MELEE
        for (int i = 0; i < 3; i++) {
            switch (pc->class[i].class) {
                case REAL_CLASS_FIGHTER:
                case REAL_CLASS_GLADIATOR:
                case REAL_CLASS_AIR_RANGER:
                case REAL_CLASS_WATER_RANGER:
                case REAL_CLASS_FIRE_RANGER:
                case REAL_CLASS_EARTH_RANGER:
                    if (pc->class[i].level < 7) { return 2; }
                    if (pc->class[i].level < 13) { return 3; }
                    return 4;
            }
        }
        return 2;
    } else if (item->type == ITEM_MISSILE_THROWN || item->type == ITEM_MISSILE_USE_AMMO) { // MISSILE
        for (int i = 0; i < 3; i++) {
            switch (pc->class[i].class) {
                case REAL_CLASS_FIGHTER:
                case REAL_CLASS_GLADIATOR:
                case REAL_CLASS_AIR_RANGER:
                case REAL_CLASS_WATER_RANGER:
                case REAL_CLASS_FIRE_RANGER:
                case REAL_CLASS_EARTH_RANGER:
                    if (pc->class[i].level < 7) { return 4; }
                    if (pc->class[i].level < 13) { return 6; }
                    return 8;
            }
        }
        return 2;
    }
    return 0;
}

extern int sol_dnd2e_class_max_hp(int class, int level, int con_mod) {
    if (class < 0 || class > REAL_CLASS_MAX) { return 0; }

    return level * (hit_die[class] + con_mod);
}

extern int sol_dnd2e_class_hp_die(int class) {
    return hit_die[class];
}
