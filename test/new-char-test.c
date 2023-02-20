#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "gff.h"
#include "rules.h"

void setUp() {
}

void tearDown() {
}

void test_psi_set(void) {
    sol_entity_t pc;
    psin_t psi; // psi group

    memset(&pc, 0x0, sizeof(ds_character_t));
    pc.race = RACE_HUMAN;
    pc.gender = GENDER_MALE;
    pc.class[0].class = pc.class[1].class = pc.class[2].class = -1;
    sol_dnd2e_randomize_stats_pc(&pc);
    memset(&psi, 0x0, sizeof(psi));

    sol_spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 1);
    sol_spell_set_psin(&psi, PSIONIC_PSYCHOMETABOLISM, 1);
    sol_spell_set_psin(&psi, PSIONIC_TELEPATH, 1);
    //TEST_ASSERT_EQUAL_HEX8(40, func());
    TEST_ASSERT_EQUAL_INT(1, spell_has_psin(&psi, PSIONIC_PSYCHOKINETIC));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psin(&psi, PSIONIC_PSYCHOMETABOLISM));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psin(&psi, PSIONIC_TELEPATH));
    sol_spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 0);
    sol_spell_set_psin(&psi, PSIONIC_PSYCHOMETABOLISM, 0);
    sol_spell_set_psin(&psi, PSIONIC_TELEPATH, 0);
    TEST_ASSERT_EQUAL_INT(0, spell_has_psin(&psi, PSIONIC_PSYCHOKINETIC));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psin(&psi, PSIONIC_PSYCHOMETABOLISM));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psin(&psi, PSIONIC_TELEPATH));
}

void test_single_level_exp(void) {
    sol_entity_t pc;
    psin_t psi; // psi group

    memset(&pc, 0x0, sizeof(ds_character_t));
    pc.race = RACE_HUMAN;
    pc.gender = GENDER_MALE;
    pc.class[0].class = pc.class[1].class = pc.class[2].class = -1;
    pc.class[0].class = REAL_CLASS_GLADIATOR;
    sol_dnd2e_randomize_stats_pc(&pc);
    memset(&psi, 0x0, sizeof(psi));

    sol_spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 1);

    sol_dnd2e_set_exp(&pc, 4000);
    TEST_ASSERT_EQUAL_INT(2, pc.class[0].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[1].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[2].level);

    sol_dnd2e_set_exp(&pc, 2000);
    TEST_ASSERT_EQUAL_INT(1, pc.class[0].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[1].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[2].level);

    sol_dnd2e_set_exp(&pc, 8000);
    TEST_ASSERT_EQUAL_INT(3, pc.class[0].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[1].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[2].level);
}

void test_multi_level_exp(void) {
    sol_entity_t pc;
    psin_t psi; // psi group

    memset(&pc, 0x0, sizeof(ds_character_t));
    pc.race = RACE_HALFELF;
    pc.gender = GENDER_MALE;
    pc.class[0].class = pc.class[1].class = pc.class[2].class = -1;
    pc.class[0].class = REAL_CLASS_FIGHTER;
    pc.class[1].class = REAL_CLASS_PRESERVER;
    sol_dnd2e_randomize_stats_pc(&pc);
    memset(&psi, 0x0, sizeof(psi));

    sol_spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 1);

    sol_dnd2e_set_exp(&pc, 4000);
    TEST_ASSERT_EQUAL_INT(2, pc.class[0].level);
    TEST_ASSERT_EQUAL_INT(1, pc.class[1].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[2].level);

    sol_dnd2e_set_exp(&pc, 2000);
    TEST_ASSERT_EQUAL_INT(1, pc.class[0].level);
    TEST_ASSERT_EQUAL_INT(1, pc.class[1].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[2].level);

    sol_dnd2e_set_exp(&pc, 8000);
    TEST_ASSERT_EQUAL_INT(3, pc.class[0].level);
    TEST_ASSERT_EQUAL_INT(2, pc.class[1].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[2].level);
}

void test_level_up_hp(void) {
    sol_entity_t pc;
    psin_t psi; // psi group

    memset(&pc, 0x0, sizeof(ds_character_t));
    pc.race = RACE_HUMAN;
    pc.gender = GENDER_MALE;
    pc.class[0].class = pc.class[1].class = pc.class[2].class = -1;
    pc.class[0].class = REAL_CLASS_GLADIATOR;
    sol_dnd2e_randomize_stats_pc(&pc);
    memset(&psi, 0x0, sizeof(psi));

    sol_spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 1);

    sol_dnd2e_set_exp(&pc, 0);
    TEST_ASSERT_EQUAL_INT(1, pc.class[0].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[1].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[2].level);

    int pHP = pc.stats.hp;

    sol_dnd2e_award_exp(&pc, 2500);
    TEST_ASSERT_EQUAL_INT(2, pc.class[0].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[1].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[2].level);
    TEST_ASSERT(pHP < pc.stats.hp);

    pHP = pc.stats.hp;
    sol_dnd2e_award_exp(&pc, 500);
    TEST_ASSERT_EQUAL_INT(2, pc.class[0].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[1].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[2].level);
    TEST_ASSERT(pHP == pc.stats.hp);

    pHP = pc.stats.hp;
    sol_dnd2e_award_exp(&pc, 3000);

    TEST_ASSERT_EQUAL_INT(3, pc.class[0].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[1].level);
    TEST_ASSERT_EQUAL_INT(0, pc.class[2].level);
    TEST_ASSERT(pHP < pc.stats.hp);
}

int main(void) {
    gff_init();
    gff_load_directory("/home/pwest/dosbox/DARKSUN");

    UNITY_BEGIN();
    RUN_TEST(test_psi_set);
    RUN_TEST(test_single_level_exp);
    RUN_TEST(test_multi_level_exp);
    RUN_TEST(test_level_up_hp);

    gff_cleanup();
    return UNITY_END();
}
