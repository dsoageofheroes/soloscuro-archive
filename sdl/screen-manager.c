#include "screen-manager.h"
#include "map.h"
#include "../src/dsl.h"

#define MAX_SCREENS (10)

static void (*render_functions[MAX_SCREENS]) (void*, SDL_Renderer*, const uint32_t, const uint32_t);
static void *function_data[MAX_SCREENS];

static map_t cmap;

void screen_init(SDL_Renderer *renderer) {
    for (int i = 0; i < MAX_SCREENS; i++) {
        render_functions[i] = NULL;
        function_data[i] = NULL;
    }
    map_init(&cmap);
    map_load_region(&cmap, renderer, gff_find_index("rgn2a.gff"));
    render_functions[0] = &map_render;
    function_data[0] = &cmap;
}

void screen_render(SDL_Renderer *renderer, const uint32_t xmappos, const uint32_t ymappos) {
    for (int i = 0; i < MAX_SCREENS; i++) {
        if (render_functions[i]) {
            render_functions[i](function_data[i], renderer, xmappos, ymappos);
        }
    }
}

void screen_free() {
    map_free(&cmap);
}
