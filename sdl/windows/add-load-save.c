#include "add-load-save.h"
#include "window-main.h"
#include "popup.h"
#include "../textbox.h"
#include "../main.h"
#include "../font.h"
#include "../sprite.h"
#include "../map.h"
#include "../player.h"
#include "../../src/ds-load-save.h"
#include "../../src/gff.h"
#include "../../src/gff-char.h"
#include "../../src/gfftypes.h"

#define SAVE_FORMAT "save%02d.sav"

static uint16_t background;
static uint16_t up_arrow, down_arrow;
static uint16_t action_btn, exit_btn, delete_btn;
static uint16_t title;
static uint16_t bar;
static uint32_t xoffset, yoffset;
static textbox_t *name_tb = NULL;

extern char *strdup(const char *s);

static uint16_t new_sprite_create(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_create(renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

#define RES_MAX (1<<10)

static uint16_t mousex, mousey;
static int16_t selection;
static uint16_t char_selected;
static char **entries = NULL;
static uint16_t *valids = NULL;
static uint32_t num_entries = 0;
static uint32_t top_entry = 0;
static uint32_t num_valid_entries = 0;
static int last_action = ACTION_NONE;
static uint32_t res_ids[RES_MAX]; // for the deletion.
static SDL_Renderer *renderer = NULL;
static int mode = ACTION_ADD; // ADD, LOAD, SAVE

void add_load_save_set_mode(int _mode) { mode = _mode; }

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

static char* read_save_name(const char *path) {
    int gff_id = gff_open(path);
    char buf[32];

    if (gff_id < 0) { return "<error>"; }

    gff_chunk_header_t chunk = gff_find_chunk_header(gff_id, GFF_STXT, 0);
    gff_read_chunk(gff_id, &chunk, buf, 32);
    buf[chunk.length > 31 ? 31 : chunk.length] = '\0'; // guard.

    gff_close(gff_id);

    return strdup(buf);
}

static void setup_save_load_selection() {
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
    for (int i = 0; i < id; i++) {
        //TODO: Store the name and get it back.
        snprintf(buf, BUF_MAX, SAVE_FORMAT, i);
        //entries[i] = strdup(buf);
        entries[i] = read_save_name(buf);
        valids[i] = i;
        num_entries++;
        num_valid_entries++;
    }
}

void add_load_save_init(SDL_Renderer *_renderer, const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = main_get_zoom();
    selection = -1;
    renderer = _renderer;
    num_valid_entries = 0;
    xoffset = x;
    yoffset = y;

    background = new_sprite_create(renderer, pal, 0 + x / zoom, 0 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 3009);
    up_arrow = new_sprite_create(renderer, pal, 215 + x / zoom, 30 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2093);
    down_arrow = new_sprite_create(renderer, pal, 215 + x / zoom, 130 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2094);
    exit_btn = new_sprite_create(renderer, pal, 230 + x / zoom, 50 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2058);
    delete_btn = new_sprite_create(renderer, pal, 215 + x / zoom, 150 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2097);
    bar = new_sprite_create(renderer, pal, 45 + x / zoom, 31 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 18100);
    name_tb = textbox_create(32, 50 + x / zoom, 148 + y / zoom);
    main_set_textbox(name_tb);

    switch(mode) {
        case ACTION_ADD:
            title = new_sprite_create(renderer, pal, 115 + x / zoom, 0 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 6036); // Add
            action_btn = new_sprite_create(renderer, pal, 230 + x / zoom, 30 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 6037); // add
            setup_character_selection();
            break;
        case ACTION_SAVE:
            title = new_sprite_create(renderer, pal, 115 + x / zoom, 0 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2056); // SAVE
            action_btn = new_sprite_create(renderer, pal, 230 + x / zoom, 30 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2057); // save
            setup_save_load_selection();
            break;
        case ACTION_LOAD:
            title = new_sprite_create(renderer, pal, 115 + x / zoom, 0 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 6030); // Load
            action_btn = new_sprite_create(renderer, pal, 230 + x / zoom, 30 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 6031); // load
            setup_save_load_selection();
            break;
        case ACTION_DROP:
            title = new_sprite_create(renderer, pal, 115 + x / zoom, 0 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 6038); // Drop
            action_btn = new_sprite_create(renderer, pal, 230 + x / zoom, 30 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 6039); // drop
            break;
    }
}

void add_load_save_render(void *data, SDL_Renderer *renderer) {
    const float zoom = main_get_zoom();
    sprite_render(renderer, background);
    sprite_render(renderer, up_arrow);
    sprite_render(renderer, down_arrow);
    sprite_render(renderer, exit_btn);
    sprite_render(renderer, delete_btn);
    sprite_render(renderer, title);
    sprite_render(renderer, action_btn);
    for (unsigned int i = 0; i < 10; i++) {
        sprite_set_frame(bar, 0);
        sprite_set_location(bar, zoom * 45 + xoffset, zoom * (31 + i * 11) + yoffset);

        if (selection >= 0 && (i == (selection - top_entry))) {
            sprite_set_frame(bar, 1);
        }
        if (sprite_in_rect(bar, mousex, mousey)) {
            sprite_set_frame(bar, sprite_get_frame(bar) + 2);
        }
        sprite_render(renderer, bar);
    }

    for (uint16_t i = top_entry; i < top_entry + 10 && i < num_valid_entries; i++) {
        print_line_len(renderer, (selection != i) ? FONT_GREY : FONT_REDDARK,
                entries[valids[i]],
                zoom * 47 + xoffset, zoom * (31 + (i - top_entry) * 11) + yoffset, 1<<10);
    }

    textbox_render(name_tb);
}

int add_load_save_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_spr = SPRITE_ERROR;
    uint16_t cspr = SPRITE_ERROR;
    mousex = x;
    mousey = y;

    if (sprite_in_rect(action_btn, x, y)) {
        cspr = action_btn;
    }
    if (sprite_in_rect(delete_btn, x, y)) {
        cspr = delete_btn;
    }
    if (sprite_in_rect(exit_btn, x, y)) {
        cspr = exit_btn;
    }
    if (sprite_in_rect(up_arrow, x, y)) {
        cspr = up_arrow;
    }
    if (sprite_in_rect(down_arrow, x, y)) {
        cspr = down_arrow;
    }

    sprite_set_frame(action_btn, 0);
    sprite_set_frame(exit_btn, 0);
    sprite_set_frame(delete_btn, 0);

    if (sprite_get_frame(cspr) < 2) {
        sprite_set_frame(cspr, 1);
    }

    if (last_spr != SPRITE_ERROR && last_spr != cspr) {
        if (sprite_get_frame(last_spr) < 2) {
            sprite_set_frame(last_spr, 0);
        }
    }

    last_spr = cspr;
    return 1; // means I captured the mouse movement
}

int add_load_save_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    const float zoom = main_get_zoom();
    if (sprite_in_rect(action_btn, x, y)) {
        sprite_set_frame(action_btn, 2);
    }
    if (sprite_in_rect(exit_btn, x, y)) {
        sprite_set_frame(exit_btn, 2);
    }
    if (sprite_in_rect(delete_btn, x, y)) {
        sprite_set_frame(delete_btn, 2);
    }
    if (sprite_in_rect(up_arrow, x, y)) {
        sprite_set_frame(up_arrow, 3);
    }
    if (sprite_in_rect(down_arrow, x, y)) {
        sprite_set_frame(down_arrow, 3);
    }
    for (int i = 0; i < 10; i++) {
        sprite_set_location(bar, xoffset + (zoom * 45), yoffset + (zoom * (31 + i * 11)));
        if (sprite_in_rect(bar, x, y)) {
            selection = top_entry + i;
            if (selection < num_valid_entries) {
                char_selected = res_ids[valids[selection]];
                textbox_set_text(name_tb, entries[valids[selection]]);
            }
        }
    }

    textbox_set_focus(name_tb, (textbox_is_in(name_tb, x, y)));

    return 1; // means I captured the mouse click
}

static void load_game() {
    char buf[128];
    snprintf(buf, 128, SAVE_FORMAT, selection);
    window_clear();
    player_close();
    player_init();
    ls_load_save_file(buf);
    main_center_on_player();
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

int add_load_save_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    char filename[32];

    if (sprite_in_rect(action_btn, x, y)) {
        if (mode == ACTION_LOAD) {
            load_game();
            return 1;
        }
        if (mode == ACTION_SAVE) {
            //TODO: Fix for naming
            if (selection < 0) {
                selection = find_next_save_file();
            }
            if (selection < 0) { return 0; }
            snprintf(filename, 31, SAVE_FORMAT, selection);
            ls_save_to_file(filename, textbox_get_text(name_tb));
        }
        sprite_set_frame(action_btn, 0);
        if (selection != -1) {
            last_action = mode;
            window_pop();
        }
    }
    if (sprite_in_rect(exit_btn, x, y)) {
        sprite_set_frame(exit_btn, 0);
        window_pop();
    }
    if (sprite_in_rect(delete_btn, x, y)) {
        sprite_set_frame(delete_btn, 0);
        if (selection != -1) {
            sprite_set_frame(delete_btn, 3);
            window_push(renderer, &popup_window, 90, 62);
            popup_set_message("DELETE THIS PERSON?");
            popup_set_option(0, "YES");
            popup_set_option(1, "NO");
            popup_set_option(2, "CANCEL");
        }
    }
    if (sprite_in_rect(up_arrow, x, y)) {
        sprite_set_frame(up_arrow, 0);
        if (top_entry > 0) { top_entry--; }
    }
    if (sprite_in_rect(down_arrow, x, y)) {
        sprite_set_frame(down_arrow, 0);
        if (top_entry < num_valid_entries - 10) { top_entry++; }
    }
    return 1; // means I captured the mouse click
}

void add_load_save_return_control () {
    sprite_set_frame(delete_btn, 0);
    if (popup_get_selection() == POPUP_0) {
        //printf("Need to delete! %d\n", res_ids[valids[selection]]);
        gff_char_delete(res_ids[valids[selection]]);
        setup_character_selection();
    }
    popup_clear_selection();
}

void add_load_save_free() {
    sprite_free(background);
    sprite_free(up_arrow);
    sprite_free(down_arrow);
    sprite_free(exit_btn);
    sprite_free(delete_btn);
    sprite_free(action_btn);
    sprite_free(title);
    sprite_free(bar);
    free_entries();
    textbox_free(name_tb);
    main_set_textbox(NULL);
}

int add_load_save_get_action() { return last_action; }
uint32_t add_load_save_get_selection() { return char_selected; }

wops_t als_window = {
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
