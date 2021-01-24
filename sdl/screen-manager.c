#include "screen-manager.h"
#include "map.h"
#include "animate.h"
#include "player.h"
#include "sprite.h"
#include "font.h"
#include "../src/dsl.h"

#define MAX_SCREENS (10)

static sops_t screens[MAX_SCREENS];
static uint32_t screen_pos = 0;

static map_t cmap;

void screen_init(SDL_Renderer *renderer) {
    font_init(renderer);

    sprite_init();

    for (int i = 0; i < MAX_SCREENS; i++) {
        screens[i].render = NULL;
        screens[i].mouse_movement = NULL;
        screens[i].mouse_down = NULL;
        screens[i].mouse_up = NULL;
        screens[i].data = NULL;
    }

    animate_init();
}

void screen_load_screen(SDL_Renderer *renderer, int layer, sops_t *screen, const uint32_t x, const uint32_t y) {
    if (layer < 0 || layer > MAX_SCREENS) { return; }

    if (screens[layer].render) {
        // cleanup?
    }

    screens[layer].render = NULL;
    screens[layer].mouse_movement = NULL;
    screens[layer].mouse_up = NULL;
    screens[layer].mouse_down = NULL;

    screens[layer] = *screen;

    if (screens[layer].init) {
        screens[layer].init(renderer, x, y, 2.0);
    }
}

void screen_push_screen(SDL_Renderer *renderer, sops_t *screen, const uint32_t x, const uint32_t y) {
    screen_load_screen(renderer, screen_pos++, screen, x, y);
}

void screen_load_region(SDL_Renderer *renderer) {
    // Map is a special case.
    map_init(&cmap);
    map_load_region(&cmap, renderer, gff_find_index("rgn2a.gff"));

    screens[0] = map_screen;
    screens[0].data = NULL;
    //screens[0].init(renderer);
}

void screen_render(SDL_Renderer *renderer, const uint32_t xmappos, const uint32_t ymappos) {
    SDL_RenderClear(renderer);
    for (int i = 0; i < MAX_SCREENS; i++) {
        if (screens[i].render) {
            screens[i].render(screens[i].data, renderer);
        }
    }
    player_render(renderer);
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

void screen_pop() {
    screen_pos--;
    if (screens[screen_pos].cleanup) {
        screens[screen_pos].cleanup();
        screens[screen_pos].render = NULL;
        screens[screen_pos].mouse_movement = NULL;
        screens[screen_pos].mouse_down = NULL;
        screens[screen_pos].mouse_up = NULL;
        screens[screen_pos].data = NULL;
    }

    if (screen_pos > 0 && screens[screen_pos - 1].return_control) {
        screens[screen_pos - 1].return_control();
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
