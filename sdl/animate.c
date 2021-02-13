#include <string.h>
#include <stdlib.h>
#include "animate.h"
#include "gameloop.h"
#include "sprite.h"
#include "../src/dsl.h"
#include "../src/ds-scmd.h"

#define MAX_ZPOS (128)

static animate_sprite_node_t *animate_list[MAX_ZPOS];

// Return if we need to flip
static SDL_RendererFlip animate_tick(animate_sprite_t *anim, const uint32_t xoffset, const uint32_t yoffset) {
//void sprite_render_flip(SDL_Renderer *renderer, const uint16_t sprite_id, SDL_RendererFlip flip);
    size_t pos = anim->pos;
    SDL_RendererFlip flip = 0;

    if (anim->scmd[pos].flags & SCMD_LAST) { goto out; }
    if (anim->scmd[pos].flags & SCMD_JUMP && anim->delay == 0) {
        pos = anim->pos = 0;
        sprite_set_frame(anim->spr, anim->scmd[pos].bmp_idx);
        anim->delay = anim->scmd[pos].delay;
    }
    //if (anim->scmd[pos].delay == 0) {
    if (anim->delay == 0) {
        anim->pos++; pos++;
        sprite_set_frame(anim->spr, anim->scmd[pos].bmp_idx);
        anim->delay = anim->scmd[pos].delay;
        goto out;
    }

    anim->delay--;

out:
    if (anim->scmd[pos].flags & SCMD_XMIRROR) {
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

    //printf("anim->x = %d (move_amt = %f)\n", anim->x, move_amt);
    sprite_set_location(anim->spr,
        anim->x - xoffset + anim->scmd->xoffset,
        anim->y - yoffset + anim->scmd->yoffset);

    return flip;
}

void animate_list_render(SDL_Renderer *renderer) {
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();

    // TODO: Definite performance issues.
    for (int i = 0; i < MAX_ZPOS; i++) {
        for (animate_sprite_node_t *rover = animate_list[i]; rover != NULL; rover = rover->next) {
            //printf("    (%d, %d, %d)\n", rover->anim->x, rover->anim->y, i);
            sprite_render_flip(renderer, rover->anim->spr, animate_tick(rover->anim, xoffset, yoffset));
        }
    }
}

void animate_shift_node(animate_sprite_node_t *an, const int zpos) {
    animate_sprite_node_t *prev, *next;

    //while (an && an->next && an->anim->y > an->next->anim->y) {
    while (an && an->next && !sprite_is_under(an->anim->spr, an->next->anim->spr)) {
        printf("Shift up\n");
        next = an->next;
        prev = an->prev;
        if (next->next) { next->next->prev = an; }
        next->prev = prev;
        an->next = next->next;
        an->prev = next;
        next->next = an;
        if (prev) { prev->next = next; }
        if (!prev) { animate_list[zpos] = next; }
    }
    //while (an && an->prev && an->anim->y < an->prev->anim->y) {
    while (an && an->prev && sprite_is_under(an->anim->spr, an->prev->anim->spr)) {
        printf("Shift down\n");
        next = an->next;
        prev = an->prev;
        if (prev->prev) { prev->prev->next = an; }
        an->prev = prev->prev;
        an->next = prev;
        prev->next = next;
        prev->prev = an;
        if (next) { next->prev = prev; }
        if (!an->prev) { animate_list[zpos] = an; }
    }
}

animate_sprite_node_t *animate_list_add(animate_sprite_t *anim, const int zpos) {
    animate_sprite_node_t *node = malloc(sizeof(animate_sprite_node_t));
    if (zpos < 0 || zpos >= MAX_ZPOS) {
        error("zpos is beyond range!");
        return NULL;
    }
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

//static animation_delay_t *animations[MAX_DELAY];

void animate_init() {
    memset(animate_list, 0x0, sizeof(animate_sprite_node_t*) * MAX_ZPOS);
}

void animate_clear() {
}

void animate_close() {
    animate_clear();
}
