#include "stats.h"

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

extern int8_t sol_dnd2e_melee_damage_mod(const stats_t *stats) {
    return str_mods[stats->str][STR_DAM];
}

extern int8_t sol_dnd2e_melee_hit_mod(const stats_t *stats) {
    return str_mods[stats->str][STR_HIT];
}

extern int8_t sol_dnd2e_range_damage_mod(const stats_t *stats) {
    (void)stats;
    return 0;
}

extern int8_t sol_dnd2e_range_hit_mod(const stats_t *stats) {
    return dex_mods[stats->dex][DEX_MISSILE];
}

extern int8_t sol_dnd2e_reaction_mod(const stats_t *stats) {
    return dex_mods[stats->dex][DEX_REACTION];
}

extern int8_t sol_dnd2e_ac_mod(const stats_t *stats) {
    return dex_mods[stats->dex][DEX_AC];
}

extern int8_t sol_dnd2e_hp_mod(const stats_t *stats) {
    return con_mods[stats->con][CON_HP];
}

extern int8_t sol_dnd2e_shock_mod(const stats_t *stats) {
    return con_mods[stats->con][CON_SHOCK];
}

extern int8_t sol_dnd2e_res_mod(const stats_t *stats) {
    return con_mods[stats->con][CON_RES];
}

extern int8_t sol_dnd2e_poison_mod(const stats_t *stats) {
    return con_mods[stats->con][CON_POISON_SAVE];
}

extern int8_t sol_dnd2e_regen_mod(const stats_t *stats) {
    return con_mods[stats->con][CON_REGEN];
}

extern int8_t sol_dnd2e_stats_valid(const stats_t *stats) {
    if (stats->str < 0 || stats->str > 25) { return 0; }
    if (stats->dex < 0 || stats->dex > 25) { return 0; }
    if (stats->con < 0 || stats->con > 25) { return 0; }
    if (stats->intel < 0 || stats->intel > 25) { return 0; }
    if (stats->wis < 0 || stats->wis > 25) { return 0; }
    if (stats->cha < 0 || stats->cha > 25) { return 0; }
    if (stats->hp < 1) { return 0; }
    if (stats->high_hp < 1) { return 0; }
    if (stats->high_psp < 0) { return 0; }
}
