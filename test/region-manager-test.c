#include "unity.h"
#include "../src/gff.h"
#include "../src/dsl.h"
#include "../src/region.h"
#include "../src/region-manager.h"
#include "../src/ds-player.h"

void setUp() {
    gff_init();
    gff_load_directory("ds1");
    //dsl_init();
}

void tearDown() {
    //dsl_cleanup();
    gff_cleanup();
}

int func() { return 40; }

void test_simple(void) {
    region_manager_init();
    region_t* reg = region_manager_get_region(42);

    TEST_ASSERT_NOT_NULL(reg);

    region_manager_cleanup();
}

void test_set_get(void) {
    region_manager_init();
    region_t* reg = region_manager_get_region(42);
    region_t* empty = region_create_empty();

    TEST_ASSERT_NOT_NULL(reg);
    region_manager_set_current(reg);
    region_manager_add_region(empty);
    region_manager_set_current(empty);
    TEST_ASSERT(empty == region_manager_get_current());
    region_manager_set_current(reg);
    TEST_ASSERT(empty != region_manager_get_current());

    region_manager_cleanup();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_simple);
    RUN_TEST(test_set_get);
    return UNITY_END();
}
