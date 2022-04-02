#include "unity.h"
#include "gff.h"
#include "entity.h"
#include "gff-char.h"
#include "gpl.h"
#include "gpl-manager.h"
#include "rules.h"
#include "region.h"
#include "region-manager.h"
#include "ssi-item.h"
#include "gameloop.h"
#include <string.h>
#include <stdlib.h>
#include "sol-lua-manager.h"
#include "sol-lua-settings.h"
#include "settings.h"

#define script(a) const char *lua_script = "test/lua/combat/"#a".lua";
extern const char *lua_script;

void setUp() {
    //gff_init();
    //gff_load_directory("ds1");
    sol_lua_load_preload("lua/settings.lua");
    powers_init();
    gpl_init();
    sol_lua_register_globals();
}

void tearDown() {
    sol_region_manager_cleanup(1);
    gff_cleanup();
    gpl_cleanup();
}

void the_test(void) {
    printf("Running '%s'\n", lua_script);
    if (sol_lua_load(lua_script)) { // lua_script is defined in the test
        TEST_ASSERT_MESSAGE(0, "Unable to load lua\n");
    }
    sol_game_loop();
    sol_test_info_t sti = sol_get_lua_test();
    TEST_ASSERT_MESSAGE(!sti.failed, sti.msg);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(the_test);
    return UNITY_END();
}
