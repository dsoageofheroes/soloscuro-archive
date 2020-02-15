#include "main.h"
#include "../screen-manager.h"
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

void main_free() {
}
