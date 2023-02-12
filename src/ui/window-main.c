#include "window-main.h"
#include "view-character.h"
#include "add-load-save.h"
#include "narrate.h"
#include "map.h"
#include "combat.h"
#include "combat-status.h"
#include "region-manager.h"
#include "gpl.h"
#include "gpl-manager.h"
#include "popup.h"
#include "gfftypes.h"
#include "player.h"
#include "gameloop.h"
#include "port.h"
#include "settings.h"
#include "gff.h"

static sol_sprite_t background, sun, start, create_characters, load_save, exit_dos;
static int mousex = 0, mousey = 0;
static int mouse_down = 0;
static int count_down = 0;
static uint16_t count_down_spr = SPRITE_ERROR;
static uint32_t xoffset, yoffset;

void main_init(const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = settings_zoom();
    xoffset = x;
    yoffset = y;

    sol_status_check(sol_sprite_new(pal, x / zoom, 20 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20029, &background),
            "Unable to load main sprite.");
    sol_status_check(sol_sprite_new(pal, 45 + x / zoom, 0 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20028, &sun),
            "Unable to load main sprite.");
    sol_status_check(sol_sprite_new(pal, 90 + x / zoom, 45 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2048, &start),
            "Unable to load main sprite.");
    sol_status_check(sol_sprite_new(pal, 45 + x / zoom, 62 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2049, &create_characters),
            "Unable to load main sprite.");
    sol_status_check(sol_sprite_new(pal, 60 + x / zoom, 80 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2050, &load_save),
            "Unable to load main sprite.");
    sol_status_check(sol_sprite_new(pal, 90 + x / zoom, 100 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2051, &exit_dos),
            "Unable to load main sprite.");
}

static int click_action() {
    int ret = 0;
    sol_entity_t *active;

    if (count_down_spr == exit_dos) { sol_game_loop_signal(WAIT_FINAL, 0); }
    if (count_down_spr == create_characters) { sol_window_push(&view_character_window, 0, 10); }
    if (count_down_spr == start) {
        if(sol_player_get_active(&active) == SOL_SUCCESS && active->name) {
            sol_window_pop();
            // Note: We are deallocated here, so be careful.
            info("Running Master DSL #99.\n");
            sol_gpl_lua_execute_script(99, 0, 1);
            sol_window_load_region(42);
            return 1;
        } else {
            sol_window_push(&popup_window, 100, 75);
            sol_popup_set_message("CREATE CHARACTER");
            sol_popup_set_option(0, "Ok");
            sol_popup_set_option(1, "");
            sol_popup_set_option(2, "CANCEL");
        }
    }

    if (count_down_spr == load_save) {
        sol_add_load_save_set_mode(ACTION_LOAD);
        sol_window_push(&als_window, 0, 0);
    }

    return ret;
}

void main_render(void *data) {
    sol_sprite_render(sun);
    sol_sprite_render(background);

    if (count_down > 0) {
        sol_sprite_set_frame(count_down_spr, 1 + (count_down % 2));
    }

    if (count_down > 0) {
        count_down--;
        if (count_down == 0) {
            sol_sprite_set_frame(count_down_spr, 0);
            if (click_action()) { return; }
            count_down_spr = SPRITE_ERROR;
        }
    }

    sol_sprite_render(start);
    sol_sprite_render(create_characters);
    sol_sprite_render(load_save);
    sol_sprite_render(exit_dos);
}

static uint16_t get_sprite(const uint32_t x, const uint32_t y) {
    if (sol_sprite_in_rect(start, x, y) == SOL_SUCCESS) { return start; }
    if (sol_sprite_in_rect(create_characters, x, y) == SOL_SUCCESS) { return create_characters; }
    if (sol_sprite_in_rect(load_save, x, y) == SOL_SUCCESS) { return load_save; }
    if (sol_sprite_in_rect(exit_dos, x, y) == SOL_SUCCESS) { return exit_dos; }

    return SPRITE_ERROR;
}

int main_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    mousex = x; mousey = y;
    static uint16_t last_sprite = SPRITE_ERROR;
    uint16_t cur_sprite = get_sprite(x, y);

    if (last_sprite != cur_sprite) {
        sol_sprite_increment_frame(cur_sprite, 1);
        if (last_sprite != SPRITE_ERROR) {
            sol_sprite_increment_frame(last_sprite, -1);
        }
    }
    
    last_sprite = cur_sprite;
    return 1;// handle
}

int main_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    mouse_down = 1;
    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another window may.
}

int main_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    mouse_down = 0;
    count_down = 32;
    count_down_spr = get_sprite(x, y);
    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another window may.
}

void main_free() {
    sol_status_check(sol_sprite_free(sun), "Unable to free sprite");
    sol_status_check(sol_sprite_free(background), "Unable to free sprite");
    sol_status_check(sol_sprite_free(start), "Unable to free sprite");
    sol_status_check(sol_sprite_free(create_characters), "Unable to free sprite");
    sol_status_check(sol_sprite_free(load_save), "Unable to free sprite");
    sol_status_check(sol_sprite_free(exit_dos), "Unable to free sprite");
}

sol_wops_t main_window = {
    .init = main_init,
    .cleanup = main_free,
    .render = main_render,
    .mouse_movement = main_handle_mouse_movement,
    .mouse_down = main_handle_mouse_down,
    .mouse_up = main_handle_mouse_up,
    .name = "main",
    .grey_out_map = 0,
    .data = NULL
};
