#include "popup.h"
#include "narrate.h"
#include "gff.h"
#include "port.h"
#include "gfftypes.h"
#include "settings.h"
#include <string.h>

#define BUF_SIZE (512)

static sol_sprite_t background, icon;
static char main_text[BUF_SIZE];

static void description_init(const uint32_t x, const uint32_t y) {
    sol_sprite_info_t info;
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = settings_zoom();

    sol_status_check(sol_sprite_new(pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 15002, &background),
               "Unable to load description sprite.");
    sol_status_check(sol_sprite_get_info(background, &info), "Unable to get background sprite info.");
    sol_sprite_set_location(background, (x + info.w/2) * zoom,
                                        y * zoom);
    if (icon != SPRITE_ERROR) {
        sol_sprite_set_location(icon, info.x + (8 * zoom),
             info.y + (8 * zoom) );
    }
}

static int next_pos_main_text(const int pos, const int len) {
    int next_pos = pos;

    for (int i = pos; i <= pos + len && i < BUF_SIZE; i++) {
        if (main_text[i] == ' ' || main_text[i] == '\t' || main_text[i] == '\0') {
            next_pos = i;
        }
    }

    return next_pos;
}

static void description_render(void *data) {
    sol_sprite_info_t info;
    sol_sprite_render(background);
    sol_sprite_render(icon);
    sol_status_check(sol_sprite_get_info(icon, &info), "Unable to get icon sprite info");
    uint16_t x = info.x + (20 * settings_zoom());
    uint16_t y = info.y + (8 * settings_zoom());
    int len = strlen(main_text) + 1;
    int pos = next_pos_main_text(0, 20);

    sol_status_check(sol_print_line_len(FONT_BLACK, main_text, x, y, pos), "Unable to print line.");
    x -= 20 * settings_zoom();

    int next_pos = pos + 1;
    pos = next_pos_main_text(next_pos, 26);
    while (next_pos < len) {
        y += (9 * settings_zoom());
        sol_status_check(sol_print_line_len(FONT_BLACK, main_text + next_pos, x, y, pos - next_pos), "Unable to print line.");
        next_pos = pos + 1;
        pos = next_pos_main_text(next_pos, 26);
    }
}

static int description_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    return 1;// did not handle
}

static int description_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    return 1; // means I captured the mouse click
}

static int description_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    sol_window_pop();
    return 1; // means I captured the mouse click
}

static void description_free() {
    sol_status_check(sol_sprite_free(background), "Unable to free sprite");
}

extern sol_status_t sol_description_set_message(const char *msg) {
    if (!msg) { return SOL_NULL_ARGUMENT; }
    strncpy(main_text, msg, BUF_SIZE - 1);
    main_text[BUF_SIZE - 1] = '\0';
    return SOL_SUCCESS;
}

extern sol_status_t sol_description_set_icon(const uint16_t icon_id) {
    const float zoom = settings_zoom();
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    sol_sprite_info_t info;

    sol_status_check(sol_sprite_new(pal, 0, 0, settings_zoom(), RESOURCE_GFF_INDEX, GFF_ICON, icon_id, &icon),
         "Unable to load description icon sprite");
    sol_status_check(sol_sprite_get_info(icon, &info), "Unable to get background sprite info");
    sol_sprite_set_location(icon, info.x + (8 * zoom),
             info.y + (8 * zoom) );
    return SOL_SUCCESS;
}

sol_wops_t description_window = {
    .init = description_init,
    .cleanup = description_free,
    .render = description_render,
    .mouse_movement = description_handle_mouse_movement,
    .mouse_down = description_handle_mouse_down,
    .mouse_up = description_handle_mouse_up,
    .grey_out_map = 1,
    .data = NULL
};
