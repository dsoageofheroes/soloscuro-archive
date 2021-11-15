#include "utils.h"
#include "main.h"
#include "../src/gff-image.h"

SDL_Texture* create_texture(const uint32_t gff_file,
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
    ret = SDL_CreateTextureFromSurface(main_get_rend(), surface);
    SDL_FreeSurface(surface);
    free(data);
    return ret;
}
