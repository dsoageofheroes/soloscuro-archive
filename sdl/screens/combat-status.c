#include "combat-status.h"
#include "popup.h"
#include "narrate.h"
#include "../animate.h"
#include "../main.h"
#include "../sprite.h"
#include "../font.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"
#include "../../src/region-manager.h"
#include <string.h>

static uint16_t background;
static combat_status_t combat_status;
static uint32_t xoffset, yoffset;
static uint16_t combat_attacks;
static int show_attack = 0;
static int damage_amount = 0;

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
    xoffset = x * zoom;
    yoffset = y * zoom;

    background = sprite_new(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 5016);
    combat_attacks = sprite_new(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 5014);
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

void combat_status_render(void *data, SDL_Renderer *renderer) {
    sprite_render(renderer, background);
    const float zoom = main_get_zoom();
    SDL_Rect loc;
    char buf[128];

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

void port_combat_action(combat_action_t *ca) {
    if (ca->action == CA_RED_DAMAGE) {
        animate_sprite_node_t *asn = ca->target->sprite.data;
        sprite_center_spr(combat_attacks, asn->anim->spr);
        show_attack = 1;
        damage_amount = ca->amt;
        return;
    }

    show_attack = 0;
}

int combat_status_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    return 1;// did not handle
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
    .data = NULL
};
