#include "combat.h"
#include "entity-animation.h"
#include "dsl.h"
#include "ds-player.h"
#include "region.h"
#include "gff-map.h"
#include "gff.h"
#include "gfftypes.h"
#include "port.h"

#include <stdlib.h>
#include <string.h>

static void load_tile_ids(region_t *reg);
static void load_map_flags(region_t *reg);
static void load_passives(region_t *reg, const int gff_idx, const int map_id);

#define GMAP_MAX (MAP_ROWS * MAP_COLUMNS)

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

region_t* region_create(const int gff_file) {
    if (!is_region(gff_file)) { return NULL; } // guard

    uint32_t *tids = NULL;
    region_t *reg = calloc(1, sizeof(region_t));

    reg->gff_file = gff_file;
    reg->entities = entity_list_create();

    tids = gff_get_id_list(reg->gff_file, GFF_ETAB); // temporary to find current id for palette!
    if (!tids) { error("Unable to find current id for map\n"); return NULL; }
    reg->map_id = *tids;
    free(tids);

    gff_chunk_header_t chunk = gff_find_chunk_header(reg->gff_file, GFF_ETAB, reg->map_id);
    reg->entry_table = malloc(chunk.length);
    if (!reg->entry_table) {
        error("Unable to malloc entry table for region!\n");
        free(reg);
        return NULL;
    }
    gff_read_chunk(reg->gff_file, &chunk, reg->entry_table, chunk.length);
    open_files[reg->gff_file].num_objects = chunk.length / sizeof(gff_map_object_t);
    reg->palette_id = gff_get_palette_id(DSLDATA_GFF_INDEX, reg->map_id - 1);
    //reg->ids = gff_get_id_list(reg->gff_file, GFF_TILE);

    //TODO Finish region_create!
    load_tile_ids(reg);
    load_map_flags(reg);
    load_passives(reg, reg->gff_file, reg->map_id);
    combat_init(&(reg->cr));
    //region_list_load_objs(ret->list, ret->gff_file, ret->map_id);

    return reg;
}

extern void region_remove_entity(region_t *reg, entity_t *entity) {
    if (!reg || !entity) { return; }
    entity_list_remove(reg->entities, entity_list_find(reg->entities, entity));
}

void region_free(region_t *reg) {
    if (!reg) { return; }

    if (reg->entities) {
        entity_list_free_all(reg->entities);
        reg->entities = NULL;
    }

    if (reg->entry_table) {
        free(reg->entry_table);
        reg->entry_table = NULL;
    }

    if (reg->tile_ids) {
        free(reg->tile_ids);
        reg->tile_ids = NULL;
    }
}

int region_get_tile(const region_t *reg, const uint32_t image_id,
        uint32_t *w, uint32_t *h, unsigned char **data) {
    if (!data) { return 0; }
    *w = get_frame_width(reg->gff_file, GFF_TILE, reg->tile_ids[image_id], 0);
    *h = get_frame_height(reg->gff_file, GFF_TILE, reg->tile_ids[image_id], 0);
    *data = get_frame_rgba_with_palette(reg->gff_file, GFF_TILE, reg->tile_ids[image_id], 0, reg->palette_id);
    if (!data) { return 0; }

    return 1;
}

static void load_tile_ids(region_t *reg) {
    unsigned int *rmap_ids = gff_get_id_list(reg->gff_file, GFF_RMAP);
    unsigned char *data;
    
    gff_chunk_header_t chunk = gff_find_chunk_header(reg->gff_file, GFF_RMAP, rmap_ids[0]);
    data = malloc(chunk.length);
    if (!data) {
        error ("unable to allocate data for rmap\n");
        goto out;
    }
    if (!gff_read_chunk(reg->gff_file, &chunk, data, chunk.length)) {
        error ("Unable to read RMAP!\n");
        goto out;
    }

    reg->num_tiles = gff_get_resource_length(reg->gff_file, GFF_TILE);
    memcpy(reg->tiles, data, chunk.length);

    free(data);
    reg->tile_ids = gff_get_id_list(reg->gff_file, GFF_TILE);

out:
    free(rmap_ids);
}

static void load_map_flags(region_t *reg) {
    unsigned int *gmap_ids = gff_get_id_list(reg->gff_file, GFF_GMAP);
    gff_chunk_header_t chunk = gff_find_chunk_header(reg->gff_file, GFF_GMAP, gmap_ids[0]);

    if (chunk.length > GMAP_MAX) {
        error ("chunk.length (%d) is grater that GMAP_MAX(%d)\n", chunk.length, GMAP_MAX);
        exit(1);
    }

    if (!gff_read_chunk(reg->gff_file, &chunk, reg->flags, chunk.length)) {
        error ("Unable to read GFF_GMAP chunk!\n");
        goto out;
    }

out:
    free(gmap_ids);
}

static void load_passives(region_t *reg, const int gff_idx, const int map_id) {
    if (!open_files[gff_idx].entry_table) {
        gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_ETAB, map_id);
        open_files[gff_idx].entry_table = malloc(chunk.length);
        if (!open_files[gff_idx].entry_table) {
            error ("unable to malloc for entry table!\n");
            exit(1);
        }
        gff_read_chunk(gff_idx, &chunk, open_files[gff_idx].entry_table, chunk.length);
    }
    //gff_map_object_t *entry_table = open_files[gff_idx].entry_table;
    //gff_map_object_t *entry_table = reg->entry_table;
    int len = gff_map_get_num_objects(gff_idx, map_id);
    //memset(&reg->passives, 0x0, sizeof(passive_t) * MAX_REGION_OBJS);

    for (int i = 0; i < len; i++) {
        //load_object_from_etab(rl->objs + i, entry_table, i);
        passive_load_from_etab(reg->passives + i, reg->entry_table, i);
        reg->passives[i].scmd = gff_map_get_object_scmd(gff_idx, map_id, i, 0);
    }
}

entity_t* region_find_entity_by_id(region_t *reg, const int id) {
    dude_t *dude = NULL;

    entity_list_for_each(reg->entities, dude)  {
        if (dude->ds_id == id) { return dude; }
    }

    return NULL;
}

static uint8_t* get_block(region_t *region, const int row, const int column) {
    size_t pos = row*MAP_COLUMNS + column;

    if (!region || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column)
        || pos >= sizeof(region->tiles)) {
        return NULL;
    }

    return &(region->flags[row][column]);
}

extern int region_is_block(region_t *region, int row, int column) {
    uint8_t *block = get_block(region, row, column);

    return block ? (*block & MAP_BLOCK) : -1;
}

extern void region_set_block(region_t *region, int row, int column, int val) {
    uint8_t *block = get_block(region, row, column);

    if (block) { *block |= val; }
}

extern void region_clear_block(region_t *region, int row, int column, int val) {
    uint8_t *block = get_block(region, row, column);

    if (block) { region->flags[row][column] &= ~val; }
}


extern int region_location_blocked(const region_t *reg, const int32_t x, const int32_t y) {
    dude_t *dude = NULL;
    //if (reg->flags[x][y]) { return 1; }
    entity_list_for_each(reg->entities, dude) {
        //printf("(%d, %d) ?= (%d, %d)\n", obj->mapx, obj->mapy, x, y);
        if (dude->mapx == x && dude->mapy == y) {
            return 1;
        }
    }

    return 0;
}

extern void region_add_entity(region_t *reg, entity_t *entity) {
    printf("Adding: %s %p %p\n", entity->name, reg, entity);
    if (!reg || !entity) { return; }
    //entity_t * dude = NULL;

    entity_list_add(reg->entities, entity);

    //printf("---------------\n");
    //entity_list_for_each(reg->entities, dude) {
        //printf("Entity's name: %s\n", dude->name);
    //}
}

//TODO: Ignores walls, but that might be okay right now.
static int calc_distance_to_player(entity_t *entity) {
    int min = 9999999;
    int max;

    //for (int i = 0; i < MAX_PCS; i++) {
        //if (player_exists(i)) {
            entity_t *dude = player_get_active();
            int xdiff = (entity->mapx - dude->mapx);
            int ydiff = (entity->mapy - dude->mapy);
            if (xdiff < 0) { xdiff *= -1;}
            if (ydiff < 0) { ydiff *= -1;}
            max = (xdiff > ydiff) ? xdiff : ydiff;
            min = (min < max) ? min : max;
        //}
    //}

    return min;
}


extern void region_tick(region_t *reg) {
    dude_t *bad_dude = NULL;
    int xdiff, ydiff;
    int posx, posy;
    static int ticks_per_game_round = 30;

    if (!reg || combat_player_turn() != NO_COMBAT) { return; }

    ticks_per_game_round--;
    if (ticks_per_game_round > 0) { return; }
    ticks_per_game_round = 30;

    entity_list_for_each(reg->entities, bad_dude) {
        //if (bad_dude->name) {
            //printf("bad_dude->name = %s\n", bad_dude->name);
        //}
        if (bad_dude->abilities.hunt) {
            xdiff = player_get_active()->mapx - bad_dude->mapx;
            ydiff = player_get_active()->mapy - bad_dude->mapy;
            xdiff = (xdiff < 0) ? -1 : (xdiff > 0) ? 1 : 0;
            ydiff = (ydiff < 0) ? -1 : (ydiff > 0) ? 1 : 0;
            posx = bad_dude->mapx;
            posy = bad_dude->mapy;

            if (region_location_blocked(reg, posx + xdiff, posy + ydiff)
                    ){
                if (!region_location_blocked(reg, posx, posy + ydiff)) {
                    xdiff = 0;
                } else if (!region_location_blocked(reg, posx + xdiff, posy)) {
                    ydiff = 0;
                } else {
                    xdiff = ydiff = 0;
                }
            }
            bad_dude->sprite.scmd = entity_animation_get_scmd(bad_dude->sprite.scmd, xdiff, ydiff, EA_NONE);
            if (calc_distance_to_player(bad_dude) < 5) {
                combat_initiate(reg, bad_dude->mapx, bad_dude->mapy);
                return;
            }
            port_update_entity(bad_dude, xdiff, ydiff);
        } else {
            //if (bad_dude->name) {
                //port_update_entity(bad_dude, 0, 0);
            //}
        }
    }
}

extern void region_move_to_nearest(const region_t *reg, entity_t *entity) {
    //if (!region_location_blocked(reg, entity->mapx, entity->mapy)) { return; }
    //printf("Tyring to place: %d, %d\n", entity->mapx, entity->mapy);
    if (!region_location_blocked(reg, entity->mapx, entity->mapy + 1)) {
        entity->mapy = entity->mapy + 1;
        return;
    }
    if (!region_location_blocked(reg, entity->mapx - 1, entity->mapy + 1)) {
        entity->mapx = entity->mapx - 1;
        entity->mapy = entity->mapy + 1;
        return;
    }
    if (!region_location_blocked(reg, entity->mapx - 1, entity->mapy)) {
        entity->mapx = entity->mapx - 1;
        return;
    }
    if (!region_location_blocked(reg, entity->mapx - 1, entity->mapy - 1)) {
        entity->mapx = entity->mapx - 1;
        entity->mapy = entity->mapy - 1;
        return;
    }
    if (!region_location_blocked(reg, entity->mapx, entity->mapy - 1)) {
        entity->mapy = entity->mapy - 1;
        return;
    }
    if (!region_location_blocked(reg, entity->mapx + 1, entity->mapy - 1)) {
        entity->mapx = entity->mapx + 1;
        entity->mapy = entity->mapy - 1;
        return;
    }
    if (!region_location_blocked(reg, entity->mapx + 1, entity->mapy)) {
        entity->mapx = entity->mapx + 1;
        return;
    }
    if (!region_location_blocked(reg, entity->mapx + 1, entity->mapy + 1)) {
        entity->mapx = entity->mapx + 1;
        entity->mapy = entity->mapy + 1;
        return;
    }

    error("Unable to place object, please update region_move_to_nearest!\n");
    exit(1);
}
