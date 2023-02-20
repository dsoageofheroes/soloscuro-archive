#include "interact.h"
#include "narrate.h"
#include "gfftypes.h"
#include "entity.h"
#include "gff.h"
#include "port.h"
#include "settings.h"
#include <string.h>

static sol_sprite_t background;
static sol_sprite_t talk, get, use, bar, bar_border;
static sol_sprite_t box;
static char action_text[16];
static char name_text[32];
static char info_text[32];
static sol_entity_t *entity = NULL;
static sol_dim_t action_loc, name_loc, info_loc;

void interact_set_entity(sol_entity_t *_entity) {
    entity = _entity;
    if (entity) {
        strcpy(name_text, entity->name);
        snprintf(info_text, 32, "LEVEL: %d", entity->class[0].level);
    } else {
        name_text[0] = info_text[0] = '\0';
    }
}

void interact_init(const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = settings_zoom();

    sol_status_check(sol_sprite_new(pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 3020, &background),
            "Unable to load spr check.");
    sol_status_check(sol_sprite_new(pal, 3 + x, 58 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 15105, &talk),
            "Unable to load spr check.");
    sol_status_check(sol_sprite_new(pal,  23 + x, 58 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 15106, &get),
            "Unable to load spr check.");
    sol_status_check(sol_sprite_new(pal,  43 + x, 58 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 15107, &use),
            "Unable to load spr check.");
    sol_status_check(sol_sprite_new(pal,  60 + x, 59 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 15109, &box),
            "Unable to load spr check.");
    sol_status_check(sol_sprite_new(pal,  6 + x, 7 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20107, &bar),
            "Unable to load spr check.");
    sol_status_check(sol_sprite_new(pal,  5 + x, 6 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20106, &bar_border),
            "Unable to load spr check.");

    action_loc.x = (62 + x) * zoom;
    action_loc.y = (61 + y) * zoom;
    name_loc.x = (6 + x) * zoom;
    name_loc.y = (17 + y) * zoom;
    info_loc.x = (6 + x) * zoom;
    info_loc.y = (24 + y) * zoom;

    memset(action_text, 0x0, sizeof(action_text));
    memset(name_text, 0x0, sizeof(name_text));
    memset(info_text, 0x0, sizeof(info_text));
}

void interact_render(void *data) {
    sol_sprite_info_t info;
    float amt = entity
        ? entity->stats.hp / (float) entity->stats.high_hp
        : 0;
    sol_sprite_render(background);
    sol_sprite_render(talk);
    sol_sprite_render(use);
    sol_sprite_render(get);
    sol_sprite_render(bar_border);
    sol_sprite_render(box);
    sol_status_check(sol_sprite_get_info(bar_border, &info), "Unable to get bar sprite info");
    sol_sprite_render_box(bar, info.x, info.y,
        info.w * (amt > 1.0 ? 1.0 : amt), info.h);

    sol_status_check(sol_print_line_len(FONT_GREY, action_text, action_loc.x, action_loc.y, sizeof(action_text)),
            "Unable to print.");
    sol_status_check(sol_print_line_len(FONT_GREYLIGHT, name_text, name_loc.x, name_loc.y, sizeof(name_text)),
            "Unable to print.");
    sol_status_check(sol_print_line_len(FONT_GREYLIGHT, info_text, info_loc.x, info_loc.y, sizeof(info_text)),
            "Unable to print.");
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    if (sol_sprite_in_rect(talk, x, y) == SOL_SUCCESS) { return talk; }
    if (sol_sprite_in_rect(get, x, y) == SOL_SUCCESS) { return get; }
    if (sol_sprite_in_rect(use, x, y) == SOL_SUCCESS) { return use; }
    
    return SPRITE_ERROR;
}

int interact_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;

    sol_sprite_t cur_sprite = get_sprite_mouse_is_on(x, y);

    if (last_sprite != cur_sprite) {
        sol_sprite_increment_frame(cur_sprite, 1);
        if (last_sprite != SPRITE_ERROR) {
            sol_sprite_increment_frame(last_sprite, -1);
        }
    }
    
    last_sprite = cur_sprite;
    if (cur_sprite == talk) { strcpy(action_text, "TALK");
    } else if (cur_sprite == get) { strcpy(action_text, " GET");
    } else if (cur_sprite == use) { strcpy(action_text, " USE");
    } else { action_text[0] = '\0';
    }
    return 1;// did not handle
}

int interact_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another window may.
}

int interact_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another window may.
}

void interact_free() {
    sol_status_check(sol_sprite_free(background), "Unable to free sprite");
    sol_status_check(sol_sprite_free(talk), "Unable to free sprite");
    sol_status_check(sol_sprite_free(get), "Unable to free sprite");
    sol_status_check(sol_sprite_free(use), "Unable to free sprite");
    sol_status_check(sol_sprite_free(bar), "Unable to free sprite");
    sol_status_check(sol_sprite_free(bar_border), "Unable to free sprite");
    sol_status_check(sol_sprite_free(box), "Unable to free sprite");
}

sol_wops_t interact_window = {
    .init = interact_init,
    .cleanup = interact_free,
    .render = interact_render,
    .mouse_movement = interact_handle_mouse_movement,
    .mouse_down = interact_handle_mouse_down,
    .mouse_up = interact_handle_mouse_up,
    .grey_out_map = 1,
    .data = NULL
};
