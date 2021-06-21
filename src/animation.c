#include "animation.h"
#include "port.h"

// the shift function can put the list head int he incorrect spot.
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

// The assembly makes even less sense...
static int is_less(animate_sprite_t *a0, animate_sprite_t *a1) {
    if (a0 && a0->entity && a1 && a1->entity) {
        if (a0->entity->mapz != a1->entity->mapz) {
            return a0->entity->mapz < a1->entity->mapz;
        }
    }
    int map0y = (a0 && a0->entity)
        ? (a0->entity->mapy)// * 16 + a0->entity->sprite.yoffset)
        : 0;

    int map1y = (a1 && a1->entity)
        ? (a1->entity->mapy)// * 16 + a1->entity->sprite.yoffset)
        : 0;

    if (map0y == map1y && a0 && a0->entity && a1 && a1->entity) {
        //map0y = -a0->entity->sprite.yoffset;
        //map1y = -a1->entity->sprite.yoffset;
        map0y = port_sprite_geth(a0->spr);
        map1y = port_sprite_geth(a1->spr);
    }

    return map0y < map1y;
}

// This function assumes an AND an->next are valid!
static void swap_with_next(animate_sprite_node_t *an) {
    animate_sprite_node_t *next = an->next;
    animate_sprite_node_t *prev = an->prev;

    if (next->next) { next->next->prev = an; }

    next->prev = prev;
    an->next = next->next;
    an->prev = next;
    next->next = an;

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
    while (an && an->next && is_less(an->next->anim, an->anim)) {
        swap_with_next(an);
    }

    while (an && an->prev && is_less(an->anim, an->prev->anim)) {
        swap_with_next(an->prev);
    }
}

