#include "animation.h"
#include "port.h"
#include <math.h>

// the shift function can put the list head in the incorrect spot.
// This function fixes that.
static void animation_list_fix_head(animation_list_t *al);

animation_list_t* animation_list_create() {
    return calloc(1, sizeof(animation_list_t));
}

void animation_list_free(animation_list_t *al) {
    if (!al) { return; }

    while (al->head) { animation_node_free(animation_list_remove(al, al->head)); }

    free(al);
}

animate_sprite_node_t* animate_sprite_node_create() {
    animate_sprite_node_t *ret = calloc(1, sizeof(animate_sprite_node_t));
    if (!ret) { return NULL; }
    ret->anim = calloc(1, sizeof(animate_sprite_t));
    if (!ret->anim) { free(ret); return NULL;}
    return ret;
}

void animation_node_free(animation_node_t *node) {
    free(node);
}

animation_node_t* animation_list_find(animation_list_t *al, animate_sprite_t *anim) {
    for (animation_node_t *rover = al ? al->head : NULL; rover; rover = rover->next) {
        if (rover->anim == anim) { return rover; }
    }
    return NULL;
}

animation_node_t* animation_list_add(animation_list_t *al, animate_sprite_t *anim) {
    animation_node_t *to_add;
    if (!al || !anim) { return NULL; }

    animation_list_fix_head(al);

    to_add = malloc(sizeof(animation_node_t));
    to_add->prev = NULL;
    to_add->next = al->head;
    to_add->anim = anim;

    if (al->head == NULL) {
        return (al->head = to_add);
    }

    al->head->prev = to_add;
    al->head = to_add;
    animation_shift_node(to_add);

    // The shift can change the head, so lets update it.
    while (al->head->prev) { al->head = al->head->prev; }

    return to_add;
}

static animation_node_t* animation_node_normal_remove(animation_node_t *node) {
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }

    node->next = node->prev = NULL;

    return node;
}

animation_node_t* animation_list_remove(animation_list_t *al, animation_node_t *node) {
    if (!al || !node) { return NULL; }

    animation_list_fix_head(al);

    if (al->head == node) {
        al->head = al->head->next;
        if (al->head) {
            al->head->prev = NULL;
        }
        node->next = node->prev = NULL;
        return node;
    }

    return animation_node_normal_remove(node);
}

// The assembly didn't make sense, so I'm taking a guess...
static int is_less(entity_t *e0, entity_t *e1) {
    int map0y = e0->mapy; // * 16 + a0->entity->sprite.yoffset)
    int map1y = e1->mapy; // * 16 + a1->entity->sprite.yoffset)

    if (e0->mapz != e1->mapz) {
        return e0->mapz > e1->mapz;
    }

    if (map0y == map1y) {
        map0y = port_sprite_geth(e0->anim.spr);
        map1y = port_sprite_geth(e0->anim.spr);
    }

    return map0y < map1y;
}

// This function assumes an AND an->next are valid!
static void swap_with_next(entity_list_node_t *en) {
    entity_list_node_t *next = en->next;
    entity_list_node_t *prev = en->prev;

    if (next->next) { next->next->prev = en; }

    next->prev = prev;
    en->next = next->next;
    en->prev = next;
    next->next = en;

    if (prev) { prev->next = next; }
}

static void animation_list_fix_head(animation_list_t *al) {
    if (!al || !al->head) { return; }

    while (al->head->prev) {
        al->head = al->head->prev;
    }
}

// WARNING: This may change the head of the animation list!
void animation_shift_node(animate_sprite_node_t *an) {
    exit(1);
}

void animate_sprite_tick(entity_action_t *action, entity_t *entity ) {
    if (!entity || !action) { return; }
    animate_sprite_t *anim = &entity->anim;

    anim->left_over += fmod(anim->movex ? anim->movex : anim->movey, 1.0);
    float movex_amt = floor(anim->movex + anim->left_over);
    float movey_amt = floor(anim->movey + anim->left_over);
    anim->left_over = fmod(anim->left_over, 1.0);

    //if (entity->name) { printf("anim: %s (%f, %f)\n", entity->name, anim->movex, anim->movey); }
    //if (entity->name) { printf("anim: %s (%f, %f) (%d, %d) -> ", entity->name, movex_amt, movey_amt, anim->x, anim->y); }
    if (anim->x < anim->destx) { anim->x += movex_amt; }
    if (anim->y < anim->desty) { anim->y += movey_amt; }
    if (anim->x > anim->destx) { anim->x -= movex_amt; }
    if (anim->y > anim->desty) { anim->y -= movey_amt; }
    //if (entity->name) { printf("(%d %d)\n", anim->x, anim->y); }
}

extern void animation_shift_entity(entity_list_t *list, entity_list_node_t *en) {
    if (!en || !list) { return; }

    while (en->next && is_less(en->next->entity, en->entity)) {
        swap_with_next(en);
    }

    while (en->prev && is_less(en->entity, en->prev->entity)) {
        swap_with_next(en->prev);
    }
    
    while(list->head->prev) { list->head = list->head->prev; }
}
