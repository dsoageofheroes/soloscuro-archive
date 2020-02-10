#include <stdlib.h>
#include "map.h"
#include "../src/dsl.h"

void map_init(map_t *map) {
    map->tiles = NULL;
    map->num_tiles = 0;
    map->gff_file = 0;
}

void map_free(map_t *map) {
    if (map->tiles) {
        for (int i = 0; i < map->num_tiles; i++) {
            SDL_DestroyTexture(map->tiles[i]);
        }
        free(map->tiles);
        map->tiles = NULL;
    }
}

void map_load_region(map_t *map, SDL_Renderer *renderer, int id) {
    SDL_Surface* tile = NULL;
    map->num_tiles = gff_get_gff_type_length(id, GT_TILE);
    uint32_t *ids = gff_get_id_list(id, GT_TILE);
    uint32_t palette_id = -1;
    uint32_t width, height;
    unsigned char *data;
    uint32_t *tids = gff_get_id_list(id, GT_ETAB); // temporary to find current id for palette!

    if (!tids) { free(ids); error("Unable to find current id for map\n"); return; }
    palette_id = gff_get_palette_id(DSLDATA_GFF_INDEX, *tids - 1);
    free(tids);
    tids = NULL;

    map_free(map);
    gff_load_map(id);
    map->tiles = (SDL_Texture**) malloc(sizeof(SDL_Texture*) * map->num_tiles);
    map->gff_file = id;

    for (int i = 0; i < map->num_tiles; i++) {
        width = get_frame_width(map->gff_file, GT_TILE, ids[i], 0);
        height = get_frame_height(map->gff_file, GT_TILE, ids[i], 0);
        data = get_frame_rgba_with_palette(map->gff_file, GT_TILE, ids[i], 0, palette_id);
        tile = SDL_CreateRGBSurfaceFrom(data, width, height, 32, 4*width, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
        map->tiles[ids[i]] = SDL_CreateTextureFromSurface(renderer, tile);
        SDL_FreeSurface(tile);
        free(data);
    }

    free(ids);
}

void map_render(map_t *map, SDL_Renderer *renderer, const uint32_t xoffset, const uint32_t yoffset) {
    const int stretch = 2;
    SDL_Rect tile_location = { -xoffset, -yoffset, stretch * 16, stretch * 16 };
    uint32_t tile_id = 0;

    SDL_RenderClear(renderer);

    for (int x = 0; x < 98; x++) {
        for (int y = 0; y < 128; y++) {
            tile_id = get_tile_id(map->gff_file, x, y);
            //printf("tile_id = %d\n", tile_id);
            if (tile_id >= 0) {
               SDL_RenderCopy(renderer, map->tiles[tile_id], NULL, &tile_location);
               tile_location.x += stretch * 16;
            }
        }
        tile_location.x = -xoffset;
        tile_location.y += stretch * 16;
    }
    SDL_RenderPresent(renderer);
}
