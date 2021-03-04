#include "gff.h"
#include "rules.h"
#include <stdlib.h>
#include <stdio.h>

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

//    str dex con int wis chr
const static uint8_t race_mods[][6] = {
    {  0,  0,  0,  0,  0,  0  }, // MONSTER
    {  0,  0,  0,  0,  0,  0  }, // HUMAN
    {  1, -1,  2,  0,  0, -2  }, // DWARF
    {  0,  2, -2,  1, -1,  0  }, // ELF
    {  0,  1, -1,  0,  0,  0  }, // HALFELF
    {  4, -5,  2, -5, -3, -3  }, // HALFGIANT
    { -2,  2, -1,  0,  2, -1  }, // HALFLING
    {  2,  0,  1, -1,  0, -2  }, // MUL
    {  0,  2,  0, -1,  1, -2  }, // THRI-KREEN
};

static void dnd2e_apply_race_mods(ds_character_t *pc) {
    if (pc->race < RACE_HUMAN || pc->race > RACE_THRIKREEN) { return; }
    pc->stats.str += race_mods[pc->race][0];
    pc->stats.dex += race_mods[pc->race][1];
    pc->stats.con += race_mods[pc->race][2];
    pc->stats.intel += race_mods[pc->race][3];
    pc->stats.wis += race_mods[pc->race][4];
    pc->stats.cha += race_mods[pc->race][5];
}

enum {STR_HIT,   STR_DAM,   STR_WEIGHT,   STR_PRESS,   STR_OPEN,   STR_BEND};
const int16_t str_mods[][6] = {
    {   -9,        -9,         0,            0,           0,          0 }, // 00
    {   -5,        -4,         1,            3,           1,          0 }, // 01
    {   -3,        -2,         1,            5,           1,          0 }, // 02
    {   -3,        -1,         5,           10,           2,          0 }, // 03
    {   -2,        -1,        10,           25,           3,          0 }, // 04
    {   -2,        -1,        10,           25,           3,          0 }, // 05
    {   -1,         0,        20,           55,           4,          0 }, // 06
    {   -1,         0,        20,           55,           4,          0 }, // 07
    {    0,         0,        35,           90,           5,          1 }, // 08
    {    0,         0,        35,           90,           5,          1 }, // 09
    {    0,         0,        40,          115,           6,          2 }, // 10
    {    0,         0,        40,          115,           6,          2 }, // 11
    {    0,         0,        45,          140,           7,          4 }, // 12
    {    0,         0,        45,          140,           7,          4 }, // 13
    {    0,         0,        55,          170,           8,          7 }, // 14
    {    0,         0,        55,          170,           8,          7 }, // 15
    {    0,         1,        70,          195,           9,         10 }, // 16
    {    1,         1,        85,          220,          10,         13 }, // 17
    {    1,         2,       110,          255,          11,         16 }, // 18
    {    3,         7,       485,          640,          16,         50 }, // 19
    {    3,         8,       535,          700,          17,         60 }, // 20
    {    4,         9,       635,          810,          17,         70 }, // 21
    {    4,        10,       785,          970,          18,         80 }, // 22
    {    5,        11,       935,         1130,          18,         90 }, // 23
    {    6,        12,      1235,         1440,          19,         95 }, // 24
    {    7,        14,      1535,         1750,          19,         99 }, // 25
};

enum {DEX_REACTION,   DEX_MISSILE,   DEX_AC};
const int16_t dex_mods[][3] = {
    {   -9,             -9,             9 }, // 00
    {   -6,             -6,             5 }, // 01
    {   -4,             -4,             5 }, // 02
    {   -3,             -3,             4 }, // 03
    {   -2,             -2,             3 }, // 04
    {   -1,             -1,             2 }, // 05
    {    0,              0,             1 }, // 06
    {    0,              0,             0 }, // 07
    {    0,              0,             0 }, // 08
    {    0,              0,             0 }, // 09
    {    0,              0,             0 }, // 10
    {    0,              0,             0 }, // 11
    {    0,              0,             0 }, // 12
    {    0,              0,             0 }, // 13
    {    0,              0,             0 }, // 14
    {    0,              0,            -1 }, // 15
    {    1,              1,            -2 }, // 16
    {    2,              2,            -3 }, // 17
    {    2,              2,            -4 }, // 18
    {    3,              3,            -4 }, // 19
    {    3,              3,            -4 }, // 20
    {    4,              4,            -5 }, // 21
    {    4,              4,            -5 }, // 22
    {    4,              4,            -5 }, // 23
    {    5,              5,            -6 }, // 24
    {    5,              5,            -6 }, // 25
};

enum {CON_HP,   CON_SHOCK,   CON_RES,   CON_POISON_SAVE,   CON_REGEN};
const int16_t con_mods[][5] = {
    {   -9,        0,           0,        -9,                 0 }, // 00
    {   -3,       25,          30,        -2,                 0 }, // 01
    {   -2,       30,          35,        -1,                 0 }, // 02
    {   -2,       35,          40,         0,                 0 }, // 03
    {   -1,       40,          45,         0,                 0 }, // 04
    {   -1,       45,          50,         0,                 0 }, // 05
    {   -1,       50,          55,         0,                 0 }, // 06
    {    0,       55,          60,         0,                 0 }, // 07
    {    0,       60,          65,         0,                 0 }, // 08
    {    0,       65,          70,         0,                 0 }, // 09
    {    0,       70,          75,         0,                 0 }, // 10
    {    0,       75,          80,         0,                 0 }, // 11
    {    0,       80,          85,         0,                 0 }, // 12
    {    0,       85,          90,         0,                 0 }, // 13
    {    0,       88,          92,         0,                 0 }, // 14
    {    1,       90,          94,         0,                 0 }, // 15
    {    2,       95,          96,         0,                 0 }, // 16
    {    3,       97,          98,         0,                 0 }, // 17
    {    4,       99,         100,         0,                 0 }, // 18
    {    5,       99,         100,         1,                 0 }, // 19
    {    5,       99,         100,         1,                 6 }, // 20
    {    6,       99,         100,         2,                 5 }, // 21
    {    6,       99,         100,         2,                 4 }, // 22
    {    6,       99,         100,         3,                 3 }, // 23
    {    7,       99,         100,         3,                 2 }, // 24
    {    7,      100,         100,         4,                 1 }, // 25
};

enum {INT_LANG,   INT_SL,   INT_CHANCE,   INT_MAX_SL,   INT_ILLUSIONS};
const int16_t int_mods[][5] = {
    {    0,          0,        0,            0,            0 }, // 00
    {    0,          0,        0,            0,            0 }, // 01
    {    1,          0,        0,            0,            0 }, // 02
    {    1,          0,        0,            0,            0 }, // 03
    {    1,          0,        0,            0,            0 }, // 04
    {    1,          0,        0,            0,            0 }, // 05
    {    1,          0,        0,            0,            0 }, // 06
    {    1,          0,        0,            0,            0 }, // 07
    {    1,          0,        0,            0,            0 }, // 08
    {    2,          4,       35,            6,            0 }, // 09
    {    2,          5,       40,            7,            0 }, // 10
    {    2,          5,       45,            7,            0 }, // 11
    {    3,          6,       50,            7,            0 }, // 12
    {    3,          6,       55,            9,            0 }, // 13
    {    4,          7,       60,            9,            0 }, // 14
    {    4,          7,       65,           11,            0 }, // 15
    {    5,          8,       70,           11,            0 }, // 16
    {    6,          8,       75,           14,            0 }, // 17
    {    7,          9,       85,           18,            0 }, // 18
    {    8,          9,       95,         9999,            1 }, // 19
    {    9,          9,       96,         9999,            2 }, // 20
    {   10,          9,       97,         9999,            3 }, // 21
    {   11,          9,       98,         9999,            4 }, // 22
    {   12,          9,       99,         9999,            5 }, // 23
    {   15,          9,      100,         9999,            6 }, // 24
    {   20,          9,      100,         9999,            7 }, // 25
};

enum {WIS_MAG_DEF,   WIS_BONUS_SPELL,   WIS_SPELL_FAILURE};
const int16_t wis_mod[][3] = {
    {   -9,        0x0000,               100 }, // 00
    {   -6,        0x0000,                80 }, // 01
    {   -4,        0x0000,                60 }, // 02
    {   -3,        0x0000,                50 }, // 03
    {   -2,        0x0000,                45 }, // 04
    {   -1,        0x0000,                40 }, // 05
    {   -1,        0x0000,                35 }, // 06
    {   -1,        0x0000,                30 }, // 07
    {    0,        0x0000,                25 }, // 08
    {    0,        0x0000,                20 }, // 09
    {    0,        0x0000,                15 }, // 10
    {    0,        0x0000,                10 }, // 11
    {    0,        0x0000,                 5 }, // 12
    {    0,        0x0001,                 0 }, // 13
    {    0,        0x0001,                 0 }, // 14
    {    1,        0x0002,                 0 }, // 15
    {    2,        0x0002,                 0 }, // 16
    {    3,        0x0003,                 0 }, // 17
    {    4,        0x0004,                 0 }, // 18
    {    4,        0x0103,                 0 }, // 19 - 1st and 3rd bonuses
    {    4,        0x0204,                 0 }, // 20
    {    4,        0x0305,                 0 }, // 21
    {    4,        0x0405,                 0 }, // 22
    {    4,        0x0106,                 0 }, // 23
    {    4,        0x0506,                 0 }, // 24
    {    4,        0x0607,                 0 }, // 25
};

enum {CHA_HENCH,   CHA_LOYALTY,   CHA_REACTION};
const int16_t cha_mods[][3] = {
    {    0,          -9,            -9 }, // 00
    {    0,          -8,            -7 }, // 01
    {    1,          -7,            -6 }, // 02
    {    1,          -6,            -5 }, // 03
    {    1,          -5,            -4 }, // 04
    {    2,          -4,            -3 }, // 05
    {    2,          -3,            -2 }, // 06
    {    3,          -2,            -1 }, // 07
    {    3,          -1,             0 }, // 08
    {    4,           0,             0 }, // 09
    {    4,           0,             0 }, // 10
    {    4,           0,             0 }, // 11
    {    5,           0,             0 }, // 12
    {    5,           0,             1 }, // 13
    {    6,           1,             2 }, // 14
    {    7,           3,             3 }, // 15
    {    8,           4,             5 }, // 16
    {   10,           6,             6 }, // 17
    {   15,           8,             7 }, // 18
    {   20,          10,             8 }, // 19
    {   25,          12,             9 }, // 20
    {   30,          14,            10 }, // 21
    {   35,          16,            11 }, // 22
    {   40,          18,            12 }, // 23
    {   45,          20,            13 }, // 24
    {   50,          20,            14 }, // 25
};

//        exp    hdlimit      hapr
const uint32_t fighter_levels[][3] = {
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
const uint32_t ranger_levels[][3] = {
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
const uint32_t preserver_levels[][3] = {
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
const uint32_t defiler_levels[][3] = {
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

// TODO: REMEMBER TO UPDATE get_hit_die!!!!

static const int8_t hit_die[] = {
     0,  8, 8, 8,  8,  8,  8,  8, 8,
    10, 10, 4, 6, 10, 10, 10, 10, 6
};

// FIXME - Templars can only be "one step away" from their
//         patron in either axis, but not both
static const uint32_t class_alignments[] = {
//  REAL_CLASS_NONE
    ANY_ALIGNMENT,
//  Air Cleric            Earth Cleric          Fire Cleric           Water Cleric
    ANY_ALIGNMENT,        ANY_ALIGNMENT,        ANY_ALIGNMENT,        ANY_ALIGNMENT,    
//  Air Druid             Earth Druid           Fire Druid            Water Druid
    ONLY_TRUE_NEUTRAL,    ONLY_TRUE_NEUTRAL,    ONLY_TRUE_NEUTRAL,    ONLY_TRUE_NEUTRAL,
//  Fighter
    ANY_ALIGNMENT,
//  Gladiator
    ANY_ALIGNMENT,
//  Preserver
    NOT_EVIL_MORALS,
//  Psionicist
    ANY_ALIGNMENT,
//  Air Ranger            Earth Ranger          Fire Ranger           Water Ranger
    ONLY_GOOD_MORALS,     ONLY_GOOD_MORALS,     ONLY_GOOD_MORALS,     ONLY_GOOD_MORALS,
//  Thief
    NOT_LAWFUL_GOOD,
//  Defiler
    NOT_GOOD_MORALS,
//  Templar
    NOT_GOOD_MORALS,
};

static const uint32_t alignment_flags[] = {
    // Lawful Good
    ONLY_LAWFUL_ETHICS   | NOT_NEUTRAL_ETHICS      | NOT_CHAOTIC_ETHICS    |
    ONLY_GOOD_MORALS     | NOT_NEUTRAL_MORALS      | NOT_EVIL_MORALS       |
    ONLY_LAWFUL_GOOD     | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // Lawful Neutral
    ONLY_LAWFUL_ETHICS   | NOT_NEUTRAL_ETHICS      | NOT_CHAOTIC_ETHICS    |
    NOT_GOOD_MORALS      | ONLY_NEUTRAL_MORALS     | NOT_EVIL_MORALS       |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    ONLY_LAWFUL_NEUTRAL  | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // Lawful Evil
    ONLY_LAWFUL_ETHICS   | NOT_NEUTRAL_ETHICS      | NOT_CHAOTIC_ETHICS    |
    NOT_GOOD_MORALS      | NOT_NEUTRAL_MORALS      | ONLY_EVIL_MORALS      |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    ONLY_LAWFUL_EVIL     | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // Neutral Good
    NOT_LAWFUL_ETHICS    | ONLY_NEUTRAL_ETHICS     | NOT_CHAOTIC_ETHICS    |
    ONLY_GOOD_MORALS     | NOT_NEUTRAL_MORALS      | NOT_EVIL_MORALS       |
    NOT_LAWFUL_GOOD      | ONLY_NEUTRAL_GOOD       | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // True Neutral
    NOT_LAWFUL_ETHICS    | ONLY_NEUTRAL_ETHICS     | NOT_CHAOTIC_ETHICS    |
    NOT_GOOD_MORALS      | ONLY_NEUTRAL_MORALS     | NOT_EVIL_MORALS       |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | ONLY_TRUE_NEUTRAL       | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // Neutral Evil
    NOT_LAWFUL_ETHICS    | ONLY_NEUTRAL_ETHICS     | NOT_CHAOTIC_ETHICS    |
    NOT_GOOD_MORALS      | NOT_NEUTRAL_MORALS      | ONLY_EVIL_MORALS      |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | ONLY_NEUTRAL_EVIL       | NOT_CHAOTIC_EVIL,

    // Chaotic Good
    NOT_LAWFUL_ETHICS    | NOT_NEUTRAL_ETHICS      | ONLY_CHAOTIC_ETHICS   |
    ONLY_GOOD_MORALS     | NOT_NEUTRAL_MORALS      | NOT_EVIL_MORALS       |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | ONLY_CHAOTIC_GOOD     |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // Chaotic Neutral
    NOT_LAWFUL_ETHICS    | NOT_NEUTRAL_ETHICS      | ONLY_CHAOTIC_ETHICS   |
    NOT_GOOD_MORALS      | ONLY_NEUTRAL_MORALS     | NOT_EVIL_MORALS       |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | ONLY_CHAOTIC_NEUTRAL  |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // Chaotic Evil
    NOT_LAWFUL_ETHICS    | NOT_NEUTRAL_ETHICS      | ONLY_CHAOTIC_ETHICS   |
    NOT_GOOD_MORALS      | NOT_NEUTRAL_MORALS      | ONLY_EVIL_MORALS      |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | ONLY_CHAOTIC_EVIL,
};

static int has_class(ds_character_t *pc, const int16_t class){
    return pc->real_class[0] == class
        || pc->real_class[1] == class
        || pc->real_class[2] == class;
}


static const uint32_t* get_xp_table(const uint8_t class) {
    switch (class) {
        case REAL_CLASS_AIR_CLERIC:
        case REAL_CLASS_EARTH_CLERIC:
        case REAL_CLASS_FIRE_CLERIC:
        case REAL_CLASS_WATER_CLERIC:
            return (const uint32_t *)ranger_levels;
        case REAL_CLASS_AIR_DRUID:
        case REAL_CLASS_EARTH_DRUID:
        case REAL_CLASS_FIRE_DRUID:
        case REAL_CLASS_WATER_DRUID:
            return (const uint32_t *)ranger_levels;
        case REAL_CLASS_FIGHTER:
            return (const uint32_t *)fighter_levels;
        case REAL_CLASS_GLADIATOR:
            return (const uint32_t *)ranger_levels;
        case REAL_CLASS_PRESERVER:
            return (const uint32_t *)preserver_levels;
        case REAL_CLASS_PSIONICIST:
            return (const uint32_t *)fighter_levels;
        case REAL_CLASS_AIR_RANGER:
        case REAL_CLASS_EARTH_RANGER:
        case REAL_CLASS_FIRE_RANGER:
        case REAL_CLASS_WATER_RANGER:
            return (const uint32_t *)ranger_levels;
        case REAL_CLASS_THIEF:
            return (const uint32_t *)fighter_levels;
    }
    return NULL;
}

static int get_class_level(const uint8_t class, const uint32_t xp) {
    const uint32_t *xp_table = get_xp_table(class);
    if (!xp_table) { return 0; }

    for (int i = 0; i < 19; i++) {
        //printf("xp_table[%d] = %d, xp = %u\n", 3*i, xp_table[3*i], xp);
        if (xp_table[3*(i+1)] > xp) { return i; }
    }

    return 20; // Current Max is 20.
}

static int get_total_hit_die(const uint8_t class, const uint8_t level) {
    const uint32_t *xp_table = get_xp_table(class);
    if (!xp_table) { return 0; }
    if (level < 0 || level > 20) { return 0; }
    return xp_table[3 * (level + 1)];
}

int dnd2e_is_class_allowed(const uint8_t race, const int8_t classes[3]) {
    uint16_t class = convert_to_class_sel(classes[0]);
    const uint16_t *allowed = NULL;
    if (classes[0] == -1) { return 0; }

    //printf("race = %d (%d, %d, %d)\n", race, classes[0], classes[1], classes[2]);
    if (classes[1] != -1) {
        // NO DUPS.
        if (classes[0] == classes[1] || classes[1] == classes[2]
            || classes[0] == classes[2]) { return 0; }
        class |= convert_to_class_sel(classes[1]);
        if (classes[2] != -1) {
            class |= convert_to_class_sel(classes[2]);
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

int dnd2e_is_alignment_allowed(const uint8_t alignment, const int8_t classes[3], int target_is_pc) {
    int allowed = 1;
    int game_type = gff_get_game_type();

    for (int i = 0; i < 3; i++)
    {
        if (classes[i] == -1) {
            break;
        }

        // Alignment has to match alignment flags for the class
        // NPCs can be any alignment - PCs can only be non-evil UNLESS the game is DSO
        allowed = (  class_alignments[classes[i]] == ANY_ALIGNMENT ||
                     class_alignments[classes[i]] & alignment_flags[alignment] ) &&
                   ( !target_is_pc || ( game_type != DARKSUN_ONLINE && alignment_flags[alignment] & NOT_EVIL_MORALS ) );

        if (!allowed) {
            return 0;
        }
    }

    return 1;
}

static void set_psp(ds_character_t *pc) {
    pc->base_psp = 0;
    if (pc->stats.con > 15) {
        pc->base_psp += pc->stats.con - 15;
    }
    if (pc->stats.intel > 15) {
        pc->base_psp += pc->stats.intel - 15;
    }
    if (pc->stats.wis > 15) {
        pc->base_psp += 20 + 2 * (pc->stats.intel - 15);
    }
    if (has_class(pc, REAL_CLASS_PSIONICIST)) {
        int psi_level = 0;
        if (pc->real_class[0] == REAL_CLASS_PSIONICIST) { psi_level = pc->level[0]; }
        if (pc->real_class[1] == REAL_CLASS_PSIONICIST) { psi_level = pc->level[1]; }
        if (pc->real_class[2] == REAL_CLASS_PSIONICIST) { psi_level = pc->level[2]; }
        pc->base_psp += 10 * (psi_level);
        if (pc->stats.wis > 15) {
            pc->base_psp += (20 + 2 * (pc->stats.intel - 15)) * (psi_level - 1);
        }
    }
}

static void do_level_up(ds_character_t *pc, const uint32_t class_idx, const uint32_t class) {
    int clevel = pc->level[class];
    int num_classes = 0;

    for (int i = 0; i < 3 && pc->real_class[i] > -1; i++) {
        num_classes++;
    }

    // Roll the next HP:
    int current_hit_die = get_total_hit_die(class, clevel);
    int next_hit_die = get_total_hit_die(class, clevel + 1);
    //printf("chd = %d, nhd = %d\n", current_hit_die, next_hit_die);
    int hp = 0;
    if (next_hit_die > current_hit_die) {
        //printf("Need to increase HP by up to %d\n", hit_die[class_idx]);
        hp = 1 + (rand() % hit_die[class]);
        hp += con_mods[pc->stats.con][CON_HP];
        hp = (hp / num_classes) + (((hp % num_classes) > 0) ? 1 : 0);
        if (hp < 1) { hp = 1; }
    } else {
        hp = 2;
    }
    //printf("Increasing HP by %d\n", hp);
    pc->base_hp += hp;
    pc->high_hp += hp;
    pc->level[class_idx]++;
    set_psp(pc);
}

void dnd2e_set_exp(ds_character_t *pc, const uint32_t amt) {
    pc->base_hp = pc->high_hp = 0;
    for (int i = 0; i < 3; i++) {
        if (pc->real_class[i] > -1) {
            pc->level[i] = 0;
            do_level_up(pc, i, pc->real_class[i]);
        }
    }
    pc->current_xp = 0;
    dnd2e_award_exp(pc, amt);
}

void dnd2e_award_exp(ds_character_t *pc, const uint32_t amt) {
    int num_classes = 0;
    pc->current_xp += amt;
    for (int i = 0; i < 3 && pc->level[i]; i++) {
        //printf("pc->leve[%d] = %d\n",i, pc->level[i]);
        num_classes++;
    }
    for (int i = 0; i < 3 && pc->level[i]; i++) {
        int next_level = get_class_level(pc->real_class[i], pc->current_xp / num_classes);
        while (next_level > pc->level[i]) {
            do_level_up(pc, i, pc->real_class[i]);
        }
    }
}

int32_t dnd2e_exp_to_next_level_up(ds_character_t *pc) {
    int next_exp = 999999999;
    for (int i = 0; i < 3; i++) {
        if (pc->real_class[i] > -1) {
            const uint32_t *xp_table = get_xp_table(pc->real_class[i]);
            if (!xp_table) { return next_exp; }
            int next = xp_table[3 * (pc->level[i] + 1)];
            if (next < next_exp) {
                next_exp = next;
            }
        }
    }
    return next_exp;
}

int16_t dnd2e_get_ac_pc(ds_character_t *pc, ds_inventory_t *inv) {
    int ac_bonus = 0;
    if (inv) {
        ds1_item_t *item = (ds1_item_t*)(inv);
        for (int i = 0; i <= SLOT_FOOT; i++) {
            if (item[i].id) {
                ac_bonus += -ds_get_item1r(item[i].item_index)->base_AC;
                //printf("slot: %d, id: %d it1r: %d, ac: %d\n", i, item[i].id, item[i].item_index, ds_get_item1r(item[i].item_index)->base_AC);
            }
        }
    }
    return pc->base_ac + dex_mods[pc->stats.dex][2] + ac_bonus;
}

// return in half attacks.
static int16_t class_attack_num(const entity_t *pc, const ds1_item_t *item) {
    const ds_item1r_t *it1r = ds_get_item1r(item->item_index);
    if (it1r->weapon_type == 1) { // MELEE
        for (int i = 0; i < 3; i++) {
            switch (pc->class[i].class) {
                case REAL_CLASS_FIGHTER:
                case REAL_CLASS_GLADIATOR:
                case REAL_CLASS_AIR_RANGER:
                case REAL_CLASS_WATER_RANGER:
                case REAL_CLASS_FIRE_RANGER:
                case REAL_CLASS_EARTH_RANGER:
                    if (pc->class[i].level < 7) { return 1; }
                    if (pc->class[i].level < 13) { return 2; }
                    return 3;
            }
        }
    } else if (it1r->weapon_type == 2) { // MISSILE
        for (int i = 0; i < 3; i++) {
            switch (pc->class[i].class) {
                case REAL_CLASS_FIGHTER:
                case REAL_CLASS_GLADIATOR:
                case REAL_CLASS_AIR_RANGER:
                case REAL_CLASS_WATER_RANGER:
                case REAL_CLASS_FIRE_RANGER:
                case REAL_CLASS_EARTH_RANGER:
                    if (pc->class[i].level < 7) { return 2; }
                    if (pc->class[i].level < 13) { return 4; }
                    return 6;
            }
        }
    }
    return 0;
}

int16_t dnd2e_get_attack_num_pc(const entity_t *pc, const ds1_item_t *item) {
    if (item == NULL) { return 0; }
    const ds_item1r_t *it1r = ds_get_item1r(item->item_index);
    // For some reason double attacks are stored for missiles...
    uint16_t base_attacks = (it1r->weapon_type == 2) ? it1r->num_attacks : it1r->num_attacks * 2;
    return base_attacks + class_attack_num(pc, item);
}

int16_t dnd2e_get_attack_sides_pc(const entity_t *pc, const ds1_item_t *item) {
    if (item == NULL) { return 0; }
    return ds_get_item1r(item->item_index)->sides;
}

int16_t dnd2e_get_attack_die_pc(const entity_t *pc, const ds1_item_t *item) {
    if (item == NULL) { return 0; }
    return ds_get_item1r(item->item_index)->dice;
}

enum {
    MATERIAL_WOOD,
    MATERIAL_BONE =  (1<<0),
    MATERIAL_STONE = (1<<1),
    MATERIAL_OBSIDIAN = (1<<2),
    MATERIAL_METAL = (1<<3),
    MATERIAL_LEATHER = (1<<3),
};

int16_t dnd2e_get_attack_mod_pc(const entity_t *pc, const ds1_item_t *item) {
    if (item == NULL) { return 0; }
    uint16_t material_mod = 0;
    const ds_item1r_t *it1r = ds_get_item1r(item->item_index);

    switch(it1r->material) {
        case MATERIAL_WOOD: material_mod = -2; break;
        case MATERIAL_STONE:
        case MATERIAL_BONE:
        case MATERIAL_OBSIDIAN:
            material_mod = -1; break;
    }

    return it1r->mod + material_mod + 
            str_mods[pc->stats.str][STR_DAM];
}

void dnd2e_randomize_stats_pc(ds_character_t *pc) {
    pc->stats.str = 10 + (rand() % 11);
    pc->stats.dex = 10 + (rand() % 11);
    pc->stats.con = 10 + (rand() % 11);
    pc->stats.intel = 10 + (rand() % 11);
    pc->stats.wis = 10 + (rand() % 11);
    pc->stats.cha = 10 + (rand() % 11);
    pc->base_ac = 10;
    dnd2e_apply_race_mods(pc);
    dnd2e_fix_stats_pc(pc); // Get our base stats correct first.

    dnd2e_set_exp(pc, pc->current_xp); // Also sets HP & PSP
}

void dnd2e_fix_stats_pc(ds_character_t *pc) {
    if (pc->stats.str < 5) { pc->stats.str = 9; }
    if (pc->stats.dex < 5) { pc->stats.dex = 9; }
    if (pc->stats.con < 5) { pc->stats.con = 9; }
    if (pc->stats.intel < 5) { pc->stats.intel = 9; }
    if (pc->stats.wis < 5) { pc->stats.wis = 9; }
    if (pc->stats.cha < 5) { pc->stats.cha = 9; }
}

int dnd2e_character_is_valid(ds_character_t *pc) {
    if (pc->stats.str < 0 || pc->stats.str > 25) { return 0; }
    if (pc->stats.dex < 0 || pc->stats.dex > 25) { return 0; }
    if (pc->stats.con < 0 || pc->stats.con > 25) { return 0; }
    if (pc->stats.intel < 0 || pc->stats.intel > 25) { return 0; }
    if (pc->stats.wis < 0 || pc->stats.wis > 25) { return 0; }
    if (pc->stats.cha < 0 || pc->stats.cha > 25) { return 0; }
    if (pc->base_hp < 1) { return 0; }
    if (pc->high_hp < 1) { return 0; }
    if (pc->current_xp < 1) { return 0; }
    if (pc->base_psp < 0) { return 0; }
    if (!dnd2e_is_class_allowed(pc->race, pc->real_class)) { return 0; }
    if (pc->gender != GENDER_MALE && pc->gender != GENDER_FEMALE) { return 0; }
    if (pc->alignment < LAWFUL_GOOD || pc->alignment > CHAOTIC_EVIL) { return 0; }
    if (pc->level[0] < 1) { return 0; }
    if (pc->real_class[1] > -1 && pc->level[1] < 1) { return 0; }
    if (pc->real_class[2] > -1 &&pc->level[2] < 1) { return 0; }
    if (pc->magic_resistance < 0 || pc->magic_resistance > 100) { return 0; }
    if (pc->allegiance != 1) { return 0; }
    if (pc->size < 0) { return 0; }
    // Not checked:
    // pc->id
    // pc->data1
    // pc->legalClass
    // pc->base_ac
    // pc->base_move
    // pc->num_blows
    // pc->num_attacks
    // pc->num_dice
    // pc->num_sides
    // pc->num_bonuses
    // pc->saving_throw
    // pc->spell_group
    // pc->high_level
    // pc->soundfx
    // pc->attack_sound
    // pc->psi_group
    // pc->pallette
    return 1;// passed the checks.
}

int dnd2e_psin_is_valid(ds_character_t *pc, psin_t *psi) {
    int num_psionics = 0;
    int is_psionicist = 0;

    for (int i = 0; i < 7; i++) {
        if (psi->types[i]) { num_psionics++; }
    }
    for (int i = 0; i < 3; i++) {
        if (pc->real_class[i] == REAL_CLASS_PSIONICIST) { is_psionicist = 1; }
    }

    if (is_psionicist) { return num_psionics == 3; }

    return num_psionics == 1;
}

int16_t dnd2e_get_move_pc(ds_character_t *pc) {
    return 12;
}

int16_t dnd2e_get_thac0_pc(ds_character_t *pc) {
    return 20; //TODO: update
}

// Missle modification is in original DS engine, so is d10. Although DS was highest goes first, we are lowest.
// TODO: Add mods for race, spell efects, etc...
int dnd2e_roll_initiative(entity_t *entity) {
    return (rand() % 10) - dex_mods[entity->stats.dex][DEX_MISSILE];
}
int dnd2e_roll_sub_roll() {
    return rand();
}
