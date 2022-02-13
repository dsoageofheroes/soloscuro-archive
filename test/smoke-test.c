#include "unity.h"
#include "gff.h"

void setUp() {
    gff_init();
    gff_load_directory("/home/pwest/dosbox/DARKSUN");
}

void tearDown() {
    gff_cleanup();
}

int func() { return 40; }

void test_start_close(void) {
    TEST_ASSERT_EQUAL_HEX8(40, func());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_start_close);
    return UNITY_END();
}
