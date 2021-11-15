#include "window-main.h"
#include "../main.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"
#include "../../src/player.h"
#include "../../src/settings.h"
#include "../sprite.h"
#include "../map.h"
#include "../player.h"
#include "view-character.h"
#include "add-load-save.h"
#include "narrate.h"
#include "popup.h"

static uint16_t background, sun, start, create_characters, load_save, exit_dos;
static int mousex = 0, mousey = 0;
static int mouse_down = 0;
static int count_down = 0;
static uint16_t count_down_spr = SPRITE_ERROR;
static uint32_t xoffset, yoffset;

SDL_Renderer *renderer = NULL;

uint16_t main_sprite_create(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_create(renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

void main_init(const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    SDL_Renderer *renderer = main_get_rend();
    const float zoom = settings_zoom();
    xoffset = x;
    yoffset = y;

    background = main_sprite_create(renderer, pal, x / zoom, 20 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20029);
    sun = main_sprite_create(renderer, pal, 45 + x / zoom, 0 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20028);
    start = main_sprite_create(renderer, pal, 90 + x / zoom, 45 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2048);
    create_characters = main_sprite_create(renderer, pal, 45 + x / zoom, 62 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2049);
    load_save = main_sprite_create(renderer, pal, 60 + x / zoom, 80 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2050);
    exit_dos = main_sprite_create(renderer, pal, 90 + x / zoom, 100 + y / zoom, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2051);
}

static int click_action() {
    int ret = 0;
    if (count_down_spr == exit_dos) { main_exit_system(); }
    if (count_down_spr == create_characters) { window_push(&view_character_window, 0, 10); }
    if (count_down_spr == start) {
        if(player_get_active()->name) {
            window_pop();
            ret = 1;
            window_load_region(42);
        } else {
            window_push(&popup_window, 100, 75);
            popup_set_message("CREATE CHARACTER");
            popup_set_option(0, "Ok");
            popup_set_option(1, "");
            popup_set_option(2, "CANCEL");
        }
    }

    if (count_down_spr == load_save) {
        add_load_save_set_mode(ACTION_LOAD);
        window_push(&als_window, 0, 0);
    }

    return ret;
}

void main_render(void *data) {
    SDL_Renderer *renderer = main_get_rend();
    sprite_render(renderer, sun);
    sprite_render(renderer, background);

    if (count_down > 0) {
        sprite_set_frame(count_down_spr, 1 + (count_down % 2));
    }

    if (count_down > 0) {
        count_down--;
        if (count_down == 0) {
            sprite_set_frame(count_down_spr, 0);
            if (click_action()) { return; }
            count_down_spr = SPRITE_ERROR;
        }
    }

    sprite_render(renderer, start);
    sprite_render(renderer, create_characters);
    sprite_render(renderer, load_save);
    sprite_render(renderer, exit_dos);
}

static uint16_t get_sprite(const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(start, x, y)) { return start; }
    if (sprite_in_rect(create_characters, x, y)) { return create_characters; }
    if (sprite_in_rect(load_save, x, y)) { return load_save; }
    if (sprite_in_rect(exit_dos, x, y)) { return exit_dos; }

    return SPRITE_ERROR;
}

int main_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    mousex = x; mousey = y;
    static uint16_t last_sprite = SPRITE_ERROR;
    uint16_t cur_sprite = get_sprite(x, y);

    if (last_sprite != cur_sprite) {
        sprite_set_frame(cur_sprite, sprite_get_frame(cur_sprite) + 1);
        if (last_sprite != SPRITE_ERROR) {
            sprite_set_frame(last_sprite, sprite_get_frame(last_sprite) - 1);
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
    sprite_free(sun);
    sprite_free(background);
    sprite_free(start);
    sprite_free(create_characters);
    sprite_free(load_save);
    sprite_free(exit_dos);
}

wops_t main_window = {
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
