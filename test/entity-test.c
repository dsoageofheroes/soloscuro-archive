#include "unity.h"
#include "gff.h"
#include "entity.h"
#include "gff-char.h"
#include "rules.h"
#include "region.h"
#include "region-manager.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void setUp() {
    gff_init();
    gff_load_directory("ds1");
}

void tearDown() {
    gff_cleanup();
}

void test_basic(void) {
    sol_entity_t* slig;
    sol_entity_create_from_objex(-269, &slig);
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
    sol_entity_t* slig2;
    sol_entity_create_clone(slig, &slig2);
    TEST_ASSERT_EQUAL_INT(slig->stats.base_move, slig2->stats.base_move);
    sol_entity_free(slig);
    sol_entity_free(slig2);
}

void test_fake(void) {
    sol_entity_t* fake;
    sol_entity_create_fake(1, 1, &fake);
    TEST_ASSERT_EQUAL_INT(1, sol_entity_is_fake(fake));

    sol_entity_free(fake);
}

void test_etab(void) {
    sol_region_t* reg;
    sol_region_manager_get_region(42, 0, &reg);
    sol_entity_t* obj;
    sol_entity_create_from_etab(reg->entry_table, 3, &obj);
    sol_entity_free(obj);
}

// TODO:
void test_gff(void) {
    //sol_entity_t dude;
    //entity_load_from_gff(&dude, const int gff_idx, const int player, const int res_id);
    //sol_entity_t* obj = entity_create_from_etab(reg->entry_table, 3);
    //entity_free(obj);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    RUN_TEST(test_fake);
    RUN_TEST(test_etab);
    RUN_TEST(test_gff);
    return UNITY_END();
}
