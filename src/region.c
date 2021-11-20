#include "animation.h"
#include "combat.h"
#include "entity-animation.h"
#include "dsl.h"
#include "player.h"
#include "region.h"
#include "gff-map.h"
#include "gff.h"
#include "gfftypes.h"
#include "port.h"
#include "region-manager.h"

#include <stdlib.h>
#include <string.h>

static void load_tile_ids(sol_region_t *reg);
static void load_map_flags(sol_region_t *reg);
static void load_passives(sol_region_t *reg, const int gff_idx, const int map_id);

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

extern sol_region_t* region_create_empty() {
    sol_region_t *reg = calloc(1, sizeof(sol_region_t));
    reg->entities = entity_list_create();
    //reg->anims = animation_list_create();

    return reg;
}

sol_region_t* region_create(const int gff_file) {
    if (!is_region(gff_file)) { return NULL; } // guard

    uint32_t *tids = NULL;
    sol_region_t *reg = region_create_empty();

    reg->gff_file = gff_file;

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

extern void region_remove_entity(sol_region_t *reg, entity_t *entity) {
    if (!reg || !entity) { return; }
    entity_list_remove(reg->entities, entity_list_find(reg->entities, entity));
    //animation_list_remove(reg->anims, animation_list_find(reg->anims, &(entity->anim)));
}

void region_free(sol_region_t *reg) {
    if (!reg) { return; }

    //if (reg->anims) {
        //animation_list_free(reg->anims);
        //reg->anims = NULL;
    //}
    region_manager_remove_players();

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
    combat_free(&(reg->cr));
    free(reg);
}

int region_get_tile(const sol_region_t *reg, const uint32_t image_id,
        uint32_t *w, uint32_t *h, unsigned char **data) {
    if (!data) { return 0; }

    if (gff_image_is_png(reg->gff_file, GFF_TILE, image_id, 0)) {
        return gff_image_load_png(reg->gff_file, GFF_TILE, image_id, 0, w, h, data);
    }

    *data = gff_get_frame_rgba_with_palette(reg->gff_file, GFF_TILE, image_id, 0, reg->palette_id);
    if (!data) { return 0; }
    *w = gff_get_frame_width(reg->gff_file, GFF_TILE, image_id, 0);
    *h = gff_get_frame_height(reg->gff_file, GFF_TILE, image_id, 0);

    return 1;
}

static void load_tile_ids(sol_region_t *reg) {
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

static void load_map_flags(sol_region_t *reg) {
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

static void load_passives(sol_region_t *reg, const int gff_idx, const int map_id) {
    if (!open_files[gff_idx].entry_table) {
        gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_ETAB, map_id);
        open_files[gff_idx].entry_table = malloc(chunk.length);
        if (!open_files[gff_idx].entry_table) {
            error ("unable to malloc for entry table!\n");
            exit(1);
        }
        gff_read_chunk(gff_idx, &chunk, open_files[gff_idx].entry_table, chunk.length);
    }

    int len = gff_map_get_num_objects(gff_idx, map_id);

    for (int i = 0; i < len; i++) {
        passive_load_from_etab(reg->passives + i, reg->entry_table, i);
        reg->passives[i].scmd = gff_map_get_object_scmd(gff_idx, map_id, i, 0);
    }
}

entity_t* region_find_entity_by_id(sol_region_t *reg, const int id) {
    dude_t *dude = NULL;

    entity_list_for_each(reg->entities, dude)  {
        if (dude->ds_id == id) { return dude; }
    }

    return NULL;
}

static uint8_t* get_block(sol_region_t *region, const int row, const int column) {
    size_t pos = row*MAP_COLUMNS + column;

    if (!region || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column)
        || pos >= sizeof(region->tiles)) {
        return NULL;
    }

    return &(region->flags[row][column]);
}

extern int region_is_block(sol_region_t *region, int row, int column) {
    uint8_t *block = get_block(region, row, column);

    return block ? (*block & MAP_BLOCK) : -1;
}

extern void region_set_block(sol_region_t *region, int row, int column, int val) {
    uint8_t *block = get_block(region, row, column);

    if (block) { *block |= val; }
}

extern void region_clear_block(sol_region_t *region, int row, int column, int val) {
    uint8_t *block = get_block(region, row, column);

    if (block) { region->flags[row][column] &= ~val; }
}


extern int region_location_blocked(const sol_region_t *reg, const int32_t x, const int32_t y) {
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

extern void region_add_entity(sol_region_t *reg, entity_t *entity) {
    if (!reg || !entity) { return; }

    animation_shift_entity(reg->entities, entity_list_add(reg->entities, entity));
    //animation_list_add(reg->anims, &(entity->anim));
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


extern void sol_region_tick(sol_region_t *reg) {
    dude_t *bad_dude = NULL;
    int xdiff, ydiff;
    int posx, posy;
    enum entity_action_e action;
    //static int ticks_per_game_round = 30;

/*
    entity_list_for_each(reg->entities, bad_dude) {
        if (entity_animation_execute(bad_dude)) {
            //printf("ACTION! %d, %d\n", bad_dude->mapx, bad_dude->mapy);
            continue;
        }
    }
    */

    //ticks_per_game_round--;
    //if (ticks_per_game_round > 0) { return; }
    //ticks_per_game_round = 30;

    if (!reg || combat_player_turn() != NO_COMBAT) { return; }

    entity_list_for_each(reg->entities, bad_dude) {
        if (entity_animation_execute(bad_dude)) {
            //printf("ACTION! %s %d, %d\n", bad_dude->name, bad_dude->mapx, bad_dude->mapy);
            continue;
        }
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
            action =
                  (xdiff == 1 && ydiff == 1) ? EA_WALK_DOWNRIGHT
                : (xdiff == 1 && ydiff == -1) ? EA_WALK_UPRIGHT
                : (xdiff == -1 && ydiff == -1) ? EA_WALK_UPLEFT
                : (xdiff == -1 && ydiff == 1) ? EA_WALK_DOWNLEFT
                : (xdiff == 1) ? EA_WALK_RIGHT
                : (xdiff == -1) ? EA_WALK_LEFT
                : (ydiff == 1) ? EA_WALK_DOWN
                : (ydiff == -1) ? EA_WALK_UP
                : EA_NONE;
            entity_animation_list_add(&(bad_dude->actions), action, bad_dude, NULL, NULL, 30);
            bad_dude->mapx += xdiff;
            bad_dude->mapy += ydiff;
            bad_dude->anim.destx += (xdiff * 32);
            bad_dude->anim.desty += (ydiff * 32);
            //animation_shift_entity(reg->entities, __el_rover);
            //bad_dude->anim.scmd = entity_animation_get_scmd(bad_dude->anim.scmd, xdiff, ydiff, EA_NONE);
            if (calc_distance_to_player(bad_dude) < 5) {
                //combat_initiate(reg, bad_dude->mapx, bad_dude->mapy);
                //return;
            }
            //port_update_entity(bad_dude, xdiff, ydiff);
        } else {
            //if (bad_dude->name) {
                //port_update_entity(bad_dude, 0, 0);
            //}
        }
    }
}

extern void region_move_to_nearest(const sol_region_t *reg, entity_t *entity) {
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
