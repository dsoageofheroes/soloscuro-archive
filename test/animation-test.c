#include "unity.h"
#include "gff.h"
#include "entity.h"
#include "gff-char.h"
#include "rules.h"
#include "region.h"
#include "region-manager.h"
#include "animation.h"
#include <string.h>
#include <stdlib.h>

void setUp() {
    gff_init();
    gff_load_directory("ds1");
}

void tearDown() {
    gff_cleanup();
}

void test_create(void) {
    animation_list_t *al = animation_list_create();
    animate_sprite_t *as = malloc(sizeof(animate_sprite_t));
    animation_node_t *an = animation_list_add(al, as);
    TEST_ASSERT(an != NULL);
    TEST_ASSERT(al->head == an);
    animation_node_free(animation_list_remove(al, an));
    TEST_ASSERT(al->head == NULL);
    animation_list_free(al);
    free(as);
}

void test_create_bounds(void) {
    animation_list_t *al = animation_list_create();
    animate_sprite_t *as = calloc(1, sizeof(animate_sprite_t));
    animation_node_t *an = animation_list_add(al, as);
    TEST_ASSERT(al->head == an);
    animation_node_t *an2 = animation_list_add(al, as);
    TEST_ASSERT(an != NULL);
    TEST_ASSERT(al->head == an2);
    animation_node_free(animation_list_remove(al, an));
    TEST_ASSERT(al->head == an2);
    an = animation_list_add(al, as);
    TEST_ASSERT(al->head == an);
    animation_node_t *an3 = animation_list_add(al, as);
    TEST_ASSERT(al->head == an3);
    animation_node_free(animation_list_remove(al, an));
    TEST_ASSERT(al->head == an3);
    animation_node_free(animation_list_remove(al, an3));
    TEST_ASSERT(al->head == an2);
    animation_node_free(animation_list_remove(al, an2));
    TEST_ASSERT(al->head == NULL);
    animation_list_free(al);
    free(as);
}

static void animation_list_fix_head(animation_list_t *al) {
    if (!al || !al->head) { return; }

    while (al->head->prev) {
        al->head = al->head->prev;
    }
}

void test_swap(void) {
    animate_sprite_t *as[3];
    animation_node_t *an[3];
    //entity_t *dude[3];
    animation_list_t *al = animation_list_create();

    as[0] = calloc(1, sizeof(animate_sprite_t));
    as[1] = calloc(1, sizeof(animate_sprite_t));
    as[2] = calloc(1, sizeof(animate_sprite_t));

    an[0] = animation_list_add(al, as[0]);
    an[1] = animation_list_add(al, as[1]);
    an[2] = animation_list_add(al, as[2]);

    an[0]->anim->entity = entity_create_fake(0, 0);
    an[1]->anim->entity = entity_create_fake(0, 0);
    an[2]->anim->entity = entity_create_fake(0, 0);

    as[0]->entity->mapy = 30;
    as[1]->entity->mapy = 20;
    as[2]->entity->mapy = 10;

    animation_shift_node(an[2]);
    TEST_ASSERT(al->head == an[2]);

    as[0]->entity->mapy = 0;
    animation_shift_node(an[0]);
    animation_list_fix_head(al);
    TEST_ASSERT(al->head == an[0]);

    as[0]->entity->mapy = 10;
    as[0]->spr = 10;
    as[2]->spr = 0;
    animation_shift_node(an[0]);
    animation_list_fix_head(al);
    TEST_ASSERT(al->head == an[2]);

    entity_free(an[0]->anim->entity);
    entity_free(an[1]->anim->entity);
    entity_free(an[2]->anim->entity);
    an[0]->anim->entity = an[1]->anim->entity = an[2]->anim->entity = NULL;

    animation_node_free(animation_list_remove(al, an[0]));
    animation_node_free(animation_list_remove(al, an[1]));
    animation_node_free(animation_list_remove(al, an[2]));
    free(as[0]); free(as[1]); free(as[2]);
    animation_list_free(al);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create);
    RUN_TEST(test_create_bounds);
    RUN_TEST(test_swap);
    return UNITY_END();
}
