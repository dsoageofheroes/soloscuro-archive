#include "dsl.h"
#include "dsl-region.h"
#include "gff-map.h"
#include "gff.h"
#include "gfftypes.h"

#include <stdlib.h>
#include <string.h>

static void dsl_load_map_tile_ids(dsl_region_t* region);
static void dsl_load_map_flags(dsl_region_t *region);

dsl_region_t* dsl_load_region(const int gff_file) {
    uint32_t *tids = NULL;
    dsl_region_t *ret = malloc(sizeof(dsl_region_t));

    memset(ret, 0x0, sizeof(dsl_region_t));
    ret->gff_file = gff_file;

    tids = gff_get_id_list(ret->gff_file, GT_ETAB); // temporary to find current id for palette!
    if (!tids) { error("Unable to find current id for map\n"); return NULL; }
    ret->map_id = *tids;
    free(tids);

    ret->palette_id = gff_get_palette_id(DSLDATA_GFF_INDEX, ret->map_id - 1);
    ret->ids = gff_get_id_list(ret->gff_file, GT_TILE);
    ret->num_objs = gff_map_get_num_objects(ret->gff_file, ret->map_id);

    dsl_load_map_tile_ids(ret);
    dsl_load_map_flags(ret);

    return ret;
}

static void dsl_load_map_tile_ids(dsl_region_t *region) {
    unsigned int *rmap_ids = gff_get_id_list(region->gff_file, GT_RMAP);
    unsigned long len;
    char* data = gff_get_raw_bytes(region->gff_file, GT_RMAP, rmap_ids[0], &len);

    if (!data) { return ; }

    region->tile_ids_size = len;
    region->num_tiles = gff_get_gff_type_length(region->gff_file, GT_TILE);
    memcpy(region->tile_ids, data, len);

    free(rmap_ids);
}

static void dsl_load_map_flags(dsl_region_t *region) {
    unsigned long len;
    unsigned int *gmap_ids = gff_get_id_list(region->gff_file, GT_GMAP);
    char* data = gff_get_raw_bytes(region->gff_file, GT_GMAP, gmap_ids[0], &len);

    if (!data) { return ; }

    region->flags_size = len;
    memcpy(region->flags, data, region->flags_size);

    free(gmap_ids);
}

int dsl_region_is_block(dsl_region_t *region, int row, int column) {
    int pos = row*MAP_COLUMNS + column;

    if (!region || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= region->tile_ids_size) {
        return -1;
    }

    return (int32_t)(region->flags[row][column] & MAP_BLOCK);
}

int dsl_region_is_actor(dsl_region_t *region, int row, int column) {
    int pos = row*MAP_COLUMNS + column;

    if (!region || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= region->tile_ids_size) {
        return -1;
    }

    return (int32_t)(region->flags[row][column] & MAP_ACTOR);
}

int dsl_region_is_danger(dsl_region_t *region, int row, int column) {
    int pos = row*MAP_COLUMNS + column;

    if (!region || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= region->tile_ids_size) {
        return -1;
    }

    return (int32_t)(region->flags[row][column] & MAP_DANGER);
}

int32_t region_tile_id(dsl_region_t *region, int row, int column) {
    int pos = row*MAP_COLUMNS + column;

    if (!region || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= region->tile_ids_size) {
        return -1;
    }

    return (int32_t)(region->tile_ids[row][column]);
}

int dsl_region_get_image(const dsl_region_t *region, const uint32_t image_id,
        uint32_t *w, uint32_t *h, unsigned char **data) {
    if (!data) { return 0; }
    *w = get_frame_width(region->gff_file, GT_TILE, region->ids[image_id], 0);
    *h = get_frame_height(region->gff_file, GT_TILE, region->ids[image_id], 0);
    *data = get_frame_rgba_with_palette(region->gff_file, GT_TILE, region->ids[image_id], 0, region->palette_id);
    if (!data) { return 0; }

    return 1;
}
