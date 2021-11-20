#include "popup.h"
#include "narrate.h"
#include "gff.h"
#include "../../src/port.h"
#include "gfftypes.h"
#include "../../src/settings.h"
#include <string.h>

static sol_sprite_t background, popup_return, option[3];
static char main_text[32], option_text[3][32];
static uint8_t selection = POPUP_NOTHING;
static sol_font_t option_font[3];

sol_dim_t main_loc, option_loc[3];

static sol_dim_t initial_locs[] = {{ 10, 4, 0, 0 }, // main
                                  { 10, 17, 0, 0 }, // option 0
                                  { 10, 29, 0, 0 }, // option 1
                                  { 10, 41, 0, 0 }, // option 2
};

extern void sol_popup_set_message(const char *msg) {
    strncpy(main_text, msg, 31);
    main_text[31] = '\0';
}

extern void sol_popup_set_option(const sol_popup_t idx, const char *msg) {
    if (idx < 0 || idx > 2) { return; }
    strncpy(option_text[idx], msg, 31);
    option_text[idx][31] = '\0';
}

static void popup_init(const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = settings_zoom();

    background = sol_sprite_new(pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 14000);
    popup_return = sol_sprite_new(pal, 103 + x, 36 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 5012);
    option[0] = sol_sprite_new(pal, 8 + x, 17 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 14101);
    option[1] = sol_sprite_new(pal, 8 + x, 29 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 14102);
    option[2] = sol_sprite_new(pal, 8 + x, 41 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 14103);

    memset(main_text, 0x0, sizeof(main_text));
    memset(option_text, 0x0, sizeof(option_text));

    main_loc.x = (initial_locs[0].x + x) * zoom;
    main_loc.y = (initial_locs[0].y + y) * zoom;
    for (int i = 0; i < 3; i++) {
        option_loc[i].x = (initial_locs[i + 1].x + x) * zoom;
        option_loc[i].y = (initial_locs[i + 1].y + y) * zoom;
    }

    option_font[0] = FONT_BLACK;
    option_font[1] = FONT_BLACK;
    option_font[2] = FONT_BLACK;

    selection = POPUP_NOTHING;
}

static void popup_render(void *data) {
    sol_sprite_render(background);
    sol_sprite_render(popup_return);
    sol_sprite_render(option[0]);
    sol_sprite_render(option[1]);
    sol_sprite_render(option[2]);

    sol_print_line_len(FONT_GREY, main_text, main_loc.x, main_loc.y, sizeof(main_text));
    sol_print_line_len(option_font[0], option_text[0], option_loc[0].x, option_loc[0].y, sizeof(option_text[0]));
    sol_print_line_len(option_font[1], option_text[1], option_loc[1].x, option_loc[1].y, sizeof(option_text[1]));
    sol_print_line_len(option_font[2], option_text[2], option_loc[2].x, option_loc[2].y, sizeof(option_text[2]));
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    if (sol_sprite_in_rect(popup_return, x, y)) { return popup_return; }
    if (sol_sprite_in_rect(option[0], x, y)) { return option[0]; }
    if (sol_sprite_in_rect(option[1], x, y)) { return option[1]; }
    if (sol_sprite_in_rect(option[2], x, y)) { return option[2]; }
    
    return SPRITE_ERROR;
}

static int popup_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;

    uint16_t cur_sprite = get_sprite_mouse_is_on(x, y);

    if (last_sprite != cur_sprite) {
        if (cur_sprite == option[0]) { option_font[0] = FONT_RED; }
        if (cur_sprite == option[1]) { option_font[1] = FONT_RED; }
        if (cur_sprite == option[2]) { option_font[2] = FONT_RED; }
        sol_sprite_set_frame(cur_sprite, sol_sprite_get_frame(cur_sprite) + 1);
        if (last_sprite != SPRITE_ERROR) {
            sol_sprite_set_frame(last_sprite, sol_sprite_get_frame(last_sprite) - 1);
            if (last_sprite == option[0]) { option_font[0] = FONT_BLACK; }
            if (last_sprite == option[1]) { option_font[1] = FONT_BLACK; }
            if (last_sprite == option[2]) { option_font[2] = FONT_BLACK; }
        }
    }
    
    last_sprite = cur_sprite;
    return 1;// did not handle
}

static int popup_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (sol_sprite_in_rect(option[0], x, y)) { return option[0]; }
    if (sol_sprite_in_rect(option[1], x, y)) { return option[1]; }
    if (sol_sprite_in_rect(option[2], x, y)) { return option[2]; }

    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another window may.
}

static int popup_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (sol_sprite_in_rect(option[0], x, y)) { selection = POPUP_0; }
    if (sol_sprite_in_rect(option[1], x, y)) { selection = POPUP_1; }
    if (sol_sprite_in_rect(option[2], x, y)) { selection = POPUP_2; }
    if (sol_sprite_in_rect(popup_return, x, y)) { selection = POPUP_CANCEL; }

    if (selection != POPUP_NOTHING) {
        sol_window_pop();
    }

    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another window may.
}

static void popup_free() {
    sol_sprite_free(background);
    sol_sprite_free(popup_return);
    sol_sprite_free(option[0]);
    sol_sprite_free(option[1]);
    sol_sprite_free(option[2]);
}

uint8_t sol_popup_get_selection() { return selection; }
void sol_popup_clear_selection() { selection = POPUP_NOTHING; }

sol_wops_t popup_window = {
    .init = popup_init,
    .cleanup = popup_free,
    .render = popup_render,
    .mouse_movement = popup_handle_mouse_movement,
    .mouse_down = popup_handle_mouse_down,
    .mouse_up = popup_handle_mouse_up,
    .grey_out_map = 1,
    .data = NULL
};
