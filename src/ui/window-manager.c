#include <stdlib.h>
#include <string.h>

#include "window-manager.h"
#include "gpl.h"
#include "description.h"
#include "player.h"
#include "region.h"
#include "settings.h"
#include "port.h"
#include "entity-animation.h"
#include "font.h"
#include "narrate.h"
#include "inventory.h"
#include "combat-status.h"
#include "new-character.h"
#include "view-character.h"
#include "window-main.h"
#include "region-manager.h"
#include "map.h"

#define MAX_SCREENS (10)

static sol_wops_t windows[MAX_SCREENS];
static uint32_t window_pos = 0;

extern void sol_window_init() {
    memset(windows, 0x0, sizeof(sol_wops_t) * MAX_SCREENS);
}

static uint32_t default_get_width() { return 320 * settings_zoom(); }
static uint32_t default_get_height() { return 200 * settings_zoom(); }
uint32_t main_get_width()  { return 800; }
uint32_t main_get_height() { return 600; }

extern void sol_window_load(int layer, sol_wops_t *window, const uint32_t x, const uint32_t y) {
    int grey_out_map = 0;

    if (layer < 0 || layer > MAX_SCREENS) { return; }

    if (windows[layer].render) {
        // cleanup?
    }

    windows[layer] = *window;

    if (!windows[layer].get_width) { windows[layer].get_width = default_get_width; }
    if (!windows[layer].get_height) { windows[layer].get_height = default_get_height; }

    if (windows[layer].init) {
        windows[layer].init(
            ((main_get_width() - windows[layer].get_width()) / 2),
            ((main_get_height() - windows[layer].get_height()) / 2));
    }

    for (uint32_t i = 0; i < window_pos && !grey_out_map; i++) {
        grey_out_map = windows[i].grey_out_map;
    }

    if (grey_out_map) {
        map_apply_alpha(127);
    }
}

extern void sol_window_toggle(sol_wops_t *the_window, const uint32_t x, const uint32_t y) {
    uint32_t pos;
    sol_wops_t tmp;
    for (pos = 0; pos < window_pos; pos++) {
        if (windows[pos].render == the_window->render) { break; }
    }

    if (pos >= window_pos) { // not found so turn on.
        sol_window_load(window_pos++, the_window, x, y);
        return;
    }

    tmp = windows[pos]; // Found, so we bring to the front, then pop.

    while (pos < (window_pos - 1)) {
        windows[pos] = windows[pos + 1];
        pos++;
    }

    windows[window_pos - 1] = tmp;

    sol_window_pop();
}

extern void sol_window_push(sol_wops_t *window, const uint32_t x, const uint32_t y) {
    sol_wops_t tmp;
    for (uint32_t i = 0; i <= window_pos; i++) {
        if (windows[i].render == window->render) {
            tmp = windows[i];
            for (uint32_t j = i + 1; j <= window_pos; j++) {
                windows[j - 1] = windows[j];
            }
            windows[window_pos - 1] = tmp;
            return;
        }
    }
    sol_window_load(window_pos++, window, x, y);
}

void port_change_region(sol_region_t *reg) {
    map_load_region(reg);

    for (int i = 0; i < MAX_PCS; i++) {
        sol_player_load_zoom(i, settings_zoom());
    }
}

int port_load_region(const int region) {
    return sol_window_load_region(region);
}

extern int sol_window_load_region(const int region) {
    sol_region_manager_set_current(sol_region_manager_get_region(region));

    sol_window_push(&map_window, 0, 0);
    sol_window_push(&narrate_window, 0, 0);
    sol_window_push(&combat_status_window, 295, 5);

    return 1;
}

extern void sol_window_render(const uint32_t xmappos, const uint32_t ymappos) {
    port_start_display_frame();

    for (int i = 0; i < MAX_SCREENS; i++) {
        if (windows[i].render) {
            windows[i].render(windows[i].data);
        }
    }

    port_commit_display_frame();
}

extern void sol_window_handle_mouse(const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].mouse_movement && windows[i].mouse_movement(x, y)) {
            i = 0; // exit loop, mouse has been handled!
        }
    }
}

extern void sol_window_handle_mouse_down(const sol_mouse_button_t button, const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].mouse_down && windows[i].mouse_down(button, x, y)) {
            i = 0; // exit loop, mouse has been handled!
        }
    }
}

extern int sol_window_handle_key_down(const enum entity_action_e action) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].key_down && windows[i].key_down(action)) {
            return 1;
        }
    }
    return 0;
}

extern void sol_window_handle_mouse_up(const sol_mouse_button_t button, const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].mouse_up && windows[i].mouse_up(button, x, y)) {
            i = 0; // exit loop, mouse has been handled!
        }
    }
}

static void destroy_window(const int pos) {
    if (windows[pos].cleanup) {
        windows[pos].cleanup();
    }

    memset(windows + pos, 0x0, sizeof(sol_wops_t));
}

void sol_window_clear() {
    while(window_pos) {
        sol_window_pop();
    }
}

void sol_window_pop() {
    int grey_out_map = 0;

    destroy_window(--window_pos);

    if (window_pos > 0 && windows[window_pos - 1].return_control) {
        windows[window_pos - 1].return_control();
    }

    for (uint32_t i = 0; i < window_pos && !grey_out_map; i++) {
        grey_out_map = windows[i].grey_out_map;
    }

    if (!grey_out_map) {
        map_apply_alpha(255);
    }
}

void sol_window_free() {
    for (int i = 0; i < MAX_SCREENS; i++) {
        if (windows[i].cleanup) {
            windows[i].cleanup();
        }
    }
    printf("TODO: Do we need to close animations?\n");
    //animate_close();
    sol_font_free();
}
