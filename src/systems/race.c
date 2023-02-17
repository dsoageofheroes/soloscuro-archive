#include "race.h"

//    str dex con int wis chr
const static int8_t race_mods[][6] = {
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

static const int8_t racial_stats[][6][2] = {
    // Monster
    { {  1, 25 },
      {  1, 25 },
      {  1, 25 },
      {  1, 25 },
      {  1, 25 },
      {  1, 25 }, },
    // Human
    { {  9, 20 },
      {  9, 20 },
      {  9, 20 },
      {  9, 20 },
      {  9, 20 },
      {  9, 20 }, },
    // Dwarf
    { { 11, 21 },
      {  4, 19 },
      { 16, 22 },
      {  5, 20 },
      {  5, 20 },
      {  3, 18 }, },
    // Elf
    { {  5, 20 },
      { 14, 22 },
      {  6, 18 },
      {  9, 21 },
      {  4, 19 },
      {  5, 20 }, },
    // half-elf
    { {  5, 20 },
      {  9, 21 },
      {  4, 19 },
      {  5, 20 },
      {  5, 20 },
      {  5, 20 }, },
    // half-giant
    { { 21, 24 },
      {  8, 15 },
      { 17, 22 },
      {  3, 13 },
      {  3, 15 },
      {  3, 15 }, },
    // halfling
    { {  3, 16 },
      { 14, 22 },
      {  4, 19 },
      {  5, 20 },
      {  9, 22 },
      {  4, 19 }, },
    // Mul
    { { 12, 22 },
      {  5, 20 },
      {  9, 21 },
      {  4, 19 },
      {  5, 20 },
      {  3, 18 }, },
    // Thri-kreen
    { {  8, 20 },
      { 17, 22 },
      {  8, 20 },
      {  4, 19 },
      {  6, 21 },
      {  3, 15 }, },
};

extern sol_status_t sol_dnd2e_race_apply_mods(sol_entity_t *pc) {
    if (!pc) { return SOL_NULL_ARGUMENT; }
    if (pc->race < RACE_HUMAN || pc->race > RACE_THRIKREEN) { return SOL_OUT_OF_RANGE; }
    pc->stats.str += race_mods[pc->race][0];
    pc->stats.dex += race_mods[pc->race][1];
    pc->stats.con += race_mods[pc->race][2];
    pc->stats.intel += race_mods[pc->race][3];
    pc->stats.wis += race_mods[pc->race][4];
    pc->stats.cha += race_mods[pc->race][5];
    return SOL_SUCCESS;
}

extern sol_status_t sol_dnd2e_race_apply_initial_stats(sol_entity_t *pc) {
    if (!pc) { return SOL_NULL_ARGUMENT; }
    if (pc->stats.str   < racial_stats[pc->race][0][0]) { pc->stats.str   = racial_stats[pc->race][0][1]; }
    if (pc->stats.dex   < racial_stats[pc->race][1][0]) { pc->stats.dex   = racial_stats[pc->race][1][1]; }
    if (pc->stats.con   < racial_stats[pc->race][2][0]) { pc->stats.con   = racial_stats[pc->race][2][1]; }
    if (pc->stats.intel < racial_stats[pc->race][3][0]) { pc->stats.intel = racial_stats[pc->race][3][1]; }
    if (pc->stats.wis   < racial_stats[pc->race][4][0]) { pc->stats.wis   = racial_stats[pc->race][4][1]; }
    if (pc->stats.cha   < racial_stats[pc->race][5][0]) { pc->stats.cha   = racial_stats[pc->race][5][1]; }

    if (pc->stats.str   > racial_stats[pc->race][0][1]) { pc->stats.str   = racial_stats[pc->race][0][0]; }
    if (pc->stats.dex   > racial_stats[pc->race][1][1]) { pc->stats.dex   = racial_stats[pc->race][1][0]; }
    if (pc->stats.con   > racial_stats[pc->race][2][1]) { pc->stats.con   = racial_stats[pc->race][2][0]; }
    if (pc->stats.intel > racial_stats[pc->race][3][1]) { pc->stats.intel = racial_stats[pc->race][3][0]; }
    if (pc->stats.wis   > racial_stats[pc->race][4][1]) { pc->stats.wis   = racial_stats[pc->race][4][0]; }
    if (pc->stats.cha   > racial_stats[pc->race][5][1]) { pc->stats.cha   = racial_stats[pc->race][5][0]; }
    return SOL_SUCCESS;
}
