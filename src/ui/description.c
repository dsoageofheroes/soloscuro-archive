#include "popup.h"
#include "narrate.h"
#include "gff.h"
#include "port.h"
#include "gfftypes.h"
#include "settings.h"
#include <string.h>

static sol_sprite_t background, icon;
static char main_text[128];
static sol_dim_t main_loc;

static void description_init(const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = settings_zoom();

    background = sol_sprite_new(pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 15002);
    main_loc.x = (x + sol_sprite_getw(background)/2) * zoom;
    main_loc.y = (y) * zoom;

    sol_sprite_set_location(background, main_loc.x, main_loc.y);
    sol_sprite_set_location(icon, main_loc.x + (8 * zoom) , main_loc.y + (8 * zoom) );
}

static int next_pos_main_text(const int pos, const int len) {
    int next_pos = pos;

    for (int i = pos; i < pos + len && i < 128; i++) {
        if (main_text[i] == ' ') {
            next_pos = i;
        }
    }

    return next_pos;
}

static void description_render(void *data) {
    sol_sprite_render(background);
    sol_sprite_render(icon);
    uint16_t x = main_loc.x + (27 * settings_zoom());
    uint16_t y = main_loc.y + (15 * settings_zoom());
    int len = strlen(main_text);

    int pos = next_pos_main_text(0, 20);
    //sol_print_line_len(FONT_GREY, main_text, main_loc.x, main_loc.y, sizeof(main_text));
    sol_print_line_len(FONT_BLACK, main_text, x, y, pos);
    x -= 15 * settings_zoom();

    int next_pos = pos + 1;
    pos = next_pos_main_text(next_pos, 30);
    while (next_pos < len) {
        y += (10 * settings_zoom());
        sol_print_line_len(FONT_BLACK, main_text + next_pos, x, y, pos);
        next_pos = pos + 1;
        pos = next_pos_main_text(next_pos, 30);
        printf("pos = %d\n", pos);
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
    sol_sprite_free(background);
}

extern void sol_description_set_message(const char *msg) {
    strncpy(main_text, msg, 128);
}

extern void sol_description_set_icon(const uint16_t icon_id) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    icon = sol_sprite_new(pal, 0, 0, settings_zoom(), RESOURCE_GFF_INDEX, GFF_ICON, icon_id);
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
