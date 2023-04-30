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
#include "gpl-manager.h"

#define MAX_SCREENS (10)

static sol_wops_t windows[MAX_SCREENS];
static uint32_t window_pos = 0;
static uint32_t mousex, mousey;

extern sol_status_t sol_window_init() {
    memset(windows, 0x0, sizeof(sol_wops_t) * MAX_SCREENS);
    return SOL_SUCCESS;
}

static uint32_t default_get_width() { return 320 * settings_zoom(); }
static uint32_t default_get_height() { return 200 * settings_zoom(); }

extern sol_status_t sol_window_load(int layer, sol_wops_t *window, const uint32_t x, const uint32_t y) {
    int grey_out_map = 0;

    if (layer < 0 || layer > MAX_SCREENS) { return SOL_OUT_OF_RANGE; }

    if (windows[layer].render) {
        // cleanup?
    }

    windows[layer] = *window;

    if (!windows[layer].get_width) { windows[layer].get_width = default_get_width; }
    if (!windows[layer].get_height) { windows[layer].get_height = default_get_height; }

    if (windows[layer].init) {
        windows[layer].init(
            ((settings_screen_width() - windows[layer].get_width()) / 2),
            ((settings_screen_height() - windows[layer].get_height()) / 2));
    }

    for (uint32_t i = 0; i < window_pos && !grey_out_map; i++) {
        grey_out_map = windows[i].grey_out_map;
    }

    if (grey_out_map) {
        sol_map_apply_alpha(127);
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_window_toggle(sol_wops_t *the_window, const uint32_t x, const uint32_t y) {
    uint32_t pos;
    sol_wops_t tmp;
    for (pos = 0; pos < window_pos; pos++) {
        if (windows[pos].render == the_window->render) { break; }
    }

    if (pos >= window_pos) { // not found so turn on.
        return sol_window_load(window_pos++, the_window, x, y);
    }

    tmp = windows[pos]; // Found, so we bring to the front, then pop.

    while (pos < (window_pos - 1)) {
        windows[pos] = windows[pos + 1];
        pos++;
    }

    windows[window_pos - 1] = tmp;

    return sol_window_pop();
}

extern sol_status_t sol_window_push(sol_wops_t *window, const uint32_t x, const uint32_t y) {
    sol_wops_t tmp;
    for (uint32_t i = 0; i <= window_pos; i++) {
        if (windows[i].render == window->render) {
            tmp = windows[i];
            for (uint32_t j = i + 1; j <= window_pos; j++) {
                windows[j - 1] = windows[j];
            }
            windows[window_pos - 1] = tmp;
            return SOL_SUCCESS;
        }
    }

    return sol_window_load(window_pos++, window, x, y);
}

extern sol_status_t sol_window_load_region(const int region) {
    sol_entity_t *dude;
    sol_region_t *reg = NULL;

    sol_region_manager_get_region(region, 0, &reg);
    sol_window_clear();
    sol_region_manager_set_current(reg);

    sol_window_push(&map_window, 0, 0);
    sol_window_push(&narrate_window, 0, 0);
    sol_window_push(&combat_status_window, 295, 5);

    sol_status_check(sol_region_manager_load_etab(reg),
            "Unable to load etab");

    sol_status_check(sol_region_load_sprites(reg), "Unable to load region's sprites.");

    sol_entity_list_for_each(reg->entities, dude) {
        //printf("ID: %d\n", dude->ds_id);
        sol_status_check(sol_entity_update_scmd(dude), "Unable to update dude's scmd!\n");
        //sol_status_check(sol_animate_shift_entity(reg->entities, entity_list_find(reg->entities, dude)),
        sol_entity_list_node_t *node;
        sol_entity_list_find(reg->entities, dude, &node);
        sol_status_check(sol_animate_shift_entity(reg->entities, node),
            "Unable to shift entity during map load.");
    }
    // 10511, 2600
    // -1203, bmp:1860
    //exit(1);

    sol_center_on_player();

    // Now that we are loaded, execute the script!
    sol_gpl_lua_execute_script(reg->map_id, 0, 1);

    return SOL_SUCCESS;
}

extern sol_status_t sol_window_render(const uint32_t xmappos, const uint32_t ymappos) {
    port_start_display_frame();

    for (int i = 0; i < MAX_SCREENS; i++) {
        if (windows[i].render) {
            windows[i].render(windows[i].data);
        }
    }

    port_commit_display_frame();

    return SOL_SUCCESS;
}

extern sol_status_t sol_window_handle_mouse(const uint32_t x, const uint32_t y) {
    mousex = x;
    mousey = y;
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].mouse_movement && windows[i].mouse_movement(x, y)) {
            i = 0; // exit loop, mouse has been handled!
        }
    }
    return SOL_SUCCESS;
}

extern sol_status_t sol_window_handle_mouse_down(const sol_mouse_button_t button, const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].mouse_down && windows[i].mouse_down(button, x, y)) {
            return SOL_SUCCESS;
        }
    }
    return SOL_NOT_HANDLED;
}

extern sol_status_t sol_window_handle_key_down(const enum sol_entity_action_e action) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].key_down && windows[i].key_down(action)) {
            return SOL_SUCCESS;
        }
    }
    return SOL_NOT_HANDLED;
}

extern sol_status_t sol_window_handle_key_press(const enum sol_key_e e) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].key_press && windows[i].key_press(e)) {
            return SOL_SUCCESS;
        }
    }
    return SOL_NOT_HANDLED;
}

extern sol_status_t sol_window_handle_mouse_up(const sol_mouse_button_t button, const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].mouse_up && windows[i].mouse_up(button, x, y)) {
            i = 0; // exit loop, mouse has been handled!
            return SOL_SUCCESS;
        }
    }
    return SOL_NOT_HANDLED;
}

static void destroy_window(const int pos) {
    if (windows[pos].cleanup) {
        windows[pos].cleanup();
    }

    memset(windows + pos, 0x0, sizeof(sol_wops_t));
}

extern sol_status_t sol_window_clear() {
    while(window_pos) {
        sol_window_pop();
    }
    return SOL_SUCCESS;
}

extern sol_status_t sol_window_pop() {
    int grey_out_map = 0;

    destroy_window(--window_pos);

    if (window_pos > 0 && windows[window_pos - 1].return_control) {
        windows[window_pos - 1].return_control();
    }

    for (uint32_t i = 0; i < window_pos && !grey_out_map; i++) {
        grey_out_map = windows[i].grey_out_map;
    }

    if (!grey_out_map) {
        sol_map_apply_alpha(255);
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_window_free() {
    for (int i = 0; i < MAX_SCREENS; i++) {
        if (windows[i].cleanup) {
            windows[i].cleanup();
        }
    }

    return sol_font_free();
}
