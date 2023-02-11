#include "unity.h"
#include "gff.h"
#include "entity.h"
#include "gff-char.h"
#include "rules.h"
#include "statics.h"
#include "entity-list.h"
#include <string.h>
#include <stdlib.h>

void setUp() {
    gff_init();
    gff_load_directory("ds1");
}

void tearDown() {
    gff_cleanup();
}

void test_basic(void) {
    sol_entity_list_t *list = NULL;
    sol_entity_list_create(&list);
    sol_entity_t* slig;
    sol_entity_create_from_objex(-269, &slig);
    sol_entity_t* screamer_beatle;
    sol_entity_create_from_objex(-150, &screamer_beatle);
    int hp_sum = 0;

    sol_entity_list_add(list, slig, NULL);
    sol_entity_list_add(list, screamer_beatle, NULL);

    sol_entity_t *dude;
    sol_entity_list_for_each(list, dude) {
        hp_sum += dude->stats.hp;
    }

    TEST_ASSERT_EQUAL_INT(33, hp_sum);

    sol_entity_list_free(list);
    sol_entity_free(slig);
    sol_entity_free(screamer_beatle);
}

void test_load_etab(void) {
    char gff_name[32];
    dude_t *dude;
    size_t xsum = 0;
    size_t ysum = 0;
    sol_entity_list_t *list = NULL;
    sol_entity_list_create(&list);
    sol_static_list_t sl;

    snprintf(gff_name, 32, "rgn%x.gff", 42);
    int gff_index = gff_find_index(gff_name);
    sol_entity_list_load_etab(list, &sl, gff_index, 42);

    sol_entity_list_for_each(list, dude) {
        xsum += dude->mapx;
        ysum += dude->mapy;
    }

    //TEST_ASSERT_EQUAL_INT(3222, xsum);
    //TEST_ASSERT_EQUAL_INT(2501, ysum);
    TEST_ASSERT_EQUAL_INT(3131, xsum);
    TEST_ASSERT_EQUAL_INT(2410, ysum);

    sol_entity_list_free_all(list);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    RUN_TEST(test_load_etab);
    return UNITY_END();
}
