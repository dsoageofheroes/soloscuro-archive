#include "unity.h"
#include "../src/gff.h"
#include "../src/dsl.h"

void game_loop_signal(int signal, int _accum) {
}

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

void test_start_close(void) {
    char buf[1024];
    gff_add_chunk(CHARSAVE_GFF_INDEX, GFF_CHAR, 25, buf, 1<<10);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_start_close);
    return UNITY_END();
}
