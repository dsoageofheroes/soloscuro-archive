#include "audio.h"
#include "combat-status.h"
#include "popup.h"
#include "narrate.h"
#include "gfftypes.h"
#include "gff.h"
#include "port.h"
#include "region-manager.h"
#include "settings.h"
#include "arbiter.h"
#include "player.h"
#include <string.h>

static sol_sprite_t background, combat_attacks;
static combat_status_t combat_status;
static uint32_t xoffset, yoffset;
static int show_attack = 0;
static int damage_amount = 0;

typedef struct animate_sprite_node_list_s {
    sol_power_t *power;
    sol_entity_t *source;
    sol_entity_t *target;
    enum sol_entity_action_e action;
    int cycles;
    int is_moving;
    uint16_t sound;
    struct animate_sprite_node_list_s *next;
} animate_sprite_node_list_t;

animate_sprite_node_list_t *list = NULL;

static void start_node() {
    if (!list) { return; }
    sol_audio_play_voc(RESOURCE_GFF_INDEX, GFF_BVOC, list->sound, 1.0);
}

extern sol_status_t sol_combat_clear_damage() {
    show_attack = 0;
    return SOL_SUCCESS;
}

static void pop_list() {
    if (!list) { return; }

    animate_sprite_node_list_t *delme = list;
    list = list->next;

    if (sol_entity_is_fake(delme->target) == SOL_SUCCESS) {
        sol_entity_free(delme->target);
    }

    if (delme->action == EA_POWER_HIT) {
        sol_power_instance_t pi;
        memset(&pi, 0x0, sizeof(sol_power_instance_t));
        pi.entity = delme->target;
        pi.stats = delme->power;
        delme->power->actions.apply(&pi, delme->target);
    }

    free(delme);
    start_node();
}

extern sol_status_t sol_combat_status_get(combat_status_t **cs) {
    *cs = &combat_status;
    return SOL_SUCCESS;
}

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

    sol_status_check(
            sol_sprite_new(pal, 0 + xoffset / settings_zoom(), 0 + yoffset / settings_zoom(), zoom, RESOURCE_GFF_INDEX, GFF_BMP, 5016, &background),
            "Unable to load background for combat status");
    sol_status_check(
            sol_sprite_new(pal, 0, 0, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 5014, &combat_attacks),
            "Unable to load attacks sprite for combat status");
}

static void get_status() {
    sol_status_t status;
    sol_combat_region_t *cr = NULL;
    sol_region_t *reg;


    sol_region_manager_get_current(&reg);
    status = sol_arbiter_combat_region(reg, &cr);
    sol_entity_t *dude;

    sol_combat_get_current(cr, &dude);
    if (dude) {
        strcpy(combat_status.name, dude->name);
        combat_status.current_hp = dude->stats.hp;
        combat_status.max_hp = dude->stats.high_hp;
        combat_status.status = 1;// Need to fix.
        combat_status.move = dude->stats.combat.move;
    }
}

static int count = 30;

void combat_status_render(void *data) {
    const float zoom = settings_zoom();
    const int delta = 5 * zoom;
    sol_dim_t loc;
    char buf[128];
    static int last_action = 0;
    sol_status_t status;
    sol_combat_region_t *cr = NULL;
    sol_sprite_info_t info;
    sol_region_t *reg;

    if (show_attack) {
        sol_status_check(sol_sprite_get_info(combat_attacks, &info), "Unable to get combat attacks sprite info");
        sol_sprite_render(combat_attacks);
        loc.x = info.x;
        loc.y = info.y
            + info.h / 2
            - 8 / 2 * settings_zoom(); // last one is font size / 2
        loc.w = info.w;
        snprintf(buf, 128, "%d", damage_amount);
        sol_font_render_center(FONT_GREYLIGHT, buf, loc.x, loc.y, loc.w);
    }

    sol_region_manager_get_current(&reg);
    status = sol_arbiter_combat_region(reg, &cr);
    if (sol_combat_active(cr) != SOL_SUCCESS) { return; }

    if (2 == 1) {
        sol_draw_cone(100, 100, 200);
    }

    sol_sprite_render(background);
    get_status();

    sol_status_check(sol_sprite_get_info(background, &info), "Unable to get background sprite info.");
    loc.x = xoffset;
    loc.y = yoffset + 2 * zoom;
    loc.w = info.w;

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
}

// DO NOT STORE THE POINTER TO CA!!!!!!!!
extern sol_status_t sol_combat_action(const sol_entity_action_t *ca) {
    if (!ca) { return SOL_NULL_ARGUMENT; }
    // DO NOT STORE THE POINTER TO CA!!!!!!!!
    float const zoom = settings_zoom();
    animate_sprite_t *as = NULL, *source = NULL, *cast = NULL,
        *dest = NULL, *hit = NULL, *throw = NULL;
    int dir = 0;
    sol_sprite_info_t info;

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
            damage_amount = ca->damage;
            return SOL_SUCCESS;
        case EA_POWER_CAST:
            sol_status_check(sol_sprite_get_info(cast->spr, &info), "Unable to get cast sprite info");
            source = &(ca->source->anim);
            cast = &(ca->power->cast);
            //sol_sprite_center_spr(cast->spr, source->spr);
            cast->x = info.x + sol_get_camerax();
            cast->y = info.y + sol_get_camerax();
            cast->destx = cast->x;
            cast->desty = cast->y;
            sol_combat_get_scmd(COMBAT_POWER_CAST, &cast->scmd);
            //add_node_list(cast, ca->power->cast_sound, ca);
            break;
        case EA_POWER_THROW: // handled in entity-animations
            break;
        case EA_POWER_HIT:
            sol_status_check(sol_sprite_get_info(hit->spr, &info), "Unable to get cast sprite info");
            dest = &(ca->target->anim);
            hit = &(ca->power->hit);
            //sol_sprite_center_spr(hit->spr, dest->spr);
            hit->x = info.x + sol_get_camerax();
            hit->y = info.y + sol_get_cameray();
            hit->destx = hit->x;
            hit->desty = hit->y;
            sol_combat_get_scmd(COMBAT_POWER_CAST, &hit->scmd);
            //add_node_list(hit, ca->power->hit_sound, ca);
        default:
            break;
    }

    show_attack = 0;
    return SOL_SUCCESS;
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
    sol_status_check(sol_sprite_free(background), "Unable to free sprite");
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
