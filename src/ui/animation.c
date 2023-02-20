#include "animation.h"
#include "port.h"
#include <math.h>

// The assembly didn't make sense, so I'm taking a guess...
static int is_less(sol_entity_t *e0, sol_entity_t *e1) {
    int map0y = e0->mapy; // * 16 + a0->entity->sprite.yoffset)
    int map1y = e1->mapy; // * 16 + a1->entity->sprite.yoffset)
    sol_sprite_info_t info;

    if (e0->mapz != e1->mapz) {
        return e0->mapz > e1->mapz;
    }

    if (map0y == map1y) {
        sol_status_check(sol_sprite_get_info(e0->anim.spr, &info), "Unable to get entity0 sprite info");
        map0y = info.y - info.h;
        sol_status_check(sol_sprite_get_info(e1->anim.spr, &info), "Unable to get entity1 sprite info");
        map1y = info.y - info.h;
    }

    return map0y < map1y;
}

// This function assumes an AND an->next are valid!
static void swap_with_next(sol_entity_list_node_t *en) {
    sol_entity_list_node_t *next = en->next;
    sol_entity_list_node_t *prev = en->prev;

    if (next->next) { next->next->prev = en; }

    next->prev = prev;
    en->next = next->next;
    en->prev = next;
    next->next = en;

    if (prev) { prev->next = next; }
}

// WARNING: This may change the head of the animation list!
extern sol_status_t sol_animate_sprite_tick(sol_entity_action_t *action, sol_entity_t *entity ) {
    if (!entity || !action)        { return SOL_NULL_ARGUMENT; }
    if (action->action == EA_SCMD) { return SOL_SUCCESS; }
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
    return SOL_SUCCESS;
}

extern sol_status_t sol_animate_shift_entity(sol_entity_list_t *list, sol_entity_list_node_t *en) {
    if (!en || !list) { return SOL_NULL_ARGUMENT; }

    while (en->next && is_less(en->next->entity, en->entity)) {
        swap_with_next(en);
    }

    while (en->prev && is_less(en->entity, en->prev->entity)) {
        swap_with_next(en->prev);
    }
    
    while(list->head->prev) { list->head = list->head->prev; }

    return SOL_SUCCESS;
}
