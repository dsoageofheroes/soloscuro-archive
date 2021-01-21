#include "screen-main.h"
#include "../main.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"

static SDL_Texture *background = NULL;
static SDL_Texture *sun = NULL;
static SDL_Texture* start[4] = {NULL, NULL, NULL, NULL};
static SDL_Texture* create_characters[4] = {NULL, NULL, NULL, NULL};
static SDL_Texture* load_save[4] = {NULL, NULL, NULL, NULL};
static SDL_Texture* exit_dos[4] = {NULL, NULL, NULL, NULL};

static SDL_Rect initial_locs[] = {{ 45, 0, 0, 0 }, // sun
                                  { 0, 20, 0, 0 }, // background
                                  { 90, 45, 0, 0 }, // start
                                  { 45, 62, 0, 0 }, // create
                                  { 60, 80, 0, 0 }, // load save
                                  { 90, 100, 0, 0 }, // exit
};

static SDL_Rect sun_loc, background_loc, start_loc, create_loc, load_save_loc, exit_loc;

static int mousex = 0, mousey = 0;
static int mouse_down = 0;
static int count_down = 0;

static void set_zoom(SDL_Rect *loc, float zoom) {
    loc->x *= zoom;
    loc->y *= zoom;
    loc->w *= zoom;
    loc->h *= zoom;
}

static SDL_Rect apply_params(const SDL_Rect rect, const uint32_t x, const uint32_t y) {
    SDL_Rect ret = {rect.x + x, rect.y + y, rect.w, rect.h};
    return ret;
}

void main_init(SDL_Renderer *renderer, const uint32_t x, const uint32_t y, const float zoom) {
    uint32_t palette_id = gff_get_palette_id(RESOURCE_GFF_INDEX, 0);

    printf("-------------x = %d------------\n", x);

    sun_loc = apply_params(initial_locs[0], x, y);
    background_loc = apply_params(initial_locs[1], x, y);
    start_loc = apply_params(initial_locs[2], x, y);
    create_loc = apply_params(initial_locs[3], x, y);
    load_save_loc = apply_params(initial_locs[4], x, y);
    exit_loc = apply_params(initial_locs[5], x, y);

    sun = create_texture(renderer, RESOURCE_GFF_INDEX, GFF_BMP, 20028, 0, palette_id, &sun_loc);
    background = create_texture(renderer, RESOURCE_GFF_INDEX, GFF_BMP, 20029, 0, palette_id, &background_loc);

    for (int i = 0; i < 4; i++) {
        start[i] = create_texture(renderer, RESOURCE_GFF_INDEX, GFF_ICON, 2048, i, palette_id, &start_loc);
        create_characters[i] = create_texture(renderer, RESOURCE_GFF_INDEX, GFF_ICON, 2049, i, palette_id, &create_loc);
        load_save[i] = create_texture(renderer, RESOURCE_GFF_INDEX, GFF_ICON, 2050, i, palette_id, &load_save_loc);
        exit_dos[i] = create_texture(renderer, RESOURCE_GFF_INDEX, GFF_ICON, 2051, i, palette_id, &exit_loc);
    }

    set_zoom(&sun_loc, zoom);
    set_zoom(&background_loc, zoom);
    set_zoom(&start_loc, zoom);
    set_zoom(&create_loc, zoom);
    set_zoom(&load_save_loc, zoom);
    set_zoom(&exit_loc, zoom);
}

static int is_in_rect(SDL_Rect *rect) {
    return (mousex >= rect->x && mousex <= (rect->x + rect->w)
        && mousey >= rect->y && mousey <= (rect->y + rect->h));
}

static void clicked(SDL_Renderer *renderer) {
    if (is_in_rect(&exit_loc)) {
        main_exit_system();
    }
}

static void render(SDL_Renderer *renderer, SDL_Texture *tex[4], SDL_Rect *rect) {
    //if (mousex >= rect->x && mousex <= (rect->x + rect->w)
        //&& mousey >= rect->y && mousey <= (rect->y + rect->h)) {
    if (is_in_rect(rect)) {
        if (mouse_down) {
            SDL_RenderCopy(renderer, tex[3], NULL, rect);
        } else {
            if (count_down > 0 && ((count_down / 4) % 2) ) {
                SDL_RenderCopy(renderer, tex[3], NULL, rect);
            } else {
                SDL_RenderCopy(renderer, tex[1], NULL, rect);
            }
        }
    } else {
        SDL_RenderCopy(renderer, tex[0], NULL, rect);
    }
    
}

void main_render(void *data, SDL_Renderer *renderer) {
    SDL_RenderCopy(renderer, sun, NULL, &sun_loc);
    SDL_RenderCopy(renderer, background, NULL, &background_loc);
    render(renderer, start, &start_loc);
    render(renderer, create_characters, &create_loc);
    render(renderer, load_save, &load_save_loc);
    render(renderer, exit_dos, &exit_loc);

    if (count_down == 1) {
        clicked(renderer);
    }

    if (count_down > 0) { count_down--; }
}

int main_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    mousex = x; mousey = y;
    return 1; // means I captured the mouse movement
    //return 0; // zero means I did not handle the mouse, so another screen may.
}

int main_handle_mouse_down(const uint32_t x, const uint32_t y) {
    mouse_down = 1;
    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another screen may.
}

int main_handle_mouse_up(const uint32_t x, const uint32_t y) {
    mouse_down = 0;
    count_down = 32;
    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another screen may.
}

void main_free() {
    SDL_DestroyTexture(sun);
    SDL_DestroyTexture(background);

    for (int i = 0; i < 4; i++) {
        SDL_DestroyTexture(start[i]);
        SDL_DestroyTexture(create_characters[i]);
        SDL_DestroyTexture(load_save[i]);
        SDL_DestroyTexture(exit_dos[i]);
    }
}

sops_t main_screen = {
    .init = main_init,
    .cleanup = main_free,
    .render = main_render,
    .mouse_movement = main_handle_mouse_movement,
    .mouse_down = main_handle_mouse_down,
    .mouse_up = main_handle_mouse_up,
    .data = NULL
};
