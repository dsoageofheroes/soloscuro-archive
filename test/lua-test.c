#include "unity.h"
#include "gff.h"
#include "entity.h"
#include "gff-char.h"
#include "gpl-manager.h"
#include "rules.h"
#include "region.h"
#include "region-manager.h"
#include <string.h>
#include <stdlib.h>
#include "sol-lua-manager.h"
#include "settings.h"

void setUp() {
    gff_init();
    gff_load_directory("ds1");
}

void tearDown() {
    gff_cleanup();
}

void test_basic(void) {
    sol_gpl_manager_init();
    sol_gpl_push_context();
    gpl_pop_context();
    sol_gpl_manager_cleanup();
}

const char basic_test[] = 
    "a = 10\n"
    "print(a)\n"
    ""
    ;

void test_basic_script(void) {
    sol_gpl_manager_init();
    sol_gpl_push_context();
    sol_gpl_execute_string(basic_test);
    gpl_pop_context();
    sol_gpl_manager_cleanup();
}

void test_combat_smoke(void) {
    sol_gpl_manager_init();
    sol_gpl_push_context();
    if (!sol_lua_load("test/lua/combat/00-combat-smoke-test.lua")) {
        TEST_ASSERT_MESSAGE(0, "Unable to load lua\n");
    }
    //sol_test_info_t sti = sol_get_lua_test();
    //TEST_ASSERT_MESSAGE(!sti.failed, sti.msg);
    gpl_pop_context();
    sol_gpl_manager_cleanup();
}

void test_combat_init(void) {
    if (!sol_lua_load("test/lua/combat/01-detect-in-combat.lua")) {
        TEST_ASSERT_MESSAGE(0, "Unable to load lua\n");
    }
    //sol_test_info_t sti = sol_get_lua_test();
    //TEST_ASSERT_MESSAGE(!sti.failed, sti.msg);
}

void test_combat_basic(void) {
    if (!sol_lua_load("test/lua/combat/02-get-attacked.lua")) {
        TEST_ASSERT_MESSAGE(0, "Unable to load lua\n");
    }
    //sol_test_info_t sti = sol_get_lua_test();
    //TEST_ASSERT_MESSAGE(!sti.failed, sti.msg);
}

void test_combat_move(void) {
    if (!sol_lua_load("test/lua/combat/03-far-attack.lua")) {
        TEST_ASSERT_MESSAGE(0, "Unable to load lua\n");
    }
    //sol_test_info_t sti = sol_get_lua_test();
    //TEST_ASSERT_MESSAGE(!sti.failed, sti.msg);
}

void test_combat_multi_round(void) {
    sol_gpl_manager_init();
    sol_gpl_push_context();
    if (!sol_lua_load("test/lua/combat/04-player-attack.lua")) {
        TEST_ASSERT_MESSAGE(0, "Unable to load lua\n");
    }
    //sol_test_info_t sti = sol_get_lua_test();
    //printf("----------------->%s\n", sti.msg);
    //TEST_ASSERT_MESSAGE(!sti.failed, sti.msg);
    gpl_pop_context();
    sol_gpl_manager_cleanup();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    RUN_TEST(test_basic_script);
    RUN_TEST(test_combat_smoke);
    //RUN_TEST(test_combat_init);
    //RUN_TEST(test_combat_basic);
    //RUN_TEST(test_combat_move);
    //RUN_TEST(test_combat_multi_round);
    return UNITY_END();
}
