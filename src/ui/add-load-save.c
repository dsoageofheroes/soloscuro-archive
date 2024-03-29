#include "add-load-save.h"
#include "gfftypes.h"
#include "popup.h"
#include "window-main.h"
#include "sol_textbox.h"
#include "mouse.h"
#include "ds-load-save.h"
#include "gff.h"
#include "save-load.h"
#include "gff-char.h"
#include "port.h"
#include "settings.h"
#include "gpl.h"
#include "player.h"
#include "status.h"

#include <string.h>

#define SAVE_FORMAT "save%02d.sav"

static sol_sprite_t background;
static sol_sprite_t up_arrow, down_arrow;
static sol_sprite_t action_btn, exit_btn, delete_btn;
static sol_sprite_t title;
static sol_sprite_t bar;
static uint32_t xoffset, yoffset;
static sol_textbox_t *name_tb = NULL;

extern char *strdup(const char *s);

#define RES_MAX (1<<10)

static uint16_t mousex, mousey;
static int16_t selection;
static int16_t char_selected = -1;
static char **entries = NULL;
static uint16_t *valids = NULL;
static uint32_t num_entries = 0;
static uint32_t top_entry = 0;
static uint32_t num_valid_entries = 0;
static int last_action = ACTION_NONE;
static uint32_t res_ids[RES_MAX]; // for the deletion.
static sol_als_action_t mode = ACTION_ADD; // ADD, LOAD, SAVE

extern void sol_add_load_save_set_mode(const sol_als_action_t _mode) { mode = _mode; }

static void free_entries() {
    if (entries) {
        for (uint32_t i = 0; i < num_entries; i++) {
            free(entries[i]);
        }
        free(entries);
        entries = NULL;
    }
    if (valids) {
        free(valids);
        valids = NULL;
    }
    num_entries = 0;
    num_valid_entries = 0;
}

#define BUF_MAX (1<<12)

static void setup_character_selection() {
    char buf[BUF_MAX];
    int16_t id;

    free_entries();

    num_entries = gff_get_resource_length(CHARSAVE_GFF_INDEX, GFF_CHAR);

    entries = malloc(sizeof(char*) * num_entries);
    valids = malloc(sizeof(uint16_t) * num_entries);
    if (!entries || !valids) { return; }
    num_valid_entries = 0;
    gff_get_resource_ids(CHARSAVE_GFF_INDEX, GFF_CHAR, res_ids);
    for (uint32_t i = 0; i < num_entries; i++) {
        gff_chunk_header_t chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_CHAR, res_ids[i]);
        gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &buf, sizeof(buf));
        ds1_combat_t *combat = (ds1_combat_t*)(buf + 10);
        entries[i] = strdup(combat->name);
        chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_CACT, res_ids[i]);
        gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &id, sizeof(id));
        if (id) {
            valids[num_valid_entries] = i;
            num_valid_entries++;
        }
    }
}

static void setup_save_load_selection() {
    char filename[128];
    char buf[BUF_MAX];
    int16_t id = 0;
    FILE *file = NULL;

    free_entries();

    snprintf(buf, BUF_MAX, SAVE_FORMAT, id);
    file = fopen(buf, "rb");
    while (file) {
        fclose(file);
        id++;
        snprintf(buf, BUF_MAX, SAVE_FORMAT, id);
        file = fopen(buf, "rb");
    }

    entries = malloc(sizeof(char*) * id);
    valids = malloc(sizeof(uint16_t) * id);
    if (!entries || !valids) { return; }
    num_valid_entries = 0;

    for (size_t i = 0; i < 100; i++) {
        sprintf(filename, SAVE_FORMAT, (int)i);
        file = fopen(filename, "rb");

        if (!file) {
            break;
        }

        sol_load_get_name(filename, buf, BUF_MAX);

        entries[i] = strdup(buf);
        valids[i] = i;
        num_entries++;
        num_valid_entries++;
        
        fclose(file);
    }
    return;
}

static void setup_title_action(gff_palette_t *pal, int tid, int aid) {
    const float zoom = settings_zoom();
    sol_status_check(sol_sprite_new(pal, 115 + xoffset / zoom, 0 + yoffset / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, tid, &title),
            "Unable to create title sprite");
    sol_status_check(sol_sprite_new(pal, 230 + xoffset / zoom, 30 + yoffset / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, aid, &action_btn),
            "Unable to create action btn sprite");
}

void add_load_save_init(const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = settings_zoom();
    selection = -1;
    num_valid_entries = 0;
    xoffset = x;
    yoffset = y;

    sol_status_check(
            sol_sprite_new(pal, 0 + x / zoom, 0 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 3009, &background),
            "Unable to load als background.");
    sol_status_check(
        sol_sprite_new(pal, 215 + x / zoom, 30 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2093, &up_arrow),
        "Unable to load als up_arrow.");
    sol_status_check(
        sol_sprite_new(pal, 215 + x / zoom, 130 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2094, &down_arrow),
        "Unable to load als down_arrow.");
    sol_status_check(
        sol_sprite_new(pal, 230 + x / zoom, 50 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2058, &exit_btn),
        "Unable to load als exit_btn.");
    sol_status_check(
        sol_sprite_new(pal, 215 + x / zoom, 150 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2097, &delete_btn),
        "Unable to load als delete_btn.");
    sol_status_check(
        sol_sprite_new(pal, 45 + x / zoom, 31 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 18100, &bar),
        "Unable to load als bar.");
    sol_status_check(sol_textbox_create(32, 50 + x / zoom, 148 + y / zoom, &name_tb),
            "Unable to create name text box.");
    sol_textbox_set_current(name_tb);

    switch(mode) {
        case ACTION_ADD:
            setup_title_action(pal, 6036, 6037);
            setup_character_selection();
            break;
        case ACTION_SAVE:
            setup_title_action(pal, 2056, 2057);
            setup_save_load_selection();
            break;
        case ACTION_LOAD:
            setup_title_action(pal, 6030, 6031);
            setup_save_load_selection();
            break;
        case ACTION_DROP:
            setup_title_action(pal, 6038, 6039);
            break;
        default:
            error("action %d selected, but not implemented in als!\n", mode);
            break;
    }
}

void add_load_save_render(void *data) {
    const float zoom = settings_zoom();
    sol_sprite_render(background);
    sol_sprite_render(up_arrow);
    sol_sprite_render(down_arrow);
    sol_sprite_render(exit_btn);
    sol_sprite_render(delete_btn);
    sol_sprite_render(title);
    sol_sprite_render(action_btn);
    for (unsigned int i = 0; i < 10; i++) {
        sol_sprite_set_frame(bar, 0);
        sol_sprite_set_location(bar, zoom * 45 + xoffset, zoom * (31 + i * 11) + yoffset);

        if (selection >= 0 && (i == (selection - top_entry))) {
            sol_sprite_set_frame(bar, 1);
        }
        if (sol_sprite_in_rect(bar, mousex, mousey) == SOL_SUCCESS) {
            sol_sprite_increment_frame(bar, 2);
        }
        sol_sprite_render(bar);
    }

    for (uint16_t i = top_entry; i < top_entry + 10 && i < num_valid_entries; i++) {
        sol_status_check(
            sol_print_line_len((selection != i) ? FONT_GREY : FONT_REDDARK,
                entries[valids[i]],
                zoom * 47 + xoffset, zoom * (31 + (i - top_entry) * 11) + yoffset, 1<<10),
            "Unable to print.");
    }

    sol_textbox_render(name_tb);
}

int add_load_save_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_spr = SPRITE_ERROR;
    uint16_t cspr = SPRITE_ERROR;
    sol_sprite_info_t info;
    mousex = x;
    mousey = y;

    if (sol_sprite_in_rect(action_btn, x, y) == SOL_SUCCESS) {
        cspr = action_btn;
    }
    if (sol_sprite_in_rect(delete_btn, x, y) == SOL_SUCCESS) {
        cspr = delete_btn;
    }
    if (sol_sprite_in_rect(exit_btn, x, y) == SOL_SUCCESS) {
        cspr = exit_btn;
    }
    if (sol_sprite_in_rect(up_arrow, x, y) == SOL_SUCCESS) {
        cspr = up_arrow;
    }
    if (sol_sprite_in_rect(down_arrow, x, y) == SOL_SUCCESS) {
        cspr = down_arrow;
    }

    sol_sprite_set_frame(action_btn, 0);
    sol_sprite_set_frame(exit_btn, 0);
    sol_sprite_set_frame(delete_btn, 0);

    if (sol_sprite_get_info(cspr, &info) == SOL_SUCCESS
            && (info.current_frame < 2)) {
        sol_sprite_set_frame(cspr, 1);
    }

    if (last_spr != SPRITE_ERROR && last_spr != cspr) {
        sol_status_check(sol_sprite_get_info(last_spr, &info), "Unable to get sprite info.");
        if (info.current_frame < 2) {
            sol_sprite_set_frame(last_spr, 0);
        }
    }

    last_spr = cspr;
    return 1; // means I captured the mouse movement
}

int add_load_save_handle_mouse_down(const sol_mouse_button_t button, const uint32_t x, const uint32_t y) {
    const float zoom = settings_zoom();

    if (sol_sprite_in_rect(action_btn, x, y) == SOL_SUCCESS) {
        sol_sprite_set_frame(action_btn, 2);
    }
    if (sol_sprite_in_rect(exit_btn, x, y) == SOL_SUCCESS) {
        sol_sprite_set_frame(exit_btn, 2);
    }
    if (sol_sprite_in_rect(delete_btn, x, y) == SOL_SUCCESS) {
        sol_sprite_set_frame(delete_btn, 2);
    }
    if (sol_sprite_in_rect(up_arrow, x, y) == SOL_SUCCESS) {
        sol_sprite_set_frame(up_arrow, 3);
    }
    if (sol_sprite_in_rect(down_arrow, x, y) == SOL_SUCCESS) {
        sol_sprite_set_frame(down_arrow, 3);
    }
    for (int i = 0; i < 10; i++) {
        sol_sprite_set_location(bar, xoffset + (zoom * 45), yoffset + (zoom * (31 + i * 11)));
        if (sol_sprite_in_rect(bar, x, y) == SOL_SUCCESS) {
            selection = top_entry + i;
            if (selection < num_valid_entries) {
                char_selected = res_ids[valids[selection]];
                sol_textbox_set_text(name_tb, entries[valids[selection]]);
            } else {
                char_selected = -1;
            }
        }
    }

    sol_textbox_set_focus(name_tb, (sol_textbox_is_in(name_tb, x, y)) == SOL_SUCCESS);

    return 1; // means I captured the mouse click
}

int16_t find_next_save_file() {
    char buf[32];

    for (int i = 0; i < 100; i++) {
        snprintf(buf, 31, SAVE_FORMAT, i);
        FILE *file = fopen(buf, "rb");
        if (!file) { return i; }
        fclose (file);
    }

    return -1;
}

int add_load_save_handle_mouse_up(const sol_mouse_button_t button, const uint32_t x, const uint32_t y) {
    char filename[32];
    char msg[128];

    if (sol_sprite_in_rect(action_btn, x, y) == SOL_SUCCESS) {
        if (mode == ACTION_LOAD) {
            if (entries && selection >= 0 && entries[selection]) {
                sprintf(filename, SAVE_FORMAT, selection);
                sol_load_from_file(filename);
            }
            return 1;
        }
        if (mode == ACTION_SAVE) {
            if (selection < 0) {
                selection = find_next_save_file();
            }
            if (selection < 0) { return 0; }
            snprintf(filename, 31, SAVE_FORMAT, selection);
            if (sol_save_to_file(filename, name_tb->text) == SOL_SUCCESS) {
                sprintf(msg, "Game %s saved.", name_tb->text);
                sol_window_pop();
                sol_window_push(&popup_window, 100, 75);
                sol_popup_quick_message(msg);
            } else {
                sol_window_push(&popup_window, 100, 75);
                sol_popup_quick_message("Can't save");
            }
            sol_window_push(&popup_window, 0, 0);
            return 1;
        }
        sol_sprite_set_frame(action_btn, 0);
        if (selection != -1) {
            last_action = mode;
            sol_window_pop();
        }
    }
    if (sol_sprite_in_rect(exit_btn, x, y) == SOL_SUCCESS) {
        sol_sprite_set_frame(exit_btn, 0);
        sol_window_pop();
    }
    if (sol_sprite_in_rect(delete_btn, x, y) == SOL_SUCCESS) {
        sol_sprite_set_frame(delete_btn, 0);
        if (selection != -1) {
            sol_sprite_set_frame(delete_btn, 3);
            sol_window_push(&popup_window, 90, 62);
            sol_popup_set_message("DELETE THIS PERSON?");
            sol_popup_set_option(0, "YES");
            sol_popup_set_option(1, "NO");
            sol_popup_set_option(2, "CANCEL");
        }
    }
    if (sol_sprite_in_rect(up_arrow, x, y) == SOL_SUCCESS) {
        sol_sprite_set_frame(up_arrow, 0);
        if (top_entry > 0) { top_entry--; }
    }
    if (sol_sprite_in_rect(down_arrow, x, y) == SOL_SUCCESS) {
        sol_sprite_set_frame(down_arrow, 0);
        if (top_entry < num_valid_entries - 10) { top_entry++; }
    }
    return 1; // means I captured the mouse click
}

void add_load_save_return_control () {
    uint8_t sel;
    sol_sprite_set_frame(delete_btn, 0);
    if (sol_popup_get_selection(&sel) == SOL_SUCCESS && sel == POPUP_0) {
        //printf("Need to delete! %d\n", res_ids[valids[selection]]);
        gff_char_delete(res_ids[valids[selection]]);
        setup_character_selection();
    }
    sol_popup_clear_selection();
}

void add_load_save_free() {
    sol_status_check(sol_sprite_free(background), "Unable to free sprite");
    sol_status_check(sol_sprite_free(up_arrow), "Unable to free sprite");
    sol_status_check(sol_sprite_free(down_arrow), "Unable to free sprite");
    sol_status_check(sol_sprite_free(exit_btn), "Unable to free sprite");
    sol_status_check(sol_sprite_free(delete_btn), "Unable to free sprite");
    sol_status_check(sol_sprite_free(action_btn), "Unable to free sprite");
    sol_status_check(sol_sprite_free(title), "Unable to free sprite");
    sol_status_check(sol_sprite_free(bar), "Unable to free sprite");
    free_entries();
    sol_textbox_free(name_tb);
    sol_textbox_set_current(NULL);
}

extern int sol_add_load_save_get_action() { return last_action; }
extern uint32_t sol_add_load_save_get_selection() { return char_selected; }

sol_wops_t als_window = {
    .init = add_load_save_init,
    .cleanup = add_load_save_free,
    .render = add_load_save_render,
    .mouse_movement = add_load_save_handle_mouse_movement,
    .mouse_down = add_load_save_handle_mouse_down,
    .mouse_up = add_load_save_handle_mouse_up,
    .return_control = add_load_save_return_control,
    .grey_out_map = 1,
    .data = NULL
};
