#include "combat-status.h"
#include "popup.h"
#include "narrate.h"
#include "../animate.h"
#include "../main.h"
#include "../sprite.h"
#include "../font.h"
#include "../../src/gff.h"
#include "../../src/port.h"
#include "../../src/gfftypes.h"
#include "../../src/region-manager.h"
#include <string.h>

static uint16_t background;
static combat_status_t combat_status;
static uint32_t xoffset, yoffset;
static uint16_t combat_attacks;
static int show_attack = 0;
static int damage_amount = 0;

typedef struct animate_sprite_node_list_s {
    animate_sprite_node_t *an;
    int cycles;
    struct animate_sprite_node_list_s *next;
} animate_sprite_node_list_t;

animate_sprite_node_list_t *list = NULL;

static void add_node_list(animate_sprite_node_t *an) {
    animate_sprite_node_list_t *rover = list;
    animate_sprite_node_list_t *toadd = calloc(1, sizeof(animate_sprite_node_list_t));
    toadd->an = an;
    if (!list) {
        list = toadd;
        animate_list_node_add(an, 100);
        return;
    }
    while(rover->next) { rover = rover->next; }
    rover->next = toadd;
}

static void pop_list() {
    if (!list) { return; }
    animate_list_remove(list->an, 100);
    animate_sprite_node_list_t *delme = list;
    list = list->next;
    free(delme);
    if (list && list->an) {
        animate_list_node_add(list->an, 100);
    }
}

combat_status_t* combat_status_get() { return &combat_status; }

#define MAX_STATUS 9
const static char *statuses[] = {
    "New",
    "Okay",
    "Stunned",
    "Out Cold",
    "Dying",
    "Animated",
    "Petrified",
    "Dead",
    "Gone"
};

void combat_status_init(SDL_Renderer *renderer, const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = main_get_zoom();
    xoffset = main_get_width() - 100 * main_get_zoom();
    yoffset = 5 * main_get_zoom();

    background = sprite_new(renderer, pal, 0 + xoffset / main_get_zoom(), 0 + yoffset / main_get_zoom(), zoom, RESOURCE_GFF_INDEX, GFF_BMP, 5016);
    combat_attacks = sprite_new(renderer, pal, 0, 0, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 5014);
}

static void get_status() {
    entity_t* dude = combat_get_current(&(region_manager_get_current()->cr));

    if (dude) {
        strcpy(combat_status.name, dude->name);
        combat_status.current_hp = dude->stats.hp;
        combat_status.max_hp = dude->stats.high_hp;
        combat_status.status = 1;// Need to fix.
        combat_status.move = dude->stats.move;
    }
}

static int count = 30;

void combat_status_render(void *data, SDL_Renderer *renderer) {
    const float zoom = main_get_zoom();
    const int delta = 4 * zoom;
    SDL_Rect loc;
    char buf[128];

    if (list && list->an) {
        count--;
        // Static animation, needs ot move to location
        if (list->an->anim->scmd->flags & SCMD_LAST) {
            if (abs(list->an->anim->x - list->an->anim->destx) < delta
                    && abs(list->an->anim->y == list->an->anim->desty) < delta) {
                pop_list();
            }
        } else if (list->an->anim->scmd[list->an->anim->pos].flags & SCMD_JUMP) {
            list->cycles++;
        } else if (list->cycles) {
            pop_list();
            count = 30;
        }
    }

    if (combat_player_turn() == NO_COMBAT) { return; }

    sprite_render(renderer, background);
    get_status();

    loc.x = xoffset;
    loc.y = yoffset + 2 * zoom;
    loc.w = sprite_getw(background);

    font_render_center(renderer, FONT_GREYLIGHT, combat_status.name, loc);
    snprintf(buf, 127, "%d/%d", combat_status.current_hp, combat_status.max_hp);
    loc.y += 6 * zoom;
    font_render_center(renderer, FONT_GREYLIGHT, buf, loc);
    loc.y += 6 * zoom;
    if (combat_status.status >= 0 && combat_status.status < MAX_STATUS) {
        font_render_center(renderer, FONT_GREYLIGHT, statuses[combat_status.status], loc);
    }
    loc.y += 6 * zoom;
    snprintf(buf, 127, "Move : %d", combat_status.move);
    font_render_center(renderer, FONT_GREYLIGHT, buf, loc);

    if (show_attack) {
        sprite_render(renderer, combat_attacks);
        loc.x = sprite_getx(combat_attacks);
        loc.y = sprite_gety(combat_attacks)
            + sprite_geth(combat_attacks) / 2
            - 8 / 2 * main_get_zoom(); // last one is font size / 2
        loc.w = sprite_getw(combat_attacks);
        snprintf(buf, 128, "%d", damage_amount);
        font_render_center(renderer, FONT_GREYLIGHT, buf, loc);
    }
}

enum {
    DIRECTION_U, // up
    DIRECTION_UUR, // up up right
    DIRECTION_UR, // up right
    DIRECTION_URR, // up right up
    DIRECTION_R, // right
    DIRECTION_RRD, // right right down
    DIRECTION_RD, // right down
    DIRECTION_RDD, // right down down
    DIRECTION_D, // down
    DIRECTION_DDL, // down down left
    DIRECTION_DL, // down left
    DIRECTION_DLL, // down left left
    DIRECTION_L, // left
    DIRECTION_LLU, // left left up
    DIRECTION_LU, // left up
    DIRECTION_LUU, // left up up
};

int get_direction(entity_t *source, entity_t *target) {
    int diffx = source->mapx - target->mapx;
    int diffy = source->mapy - target->mapy;

    if (diffx == 0) {
        if (diffy < 0) { return DIRECTION_D; }
        return DIRECTION_U;
    } else if(diffy == 0) {
        if (diffx < 0) { return DIRECTION_R; }
        return DIRECTION_L;
    } else if (diffx < 0) {
        if (diffy < 0) { return DIRECTION_RD; }
        return DIRECTION_UR;
    } else if (diffx > 0) {
        if (diffy < 0) { return DIRECTION_DL; }
        return DIRECTION_LU;
    }

    return DIRECTION_RDD;
}

void port_combat_action(entity_action_t *ca) {
    if (ca->action == EA_RED_DAMAGE) {
        animate_sprite_node_t *asn = ca->target->sprite.data;
        sprite_center_spr(combat_attacks, asn->anim->spr);
        show_attack = 1;
        damage_amount = ca->amt;
        return;
    } else if (ca->action == EA_POWER_CAST) {
        animate_sprite_node_t *source = ca->source->sprite.data;
        animate_sprite_node_t *cast = ca->power->cast.data;
        sprite_center_spr(cast->anim->spr, source->anim->spr);
        cast->anim->x = sprite_getx(cast->anim->spr) + getCameraX();
        cast->anim->y = sprite_gety(cast->anim->spr) + getCameraY();
        cast->anim->destx = cast->anim->x;
        cast->anim->desty = cast->anim->y;
        cast->anim->scmd = combat_get_scmd(COMBAT_POWER_CAST);
        add_node_list(cast);
    } else if (ca->action == EA_POWER_THROW) {
        int dir = get_direction(ca->source, ca->target);
        animate_sprite_node_t *throw = ca->power->thrown.data;
        animate_sprite_node_t *source = ca->source->sprite.data;
        animate_sprite_node_t *dest = ca->target->sprite.data;
        throw->anim->scmd = combat_get_scmd(COMBAT_POWER_THROW_STATIC_U + dir);
        throw->anim->x = sprite_getx(source->anim->spr) + getCameraX();
        throw->anim->y = sprite_gety(source->anim->spr) + getCameraY();
        throw->anim->destx = sprite_getx(dest->anim->spr) + sprite_getw(dest->anim->spr) / 2 + getCameraX();
        throw->anim->desty = sprite_gety(dest->anim->spr) + sprite_geth(dest->anim->spr) / 2 + getCameraY();
        //printf("(%d, %d) -> (%d, %d)\n", throw->anim->x, throw->anim->y,
            //throw->anim->destx, throw->anim->desty);
        throw->anim->movex = abs(throw->anim->destx - throw->anim->x) / 30;
        throw->anim->movey = abs(throw->anim->desty - throw->anim->y) / 30;
        add_node_list(throw);
    } else if (ca->action == EA_POWER_HIT) {
        animate_sprite_node_t *dest = ca->target->sprite.data;
        animate_sprite_node_t *hit = ca->power->hit.data;
        sprite_center_spr(hit->anim->spr, dest->anim->spr);
        hit->anim->x = sprite_getx(hit->anim->spr) + getCameraX();
        hit->anim->y = sprite_gety(hit->anim->spr) + getCameraY();
        hit->anim->destx = hit->anim->x;
        hit->anim->desty = hit->anim->y;
        hit->anim->scmd = combat_get_scmd(COMBAT_POWER_CAST);
        add_node_list(hit);
    }

    show_attack = 0;
}

int combat_status_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    return 0;// did not handle, we need to let the map handle movement
}

int combat_status_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    //return 1; // means I captured the mouse click
    return 0; // zero means I did not handle the mouse click, so another screen may.
}

int combat_status_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    //return 1; // means I captured the mouse click
    return 0; // zero means I did not handle the mouse click, so another screen may.
}

void combat_status_free() {
    sprite_free(background);
}

sops_t combat_status_screen = {
    .init = combat_status_init,
    .cleanup = combat_status_free,
    .render = combat_status_render,
    .mouse_movement = combat_status_handle_mouse_movement,
    .mouse_down = combat_status_handle_mouse_down,
    .mouse_up = combat_status_handle_mouse_up,
    .grey_out_map = 0,
    .data = NULL
};
