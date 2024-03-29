#include "unity.h"
#include "gff.h"
#include "region.h"
#include "region-manager.h"
#include "player.h"

void setUp() {
    gff_init();
    gff_load_directory("ds1");
}

void tearDown() {
    gff_cleanup();
}

int func() { return 40; }

void test_simple(void) {
    sol_region_manager_init();
    sol_region_t* reg;
    sol_region_manager_get_region(42, 0, &reg);

    TEST_ASSERT_NOT_NULL(reg);

    sol_region_manager_cleanup(1);
}

void test_set_get(void) {
    sol_region_manager_init();
    sol_region_t* reg, *test;
    sol_region_manager_get_region(42, 0, &reg);
    sol_region_t* empty = NULL;
    sol_region_create_empty(&reg);

    TEST_ASSERT_NOT_NULL(reg);
    sol_region_manager_set_current(reg);
    sol_region_manager_add_region(empty);
    sol_region_manager_set_current(empty);
    sol_region_manager_get_current(&test);
    TEST_ASSERT(empty == test);
    sol_region_manager_set_current(reg);
    sol_region_manager_get_current(&test);
    TEST_ASSERT(empty != test);

    sol_region_manager_cleanup(1);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_simple);
    RUN_TEST(test_set_get);
    return UNITY_END();
}
