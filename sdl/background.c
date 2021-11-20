#include "background.h"
#include "main.h"
#include "gff.h"
#include "../src/port.h"
#include "../src/settings.h"
#include <SDL2/SDL.h>

static SDL_Texture **tiles = NULL;
static sol_region_t *region = NULL;;

extern void sol_background_load_region(sol_region_t *_region) {
    SDL_Surface* tile = NULL;
    uint32_t *ids = NULL;
    uint32_t width, height;
    size_t max_id = 0;
    unsigned char *data;
    //gff_palette_t *pal = NULL;
    int offset = 0;
    region = _region;

    if (region->map_id < 100 && region->map_id > 0) {
        //pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + region->map_id - 1;
        offset = 1;
    }
    ids = region->tile_ids;
    max_id = 256;
    tiles = (SDL_Texture**) malloc(sizeof(SDL_Texture*) * max_id);
    memset(tiles, 0x0, sizeof(SDL_Texture*) * max_id);

    for (uint32_t i = 0; i < region->num_tiles; i++) {
        //printf("i = %d\n", i);
        region_get_tile(region, ids[i], &width, &height, &data);

        if (data && *data) {
            tile = SDL_CreateRGBSurfaceFrom(data, width, height, 32, 4*width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

            tiles[i + offset] = SDL_CreateTextureFromSurface(main_get_rend(), tile);
            //map->tiles[i] = SDL_CreateTextureFromSurface(cren, tile);
            SDL_FreeSurface(tile);

            free(data);
        }
    }
}

extern void sol_background_free() {
    if (!tiles) { return; }
    for (uint32_t i = 0; i < region->num_tiles; i++) {
        if (tiles[i]) {
            SDL_DestroyTexture(tiles[i]);
            tiles[i] = NULL;
        }
    }
    free(tiles);
    tiles = NULL;
}

extern void sol_background_apply_alpha(const uint8_t alpha) {
    if (!tiles) { return; }
    for (uint32_t i = 0; i < region->num_tiles; i++) {
        SDL_SetTextureAlphaMod(tiles[i + 1], alpha);
    }
}

extern void sol_background_render() {
    if (!tiles) { return; }
    const int stretch = settings_zoom();
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    SDL_Rect tile_loc = { -xoffset, -yoffset, stretch * 16, stretch * 16 };
    uint32_t tile_id = 0;
    SDL_Renderer *renderer = main_get_rend();

    SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear(renderer);

    for (int x = 0; x < 98; x++) {
        for (int y = 0; y < 128; y++) {
            tile_id = region->tiles[x][y];
            if (tile_id >= 0) {
               SDL_RenderCopy(renderer, tiles[tile_id], NULL, &tile_loc);
               tile_loc.x += stretch * 16;
            }
        }
        tile_loc.x = -xoffset;
        tile_loc.y += stretch * 16;
    }
}