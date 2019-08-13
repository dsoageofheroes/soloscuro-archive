#include <stdlib.h>
#include <string.h>
#include "gff-map.h"
#include "gff.h"
#include "gfftypes.h"

static gff_map_t* get_map(int gff_file) {
    if (gff_file < 0 || gff_file >= NUM_FILES) { return NULL; }

    if (!open_files[gff_file].map) {
        open_files[gff_file].map = malloc(sizeof(gff_map_t));
        if (open_files[gff_file].map) {
            memset(open_files[gff_file].map, 0, sizeof(gff_map_t));
        }
    }

    return open_files[gff_file].map;
}

int gff_load_map_tile_ids(int gff_file, int res_id) {
    unsigned long len;
    gff_map_t *map = get_map(gff_file);
    char* data = gff_get_raw_bytes(gff_file, GT_RMAP, res_id, &len);

    printf("------------------len = %lu\n", len);
    if (!data || len < 1 || !map) { return 0; }

    printf("------------------setting tile_ids_size = %lu\n", len);
    map->tile_ids_size = len;
    memcpy(map->tile_ids, data, len);

    return 1;
}

int gff_load_map_flags(int gff_file, int res_id) {
    unsigned long len;
    gff_map_t *map = get_map(gff_file);
    char* data = gff_get_raw_bytes(gff_file, GT_GMAP, res_id, &len);

    if (!data || len < 1 || !map) { return 0; }

    map->flags_size = len;
    memcpy(map->flags, data, map->flags_size);

    return 1;
}

int gff_load_map(int gff_file) {
    unsigned int *gmap_ids = gff_get_id_list(gff_file, GT_GMAP);
    unsigned int *rmap_ids = gff_get_id_list(gff_file, GT_RMAP);
    if (gmap_ids && rmap_ids) {
        gff_load_map_flags(gff_file, gmap_ids[0]);
        gff_load_map_tile_ids(gff_file, rmap_ids[0]);
    }

    if (gmap_ids) { free(gmap_ids); }
    if (rmap_ids) { free(rmap_ids); }

    return 0;
}

int32_t get_tile_id(int gff_file, int row, int column) {
    gff_map_t *map = get_map(gff_file);
    int pos = row*MAP_COLUMNS + column;

    if (!map || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= map->tile_ids_size) {
        return -1;
    }

    return (int32_t)(map->tile_ids[row][column]);
}

int gff_map_is_block(int gff_file, int row, int column) {
    gff_map_t *map = get_map(gff_file);
    int pos = row*MAP_COLUMNS + column;

    if (!map || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= map->tile_ids_size) {
        return -1;
    }

    return (int32_t)(map->flags[row][column] & MAP_BLOCK);
}

int gff_map_is_actor(int gff_file, int row, int column) {
    gff_map_t *map = get_map(gff_file);
    int pos = row*MAP_COLUMNS + column;

    if (!map || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= map->tile_ids_size) {
        return -1;
    }

    return (int32_t)(map->flags[row][column] & MAP_ACTOR);
}

int gff_map_is_danger(int gff_file, int row, int column) {
    gff_map_t *map = get_map(gff_file);
    int pos = row*MAP_COLUMNS + column;

    if (!map || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= map->tile_ids_size) {
        return -1;
    }

    return (int32_t)(map->flags[row][column] & MAP_DANGER);
}
