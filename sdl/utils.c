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

extern void sol_draw_cone(int sx, int sy, int range) {
    //Lets draw a triangle...
    SDL_Renderer *renderer = main_get_rend();
    SDL_SetRenderDrawColor(renderer, 192, 192, 192, 128);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    /*
    for (int i = 0; i < range; i++) {
        //SDL_RenderDrawPoint(renderer, 100, i + 100);
        SDL_RenderDrawLine(renderer, 100 + i, 100 - i/2, 100 + i, 100 + i/2);
    }
    */
    for (int w = 0; w < range; w++) {
        for (int h = 0; h < range; h++) {
            int dx = range - w;
            int dy = range - h;
            // The 1.0 is 1/8th a circle, can be raise to a 2.0 for 1/4th circle or
            // less than 1.0 for smaller that 1/8.
            if ((dx*dx + dy*dy) <= (range * range) && dy < 1.0 * dx) {
                SDL_RenderDrawPoint(renderer, sx + dx, sy + dy);
            }
        }
    }
}

