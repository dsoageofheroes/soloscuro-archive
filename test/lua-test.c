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
    gpl_manager_init();
    gpl_push_context();
    gpl_pop_context();
    gpl_manager_cleanup();
}

const char basic_test[] = 
    "a = 10\n"
    "print(a)\n"
    ""
    ;

void test_basic_script(void) {
    gpl_manager_init();
    gpl_push_context();
    gpl_execute_string(basic_test);
    gpl_pop_context();
    gpl_manager_cleanup();
}

void test_combat_smoke(void) {
    if (sol_lua_load("test/lua/combat/00-combat-smoke-test.lua")) {
        TEST_ASSERT_MESSAGE(0, "Unable to load lua\n");
    }
    sol_test_info_t sti = sol_get_lua_test();
    TEST_ASSERT_MESSAGE(!sti.failed, sti.msg);
}

void test_combat_init(void) {
    if (sol_lua_load("test/lua/combat/01-detect-in-combat.lua")) {
        TEST_ASSERT_MESSAGE(0, "Unable to load lua\n");
    }
    sol_test_info_t sti = sol_get_lua_test();
    TEST_ASSERT_MESSAGE(!sti.failed, sti.msg);
}

void test_combat_basic(void) {
    if (sol_lua_load("test/lua/combat/02-get-attacked.lua")) {
        TEST_ASSERT_MESSAGE(0, "Unable to load lua\n");
    }
    sol_test_info_t sti = sol_get_lua_test();
    TEST_ASSERT_MESSAGE(!sti.failed, sti.msg);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    RUN_TEST(test_basic_script);
    RUN_TEST(test_combat_smoke);
    RUN_TEST(test_combat_init);
    RUN_TEST(test_combat_basic);
    return UNITY_END();
}
