#include "window-manager.h"
#include "map.h"
#include "main.h"
#include "animate.h"
#include "player.h"
#include "windows/narrate.h"
#include "windows/combat-status.h"
#include "sprite.h"
#include "font.h"
#include "../src/dsl.h"
#include "../src/player.h"
#include "../src/region.h"
#include "../src/port.h"
#include "windows/inventory.h"

#define MAX_SCREENS (10)

static wops_t windows[MAX_SCREENS];
static uint32_t window_pos = 0;

void window_init(SDL_Renderer *renderer) {
    sprite_init();

    memset(windows, 0x0, sizeof(wops_t) * MAX_SCREENS);

    animate_init();
}

static uint32_t default_get_width() { return 320 * main_get_zoom(); }
static uint32_t default_get_height() { return 200 * main_get_zoom(); }

void window_load(SDL_Renderer *renderer, int layer, wops_t *window, const uint32_t x, const uint32_t y) {
    int grey_out_map = 0;

    if (layer < 0 || layer > MAX_SCREENS) { return; }

    if (windows[layer].render) {
        // cleanup?
    }

    windows[layer] = *window;

    if (!windows[layer].get_width) { windows[layer].get_width = default_get_width; }
    if (!windows[layer].get_height) { windows[layer].get_height = default_get_height; }

    if (windows[layer].init) {
        windows[layer].init(renderer,
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

void window_toggle(SDL_Renderer *renderer, wops_t *the_window, const uint32_t x, const uint32_t y) {
    uint32_t pos;
    wops_t tmp;
    for (pos = 0; pos < window_pos; pos++) {
        if (windows[pos].render == the_window->render) { break; }
    }

    if (pos >= window_pos) { // not found so turn on.
        window_load(renderer, window_pos++, the_window, x, y);
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
        case WINDOW_INVENTORY: window_toggle(main_get_rend(), &inventory_window, 0, 0); break;
    }
}

void window_push(SDL_Renderer *renderer, wops_t *window, const uint32_t x, const uint32_t y) {
    for (uint32_t i = 0; i < window_pos; i++) {
        if (windows[i].render == window->render) { return; }
    }
    window_load(renderer, window_pos++, window, x, y);
}

void port_change_region(region_t *reg) {
    map_load_region(reg, main_get_rend());

    for (int i = 0; i < MAX_PCS; i++) {
        player_load(i, main_get_zoom());
    }
}

int port_load_region(const int region) {
    return window_load_region(main_get_rend(), region);
}

int window_load_region(SDL_Renderer *renderer, const int region) {
    map_load_map(renderer, region);

    window_push(renderer, &map_window, 0, 0);
    window_push(renderer, &narrate_window, 0, 0);
    window_push(renderer, &combat_status_window, 295, 5);

    return 1;
}

void window_render(SDL_Renderer *renderer, const uint32_t xmappos, const uint32_t ymappos) {
    SDL_RenderClear(renderer);
    for (int i = 0; i < MAX_SCREENS; i++) {
        if (windows[i].render) {
            windows[i].render(windows[i].data, renderer);
        }
    }
    //font_render_ttf("TEST!!!!!!!!!!!", 100, 100, 0xFFFF00FF);
    //player_render(renderer);
    SDL_RenderPresent(renderer);
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

void window_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (windows[i].mouse_up && windows[i].mouse_up(button, x, y)) {
            i = 0; // exit loop, mouse has been handled!
        }
    }
}

SDL_Texture* create_texture(SDL_Renderer *renderer, const uint32_t gff_file,
        const uint32_t type, const uint32_t id, const uint32_t frame_id,
        const uint32_t palette_id, SDL_Rect *loc) {
    unsigned char *data;
    SDL_Surface *surface = NULL;
    SDL_Texture *ret = NULL;
    loc->w = get_frame_width(gff_file, type, id, frame_id);
    loc->h = get_frame_height(gff_file, type, id, frame_id);
    data = get_frame_rgba_with_palette(gff_file, type, id, frame_id, palette_id);
    surface = SDL_CreateRGBSurfaceFrom(data, loc->w, loc->h, 32, 4*loc->w,
            0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    ret = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    free(data);
    return ret;
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
    font_free(main_get_rend());
}
