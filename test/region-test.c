#include "unity.h"
#include "gff.h"
#include "region.h"

void setUp() {
    gff_init();
    gff_load_directory("ds1");
}

void tearDown() {
    gff_cleanup();
}

int func() { return 40; }

void test_simple(void) {
    sol_region_t* reg = sol_region_create_empty();
    sol_region_tick(reg);

    for (size_t i = 0; i < MAP_ROWS; i++) {
        for (size_t j = 0; j < MAP_COLUMNS; j++) {
            TEST_ASSERT_EQUAL_INT(0, sol_region_is_block(reg, i, j));
        }
    }

    for (size_t i = 0; i < MAP_ROWS; i++) {
        for (size_t j = 0; j < MAP_COLUMNS; j++) {
            sol_region_set_block(reg, i, j, 0x4F);
        }
    }

    for (size_t i = 0; i < MAP_ROWS; i++) {
        for (size_t j = 0; j < MAP_COLUMNS; j++) {
            TEST_ASSERT_EQUAL_INT(0x40, sol_region_is_block(reg, i, j));
        }
    }

    for (size_t i = 0; i < MAP_ROWS; i++) {
        for (size_t j = 0; j < MAP_COLUMNS; j++) {
            sol_region_clear_block(reg, i, j, 0x40);
        }
    }

    for (size_t i = 0; i < MAP_ROWS; i++) {
        for (size_t j = 0; j < MAP_COLUMNS; j++) {
            TEST_ASSERT_EQUAL_INT(0x00, sol_region_is_block(reg, i, j));
        }
    }

    sol_region_free(reg);
}

void test_ds1_gff(void) {
    sol_region_t *reg = sol_region_create(gff_find_index("rgn2a.gff"));
    sol_region_tick(reg);
    sol_combat_update(reg);

    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < MAP_COLUMNS; j++) {
            TEST_ASSERT_EQUAL_INT(0x40, sol_region_is_block(reg, i, j));
        }
    }

    TEST_ASSERT_EQUAL_INT(0x00, sol_region_is_block(reg, 12, 36));

    for (size_t i = 0; i < MAP_ROWS; i++) {
        for (size_t j = 0; j < MAP_COLUMNS; j++) {
            sol_region_set_block(reg, i, j, 0x4F);
        }
    }

    for (size_t i = 0; i < MAP_ROWS; i++) {
        for (size_t j = 0; j < MAP_COLUMNS; j++) {
            TEST_ASSERT_EQUAL_INT(0x40, sol_region_is_block(reg, i, j));
        }
    }

    for (size_t i = 0; i < MAP_ROWS; i++) {
        for (size_t j = 0; j < MAP_COLUMNS; j++) {
            sol_region_clear_block(reg, i, j, 0x40);
        }
    }

    for (size_t i = 0; i < MAP_ROWS; i++) {
        for (size_t j = 0; j < MAP_COLUMNS; j++) {
            TEST_ASSERT_EQUAL_INT(0x00, sol_region_is_block(reg, i, j));
        }
    }

    sol_region_free(reg);
}

void test_get_ds1_tiles(void) {
    sol_region_t *reg = sol_region_create(gff_find_index("rgn2a.gff"));
    uint32_t w, h;
    unsigned char *data = NULL;

    for (size_t i = 1; i < 230; i++) {
        w = h = 0;
        data = NULL;
        sol_region_get_tile(reg, i, &w, &h, &data);
        TEST_ASSERT_EQUAL_INT(16, w);
        TEST_ASSERT_EQUAL_INT(16, h);
        TEST_ASSERT_NOT_NULL(data);
        free(data);
    }

    sol_region_free(reg);
}

void test_get_png_tiles(void) {
    int gff_file = gff_open("test/data/simple.gff");
    sol_region_t* reg = sol_region_create_empty();
    uint32_t w, h;
    unsigned char *data = NULL;

    sol_combat_update(reg);
    reg->gff_file = gff_file;
    w = h = 0;
    data = NULL;
    sol_region_get_tile(reg, 142, &w, &h, &data);
    TEST_ASSERT_EQUAL_INT(16, w);
    TEST_ASSERT_EQUAL_INT(16, h);
    TEST_ASSERT_NOT_NULL(data);
    free(data);

    sol_region_free(reg);
    gff_close(gff_file);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_simple);
    RUN_TEST(test_ds1_gff);
    RUN_TEST(test_get_ds1_tiles);
    //RUN_TEST(test_get_png_tiles);
    return UNITY_END();
}
