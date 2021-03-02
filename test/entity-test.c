#include "unity.h"
#include "../src/gff.h"
#include "../src/entity.h"
#include "../src/gff-char.h"
#include "../src/rules.h"
#include "../src/dsl.h"
#include <string.h>
#include <stdlib.h>

// A hack, but inclue base for default procedures
#include "base.c"

void setUp() {
    gff_init();
    gff_load_directory("ds1");
    dsl_init();
}

void tearDown() {
    dsl_cleanup();
    gff_cleanup();
}

void test_basic(void) {
    entity_t* slig = entity_create_from_objex(-269);
    TEST_ASSERT_EQUAL_INT(18, slig->stats.hp);
    TEST_ASSERT_EQUAL_INT(12, slig->stats.str);
    TEST_ASSERT_EQUAL_INT(12, slig->stats.dex);
    TEST_ASSERT_EQUAL_INT(12, slig->stats.intel);
    TEST_ASSERT_EQUAL_INT(12, slig->stats.wis);
    TEST_ASSERT_EQUAL_INT(12, slig->stats.cha);
    TEST_ASSERT_EQUAL_INT(3, slig->stats.base_ac);
    TEST_ASSERT_EQUAL_INT(9, slig->stats.base_move);
    TEST_ASSERT_EQUAL_INT(4, slig->stats.attacks[0].number);
    TEST_ASSERT_EQUAL_INT(1, slig->stats.attacks[0].num_dice);
    TEST_ASSERT_EQUAL_INT(4, slig->stats.attacks[0].sides);
    TEST_ASSERT_EQUAL_INT(0, slig->stats.attacks[0].bonus);
    TEST_ASSERT_EQUAL_INT(0, slig->stats.attacks[1].number);
    TEST_ASSERT_EQUAL_INT(0, slig->stats.attacks[2].number);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    return UNITY_END();
}
