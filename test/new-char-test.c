#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "../src/gff.h"
#include "../src/dsl.h"
#include "../src/spells.h"
#include "../src/rules.h"

void setUp() {
}

void tearDown() {
}

void test_psi_set(void) {
    ds_character_t pc;
    psin_t psi; // psi group

    memset(&pc, 0x0, sizeof(ds_character_t));
    pc.race = RACE_HUMAN;
    pc.gender = GENDER_MALE;
    pc.real_class[0] = pc.real_class[1] = pc.real_class[2] = -1;
    dnd2e_randomize_stats_pc(&pc);
    memset(&psi, 0x0, sizeof(psi));

    spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 1);
    spell_set_psin(&psi, PSIONIC_PSYCHOMETABOLISM, 1);
    spell_set_psin(&psi, PSIONIC_TELEPATH, 1);
    //TEST_ASSERT_EQUAL_HEX8(40, func());
    TEST_ASSERT_EQUAL_INT(1, spell_has_psin(&psi, PSIONIC_PSYCHOKINETIC));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psin(&psi, PSIONIC_PSYCHOMETABOLISM));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psin(&psi, PSIONIC_TELEPATH));
    spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 0);
    spell_set_psin(&psi, PSIONIC_PSYCHOMETABOLISM, 0);
    spell_set_psin(&psi, PSIONIC_TELEPATH, 0);
    TEST_ASSERT_EQUAL_INT(0, spell_has_psin(&psi, PSIONIC_PSYCHOKINETIC));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psin(&psi, PSIONIC_PSYCHOMETABOLISM));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psin(&psi, PSIONIC_TELEPATH));
}

void test_single_level_exp(void) {
    ds_character_t pc;
    psin_t psi; // psi group

    memset(&pc, 0x0, sizeof(ds_character_t));
    pc.race = RACE_HUMAN;
    pc.gender = GENDER_MALE;
    pc.real_class[0] = pc.real_class[1] = pc.real_class[2] = -1;
    pc.real_class[0] = REAL_CLASS_GLADIATOR;
    dnd2e_randomize_stats_pc(&pc);
    memset(&psi, 0x0, sizeof(psi));

    spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 1);

    dnd2e_set_exp(&pc, 4000);
    TEST_ASSERT_EQUAL_INT(2, pc.level[0]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[1]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[2]);

    dnd2e_set_exp(&pc, 2000);
    TEST_ASSERT_EQUAL_INT(1, pc.level[0]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[1]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[2]);

    dnd2e_set_exp(&pc, 8000);
    TEST_ASSERT_EQUAL_INT(3, pc.level[0]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[1]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[2]);
}

void test_multi_level_exp(void) {
    ds_character_t pc;
    psin_t psi; // psi group

    memset(&pc, 0x0, sizeof(ds_character_t));
    pc.race = RACE_HALFELF;
    pc.gender = GENDER_MALE;
    pc.real_class[0] = pc.real_class[1] = pc.real_class[2] = -1;
    pc.real_class[0] = REAL_CLASS_FIGHTER;
    pc.real_class[1] = REAL_CLASS_PRESERVER;
    dnd2e_randomize_stats_pc(&pc);
    memset(&psi, 0x0, sizeof(psi));

    spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 1);

    dnd2e_set_exp(&pc, 4000);
    TEST_ASSERT_EQUAL_INT(2, pc.level[0]);
    TEST_ASSERT_EQUAL_INT(1, pc.level[1]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[2]);

    dnd2e_set_exp(&pc, 2000);
    TEST_ASSERT_EQUAL_INT(1, pc.level[0]);
    TEST_ASSERT_EQUAL_INT(1, pc.level[1]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[2]);

    dnd2e_set_exp(&pc, 8000);
    TEST_ASSERT_EQUAL_INT(3, pc.level[0]);
    TEST_ASSERT_EQUAL_INT(2, pc.level[1]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[2]);
}

void test_level_up_hp(void) {
    ds_character_t pc;
    psin_t psi; // psi group

    memset(&pc, 0x0, sizeof(ds_character_t));
    pc.race = RACE_HUMAN;
    pc.gender = GENDER_MALE;
    pc.real_class[0] = pc.real_class[1] = pc.real_class[2] = -1;
    pc.real_class[0] = REAL_CLASS_GLADIATOR;
    dnd2e_randomize_stats_pc(&pc);
    memset(&psi, 0x0, sizeof(psi));

    spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 1);

    dnd2e_set_exp(&pc, 0);
    TEST_ASSERT_EQUAL_INT(1, pc.level[0]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[1]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[2]);

    int pHP = pc.base_hp;

    dnd2e_award_exp(&pc, 2500);
    TEST_ASSERT_EQUAL_INT(2, pc.level[0]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[1]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[2]);
    TEST_ASSERT(pHP < pc.base_hp);

    pHP = pc.base_hp;
    dnd2e_award_exp(&pc, 500);
    TEST_ASSERT_EQUAL_INT(2, pc.level[0]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[1]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[2]);
    TEST_ASSERT(pHP == pc.base_hp);

    pHP = pc.base_hp;
    dnd2e_award_exp(&pc, 3000);
    TEST_ASSERT_EQUAL_INT(3, pc.level[0]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[1]);
    TEST_ASSERT_EQUAL_INT(0, pc.level[2]);
    TEST_ASSERT(pHP < pc.base_hp);
}

int main(void) {
    gff_init();
    gff_load_directory("/home/pwest/dosbox/DARKSUN");
    dsl_init();

    UNITY_BEGIN();
    RUN_TEST(test_psi_set);
    RUN_TEST(test_single_level_exp);
    RUN_TEST(test_multi_level_exp);
    RUN_TEST(test_level_up_hp);

    dsl_cleanup();
    gff_cleanup();
    return UNITY_END();
}
