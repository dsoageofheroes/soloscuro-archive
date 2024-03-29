#include "examine.h"
#include "gfftypes.h"
#include "ssi-gui.h"
#include "port.h"
#include "settings.h"
#include "trigger.h"
#include <string.h>

static sol_window_t *win = NULL;
static sol_dude_t       *entity_to_examine = NULL;
static uint32_t      winx, winy;
static sol_sprite_t  bar = SPRITE_ERROR;
static char          level_text[32];

static void update_ui() {
    if (!win || !entity_to_examine) { return; }

    if (entity_to_examine) {
        sol_sprite_set_frame_keep_loc(win->buttons[0].spr,
            sol_trigger_has_look(entity_to_examine->ds_id) == SOL_SUCCESS ? 0 : 2); // GET
        sol_sprite_set_frame_keep_loc(win->buttons[1].spr,
            sol_trigger_has_use(entity_to_examine->ds_id) == SOL_SUCCESS ? 0 : 2); // USE
        sol_sprite_set_frame_keep_loc(win->buttons[2].spr,
            sol_trigger_has_talkto(entity_to_examine->ds_id) == SOL_SUCCESS ? 0 : 2); // TALK

        if (entity_to_examine->name == NULL) {
            sol_entity_t* t;
            sol_entity_create_from_objex(entity_to_examine->ds_id, &t);
            entity_to_examine->name = strdup(t->name ? t->name : "");
            entity_to_examine->class[0].level = t->class[0].level;
            t->ds_id = 9999;
            sol_entity_free(t);
        }
        sprintf(level_text, "LEVEL: %d", entity_to_examine->class[0].level);
    } else {
        sol_sprite_set_frame_keep_loc(win->buttons[0].spr, 2);
        sol_sprite_set_frame_keep_loc(win->buttons[1].spr, 2);
        sol_sprite_set_frame_keep_loc(win->buttons[2].spr, 2);
        level_text[0] = '\0';
    }
}

extern sol_status_t sol_examine_is_open() { return entity_to_examine == NULL ? SOL_IS_CLOSED : SOL_SUCCESS; }

// returns if examine window should pop up
extern sol_status_t sol_examine_entity(sol_entity_t *dude) {
    int amt = 0;

    amt += sol_trigger_has_talkto(dude->ds_id) == SOL_SUCCESS ? 1 : 0;
    amt += sol_trigger_has_look(dude->ds_id) == SOL_SUCCESS ? 1 : 0;
    amt += sol_trigger_has_use(dude->ds_id) == SOL_SUCCESS ? 1 : 0;

    if (amt < 1) { return SOL_NO_EXAMINE_TRIGGER; }

    entity_to_examine = dude;
    update_ui();

    return SOL_SUCCESS;
}

static void examine_init(const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    winx = x;
    winy = y;
    win = sol_window_from_gff(3020);
    sol_window_set_pos(win, winx, winy);
    sol_status_check(sol_sprite_new(pal, 0, 0, 2.0, RESOURCE_GFF_INDEX, GFF_BMP, 20107, &bar),
        "unable to create bar spr for examine.");
    sol_sprite_set_location(bar, winx + 6 * settings_zoom(), winy + 6 * settings_zoom());
    update_ui();
}

const char *button_texts[] = {
    "GET",
    "USE",
    "TALK",
    "",
};

static int examine_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static size_t     last_button = -1;
    size_t            button = sol_window_get_button(win, x, y);
    sol_sprite_info_t info;

    if (button != last_button) {
        //printf("button = %zu, last_button = %zu\n", button, last_button);
        sol_status_check(sol_sprite_get_info(win->buttons[last_button].spr, &info), "Unable to get sprite info");
        if (last_button < win->num_buttons && info.current_frame != 2) {
            sol_sprite_set_frame_keep_loc(win->buttons[last_button].spr, 0);
            sol_button_set_text(win->buttons + 3, "", FONT_GREY);
        }
        sol_status_check(sol_sprite_get_info(win->buttons[button].spr, &info), "Unable to get sprite info");
        if (button < win->num_buttons && info.current_frame != 2) {
            sol_sprite_set_frame_keep_loc(win->buttons[button].spr, 1);
            sol_button_set_text(win->buttons + 3, button_texts[button], FONT_GREY);
        }
    }

    last_button = button;
    return 1;
}

static int examine_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    (void) button;
    (void) x;
    (void) y;
    return 1; // ignore all mouse down.
}

static int examine_handle_mouse_up(const uint32_t _button, const uint32_t x, const uint32_t y) {
    (void)            _button;
    size_t            button = sol_window_get_button(win, x, y);
    int               id = entity_to_examine->ds_id;
    sol_sprite_info_t info;

    sol_status_check(sol_sprite_get_info(win->buttons[button].spr, &info), "Unable to get sprite info");
    switch (button) {
        case 0: // GET
            if (info.current_frame == 2) { break; }
            sol_window_pop();
            //sol_trigger_use_click(id);
            break;
        case 1: // USE
            if (info.current_frame == 2) { break; }
            sol_window_pop();
            sol_trigger_use(id);
            break;
        case 2: // TALK
            if (info.current_frame == 2) { break; }
            sol_window_pop();
            sol_trigger_talk_click(id);
            break;
    }
    return 1;
}

static void examine_free() {
    sol_window_free_base(win);
    sol_status_check(sol_sprite_free(bar), "Unable to free sprite");
    entity_to_examine = NULL;
    bar = SPRITE_ERROR;
}

static void examine_render(void *data) {
    (void) data;
    sol_window_render_base(win);
    sol_sprite_render(bar);
    if (entity_to_examine) {
        sol_status_check(sol_print_line_len(FONT_GREYLIGHT, entity_to_examine->name,
            winx + 6 * settings_zoom(),
            winx + 26 * settings_zoom(), 32),
                "Unable to print.");
        sol_status_check(sol_print_line_len(FONT_GREYLIGHT, level_text,
            winx + 6 * settings_zoom(),
            winx + 34 * settings_zoom(), 32),
                "Unable to print.");
    }
}

sol_wops_t examine_window = {
    .init = examine_init,
    .cleanup = examine_free,
    .render = examine_render,
    .mouse_movement = examine_handle_mouse_movement,
    .mouse_down = examine_handle_mouse_down,
    .mouse_up = examine_handle_mouse_up,
    .grey_out_map = 0,
    .data = NULL
};
