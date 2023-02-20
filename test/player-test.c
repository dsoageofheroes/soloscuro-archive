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
    sol_entity_t *dude, *player;
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(0) != SOL_SUCCESS);
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(1) != SOL_SUCCESS);
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(2) != SOL_SUCCESS);
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(3) != SOL_SUCCESS);
    sol_entity_create_fake(30, 10, &dude);
    sol_player_set(1, dude);
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(0) != SOL_SUCCESS);
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(1) != SOL_SUCCESS);
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(2) != SOL_SUCCESS);
    TEST_ASSERT_EQUAL_INT(0, sol_player_exists(3) != SOL_SUCCESS);
    TEST_ASSERT_NOT_NULL(sol_player_get(1, &player) == SOL_SUCCESS);
    player->name = strdup("HELLO!");
    sol_player_cleanup();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    return UNITY_END();
}
