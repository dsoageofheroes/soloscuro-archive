#include "rules.h"
#include <stdlib.h>
#include <stdio.h>

enum {
    A_END = 0x0, // End of Allowed Classes
    A_C = 0x1, // Allowed Cleric
    A_D = 0x2, // Allowed Druid
    A_F = 0x4, // Allowed Fighter
    A_G = 0x8, // Allowed Gladiator
    A_P = 0x10, // Allowed Preserver
    A_S = 0x20, // Allowed pSionicist
    A_R = 0x40, // Allowed Ranger
    A_T = 0x80, // Allowed Thief
};

static int convert_to_class_sel(const uint8_t class) {
    switch (class) {
        case REAL_CLASS_AIR_CLERIC:
        case REAL_CLASS_EARTH_CLERIC:
        case REAL_CLASS_FIRE_CLERIC:
        case REAL_CLASS_WATER_CLERIC:
            return A_C;
        case REAL_CLASS_AIR_DRUID:
        case REAL_CLASS_EARTH_DRUID:
        case REAL_CLASS_FIRE_DRUID:
        case REAL_CLASS_WATER_DRUID:
            return A_D;
        case REAL_CLASS_FIGHTER: return A_F;
        case REAL_CLASS_GLADIATOR: return A_G;
        case REAL_CLASS_PRESERVER: return A_P;
        case REAL_CLASS_PSIONICIST: return A_S;
        case REAL_CLASS_AIR_RANGER: return A_R;
        case REAL_CLASS_THIEF: return A_T;
    }

    return 0; // UNKNOWN CLASS
}

static const uint16_t human_classes[] = {
    A_C, A_D, A_F, A_G, A_P, A_S, A_R, A_T, A_END
};
static const uint16_t dwarf_classes[] = {
    A_C, A_F, A_G, A_S, A_T, A_C | A_F, A_C | A_S, A_F | A_S, A_F | A_T, A_S | A_T, 0x0
};
static const uint16_t elf_classes[] = {
    A_C, A_F, A_G, A_P, A_S, A_R, A_T, 
    A_C | A_F, A_C | A_P, A_C | A_S, A_C | A_R, A_S | A_T, A_C | A_T,
    A_C | A_F | A_P, A_C | A_F | A_S, A_C | A_F | A_T,
    A_C | A_P | A_S, A_C | A_P | A_R, A_C | A_P | A_T,
    A_C | A_S | A_R, A_C | A_S | A_T,
    A_C | A_R | A_T,
    A_F | A_P, A_F | A_S, A_F | A_T,
    A_F | A_P | A_S, A_F | A_P | A_T,
    A_F | A_S | A_T,
    A_P | A_S, A_P | A_R, A_P | A_T,
    A_P | A_S | A_R, A_P | A_S | A_T,
    A_S | A_R, A_S | A_T,
    A_S | A_R | A_T,
    A_R | A_T,
    A_END
};
static const uint16_t half_elf_classes[] = {
    A_C, A_D, A_F, A_G, A_P, A_S, A_R, A_T, 
    A_C | A_F, A_C | A_P, A_C | A_S, A_C | A_R, A_S | A_T, A_C | A_T,
    A_C | A_F | A_P, A_C | A_F | A_S, A_C | A_F | A_T,
    A_C | A_P | A_S, A_C | A_P | A_R, A_C | A_P | A_T,
    A_C | A_S | A_R, A_C | A_S | A_T,
    A_C | A_R | A_T,
    A_D | A_F, A_D | A_P, A_D | A_S, A_D | A_T,
    A_D | A_F | A_P, A_D | A_F | A_S, A_D | A_F | A_T,
    A_D | A_P | A_S, A_D | A_P | A_T,
    A_D | A_S | A_T,
    A_F | A_P, A_F | A_S, A_F | A_T,
    A_F | A_P | A_S, A_F | A_P | A_T,
    A_F | A_S | A_T,
    A_P | A_S, A_P | A_R, A_P | A_T, A_P | A_R,
    A_P | A_S | A_R, A_P | A_S | A_T, A_P | A_S | A_R, A_P | A_R | A_T,
    A_S | A_R, A_S | A_T, A_S | A_R,
    A_S | A_R | A_T,
    A_R | A_T,
    A_END
};
static const uint16_t half_giant_classes[] = {
    A_C, A_F, A_G, A_S, A_R,
    A_C | A_F, A_C | A_S, A_C | A_R,
    A_F | A_S,
    A_S | A_R,
    A_END
};

static const uint16_t halfling_classes[] = {
    A_C, A_D, A_F, A_G, A_S, A_R, A_T, 
    A_C | A_F, A_C | A_S, A_C | A_R, A_C | A_T,
    A_D | A_F, A_D | A_S, A_D | A_R, A_D | A_T,
    A_F | A_S, A_F | A_T,
    A_F | A_S | A_T,
    A_S | A_R, A_S | A_T,
    A_S | A_R | A_T,
    A_R | A_T,
    A_END
};
static const uint16_t mul_classes[] = {
    A_C, A_D, A_F, A_G, A_S, A_R, A_T, 
    A_C | A_F, A_C | A_S, A_C | A_T,
    A_C | A_F | A_T,
    A_D | A_F, A_D | A_S, A_D | A_T,
    A_D | A_F | A_T,
    A_F | A_S, A_F | A_T,
    A_F | A_S | A_T,
    A_S | A_T,
    A_END
};
static const uint16_t thrikeen_classes[] = {
    A_C, A_D, A_F, A_G, A_S, A_R,
    A_C | A_F, A_C | A_S, A_C | A_R,
    A_C | A_F | A_S,
    A_C | A_S | A_R,
    A_D | A_F, A_D | A_S,
    A_D | A_F | A_S,
    A_F | A_S,
    A_S | A_R,
    A_END
};

//TODO: These are not correct...
const static uint8_t race_mods[][6] = {
    { 0, 0, 0, 0, 0, 0}, // MONSTER
    { 0, 0, 0, 0, 0, 0}, // HUMAN
    { 0, -1, 1, 0, 0, -2}, // DWARF
    { 0, 1, -1, 0, 0, 0}, // ELF
    { 0, 1, -1, 0, 0, 0}, // HALFELF
    { 4, -5, 2, -5, -3, -3}, // HALFGIANT
    { 0, 1, 0, 0, -1, 0}, // HALFLING
    { 2, 0, 1, -1, 0, -2}, // MUL
    { 0, 2, 0, -1, 1, -2}, // TRIKEEN
};

static void dnd2e_apply_race_mods(ds_character_t *pc) {
    if (pc->race < RACE_HUMAN || pc->race > RACE_THRIKEEN) { return; }
    pc->stats.str += race_mods[pc->race][0];
    pc->stats.dex += race_mods[pc->race][1];
    pc->stats.con += race_mods[pc->race][2];
    pc->stats.intel += race_mods[pc->race][3];
    pc->stats.wis += race_mods[pc->race][4];
    pc->stats.cha += race_mods[pc->race][5];
}

enum {STR_HIT, STR_DAM, STR_WEIGHT, STR_PRESS, STR_OPEN, STR_BEND};

const int16_t str_mods[][6] = {
    {-9, -9, 0, 0, 0, 0},
    {-5, -4, 1, 3, 1, 0},
    {-3, -2, 1, 5, 1, 0},
    {-3, -1, 5, 10, 2, 0},
    {-2, -1, 10, 25, 3, 0},
    {-2, -1, 10, 25, 3, 0},
    {-1, 0, 20, 55, 4, 0},
    {-1, 0, 20, 55, 4, 0},
    {0, 0, 35, 90, 5, 1},
    {0, 0, 35, 90, 5, 1},
    {0, 0, 40, 115, 6, 2},
    {0, 0, 40, 115, 6, 2},
    {0, 0, 45, 140, 7, 4}, // 12
    {0, 0, 45, 140, 7, 4}, // 13
    {0, 0, 55, 170, 8, 7}, // 14
    {0, 0, 55, 170, 8, 7}, // 15
    {0, 1, 70, 195, 9, 10}, // 16
    {1, 1, 85, 220, 10, 13}, // 17
    {1, 2, 110, 255, 11, 16}, //18
    {3, 7, 485, 640, 16, 50}, // 19
    {3, 8, 535, 700, 17, 60}, // 20
    {4, 9, 635, 810, 17, 70}, // 21
    {4, 10, 785, 970, 18, 80}, // 22
    {5, 11, 935, 1130, 18, 90}, // 23
    {6, 12, 1235, 1440, 19, 95}, // 24
    {7, 14, 1535, 1750, 19, 99}, // 25
};

enum {DEX_REACTION, DEX_MISSILE, DEX_AC};

const int16_t dex_mods[][3] = {
    {-9, -9, 9},
    {-6, -6, 5},
    {-3, -3, 4},
    {-2, -2, 3},
    {-1, -1, 2},
    {0, 0, 1}, // 6
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, -1}, // 15
    {1, 1, -2},
    {2, 2, -3},
    {2, 2, -4},
    {3, 3, -4},
    {3, 3, -4},
    {4, 4, -5},
    {4, 4, -5},
    {4, 4, -5},
    {5, 5, -6},
    {5, 5, -6},
};

enum { CON_HP, CON_SHOCK, CON_RES, CON_POISON_SAVE, CON_REGEN};

const int16_t con_mods[][5] = {
    {-3, 25, 30, -2, 0},
    {-2, 30, 35, -1, 0},
    {-2, 35, 40, 0, 0},
    {-1, 40, 45, 0, 0},
    {-1, 45, 50, 0, 0},
    {-1, 50, 55, 0, 0},
    {0, 55, 60, 0, 0},
    {0, 60, 65, 0, 0},
    {0, 65, 70, 0, 0},
    {0, 70, 75, 0, 0},
    {0, 75, 80, 0, 0},
    {0, 80, 85, 0, 0},
    {0, 85, 90, 0, 0},
    {0, 88, 92, 0, 0},
    {1, 90, 94, 0, 0},
    {2, 95, 96, 0, 0},
    {3, 97, 98, 0, 0},
    {4, 99, 100, 0, 0},
    {5, 99, 100, 1, 0},
    {5, 99, 100, 1, 6},
    {6, 99, 100, 2, 5},
    {6, 99, 100, 2, 4},
    {6, 99, 100, 3, 3},
    {7, 99, 100, 3, 2},
    {7, 100, 100, 4, 1},
};

enum {INT_LANG, INT_SL, INT_CHANCE, INT_MAX_SL, INT_ILLUSIONS};

const int16_t int_mods[][5] = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0},
    {1, 0, 0, 0, 0},
    {1, 0, 0, 0, 0},
    {1, 0, 0, 0, 0},
    {1, 0, 0, 0, 0},
    {1, 0, 0, 0, 0},
    {1, 0, 0, 0, 0},
    {2, 4, 35, 6, 0},
    {2, 5, 40, 7, 0},
    {2, 5, 45, 7, 0},
    {3, 6, 50, 7, 0},
    {3, 6, 55, 9, 0},
    {4, 7, 60, 9, 0},
    {4, 7, 65, 11, 0},
    {5, 8, 70, 11, 0},
    {6, 8, 75, 14, 0},
    {7, 9, 85, 18, 0}, // 18
    {8, 9, 95, 9999, 1},
    {9, 9, 96, 9999, 2},
    {10, 9, 97, 9999, 3},
    {11, 9, 98, 9999, 4},
    {12, 9, 99, 9999, 5},
    {15, 9, 100, 9999, 6},
    {20, 9, 100, 9999, 7},
};

enum {WIS_MAG_DEF, WIS_BONUS_SPELL, WIS_SPELL_FAILURE};

const int16_t wis_mod[][3] = {
    {-9, 0x0000, 100},
    {-6, 0x0000, 80},
    {-4, 0x0000, 60},
    {-3, 0x0000, 50},
    {-2, 0x0000, 45},
    {-1, 0x0000, 40},
    {-1, 0x0000, 35},
    {-1, 0x0000, 30},
    {0, 0x0000, 25},
    {0, 0x0000, 20},
    {0, 0x0000, 15},
    {0, 0x0000, 10},
    {0, 0x0000, 5},
    {0, 0x0001, 0},
    {0, 0x0001, 0},
    {1, 0x0002, 0},
    {2, 0x0002, 0},
    {3, 0x0003, 0},
    {4, 0x0004, 0},
    {4, 0x0103, 0}, // 1st and 3rd bonuses
    {4, 0x0204, 0},
    {4, 0x0305, 0},
    {4, 0x0405, 0},
    {4, 0x0106, 0},
    {4, 0x0506, 0},
    {4, 0x0607, 0},
};

enum {CHA_HENCH, CHA_LOYALTY, CHA_REACTION};

const int16_t cha_mods[][3] = {
    {0, -9, -9},
    {0, -8, -7},
    {1, -7, -6},
    {1, -6, -5},
    {1, -5, -4},
    {2, -4, -3},
    {2, -3, -2},
    {3, -2, -1},
    {3, -1, 0},
    {4, 0, 0},
    {4, 0, 0},
    {4, 0, 0},
    {5, 0, 0},
    {5, 0, 1},
    {6, 1, 2},
    {7, 3, 3},
    {8, 4, 5},
    {10, 6, 6},
    {15, 8, 7},
    {20, 10, 8},
    {25, 12, 9},
    {30, 14, 10},
    {35, 16, 11},
    {40, 18, 12},
    {45, 20, 13},
    {50, 20, 14},
};

const uint32_t fighter_levels[][3] = {
    {0, 0, 0},
    {0, 1, 2}, // the third parameter is how many half-attacks per round.
    {2000, 2, 2},
    {4000, 3, 2},
    {8000, 4, 2},
    {16000, 5, 2},
    {32000, 6, 2},
    {64000, 7, 3},
    {125000, 8, 3},
    {250000, 9, 3},
    {500000, 9, 3},
    {750000, 9, 3},
    {1000000, 9, 3},
    {1250000, 9, 4},
    {1500000, 9, 4},
    {1750000, 9, 4},
    {2000000, 9, 4},
    {2250000, 9, 4},
    {2500000, 9, 4},
    {2750000, 9, 4},
    {3000000, 9, 4},
};

const uint32_t ranger_levels[][3] = {
    {0, 0, 0},
    {0, 1, 2}, // the third parameter is how many half-attacks per round.
    {2250, 2, 2},
    {4500, 3, 2},
    {9000, 4, 2},
    {18000, 5, 2},
    {36000, 6, 2},
    {75000, 7, 3},
    {150000, 8, 3},
    {300000, 9, 3},
    {600000, 9, 3},
    {900000, 9, 3},
    {1200000, 9, 3},
    {1500000, 9, 4},
    {1800000, 9, 4},
    {2100000, 9, 4},
    {2400000, 9, 4},
    {2700000, 9, 4},
    {3000000, 9, 4},
    {3300000, 9, 4},
    {3600000, 9, 4},
};

const uint32_t preserver_levels[][3] = {
    {0, 0, 0},
    {0, 1, 0},
    {2500, 2, 0},
    {5000, 3, 0},
    {10000, 4, 0},
    {20000, 5, 0},
    {40000, 6, 0},
    {60000, 7, 0},
    {90000, 8, 0},
    {135000, 9, 0},
    {250000, 10, 0},
    {375000, 10, 0},
    {750000, 10, 0},
    {11250000, 10, 0},
    {15000000, 10, 0},
    {18750000, 10, 0},
    {22500000, 10, 0},
    {26250000, 10, 0},
    {30000000, 10, 0},
    {33750000, 10, 0},
    {37500000, 10, 0},
};

// TODO: REMEMBER TO UPDATE get_hit_die!!!!

static const int8_t hit_die[] = {
    0, 8, 8, 8, 8, 8, 8, 8, 8,
    10, 10, 4, 6, 10, 10, 10, 10, 6
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
        case RACE_THRIKEEN: allowed = thrikeen_classes; break;
    }

    while (allowed && *allowed) {
        if (*allowed == class) { return 1; }
        allowed++;
    }
    return 0;
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
static int16_t class_attack_num(const ds_character_t *pc, const ds1_item_t *item) {
    const ds_item1r_t *it1r = ds_get_item1r(item->item_index);
    if (it1r->weapon_type == 1) { // MELEE
        for (int i = 0; i < 3; i++) {
            switch (pc->real_class[i]) {
                case REAL_CLASS_FIGHTER:
                case REAL_CLASS_GLADIATOR:
                case REAL_CLASS_AIR_RANGER:
                case REAL_CLASS_WATER_RANGER:
                case REAL_CLASS_FIRE_RANGER:
                case REAL_CLASS_EARTH_RANGER:
                    if (pc->level[i] < 7) { return 1; }
                    if (pc->level[i] < 13) { return 2; }
                    return 3;
            }
        }
    } else if (it1r->weapon_type == 2) { // MISSILE
        for (int i = 0; i < 3; i++) {
            switch (pc->real_class[i]) {
                case REAL_CLASS_FIGHTER:
                case REAL_CLASS_GLADIATOR:
                case REAL_CLASS_AIR_RANGER:
                case REAL_CLASS_WATER_RANGER:
                case REAL_CLASS_FIRE_RANGER:
                case REAL_CLASS_EARTH_RANGER:
                    if (pc->level[i] < 7) { return 2; }
                    if (pc->level[i] < 13) { return 4; }
                    return 6;
            }
        }
    }
    return 0;
}

int16_t dnd2e_get_attack_num_pc(const ds_character_t *pc, const ds1_item_t *item) {
    if (item == NULL) { return 0; }
    const ds_item1r_t *it1r = ds_get_item1r(item->item_index);
    // For some reason double attacks are stored for missiles...
    uint16_t base_attacks = (it1r->weapon_type == 2) ? it1r->num_attacks : it1r->num_attacks * 2;
    return base_attacks + class_attack_num(pc, item);
}

int16_t dnd2e_get_attack_sides_pc(const ds_character_t *pc, const ds1_item_t *item) {
    if (item == NULL) { return 0; }
    return ds_get_item1r(item->item_index)->sides;
}

int16_t dnd2e_get_attack_die_pc(const ds_character_t *pc, const ds1_item_t *item) {
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

int16_t dnd2e_get_attack_mod_pc(const ds_character_t *pc, const ds1_item_t *item) {
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
