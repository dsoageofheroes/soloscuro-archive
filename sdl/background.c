#include "background.h"
#include "main.h"
#include "gff.h"
#include "port.h"
#include "settings.h"
#include "gff-map.h"
#include "gfftypes.h"
#include <SDL2/SDL.h>
#include <string.h>

static SDL_Texture **tiles = NULL;
static sol_region_t *region = NULL;;

static sol_status_t load_sol_background() {
    unsigned char *data;
    uint32_t w, h;
    char buf[32];
    SDL_Surface* tile = NULL;

    sol_background_free();
    snprintf(buf, 32, "rgn%02x.gff", region->sol.mid);
    int gff_file = gff_find_index(buf);
    int pid = gff_get_palette_id(DSLDATA_GFF_INDEX, region->sol.mid - 1);
    //printf("gff_file = %d, pid = %d\n", gff_file, pid);
    data = gff_get_frame_rgba_with_palette(gff_file, GFF_TILE, region->sol.tid, 0, pid);
    if (!data) { return SOL_NOT_FOUND; }
    w = gff_get_frame_width(gff_file, GFF_TILE, region->sol.tid, 0);
    h = gff_get_frame_height(gff_file, GFF_TILE, region->sol.tid, 0);
    tile = SDL_CreateRGBSurfaceFrom(data, w, h, 32, 4*w, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

    tiles = (SDL_Texture**) malloc(sizeof(SDL_Texture*) * 1);
    tiles[0] = SDL_CreateTextureFromSurface(main_get_rend(), tile);
    SDL_FreeSurface(tile);

    free(data);
    return SOL_SUCCESS;
}

extern sol_status_t sol_background_load_region(sol_region_t *_region) {
    SDL_Surface* tile = NULL;
    uint32_t *ids = NULL;
    uint32_t width, height;
    size_t max_id = 0;
    unsigned char *data;
    //gff_palette_t *pal = NULL;
    int offset = 0;
    region = _region;

    if (region->sol.mid) {
        return load_sol_background();
    }

    if (region->map_id < 100 && region->map_id > 0) {
        //pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + region->map_id - 1;
        offset = 1;
    }
    sol_region_get_tile_ids(region, &ids);
    max_id = 256;
    tiles = (SDL_Texture**) malloc(sizeof(SDL_Texture*) * max_id);
    memset(tiles, 0x0, sizeof(SDL_Texture*) * max_id);

    for (uint32_t i = 0; i < region->num_tiles; i++) {
        //printf("i = %d, region = %p, ids = %p\n", i, region, ids);
        sol_region_get_tile(region, ids[i], &width, &height, &data);

        if (data && *data) {
            tile = SDL_CreateRGBSurfaceFrom(data, width, height, 32, 4*width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

            tiles[i + offset] = SDL_CreateTextureFromSurface(main_get_rend(), tile);
            //map->tiles[i] = SDL_CreateTextureFromSurface(cren, tile);
            SDL_FreeSurface(tile);

            free(data);
        }
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_background_free() {
    if (!tiles) { return SOL_ALREADY_FREED; }
    for (uint32_t i = 0; i < region->num_tiles; i++) {
        if (tiles[i]) {
            SDL_DestroyTexture(tiles[i]);
            tiles[i] = NULL;
        }
    }
    free(tiles);
    tiles = NULL;

    return SOL_SUCCESS;
}

extern sol_status_t sol_background_apply_alpha(const uint8_t alpha) {
    if (!tiles) { return SOL_NOT_LOADED; }
    for (uint32_t i = 0; i < region->num_tiles; i++) {
        SDL_SetTextureAlphaMod(tiles[i + 1], alpha);
    }
    return SOL_SUCCESS;
}

extern sol_status_t sol_background_render_box(const int32_t startx, const int32_t starty) {
    if (!tiles) { return SOL_NOT_LOADED; }
    const int stretch = settings_zoom();
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    SDL_Rect tile_loc = { -xoffset, -yoffset, stretch * 16, stretch * 16 };
    uint32_t tile_id = 0;
    SDL_Renderer *renderer = main_get_rend();

    for (int x = 0; x < 98; x++) {
        for (int y = 0; y < 128; y++) {
            tile_id = region->tiles[x][y];
            if (tile_id >= 0 && tile_loc.x >= startx && tile_loc.y >= starty) {
                SDL_RenderCopy(renderer, tiles[tile_id], NULL, &tile_loc);
            }
            tile_loc.x += stretch * 16;
        }
        tile_loc.x = -xoffset;
        tile_loc.y += stretch * 16;
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_background_render() {
    return sol_background_render_box(-16 * settings_zoom(), -16 * settings_zoom());
}
