#include "unity.h"
#include "../src/gff.h"
#include "../src/dsl.h"
#include "../src/dsl-region.h"

void setUp() {
    gff_init();
    gff_load_directory("/home/pwest/dosbox/DARKSUN");
    dsl_init();
}

void tearDown() {
    dsl_cleanup();
    gff_cleanup();
}

int func() { return 40; }

void test_load_close_region1(void) {
    dsl_region_free(dsl_load_region(1));
}

void test_load_close_regions(void) {
    for (int i = 0; i < NUM_FILES; i++) {
        dsl_region_free(dsl_load_region(i));
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_load_close_region1);
    RUN_TEST(test_load_close_regions);
    return UNITY_END();
}
