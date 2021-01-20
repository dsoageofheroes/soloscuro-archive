#include "main.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"

static SDL_Texture *background = NULL;
static SDL_Texture *sun = NULL;
static SDL_Texture* start[4] = {NULL, NULL, NULL, NULL};
static SDL_Texture* create_characters[4] = {NULL, NULL, NULL, NULL};
static SDL_Texture* load_save[4] = {NULL, NULL, NULL, NULL};
static SDL_Texture* exit_dos[4] = {NULL, NULL, NULL, NULL};
static SDL_Rect sun_loc =        { 55, 0, 0, 0 };
static SDL_Rect background_loc = { 10, 20, 0, 0 };
static SDL_Rect start_loc =      { 100, 45, 0, 0 };
static SDL_Rect create_loc =     { 55, 62, 0, 0 };
static SDL_Rect load_save_loc =  { 70, 80, 0, 0 };
static SDL_Rect exit_loc =       { 100, 100, 0, 0 };

static int mousex = 0, mousey = 0;

static void set_zoom(SDL_Rect *loc, float zoom) {
    loc->x *= zoom;
    loc->y *= zoom;
    loc->w *= zoom;
    loc->h *= zoom;
}

void main_init(SDL_Renderer *renderer) {
    uint32_t palette_id = gff_get_palette_id(RESOURCE_GFF_INDEX, 0);

    sun = create_texture(renderer, RESOURCE_GFF_INDEX, GFF_BMP, 20028, 0, palette_id, &sun_loc);
    background = create_texture(renderer, RESOURCE_GFF_INDEX, GFF_BMP, 20029, 0, palette_id, &background_loc);

    for (int i = 0; i < 4; i++) {
        start[i] = create_texture(renderer, RESOURCE_GFF_INDEX, GFF_ICON, 2048, i, palette_id, &start_loc);
        create_characters[i] = create_texture(renderer, RESOURCE_GFF_INDEX, GFF_ICON, 2049, i, palette_id, &create_loc);
        load_save[i] = create_texture(renderer, RESOURCE_GFF_INDEX, GFF_ICON, 2050, i, palette_id, &load_save_loc);
        exit_dos[i] = create_texture(renderer, RESOURCE_GFF_INDEX, GFF_ICON, 2051, i, palette_id, &exit_loc);
    }

    set_zoom(&sun_loc, 2.0);
    set_zoom(&background_loc, 2.0);
    set_zoom(&start_loc, 2.0);
    set_zoom(&create_loc, 2.0);
    set_zoom(&load_save_loc, 2.0);
    set_zoom(&exit_loc, 2.0);
}

static void render(SDL_Renderer *renderer, SDL_Texture *tex[4], SDL_Rect *rect) {
    if (mousex >= rect->x && mousex <= (rect->x + rect->w)
        && mousey >= rect->y && mousey <= (rect->y + rect->h)) {
        SDL_RenderCopy(renderer, tex[1], NULL, rect);
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
}

int main_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    mousex = x; mousey = y;
    return 1; // means I captured the mouse movement
    //return 0; // zero means I did not handle the mouse, so another screen may.
}

int main_handle_mouse_click(const uint32_t x, const uint32_t y) {
    //return 1; // means I captured the mouse click
    return 0; // zero means I did not handle the mouse click, so another screen may.
}

void main_free() {
}

sops_t main_screen = {
    .init = main_init,
    .render = main_render,
    .mouse_movement = main_handle_mouse_movement,
    .mouse_click = main_handle_mouse_click,
    .data = NULL
};
