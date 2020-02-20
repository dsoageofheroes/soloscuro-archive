#include "main.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"

static SDL_Texture *background = NULL;
static SDL_Rect background_loc = { 10, 10, 0, 0 };

void main_init(SDL_Renderer *renderer) {
    uint32_t palette_id = gff_get_palette_id(RESOURCE_GFF_INDEX, 0);
    printf("palette_id = %d\n", palette_id);
    background = create_texture(renderer, RESOURCE_GFF_INDEX, GT_BMP, 20029, 0, palette_id, &background_loc);
    background_loc.w *= 2;
    background_loc.h *= 2;
}

void main_render(void *data, SDL_Renderer *renderer) {
    // Uncomment to see...
    //SDL_RenderCopy(renderer, background, NULL, &background_loc);
}

int main_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    //return 1; // means I captured the mouse movement
    return 0; // zero means I did not handle the mouse, so another screen may.
}

int main_handle_mouse_click(const uint32_t x, const uint32_t y) {
    //return 1; // means I captured the mouse click
    return 0; // zero means I did not handle the mouse click, so another screen may.
}

void main_free() {
}

sops_t main_screen = {
    .render = main_render,
    .mouse_movement = main_handle_mouse_movement,
    .mouse_click = main_handle_mouse_click,
    .data = NULL
};
