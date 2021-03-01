#include "dsl.h"
#include "ds-player.h"
#include "ds-region.h"
#include "gff-map.h"
#include "gff.h"
#include "gfftypes.h"

#include <stdlib.h>
#include <string.h>

static void dsl_load_map_tile_ids(dsl_region_t* region);
static void dsl_load_map_flags(dsl_region_t *region);
static dsl_region_t *cregion = NULL;

static int is_region(const int gff_idx) {
    int has_rmap = 0, has_gmap = 0, has_tile = 0, has_etab = 0;

    if (gff_idx < 0 || gff_idx >= NUM_FILES) { return 0; }
    if (!open_files[gff_idx].file) { return 0; }

    for (int i = 0; i < open_files[gff_idx].num_types; i++) {
        if ((open_files[gff_idx].chunks[i]->chunk_type & GFFMAXCHUNKMASK) == GFF_RMAP) { has_rmap = 1;}
        if ((open_files[gff_idx].chunks[i]->chunk_type & GFFMAXCHUNKMASK) == GFF_GMAP) { has_gmap = 1;}
        if ((open_files[gff_idx].chunks[i]->chunk_type & GFFMAXCHUNKMASK) == GFF_TILE) { has_tile = 1;}
        if ((open_files[gff_idx].chunks[i]->chunk_type & GFFMAXCHUNKMASK) == GFF_ETAB) { has_etab = 1;}
    }

    return has_rmap && has_gmap && has_tile && has_etab;
}

dsl_region_t* init_region_from_gff(const int gff_file) {
    if (!is_region(gff_file)) { return NULL; } // guard
    uint32_t *tids = NULL;
    dsl_region_t *ret = malloc(sizeof(dsl_region_t));

    memset(ret, 0x0, sizeof(dsl_region_t));
    ret->list = region_list_create();
    ret->gff_file = gff_file;

    tids = gff_get_id_list(ret->gff_file, GFF_ETAB); // temporary to find current id for palette!
    if (!tids) { error("Unable to find current id for map\n"); return NULL; }
    ret->map_id = *tids;
    free(tids);

    gff_chunk_header_t chunk = gff_find_chunk_header(ret->gff_file, GFF_ETAB, ret->map_id);
    ret->entry_table = malloc(chunk.length);
    if (!ret->entry_table) {
        error("Unable to malloc entry table for region!\n");
        exit(1);
    }
    gff_read_chunk(ret->gff_file, &chunk, ret->entry_table, chunk.length);
    open_files[ret->gff_file].num_objects = chunk.length / sizeof(gff_map_object_t);
    ret->palette_id = gff_get_palette_id(DSLDATA_GFF_INDEX, ret->map_id - 1);
    ret->ids = gff_get_id_list(ret->gff_file, GFF_TILE);

    dsl_load_map_tile_ids(ret);
    dsl_load_map_flags(ret);

    cregion = ret;

    return ret;
}

dsl_region_t* dsl_load_region(const int gff_file) {
    dsl_region_t *ret = init_region_from_gff(gff_file);
    if (!ret) { return NULL; }

    region_list_load_objs(ret->list, ret->gff_file, ret->map_id);

    for (int i = 0; i < 4; i++) {
        ds_player_get_pos(i)->map = ret->map_id;
    }

    return ret;
}

int ds_region_location_blocked(dsl_region_t *reg, const int32_t x, const int32_t y) {
    //return dsl_region_is_block(reg, x, y);
    region_object_t *obj = NULL;
    region_list_for_each(reg->list, obj) {
        //printf("(%d, %d) ?= (%d, %d)\n", obj->mapx, obj->mapy, x, y);
        if (obj->mapx == x && obj->mapy == y) {
            return 1;
        }
    }

    return 0;
}

static void place_region_object(dsl_region_t *reg, region_object_t *robj, const int32_t x, const int32_t y) {
    if (!ds_region_location_blocked(reg, x, y)) { return; }
    if (!ds_region_location_blocked(reg, x, y + 1)) {
        robj->mapy = y + 1;
        return;
    }
    if (!ds_region_location_blocked(reg, x - 1, y + 1)) {
        robj->mapx = x - 1;
        robj->mapy = y + 1;
        return;
    }
    if (!ds_region_location_blocked(reg, x - 1, y)) {
        robj->mapx = x - 1;
        return;
    }
    if (!ds_region_location_blocked(reg, x - 1, y - 1)) {
        robj->mapx = x - 1;
        robj->mapy = y - 1;
        return;
    }
    if (!ds_region_location_blocked(reg, x, y - 1)) {
        robj->mapy = y - 1;
        return;
    }
    if (!ds_region_location_blocked(reg, x + 1, y - 1)) {
        robj->mapx = x + 1;
        robj->mapy = y - 1;
        return;
    }
    if (!ds_region_location_blocked(reg, x + 1, y)) {
        robj->mapx = x + 1;
        return;
    }
    if (!ds_region_location_blocked(reg, x + 1, y + 1)) {
        robj->mapx = x + 1;
        robj->mapy = y + 1;
        return;
    }

    error("Unable to place object, please update place_region_object!\n");
    exit(1);
}

#define RDFF_BUF_SIZE (1<<11)
uint16_t dsl_region_create_from_objex(dsl_region_t *reg, const int id, const int32_t x, const int32_t y) {
    region_object_t *robj = NULL;
    char buf[RDFF_BUF_SIZE];
    rdff_disk_object_t *rdff = (rdff_disk_object_t*) buf;
    disk_object_t dobj;
    uint16_t ret = reg->list->pos;

    gff_chunk_header_t chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_RDFF, -1 * id);
    if (chunk.length >= RDFF_BUF_SIZE) {
        error ("Unable to load RDFF, buffer size is too small.\n");
        exit(1);
    }
    if (!gff_read_chunk(OBJEX_GFF_INDEX, &chunk, buf, chunk.length)) {
        printf("unable to get obj from id: %d\n", id);
        return 0;
    }
    chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_OJFF, -1 * id);
    if (!gff_read_chunk(OBJEX_GFF_INDEX, &chunk, &(dobj), sizeof(disk_object_t))) {
        printf("unable to get obj from id: %d\n", id);
        return 0;
    }

    if (reg->list->pos >= MAX_REGION_OBJS) {
        error("Ran out of region objects!\n");
        exit(1);
    }

    robj = reg->list->objs + reg->list->pos;
    memset(robj, 0x0, sizeof(region_object_t));
    robj->disk_idx = id;
    robj->flags = dobj.flags;
    robj->gt_idx = dobj.object_index;
    robj->btc_idx = dobj.bmp_id;
    robj->xoffset = dobj.xoffset;
    robj->yoffset = dobj.yoffset;
    robj->mapx = x;
    robj->mapy = y;
    robj->combat_id = 0;
    robj->rdff_type = rdff->type;
    place_region_object(reg, robj, robj->mapx, robj->mapy);
    robj->bmpx = robj->mapx * 16;
    robj->bmpy = robj->mapy * 16;
    robj->rdff_type = rdff->type;
    robj->mapz = dobj.zpos;
    robj->entry_id = -1 * id;
    robj->obj_id = abs(id);

    switch(rdff->type) {
        case COMBAT_OBJECT:
            robj->scmd = combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
            robj->combat_id = combat_add(&(reg->cr), robj, (ds1_combat_t *) (rdff + 1));
            break;
        case CHAR_OBJECT:
            printf("CHAR OBJECT!!!!!\n");
            exit(1);
            break;
    }

    reg->list->pos++;

    return ret;
}

void dsl_region_free(dsl_region_t *region) {
    if (!region) { return; }
    if (region->list) {
        free(region->list);
        region->list = NULL;
    }
    if (region->entry_table) {
        free(region->entry_table);
        region->entry_table = NULL;
    }
    if (region->ids) {
        free(region->ids);
        region->ids = NULL;
    }
    if (open_files[region->gff_file].entry_table) {
        free(open_files[region->gff_file].entry_table);
        open_files[region->gff_file].entry_table = NULL;
    }
    free(region);
}

dsl_region_t* dsl_region_get_current() { return cregion; }
uint16_t dsl_region_set_hunt(dsl_region_t *reg, const int16_t obj_id) {
    if (!reg || obj_id == COMBAT_ERROR) { return COMBAT_ERROR; }

    for (int i = 0; i < reg->list->pos; i++) {
        if (reg->list->objs[i].obj_id == obj_id) {
            debug("Setting %d to hunt(%d).\n", obj_id, reg->list->objs[i].combat_id);
            combat_set_hunt(&(reg->cr), reg->list->objs[i].combat_id);
        }
    }
    return reg->list->objs[obj_id].combat_id;
}

uint16_t dsl_region_set_allegiance(dsl_region_t *reg, const int16_t obj_id, const uint8_t allegiance) {
    if (!reg || obj_id == COMBAT_ERROR) { return COMBAT_ERROR; }

    for (int i = 0; i < reg->list->pos; i++) {
        if (reg->list->objs[i].obj_id == obj_id) {
            debug("Setting allegiance of %d to %d\n", obj_id, allegiance);
            ds1_combat_t* combat = combat_get_combat(&(reg->cr), reg->list->objs[i].combat_id);
            combat->allegiance = allegiance;
        }
    }
    return reg->list->objs[obj_id].combat_id;
}

region_object_t* dsl_region_find_object(const int16_t disk_idx) {
    region_object_t *obj = NULL;

    region_list_for_each(cregion->list, obj) {
        if (obj->disk_idx == disk_idx) { return obj; }
    }

    return NULL;
}

int dsl_region_has_object(dsl_region_t *region, int row, int column) {
    return ds_region_location_blocked(region, row * 16, column * 16);
}


region_object_t* dsl_region_get_object(const int16_t entry_id) {
    return cregion->list->objs + entry_id;
}

#define RMAP_MAX (1<<14)
static void dsl_load_map_tile_ids(dsl_region_t *region) {
    unsigned int *rmap_ids = gff_get_id_list(region->gff_file, GFF_RMAP);
    unsigned char data[RMAP_MAX];
    
    gff_chunk_header_t chunk = gff_find_chunk_header(region->gff_file, GFF_RMAP, rmap_ids[0]);
    if (chunk.length > RMAP_MAX) {
        error ("RMAP data length (%d) is larger than RMAP_MAX (%d)\n", chunk.length, RMAP_MAX);
        exit(1);
    }
    if (!gff_read_chunk(region->gff_file, &chunk, data, chunk.length)) {
        error ("Unable to read RMAP!\n");
        goto out;
    }

    region->tile_ids_size = chunk.length;
    region->num_tiles = gff_get_resource_length(region->gff_file, GFF_TILE);
    memcpy(region->tile_ids, data, chunk.length);

out:
    free(rmap_ids);
}

unsigned char* dsl_load_object_bmp(dsl_region_t *region, const uint32_t id, const uint32_t bmp_id) {
    region_object_t *obj = region->list->objs + id;
    return gff_map_get_object_bmp_pal(region->gff_file, region->map_id, id,
            (int32_t*)&(obj->bmp_width), (int32_t*)&(obj->bmp_height), bmp_id, 
            region->palette_id);
}

#define GMAP_MAX (1<<14)

static void dsl_load_map_flags(dsl_region_t *region) {
    unsigned int *gmap_ids = gff_get_id_list(region->gff_file, GFF_GMAP);
    gff_chunk_header_t chunk = gff_find_chunk_header(region->gff_file, GFF_GMAP, gmap_ids[0]);

    if (chunk.length > GMAP_MAX) {
        error ("chunk.length (%d) is grater that GMAP_MAX(%d)\n", chunk.length, GMAP_MAX);
        exit(1);
    }

    if (!gff_read_chunk(region->gff_file, &chunk, region->flags, chunk.length)) {
        error ("Unable to read GFF_GMAP chunk!\n");
        goto out;
    }

    region->flags_size = chunk.length;
out:
    free(gmap_ids);
}

static uint8_t* get_block(dsl_region_t *region, const int row, const int column) {
    int pos = row*MAP_COLUMNS + column;

    if (!region || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= region->tile_ids_size) {
        return NULL;
    }

    return &(region->flags[row][column]);
}

int dsl_region_is_block(dsl_region_t *region, int row, int column) {
    uint8_t *block = get_block(region, row, column);

    return block ? (*block & MAP_BLOCK) : -1;
}

void dsl_region_set_block(dsl_region_t *region, int row, int column, int val) {
    uint8_t *block = get_block(region, row, column);

    if (block) { *block |= val; }
}

void dsl_region_clear_block(dsl_region_t *region, int row, int column, int val) {
    uint8_t *block = get_block(region, row, column);

    if (block) { region->flags[row][column] &= ~val; }
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

int dsl_region_get_tile(const dsl_region_t *region, const uint32_t image_id,
        uint32_t *w, uint32_t *h, unsigned char **data) {
    if (!data) { return 0; }
    *w = get_frame_width(region->gff_file, GFF_TILE, region->ids[image_id], 0);
    *h = get_frame_height(region->gff_file, GFF_TILE, region->ids[image_id], 0);
    *data = get_frame_rgba_with_palette(region->gff_file, GFF_TILE, region->ids[image_id], 0, region->palette_id);
    if (!data) { return 0; }

    return 1;
}
