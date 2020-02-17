#include "screen-manager.h"
#include "map.h"
#include "../src/dsl.h"
#include "screens/main.h"

#define MAX_SCREENS (10)

static void (*render_functions[MAX_SCREENS]) (void*, SDL_Renderer*);
static void *function_data[MAX_SCREENS];
static int (*handle_mouse[MAX_SCREENS]) (const uint32_t x, const uint32_t y);
static int (*handle_mouse_click[MAX_SCREENS]) (const uint32_t x, const uint32_t y);

static map_t cmap;

void screen_init(SDL_Renderer *renderer) {
    for (int i = 0; i < MAX_SCREENS; i++) {
        render_functions[i] = NULL;
        function_data[i] = NULL;
        handle_mouse[i] = NULL;
        handle_mouse_click[i] = NULL;
    }
    map_init(&cmap);
    map_load_region(&cmap, renderer, gff_find_index("rgn2a.gff"));
    render_functions[0] = &map_render;
    function_data[0] = &cmap;
    handle_mouse[0] = &map_handle_mouse;
    handle_mouse_click[0] = &map_handle_mouse_click;

    main_init(renderer);
    render_functions[1] = &main_render;
}

void screen_render(SDL_Renderer *renderer, const uint32_t xmappos, const uint32_t ymappos) {
    for (int i = 0; i < MAX_SCREENS; i++) {
        if (render_functions[i]) {
            render_functions[i](function_data[i], renderer);
        }
    }
    SDL_RenderPresent(renderer);
}

void screen_handle_mouse(const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (handle_mouse[i] && handle_mouse[i](x, y)) {
            i = 0; // exit loop, mouse has been handled!
        }
    }
}

void screen_handle_mouse_click(const uint32_t x, const uint32_t y) {
    for (int i = MAX_SCREENS-1; i >= 0; i--) {
        if (handle_mouse_click[i] && handle_mouse_click[i](x, y)) {
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
    return ret;
}

void screen_free() {
    map_free(&cmap);
}
