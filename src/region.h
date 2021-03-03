// A GFF map is the map/region data in the gff file.
// A DSL region is the *running* data to support the map/region.
#ifndef DSL_REGION_H
#define DSL_REGION_H

#include <stdint.h>
#include "../src/gff-map.h"
#include "ds-object.h"
#include "combat.h"
#include "passive.h"
#include "entity-list.h"

typedef struct dsl_region_s {
    gff_map_object_t *entry_table;
    uint32_t *ids;
    uint8_t flags[MAP_ROWS][MAP_COLUMNS];
    uint8_t tile_ids[MAP_ROWS][MAP_COLUMNS];
    uint32_t num_tiles;
    uint32_t entry_size;
    uint32_t palette_id;
    uint32_t flags_size, tile_ids_size;
    uint32_t gff_file;
    uint32_t map_id;
    region_list_t *list;
    combat_region_t cr;
} dsl_region_t;

// Until I can get rid of the misnaming
#define ds_region_t dsl_region_t

extern dsl_region_t* dsl_load_region(const int gff_file);
extern dsl_region_t* init_region_from_gff(const int gff_file);
extern int dsl_region_is_block(dsl_region_t *region, int row, int column);
extern void dsl_region_set_block(dsl_region_t *region, int row, int column, int val);
extern void dsl_region_clear_block(dsl_region_t *region, int row, int column, int val);
extern int dsl_region_is_actor(dsl_region_t *region, int row, int column);
extern int dsl_region_is_danger(dsl_region_t *region, int row, int column);
extern int32_t region_tile_id(dsl_region_t *region, int row, int column);
extern int dsl_region_get_tile(const dsl_region_t *region, const uint32_t image_id,
    uint32_t *w, uint32_t *h, unsigned char **data);
extern unsigned char* dsl_load_object_bmp(dsl_region_t *region, const uint32_t id, const uint32_t bmp_id);
extern dsl_region_t* dsl_region_get_current();

extern uint16_t dsl_region_create_from_objex(dsl_region_t *reg, const int id, const int32_t x, const int32_t y);
extern region_object_t* dsl_region_find_object(const int16_t entry_id);
extern region_object_t* dsl_region_get_object(const int16_t entry_id);
extern uint16_t dsl_region_set_hunt(dsl_region_t *reg, const int16_t obj_id);
extern uint16_t dsl_region_set_allegiance(dsl_region_t *reg, const int16_t obj_id, const uint8_t allegiance);
extern int dsl_region_has_object(dsl_region_t *region, int row, int column);
int ds_region_location_blocked(dsl_region_t *reg, const int32_t x, const int32_t y);

extern void dsl_region_free(dsl_region_t *region);

// external functions...
extern void combat_update(dsl_region_t *reg);

// New interface.
#define MAX_PASSIVES (1<<10)

typedef struct region_s {
    gff_map_object_t *entry_table;
    uint8_t flags[MAP_ROWS][MAP_COLUMNS];
    uint8_t tiles[MAP_ROWS][MAP_COLUMNS];
    uint32_t *tile_ids;
    uint32_t num_tiles;
    //uint32_t entry_size;
    uint32_t palette_id;
    //uint32_t flags_size, tile_ids_size;
    uint32_t gff_file;
    uint32_t map_id;
    passive_t passives[MAX_PASSIVES];
    entity_list_t *entities;
} region_t;

region_t* region_create(const int gff_file);
void region_free(region_t *region);
int region_get_tile(const region_t *reg, const uint32_t image_id,
        uint32_t *w, uint32_t *h, unsigned char **data);
entity_t* region_find_entity_by_id(region_t *reg, const int id);

#endif
