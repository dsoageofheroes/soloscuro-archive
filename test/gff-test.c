#include "unity.h"
#include "../src/gff.h"
#include "../src/dsl.h"

void setUp() {
    gff_init();
    //gff_load_directory("/home/pwest/dosbox/DARKSUN");
    //dsl_init();
}

void tearDown() {
    //dsl_cleanup();
    gff_cleanup();
}

int func() { return 40; }

void test_simple(void) {
    char buf[2], read[2];
    int id = gff_create("test.gff");

    buf[0] = 0xFF;
    buf[1] = 0x00;
    gff_add_type(id, GFF_CACT);
    //printf("id = %d\n", id);
    //gff_add_chunk(id, GFF_CACT, 1, buf, 2);
    gff_close (id);
    id = gff_open("test.gff");
    gff_add_chunk(id, GFF_CACT, 1, buf, 2);
    gff_close(id);
    id = gff_open("test.gff");
    //printf("id = %d\n", id);
    gff_chunk_header_t chunk = gff_find_chunk_header(id, GFF_CACT, 1);
    TEST_ASSERT_EQUAL_INT(2, chunk.length);
    gff_read_chunk(id, &chunk, read, chunk.length);
    TEST_ASSERT_EQUAL_INT(buf[0], read[0]);
    TEST_ASSERT_EQUAL_INT(buf[1], read[1]);
    gff_close(id);
}

void test_multi_entry(void) {
    char buf[2], read[2];
    int id = gff_create("test.gff");

    buf[0] = 0xFF;
    buf[1] = 0x00;
    gff_add_type(id, GFF_CACT);
    for (int i = 1; i < 1024; i++) {
        buf[1] = i;
        gff_add_chunk(id, GFF_CACT, i, buf, 2);
    }
    gff_close (id);
    id = gff_open("test.gff");
    for (int i = 1; i < 1024; i++) {
        buf[1] = i;
        gff_chunk_header_t chunk = gff_find_chunk_header(id, GFF_CACT, i);
        TEST_ASSERT_EQUAL_INT(2, chunk.length);
        gff_read_chunk(id, &chunk, read, chunk.length);
        TEST_ASSERT_EQUAL_INT(buf[0], read[0]);
        TEST_ASSERT_EQUAL_INT(buf[1], read[1]);
    }
    gff_close(id);
}

void test_multi_type(void) {
    char buf[2];
    char read[2];
    int id = gff_create("test.gff");

    buf[0] = 0xFF;
    buf[1] = 0x00;
    gff_add_type(id, GFF_CACT);
    gff_add_type(id, GFF_CHAR);
    gff_add_type(id, GFF_BMP);
    gff_add_type(id, GFF_PORT);
    gff_close (id);
    id = gff_open("test.gff");
    gff_add_chunk(id, GFF_CACT, 1, buf, 2);
    buf[1] = 0x01;
    gff_add_chunk(id, GFF_CHAR, 1, buf, 2);
    buf[1] = 0x02;
    gff_add_chunk(id, GFF_BMP, 1, buf, 2);
    buf[1] = 0x03;
    gff_add_chunk(id, GFF_PORT, 1, buf, 2);
    gff_close(id);
    id = gff_open("test.gff");

    buf[1] = 0x00;
    gff_chunk_header_t chunk = gff_find_chunk_header(id, GFF_CACT, 1);
    TEST_ASSERT_EQUAL_INT(2, chunk.length);
    gff_read_chunk(id, &chunk, read, chunk.length);
    TEST_ASSERT_EQUAL_INT(buf[0], read[0]);
    TEST_ASSERT_EQUAL_INT(buf[1], read[1]);

    buf[1] = 0x01;
    chunk = gff_find_chunk_header(id, GFF_CHAR, 1);
    TEST_ASSERT_EQUAL_INT(2, chunk.length);
    gff_read_chunk(id, &chunk, read, chunk.length);
    TEST_ASSERT_EQUAL_INT(buf[0], read[0]);
    TEST_ASSERT_EQUAL_INT(buf[1], read[1]);

    buf[1] = 0x02;
    chunk = gff_find_chunk_header(id, GFF_BMP, 1);
    TEST_ASSERT_EQUAL_INT(2, chunk.length);
    gff_read_chunk(id, &chunk, read, chunk.length);
    TEST_ASSERT_EQUAL_INT(buf[0], read[0]);
    TEST_ASSERT_EQUAL_INT(buf[1], read[1]);

    buf[1] = 0x03;
    chunk = gff_find_chunk_header(id, GFF_PORT, 1);
    TEST_ASSERT_EQUAL_INT(2, chunk.length);
    gff_read_chunk(id, &chunk, read, chunk.length);
    TEST_ASSERT_EQUAL_INT(buf[0], read[0]);
    TEST_ASSERT_EQUAL_INT(buf[1], read[1]);

    gff_close(id);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_simple);
    RUN_TEST(test_multi_entry);
    RUN_TEST(test_multi_type);
    return UNITY_END();
}
