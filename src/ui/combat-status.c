#include "audio.h"
#include "combat-status.h"
#include "popup.h"
#include "narrate.h"
#include "gfftypes.h"
#include "gff.h"
#include "../../src/port.h"
#include "../../src/region-manager.h"
#include "../../src/settings.h"
#include "../../src/port.h"
#include <string.h>

static sol_sprite_t background, combat_attacks;
static combat_status_t combat_status;
static uint32_t xoffset, yoffset;
static int show_attack = 0;
static int damage_amount = 0;

typedef struct animate_sprite_node_list_s {
    animate_sprite_node_t *an;
    power_t *power;
    entity_t *source;
    entity_t *target;
    enum entity_action_e action;
    int cycles;
    int is_moving;
    uint16_t sound;
    struct animate_sprite_node_list_s *next;
} animate_sprite_node_list_t;

animate_sprite_node_list_t *list = NULL;

static void start_node() {
    if (!list || !list->an) { return; }
    //animate_list_node_add(list->an, 100);
    sol_audio_play_voc(RESOURCE_GFF_INDEX, GFF_BVOC, list->sound, 1.0);
}

/*
static void add_node_list(animate_sprite_node_t *an, uint16_t sound, entity_action_t *ca) {
    // DO NOT STORE CA!!!!!!!!!!!!
    animate_sprite_node_list_t *rover = list;
    animate_sprite_node_list_t *toadd = calloc(1, sizeof(animate_sprite_node_list_t));
    toadd->an = an;
    toadd->sound = sound;
    toadd->target = ca->target;
    toadd->source = ca->source;
    toadd->power = ca->power;
    toadd->action = ca->action;
    toadd->is_moving = (an->anim->x != an->anim->destx) || (an->anim->y != an->anim->desty);
    if (!list) {
        list = toadd;
        start_node();
        return;
    }
    while(rover->next) { rover = rover->next; }
    rover->next = toadd;
}
*/

static void clear() {
    //sprite_set_location(combat_attacks, -1000, -1000);
    show_attack = 0;
}

static void pop_list() {
    if (!list) { return; }
    //animate_list_remove(list->an, 100);
    animate_sprite_node_list_t *delme = list;
    list = list->next;

    if (entity_is_fake(delme->target)) {
        entity_free(delme->target);
    }

    if (delme->action == EA_POWER_HIT) {
        power_instance_t pi;
        memset(&pi, 0x0, sizeof(power_instance_t));
        pi.entity = delme->target;
        pi.stats = delme->power;
        delme->power->actions.apply(&pi, delme->target);
    }

    free(delme);
    start_node();
}

combat_status_t* sol_combat_status_get() { return &combat_status; }

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

void combat_status_init(const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = settings_zoom();
    xoffset = settings_screen_width() - 100 * settings_zoom();
    yoffset = 5 * settings_zoom();

    background = sol_sprite_new(pal, 0 + xoffset / settings_zoom(), 0 + yoffset / settings_zoom(), zoom, RESOURCE_GFF_INDEX, GFF_BMP, 5016);
    combat_attacks = sol_sprite_new(pal, 0, 0, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 5014);
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

void combat_status_render(void *data) {
    const float zoom = settings_zoom();
    const int delta = 5 * zoom;
    sol_dim_t loc;
    char buf[128];
    static int last_action = 0;

    // Okay this can be confusing.
    // The if decides when to proceed to the next action on the combat list
    // list: List of actions
    // last_action: a flag so we want a full round before clearing the GUI.
    if ((list && list->an) || last_action) {
        count--;
        // Static animation, needs to move to location
        if (list && list->an && list->is_moving) {
            if (abs(list->an->anim->x - list->an->anim->destx) < delta
                    && abs(list->an->anim->y == list->an->anim->desty) < delta) {
                pop_list();
            }
        // animation, we want until animation complete (after SCMD_JUMP)
        } else if (list && list->an && list->an->anim->scmd[list->an->anim->pos].flags & SCMD_JUMP) {
            list->cycles++;
        } else if (list && list->cycles) {
            pop_list();
        }

        if (last_action == 0 && (!list || !list->an)) {
            last_action = 1;
            count = 30;
        };

        if (last_action && count == 0) {
            clear();
            last_action = 0;
        }

        if (count <= 0) {
            count = 30;
        }
    }

    if (combat_player_turn() == NO_COMBAT) { return; }

    if (2 == 1) {
        sol_draw_cone(100, 100, 200);
    }

    sol_sprite_render(background);
    get_status();

    loc.x = xoffset;
    loc.y = yoffset + 2 * zoom;
    loc.w = sol_sprite_getw(background);

    sol_font_render_center(FONT_GREYLIGHT, combat_status.name, loc.x, loc.y, loc.w);
    snprintf(buf, 127, "%d/%d", combat_status.current_hp, combat_status.max_hp);
    loc.y += 6 * zoom;
    sol_font_render_center(FONT_GREYLIGHT, buf, loc.x, loc.y, loc.w);
    loc.y += 6 * zoom;
    if (combat_status.status >= 0 && combat_status.status < MAX_STATUS) {
        sol_font_render_center(FONT_GREYLIGHT, statuses[combat_status.status], loc.x, loc.y, loc.w);
    }
    loc.y += 6 * zoom;
    snprintf(buf, 127, "Move : %d", combat_status.move);
    sol_font_render_center(FONT_GREYLIGHT, buf, loc.x, loc.y, loc.w);

    if (show_attack) {
        sol_sprite_render(combat_attacks);
        loc.x = sol_sprite_getx(combat_attacks);
        loc.y = sol_sprite_gety(combat_attacks)
            + sol_sprite_geth(combat_attacks) / 2
            - 8 / 2 * settings_zoom(); // last one is font size / 2
        loc.w = sol_sprite_getw(combat_attacks);
        snprintf(buf, 128, "%d", damage_amount);
        sol_font_render_center(FONT_GREYLIGHT, buf, loc.x, loc.y, loc.w);
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
        if (diffy < 0) { 
            if (abs(diffx) < abs(diffy)) {
                return DIRECTION_RDD;
            } else if (abs(diffx) > abs(diffy)) {
                return DIRECTION_RRD;
            }
            return DIRECTION_RD;
        }
        if (abs(diffx) < abs(diffy)) {
            return DIRECTION_UUR;
        } else if (abs(diffx) > abs(diffy)) {
            return DIRECTION_URR;
        }
        return DIRECTION_UR;
    } else if (diffx > 0) {
        if (diffy < 0) {
            if (abs(diffx) < abs(diffy)) {
                return DIRECTION_DDL;
            } else if (abs(diffx) > abs(diffy)) {
                return DIRECTION_DLL;
            }
            return DIRECTION_DL;
        }
        if (abs(diffx) < abs(diffy)) {
            return DIRECTION_LUU;
        } else if (abs(diffx) > abs(diffy)) {
            return DIRECTION_LLU;
        }
        return DIRECTION_LU;
    }

    return DIRECTION_RDD;
}

// DO NOT STORE THE POINTER TO CA!!!!!!!!
void port_combat_action(entity_action_t *ca) {
    // DO NOT STORE THE POINTER TO CA!!!!!!!!
    float const zoom = settings_zoom();
    animate_sprite_t *as = NULL, *source = NULL, *cast = NULL,
        *dest = NULL, *hit = NULL, *throw = NULL;
    int dir = 0;

    switch (ca->action) {
        case EA_RED_DAMAGE: sol_sprite_set_frame(combat_attacks, 0); break;
        case EA_BIG_RED_DAMAGE: sol_sprite_set_frame(combat_attacks, 1); break;
        case EA_GREEN_DAMAGE: sol_sprite_set_frame(combat_attacks, 2); break;
        case EA_MAGIC_DAMAGE: sol_sprite_set_frame(combat_attacks, 3); break;
        case EA_BROWN_DAMAGE: sol_sprite_set_frame(combat_attacks, 4); break;
        default: break;
    }

    switch (ca->action) {
        case EA_RED_DAMAGE:
        case EA_BIG_RED_DAMAGE:
        case EA_GREEN_DAMAGE:
        case EA_MAGIC_DAMAGE:
        case EA_BROWN_DAMAGE:
            as = &(ca->target->anim);
            sol_sprite_center_spr(combat_attacks, as->spr);
            show_attack = 1;
            damage_amount = ca->amt;
            return;
        case EA_POWER_CAST:
            source = &(ca->source->anim);
            cast = &(ca->power->cast);
            sol_sprite_center_spr(cast->spr, source->spr);
            cast->x = sol_sprite_getx(cast->spr) + sol_get_camerax();
            cast->y = sol_sprite_gety(cast->spr) + sol_get_camerax();
            cast->destx = cast->x;
            cast->desty = cast->y;
            cast->scmd = combat_get_scmd(COMBAT_POWER_CAST);
            //add_node_list(cast, ca->power->cast_sound, ca);
            break;
        case EA_POWER_THROW:
            dir = get_direction(ca->source, ca->target);
            throw = &(ca->power->thrown);
            source = &(ca->source->anim);
            dest = &(ca->target->anim);
            if (sol_sprite_num_frames(throw->spr) < 30) {
                throw->scmd = combat_get_scmd(COMBAT_POWER_THROW_STATIC_U + dir);
            } else {
                throw->scmd = combat_get_scmd(COMBAT_POWER_THROW_ANIM_U + dir);
            }
            throw->x = sol_sprite_getx(source->spr) + sol_get_camerax();
            throw->y = sol_sprite_gety(source->spr) + sol_get_cameray();
            throw->destx = dest
                ? sol_sprite_getx(dest->spr) + sol_sprite_getw(dest->spr) / 2 + sol_get_camerax()
                : ca->target->mapx * 16 * zoom;
            throw->desty = dest
                ? sol_sprite_gety(dest->spr) + sol_sprite_geth(dest->spr) / 2 + sol_get_cameray()
                : ca->target->mapy * 16 * zoom;
            //printf("(%d, %d) -> (%d, %d)\n", throw->anim->x, throw->anim->y,
                //throw->anim->destx, throw->anim->desty);
            throw->movex = abs(throw->destx - throw->x) / 30;
            throw->movey = abs(throw->desty - throw->y) / 30;
            //add_node_list(throw, ca->power->thrown_sound, ca);
        case EA_POWER_HIT:
            dest = &(ca->target->anim);
            hit = &(ca->power->hit);
            sol_sprite_center_spr(hit->spr, dest->spr);
            hit->x = sol_sprite_getx(hit->spr) + sol_get_camerax();
            hit->y = sol_sprite_gety(hit->spr) + sol_get_cameray();
            hit->destx = hit->x;
            hit->desty = hit->y;
            hit->scmd = combat_get_scmd(COMBAT_POWER_CAST);
            //add_node_list(hit, ca->power->hit_sound, ca);
        default:
            break;
    }

    show_attack = 0;
}

int combat_status_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    return 0;// did not handle, we need to let the map handle movement
}

int combat_status_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    //return 1; // means I captured the mouse click
    return 0; // zero means I did not handle the mouse click, so another window may.
}

int combat_status_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    //return 1; // means I captured the mouse click
    return 0; // zero means I did not handle the mouse click, so another window may.
}

void combat_status_free() {
    sol_sprite_free(background);
}

sol_wops_t combat_status_window = {
    .init = combat_status_init,
    .cleanup = combat_status_free,
    .render = combat_status_render,
    .mouse_movement = combat_status_handle_mouse_movement,
    .mouse_down = combat_status_handle_mouse_down,
    .mouse_up = combat_status_handle_mouse_up,
    .grey_out_map = 0,
    .data = NULL
};