#include "unity.h"
#include "gff.h"
#include "entity.h"
#include "gff-char.h"
#include "rules.h"
#include "player.h"
#include "region-manager.h"
#include <string.h>
#include <stdlib.h>

void setUp() {
    gff_init();
    gff_load_directory("ds1");
}

void tearDown() {
    gff_cleanup();
}

void test_basic(void) {
    sol_player_init();
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(0));
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(1));
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(2));
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(3));
    sol_player_set(1, entity_create_fake(30, 10));
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(0));
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(1));
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(2));
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(3));
    TEST_ASSERT_NOT_NULL(sol_player_get(1));
    sol_player_get(1)->name = strdup("HELLO!");
    sol_player_cleanup();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    return UNITY_END();
}
