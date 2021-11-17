#include <stdlib.h>
#include <string.h>

#include "window-manager.h"
#include "dsl.h"
#include "player.h"
#include "region.h"
#include "settings.h"
#include "port.h"
#include "entity-animation.h"
#include "../sdl/player.h"
#include "narrate.h"
#include "../sdl/windows/inventory.h"
#include "../sdl/windows/combat-status.h"
#include "../sdl/windows/new-character.h"
#include "view-character.h"
#include "../sdl/windows/window-main.h"

#include "../sdl/map.h"

#define MAX_SCREENS (10)

static wops_t windows[MAX_SCREENS];
static uint32_t window_pos = 0;

void window_init() {
    memset(windows, 0x0, sizeof(wops_t) * MAX_SCREENS);
}

static uint32_t default_get_width() { return 320 * settings_zoom(); }
static uint32_t default_get_height() { return 200 * settings_zoom(); }
uint32_t main_get_width()  { return 800; }
uint32_t main_get_height() { return 600; }

void window_load(int layer, wops_t *window, const uint32_t x, const uint32_t y) {
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

extern void port_load_window(const window_t window) {
    switch (window) {
        case WINDOW_VIEW: window_toggle(&view_character_window, 0, 0); break;
        case WINDOW_INVENTORY: window_toggle(&inventory_window, 0, 0); break;
        case WINDOW_MAIN: window_toggle(&main_window, 0, 0); break;
        case WINDOW_CHARACTER_CREATION: window_toggle(&new_character_window, 0, 0); break;
        case WINDOW_MAP: window_toggle(&map_window, 0, 0); break;
        case WINDOW_NARRATE: window_toggle(&narrate_window, 0, 0); break;
        case WINDOW_COMBAT: window_toggle(&combat_status_window, 0, 0); break;
    }
}

void window_toggle(wops_t *the_window, const uint32_t x, const uint32_t y) {
    uint32_t pos;
    wops_t tmp;
    for (pos = 0; pos < window_pos; pos++) {
        if (windows[pos].render == the_window->render) { break; }
    }

    if (pos >= window_pos) { // not found so turn on.
        window_load(window_pos++, the_window, x, y);
        return;
    }

    tmp = windows[pos]; // Found, so we bring to the front, then pop.

    while (pos < (window_pos - 1)) {
        windows[pos] = windows[pos + 1];
        pos++;
    }

    windows[window_pos - 1] = tmp;

    window_pop();
}

extern void port_toggle_window(const window_t window) {
    switch (window) {
        case WINDOW_VIEW: window_toggle(&view_character_window, 0, 0); break;
        case WINDOW_INVENTORY: window_toggle(&inventory_window, 0, 0); break;
        case WINDOW_MAIN: window_toggle(&main_window, 0, 0); break;
        case WINDOW_CHARACTER_CREATION: window_toggle(&new_character_window, 0, 0); break;
        case WINDOW_MAP: window_toggle(&map_window, 0, 0); break;
        case WINDOW_NARRATE: window_toggle(&narrate_window, 0, 0); break;
        case WINDOW_COMBAT: window_toggle(&combat_status_window, 0, 0); break;
    }
}

void window_push(wops_t *window, const uint32_t x, const uint32_t y) {
    wops_t tmp;
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
    window_load(window_pos++, window, x, y);
}

void port_change_region(region_t *reg) {
    map_load_region(reg);

    for (int i = 0; i < MAX_PCS; i++) {
        sol_player_load(i, settings_zoom());
    }
}

int port_load_region(const int region) {
    return window_load_region(region);
}

int window_load_region(const int region) {
    map_load_map(region);

    window_push(&map_window, 0, 0);
    window_push(&narrate_window, 0, 0);
    window_push(&combat_status_window, 295, 5);

    return 1;
}

void window_render(const uint32_t xmappos, const uint32_t ymappos) {
    port_start_display_frame();

    for (int i = 0; i < MAX_SCREENS; i++) {
        if (windows[i].render) {
            windows[i].render(windows[i].data);
        }
    }

    port_commit_display_frame();
}

void window_handle_mouse(const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].mouse_movement && windows[i].mouse_movement(x, y)) {
            i = 0; // exit loop, mouse has been handled!
        }
    }
}

void window_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].mouse_down && windows[i].mouse_down(button, x, y)) {
            i = 0; // exit loop, mouse has been handled!
        }
    }
}

int window_handle_key_down(const enum entity_action_e action) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].key_down && windows[i].key_down(action)) {
            return 1;
        }
    }
    return 0;
}

void window_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
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

    memset(windows + pos, 0x0, sizeof(wops_t));
}

void window_clear() {
    while(window_pos) {
        window_pop();
    }
}

void window_pop() {
    int grey_out_map = 0;

    destroy_window(--window_pos);

    printf("window_pos = %d\n", window_pos);
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

void window_free() {
    for (int i = 0; i < MAX_SCREENS; i++) {
        if (windows[i].cleanup) {
            windows[i].cleanup();
        }
    }
    animate_close();
    font_free();
}
