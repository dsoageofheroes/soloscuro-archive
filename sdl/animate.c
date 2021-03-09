#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "animate.h"
#include "gameloop.h"
#include "sprite.h"
#include "../src/dsl.h"
#include "../src/ds-scmd.h"

#define MAX_ZPOS (128)

static animate_sprite_node_t *animate_list[MAX_ZPOS];

// Return if we need to flip
static SDL_RendererFlip animate_tick(animate_sprite_t *anim, const uint32_t xoffset, const uint32_t yoffset) {
    size_t pos = anim->pos;
    SDL_RendererFlip flip = 0;

    sprite_set_frame(anim->spr, anim->scmd[pos].bmp_idx);
    if (anim->scmd[pos].flags & SCMD_LAST) { goto out; }

    if (anim->scmd[pos].flags & SCMD_JUMP && anim->delay == 0) {
        pos = anim->pos = 0;
        sprite_set_frame(anim->spr, anim->scmd[pos].bmp_idx);
        anim->delay = anim->scmd[pos].delay;
    }

    if (anim->delay == 0) {
        anim->pos++; pos++;
        sprite_set_frame(anim->spr, anim->scmd[pos].bmp_idx);
        anim->delay = anim->scmd[pos].delay;
        goto out;
    }

    while (anim->delay == 0 && pos > 0) {
        anim->pos++; pos++;
        if (pos >= SCMD_MAX_SIZE) {
            anim->pos = pos = 0;
        }
    }

    anim->delay--;

out:
    if (anim->scmd[pos].flags & SCMD_XMIRROR
        || (anim->entity && anim->entity->sprite.flags & 0x80)) {
        flip |= SDL_FLIP_HORIZONTAL;
    }
    if (anim->scmd[pos].flags & SCMD_YMIRROR) {
        flip |= SDL_FLIP_VERTICAL;
    }

    anim->left_over += fmod(anim->move, 1.0);
    float move_amt = floor(anim->move + anim->left_over);
    anim->left_over = fmod(anim->left_over, 1.0);

    if (anim->x < anim->destx) { anim->x += move_amt; }
    if (anim->y < anim->desty) { anim->y += move_amt; }
    if (anim->x > anim->destx) { anim->x -= move_amt; }
    if (anim->y > anim->desty) { anim->y -= move_amt; }

    //printf("%d: anim->x = %d (move_amt = %f)\n", anim->obj->combat_id, anim->x, move_amt);
    sprite_set_location(anim->spr,
        anim->x - xoffset, // + scmd_xoffset,
        anim->y - yoffset); // + anim->scmd->yoffset);

    return flip;
}

void animate_list_render(SDL_Renderer *renderer) {
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();

    // TODO: Definite performance issues.
    for (int i = 0; i < MAX_ZPOS; i++) {
        for (animate_sprite_node_t *rover = animate_list[i]; rover != NULL; rover = rover->next) {
            sprite_render_flip(renderer, rover->anim->spr, animate_tick(rover->anim, xoffset, yoffset));
        }
    }
}

// The assembly makes even less sense...
static int is_less(animate_sprite_t *a0, animate_sprite_t *a1) {
    const int map0y = a0->entity
        ? (a0->entity->mapy * 16 - a0->entity->sprite.yoffset)
        //: (a0->obj)
        //? ((a0->obj->mapy + 1) * 16 + a0->obj->yoffset)
        : 9999;

    const int map1y = a1->entity
        ? (a1->entity->mapy * 16 - a1->entity->sprite.yoffset)
        //: (a1->obj)
        //? ((a1->obj->mapy + 1) * 16 + a1->obj->yoffset)
        : 9999;


    /*
    const int map0y = a0->entity
        //? ((a0->obj->mapy * zoom) - sprite_geth(a0->spr) )
        //? ((a0->obj->mapy) * zoom) - ((sprite_geth(a0->spr) - 16*zoom) / 2)
        ? ((sprite_geth(a0->spr)))
        //? ((a0->obj->mapy) * zoom)
        : -9999;
    const int map1y = a1->entity
        //? ((a1->obj->mapy * zoom) - sprite_geth(a0->spr) )
        //? ((a1->obj->mapy) * zoom) - ((sprite_geth(a1->spr) - 16*zoom) / 2)
        ? ((sprite_geth(a1->spr)))
        //? ((a1->obj->mapy) * zoom)
        : -9999;
        */
    //printf("%p, %p\n", a0->obj, a1->obj);
    //printf("%d, %d\n", map0y, map1y);
    //printf("%p, %p\n", a0->entity, a1->entity);
    return map0y < map1y;
}

static void swap_with_next(animate_sprite_node_t *an, const int zpos) {
    animate_sprite_node_t *next = an->next;
    animate_sprite_node_t *prev = an->prev;
    if (next->next) { next->next->prev = an; }
    next->prev = prev;
    an->next = next->next;
    an->prev = next;
    next->next = an;
    if (prev) { prev->next = next; }
    if (!prev) { animate_list[zpos] = next; }
}

void animate_shift_node(animate_sprite_node_t *an, const int zpos) {

    while (an && an->next && !is_less(an->anim, an->next->anim)) {
        swap_with_next(an, zpos);
    }

    while (an && an->prev && is_less(an->anim, an->prev->anim)) {
        swap_with_next(an->prev, zpos);
    }
}

void animate_list_remove(animate_sprite_node_t *an, const int zpos) {
    animate_sprite_node_t *next = an->next;
    animate_sprite_node_t *prev = an->prev;

    if (next) {
        next->prev = prev;
    }

    if (prev) {
        prev->next = next;
    } else {
        animate_list[zpos] = next;
    }

    free(an);
}

animate_sprite_node_t *animate_list_add(animate_sprite_t *anim, const int zpos) {
    if (zpos < 0 || zpos >= MAX_ZPOS) {
        error("zpos is beyond range!");
        return NULL;
    }
    animate_sprite_node_t *node = malloc(sizeof(animate_sprite_node_t));
    node->anim = anim;
    node->next = animate_list[zpos];
    if (animate_list[zpos]) {
        animate_list[zpos]->prev = node;
    }
    node->prev = NULL;;
    sprite_set_frame(anim->spr, anim->scmd->bmp_idx);
    animate_list[zpos] = node;
    animate_shift_node(animate_list[zpos], zpos);

    return node;
}

void animate_set_animation(animate_sprite_t *as, scmd_t *scmd, const uint32_t ticks_per_move) {
    //printf("%d -> %d, %d -> %d\n", as->x, as->destx, as->y, as->desty);
    const int diffx = abs(as->x - as->destx);
    const int diffy = abs(as->y - as->desty);
    const float distance = sqrt(diffx * diffx + diffy * diffy);

    as->scmd = scmd;
    as->pos = 0;
    as->move = distance == 0 ? 0 : distance / ((float)ticks_per_move * 2);

    sprite_set_frame(as->spr, as->scmd->bmp_idx);
}

void animate_init() {
    memset(animate_list, 0x0, sizeof(animate_sprite_node_t*) * MAX_ZPOS);
}

void animate_clear() {
    for (int i = 0; i < MAX_ZPOS; i++) {
        while(animate_list[i]) {
            if (animate_list[i]) {
                animate_list_remove(animate_list[i], i);
            }
            animate_list[i] = NULL;
        }
    }
}

void animate_close() {
    animate_clear();
    memset(animate_list, 0x0, sizeof(animate_sprite_node_t*) * MAX_ZPOS);
}

void port_animate_entity(entity_t *dude) {
    animate_sprite_t *anim = dude->sprite.data;

    // In case we are in the middle of an animation, skip past it.
    while (! (anim->scmd[anim->pos].flags & SCMD_LAST) && anim->pos < SCMD_MAX_SIZE) {
        anim->pos++;
    }
    if (anim->pos == SCMD_MAX_SIZE) { anim->pos = 0; }
    // No increment and loop if needed
    anim->pos++;
    while (! (anim->scmd[anim->pos].flags & SCMD_LAST) && anim->pos < SCMD_MAX_SIZE) {
        anim->pos++;
    }
    if (anim->pos == SCMD_MAX_SIZE) { anim->pos = 0; }
}
