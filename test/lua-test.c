#include "unity.h"
#include "../src/gff.h"
#include "../src/entity.h"
#include "../src/gff-char.h"
#include "../src/rules.h"
#include "../src/dsl.h"
#include "../src/region.h"
#include "../src/region-manager.h"
#include "../src/dsl-manager.h"
#include <string.h>
#include <stdlib.h>

void setUp() {
    gff_init();
    gff_load_directory("ds1");
}

void tearDown() {
    dsl_cleanup();
    gff_cleanup();
}

void test_basic(void) {
    dsl_manager_init();
    dsl_manager_cleanup();
}

const char basic_test[] = 
    "soloscuro.load_charsave(0, 15) -- load character from charsave.gff\n"
    "soloscuro.load_charsave(1, 16) -- load character from charsave.gff\n"
    //"soloscuro.set_ignore_repeat(true) -- set to false if you want repeats\n"
    "e = soloscuro.load_player(0)\n"
    "print (e.name)\n"
    //"e.mapx = 33\n"
    //"print (e.mapx)\n"
    //"print (e.mapy)\n"
    ""
    ;

void test_basic_script(void) {
    dsl_manager_init();
    dsl_execute_string(basic_test);
    dsl_manager_cleanup();
}

void test_etab(void) {
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    RUN_TEST(test_basic_script);
    //RUN_TEST(test_etab);
    return UNITY_END();
}
