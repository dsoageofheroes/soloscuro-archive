#include "narrate.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"

#define STARTX (60)

static SDL_Texture *background = NULL;
static SDL_Rect background_loc = { STARTX + 0, 0, 0, 0 };
static SDL_Texture *border = NULL;
static SDL_Rect border_loc = { STARTX + 10, 10, 0, 0 };

#define MAX_PORTRAITS (256)
static SDL_Texture *portraits[MAX_PORTRAITS];
static SDL_Rect portraits_loc[MAX_PORTRAITS];
void load_portraits(SDL_Renderer *renderer) {
    unsigned long len;
    unsigned char *data;
    unsigned int w, h, id;
    SDL_Surface *surface = NULL;
    unsigned int *ids = gff_get_id_list(DSLDATA_GFF_INDEX, GT_PORT);
    unsigned int num_ids = gff_get_gff_type_length(DSLDATA_GFF_INDEX, GT_PORT);
    memset(portraits_loc, 0x0, sizeof(SDL_Rect) * MAX_PORTRAITS);
    memset(portraits, 0x0, sizeof(SDL_Texture*) * MAX_PORTRAITS);

    for (int i = 0; i < num_ids; i++) {
        id = ids[i];
        data = (unsigned char*)gff_get_raw_bytes(DSLDATA_GFF_INDEX, GT_PORT, id, &len);
        data = get_portrait(data, &w, &h);
        surface = SDL_CreateRGBSurfaceFrom(data, w, h, 32,
            4*w, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        portraits[id] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        portraits_loc[id].w = 2 * w;
        portraits_loc[id].h = 2 * h;
        portraits_loc[id].x = STARTX + 26;
        portraits_loc[id].y = 24;
    }

    if (ids) { free(ids); }
}

void narrate_init(SDL_Renderer *renderer) {
    uint32_t palette_id = gff_get_palette_id(RESOURCE_GFF_INDEX, 0);
    background = create_texture(renderer, RESOURCE_GFF_INDEX, GT_BMP, 3007, 0, palette_id, &background_loc);
    border = create_texture(renderer, RESOURCE_GFF_INDEX, GT_BMP, 12000, 0, palette_id, &border_loc);
    background_loc.w *= 2;
    background_loc.h *= 2;
    border_loc.w *= 2;
    border_loc.h *= 2;
    load_portraits(renderer);
    SDL_SetTextureAlphaMod( background, 192 );
}

void narrate_render(void *data, SDL_Renderer *renderer) {
    // Uncomment to see...
    SDL_RenderCopy(renderer, background, NULL, &background_loc);
    SDL_RenderCopy(renderer, border, NULL, &border_loc);
    SDL_RenderCopy(renderer, portraits[119], NULL, &portraits_loc[110]);
}

int narrate_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    return 0; // zero means I did not handle the mouse, so another screen may.
}

int narrate_handle_mouse_click(const uint32_t x, const uint32_t y) {
    return 0; // zero means I did not handle the mouse click, so another screen may.
}

void narrate_free() {
}

sops_t narrate_screen = {
    .init = narrate_init,
    .render = narrate_render,
    .mouse_movement = narrate_handle_mouse_movement,
    .mouse_click = narrate_handle_mouse_click,
    .data = NULL
};
