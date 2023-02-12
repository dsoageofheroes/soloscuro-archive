#include "popup.h"
#include "narrate.h"
#include "gff.h"
#include "port.h"
#include "font.h"
#include "gfftypes.h"
#include "settings.h"
#include <string.h>

static sol_sprite_t background, popup_return, option[3], background_popup;
static char main_text[32], option_text[3][32];
static int16_t popup_count = -1;
static uint8_t selection = POPUP_NOTHING;
static sol_font_t option_font[3];
static char *popup_text = NULL;

sol_dim_t main_loc, option_loc[3];

static sol_dim_t initial_locs[] = {{ 10, 4, 0, 0 }, // main
                                  { 10, 17, 0, 0 }, // option 0
                                  { 10, 29, 0, 0 }, // option 1
                                  { 10, 41, 0, 0 }, // option 2
};

extern sol_status_t sol_popup_set_message(const char *msg) {
    strncpy(main_text, msg, 31);
    main_text[31] = '\0';
    return SOL_SUCCESS;
}

extern sol_status_t sol_popup_set_option(const sol_popup_t idx, const char *msg) {
    if (idx < 0 || idx > 2) { return SOL_OUT_OF_RANGE; }
    strncpy(option_text[idx], msg, 31);
    option_text[idx][31] = '\0';
    return SOL_SUCCESS;
}

static void popup_init(const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = settings_zoom();

    sol_status_check(sol_sprite_new(pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 14000, &background),
            "unable to load popup background sprite");
    sol_status_check(sol_sprite_new(pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 10001, &background_popup),
            "unable to load popup background_popup sprite");
    sol_status_check(sol_sprite_new(pal, 103 + x, 36 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 5012, &popup_return),
            "unable to load popup return sprite");
    sol_status_check(sol_sprite_new(pal, 8 + x, 17 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 14101, &option[0]),
            "unable to load popup option0 sprite");
    sol_status_check(sol_sprite_new(pal, 8 + x, 29 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 14102, &option[1]),
            "unable to load popup option1 sprite");
    sol_status_check(sol_sprite_new(pal, 8 + x, 41 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 14103, &option[2]),
            "unable to load popup option2 sprite");

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

static sol_status_t render_option() {
    sol_status_t status;
    if ((status = sol_sprite_render(background)) != SOL_SUCCESS) { return status; }
    if ((status = sol_sprite_render(popup_return)) != SOL_SUCCESS) { return status; }
    if ((status = sol_sprite_render(option[0])) != SOL_SUCCESS) { return status; }
    if ((status = sol_sprite_render(option[1])) != SOL_SUCCESS) { return status; }
    if ((status = sol_sprite_render(option[2])) != SOL_SUCCESS) { return status; }

    if ((status = sol_print_line_len(FONT_GREY, main_text, main_loc.x, main_loc.y, sizeof(main_text))) != SOL_SUCCESS) { return status; }
    if ((status = sol_print_line_len(option_font[0], option_text[0], option_loc[0].x, option_loc[0].y, sizeof(option_text[0]))) != SOL_SUCCESS) { return status; }
    if ((status = sol_print_line_len(option_font[1], option_text[1], option_loc[1].x, option_loc[1].y, sizeof(option_text[1]))) != SOL_SUCCESS) { return status; }
    if ((status = sol_print_line_len(option_font[2], option_text[2], option_loc[2].x, option_loc[2].y, sizeof(option_text[2]))) != SOL_SUCCESS) { return status; }

    return SOL_SUCCESS;
}

static sol_status_t render_popup() {
    sol_status_t status;
    const float zoom = settings_zoom();
    const int len = strlen(popup_text);
    uint32_t pixel_len = 0;
    
    if ((status = sol_font_pixel_width(FONT_GREY, popup_text, len, &pixel_len)) != SOL_SUCCESS) {
        return status;
    }

    sol_sprite_info_t info;
    sol_status_check(sol_sprite_get_info(background_popup, &info), "Unable to get popup sprite info");
    int x = info.x + zoom * 90 - pixel_len / 2;
    int y = info.y + zoom * 5;

    sol_sprite_render(background_popup);
    if ((status = sol_print_line_len(FONT_GREY, popup_text, x, y, len)) != SOL_SUCCESS) { return status; }

    if (popup_count > 0) {
        popup_count--;
    }
    if (popup_count == 0) {
        popup_count = -1;
        free(popup_text);
        popup_text = NULL;
        sol_window_push(&popup_window, 0, 0);
        sol_window_pop();
    }

    return SOL_SUCCESS;
}

static void popup_render(void *data) {
    (void)data;
    if (main_text[0]) { render_option(); }
    if (popup_text)   { render_popup(); }
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    if (sol_sprite_in_rect(popup_return, x, y) == SOL_SUCCESS) { return popup_return; }
    if (sol_sprite_in_rect(option[0], x, y) == SOL_SUCCESS) { return option[0]; }
    if (sol_sprite_in_rect(option[1], x, y) == SOL_SUCCESS) { return option[1]; }
    if (sol_sprite_in_rect(option[2], x, y) == SOL_SUCCESS) { return option[2]; }
    
    return SPRITE_ERROR;
}

static int popup_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;

    uint16_t cur_sprite = get_sprite_mouse_is_on(x, y);

    if (last_sprite != cur_sprite) {
        if (cur_sprite == option[0]) { option_font[0] = FONT_RED; }
        if (cur_sprite == option[1]) { option_font[1] = FONT_RED; }
        if (cur_sprite == option[2]) { option_font[2] = FONT_RED; }
        sol_sprite_increment_frame(cur_sprite, 1);
        if (last_sprite != SPRITE_ERROR) {
            sol_sprite_increment_frame(last_sprite, -1);
            if (last_sprite == option[0]) { option_font[0] = FONT_BLACK; }
            if (last_sprite == option[1]) { option_font[1] = FONT_BLACK; }
            if (last_sprite == option[2]) { option_font[2] = FONT_BLACK; }
        }
    }
    
    last_sprite = cur_sprite;
    return 1;// did not handle
}

static int popup_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (sol_sprite_in_rect(option[0], x, y) == SOL_SUCCESS) { return option[0]; }
    if (sol_sprite_in_rect(option[1], x, y) == SOL_SUCCESS) { return option[1]; }
    if (sol_sprite_in_rect(option[2], x, y) == SOL_SUCCESS) { return option[2]; }

    return 1; // means I captured the mouse click
}

static int popup_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (sol_sprite_in_rect(option[0], x, y) == SOL_SUCCESS) { selection = POPUP_0; }
    if (sol_sprite_in_rect(option[1], x, y) == SOL_SUCCESS) { selection = POPUP_1; }
    if (sol_sprite_in_rect(option[2], x, y) == SOL_SUCCESS) { selection = POPUP_2; }
    if (sol_sprite_in_rect(popup_return, x, y) == SOL_SUCCESS) { selection = POPUP_CANCEL; }

    if (selection != POPUP_NOTHING) {
        sol_window_pop();
    }

    return 1; // means I captured the mouse click
}

static void popup_free() {
    sol_status_check(sol_sprite_free(background), "Unable to free sprite");
    sol_status_check(sol_sprite_free(popup_return), "Unable to free sprite");
    sol_status_check(sol_sprite_free(option[0]), "Unable to free sprite");
    sol_status_check(sol_sprite_free(option[1]), "Unable to free sprite");
    sol_status_check(sol_sprite_free(option[2]), "Unable to free sprite");
    sol_status_check(sol_sprite_free(background_popup), "Unable to free sprite");
    if (popup_text) { free(popup_text); }
}

extern sol_status_t sol_popup_get_selection(uint8_t *sel) { *sel = selection; return SOL_SUCCESS; }
extern sol_status_t sol_popup_clear_selection() { selection = POPUP_NOTHING; return SOL_SUCCESS; }

extern sol_status_t sol_popup_quick_message(const char *msg) {
    if (popup_text) { free(popup_text); }
    popup_text = strdup(msg);
    popup_count = 60;
    return SOL_SUCCESS;
}

sol_wops_t popup_window = {
    .init = popup_init,
    .cleanup = popup_free,
    .render = popup_render,
    .mouse_movement = popup_handle_mouse_movement,
    .mouse_down = popup_handle_mouse_down,
    .mouse_up = popup_handle_mouse_up,
    .grey_out_map = 0,
    .data = NULL
};
