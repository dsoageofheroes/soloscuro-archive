#include "screen-manager.h"
#include "map.h"
#include "main.h"
#include "animate.h"
#include "player.h"
#include "screens/narrate.h"
#include "sprite.h"
#include "font.h"
#include "../src/dsl.h"
#include "../src/ds-player.h"
#include "../src/region.h"

#define MAX_SCREENS (10)

static sops_t screens[MAX_SCREENS];
static uint32_t screen_pos = 0;

void screen_init(SDL_Renderer *renderer) {
    font_init(renderer);

    sprite_init();

    memset(screens, 0x0, sizeof(sops_t) * MAX_SCREENS);

    animate_init();
}

static uint32_t default_get_width() { return 320 * main_get_zoom(); }
static uint32_t default_get_height() { return 200 * main_get_zoom(); }

void screen_load_screen(SDL_Renderer *renderer, int layer, sops_t *screen, const uint32_t x, const uint32_t y) {
    int grey_out_map = 0;

    if (layer < 0 || layer > MAX_SCREENS) { return; }

    if (screens[layer].render) {
        // cleanup?
    }

    screens[layer] = *screen;

    if (!screens[layer].get_width) { screens[layer].get_width = default_get_width; }
    if (!screens[layer].get_height) { screens[layer].get_height = default_get_height; }

    if (screens[layer].init) {
        screens[layer].init(renderer,
            ((main_get_width() - screens[layer].get_width()) / 2),
            ((main_get_height() - screens[layer].get_height()) / 2));
    }

    for (uint32_t i = 0; i < screen_pos && !grey_out_map; i++) {
        grey_out_map = screens[i].grey_out_map;
    }

    if (grey_out_map) {
        map_apply_alpha(127);
    }
}

void screen_toggle_screen(SDL_Renderer *renderer, sops_t *the_screen, const uint32_t x, const uint32_t y) {
    uint32_t pos;
    sops_t tmp;
    for (pos = 0; pos < screen_pos; pos++) {
        if (screens[pos].render == the_screen->render) { break; }
    }

    if (pos >= screen_pos) { // not found so turn on.
        screen_load_screen(renderer, screen_pos++, the_screen, x, y);
        return;
    }

    tmp = screens[pos]; // Found, so we bring to the front, then pop.

    while (pos < (screen_pos - 1)) {
        screens[pos] = screens[pos + 1];
        pos++;
    }

    screens[screen_pos - 1] = tmp;

    screen_pop();
}

void screen_push_screen(SDL_Renderer *renderer, sops_t *screen, const uint32_t x, const uint32_t y) {
    for (uint32_t i = 0; i < screen_pos; i++) {
        if (screens[i].render == screen->render) { return; }
    }
    screen_load_screen(renderer, screen_pos++, screen, x, y);
}

void port_change_region(region_t *reg) {
    for (int i = 0; i < MAX_PCS; i++) {
        player_load(i, main_get_zoom());
    }
    map_load_region(reg, main_get_rend());
}

int screen_load_region(SDL_Renderer *renderer, const int region) {
    map_load_map(renderer, region);

    screen_push_screen(renderer, &map_screen, 0, 0);
    screen_push_screen(renderer, &narrate_screen, 0, 0);

    return 1;
}

void screen_render(SDL_Renderer *renderer, const uint32_t xmappos, const uint32_t ymappos) {
    SDL_RenderClear(renderer);
    for (int i = 0; i < MAX_SCREENS; i++) {
        if (screens[i].render) {
            screens[i].render(screens[i].data, renderer);
        }
    }
    //font_render_ttf("TEST!!!!!!!!!!!", 100, 100, 0xFFFF00FF);
    //player_render(renderer);
    SDL_RenderPresent(renderer);
}

void screen_handle_mouse(const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (screens[i].mouse_movement && screens[i].mouse_movement(x, y)) {
            i = 0; // exit loop, mouse has been handled!
        }
    }
}

void screen_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (screens[i].mouse_down && screens[i].mouse_down(button, x, y)) {
            i = 0; // exit loop, mouse has been handled!
        }
    }
}

void screen_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (screens[i].mouse_up && screens[i].mouse_up(button, x, y)) {
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

static void destroy_screen(const int pos) {
    if (screens[pos].cleanup) {
        screens[pos].cleanup();
    }

    memset(screens + pos, 0x0, sizeof(sops_t));
}

void screen_clear() {
    while(screen_pos) {
        screen_pop();
    }
}

void screen_pop() {
    int grey_out_map = 0;

    destroy_screen(--screen_pos);

    if (screen_pos > 0 && screens[screen_pos - 1].return_control) {
        screens[screen_pos - 1].return_control();
    }

    for (uint32_t i = 0; i < screen_pos && !grey_out_map; i++) {
        grey_out_map = screens[i].grey_out_map;
    }

    if (!grey_out_map) {
        map_apply_alpha(255);
    }
}

void screen_free() {
    for (int i = 0; i < MAX_SCREENS; i++) {
        if (screens[i].cleanup) {
            screens[i].cleanup();
        }
    }
    animate_close();
}
