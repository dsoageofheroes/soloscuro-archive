#include "unity.h"
#include "../src/gff.h"
#include "../src/entity.h"
#include "../src/gff-char.h"
#include "../src/rules.h"
#include "../src/dsl.h"
#include "../src/player.h"
#include "../src/region-manager.h"
#include <string.h>
#include <stdlib.h>

void setUp() {
    gff_init();
    gff_load_directory("ds1");
}

void tearDown() {
    dsl_cleanup();
    gff_cleanup();
}

void test_basic(void) {
    sol_player_init();
    TEST_ASSERT_EQUAL_INT(0, player_exists(0));
    TEST_ASSERT_EQUAL_INT(0, player_exists(1));
    TEST_ASSERT_EQUAL_INT(0, player_exists(2));
    TEST_ASSERT_EQUAL_INT(0, player_exists(3));
    player_set(1, entity_create_fake(30, 10));
    TEST_ASSERT_EQUAL_INT(0, player_exists(0));
    TEST_ASSERT_EQUAL_INT(0, player_exists(1));
    TEST_ASSERT_EQUAL_INT(0, player_exists(2));
    TEST_ASSERT_EQUAL_INT(0, player_exists(3));
    TEST_ASSERT_NOT_NULL(player_get(1));
    player_get(1)->name = strdup("HELLO!");
    player_cleanup();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    return UNITY_END();
}
