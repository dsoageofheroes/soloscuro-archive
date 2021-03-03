#include "unity.h"
#include "../src/gff.h"
#include "../src/entity.h"
#include "../src/gff-char.h"
#include "../src/rules.h"
#include "../src/dsl.h"
#include "../src/entity-list.h"
#include "../src/region-manager.h"
#include <string.h>
#include <stdlib.h>

// A hack, but include base for default procedures
#include "base.c"

void setUp() {
    gff_init();
    gff_load_directory("ds1");
    dsl_init();
}

void tearDown() {
    dsl_cleanup();
    gff_cleanup();
}

void test_basic(void) {
    region_t *reg = region_manager_get_region(42);
    TEST_ASSERT_EQUAL_INT(230, reg->num_tiles);
    TEST_ASSERT_EQUAL_INT(192, reg->flags[20][20]);

    TEST_ASSERT_EQUAL_INT(0, reg->passives[5].flags);
    TEST_ASSERT_EQUAL_INT(5, reg->passives[5].entry_id);
    TEST_ASSERT_EQUAL_INT(1856, reg->passives[5].bmp_id);
    TEST_ASSERT_EQUAL_INT(189, reg->passives[5].bmpx);
    TEST_ASSERT_EQUAL_INT(37, reg->passives[5].bmpy);
    TEST_ASSERT_EQUAL_INT(28, reg->passives[5].xoffset);
    TEST_ASSERT_EQUAL_INT(60, reg->passives[5].yoffset);
    TEST_ASSERT_EQUAL_INT(15, reg->passives[5].mapx);
    TEST_ASSERT_EQUAL_INT(9, reg->passives[5].mapy);
    TEST_ASSERT_EQUAL_INT(0, reg->passives[5].mapz);
    TEST_ASSERT_EQUAL_INT(13, reg->passives[5].scmd_flags);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    return UNITY_END();
}
