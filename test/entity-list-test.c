#include "unity.h"
#include "../src/gff.h"
#include "../src/entity.h"
#include "../src/gff-char.h"
#include "../src/rules.h"
#include "../src/dsl.h"
#include "../src/entity-list.h"
#include <string.h>
#include <stdlib.h>

// A hack, but inclue base for default procedures
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
    entity_list_t *list = entity_list_create();
    entity_t* slig = entity_create_from_objex(-269);
    entity_t* screamer_beatle = entity_create_from_objex(-150);
    int hp_sum = 0;

    entity_list_add(list, slig);
    entity_list_add(list, screamer_beatle);

    entity_t *dude;
    entity_list_for_each(list, dude) {
        hp_sum += dude->stats.hp;
    }

    TEST_ASSERT_EQUAL_INT(33, hp_sum);

    entity_list_free(list);
    entity_free(slig);
    entity_free(screamer_beatle);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    return UNITY_END();
}
