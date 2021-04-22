// A GFF map is the map/region data in the gff file.
// A DSL region is the *running* data to support the map/region.
#ifndef DSL_REGION_H
#define DSL_REGION_H

#include <stdint.h>
#include "../src/gff-map.h"
#include "ds-object.h"
#include "combat-region.h"
#include "passive.h"
#include "entity.h"
#include "entity-list.h"

#define MAX_PASSIVES (1<<10)

typedef struct region_s {
    gff_map_object_t *entry_table;
    uint8_t flags[MAP_ROWS][MAP_COLUMNS];
    uint8_t tiles[MAP_ROWS][MAP_COLUMNS];
    uint32_t *tile_ids;
    uint32_t num_tiles;
    uint32_t palette_id;
    uint32_t gff_file;
    uint32_t map_id;
    passive_t passives[MAX_PASSIVES];
    entity_list_t *entities;
    combat_region_t cr;
} region_t;

// external functions...
extern void combat_update(region_t *reg);

extern region_t* region_create(const int gff_file);
extern region_t* region_create_empty();
extern void region_free(region_t *region);
extern int region_get_tile(const region_t *reg, const uint32_t image_id,
        uint32_t *w, uint32_t *h, unsigned char **data);
extern entity_t* region_find_entity_by_id(region_t *reg, const int id);
extern void region_move_to_nearest(const region_t *reg, entity_t *entity);
extern int region_location_blocked(const region_t *reg, const int32_t x, const int32_t y);
extern void region_remove_entity(region_t *reg, entity_t *entity);
extern void region_add_entity(region_t *reg, entity_t *entity);
extern void region_tick(region_t *reg);
extern int region_is_block(region_t *region, int row, int column);
extern void region_set_block(region_t *region, int row, int column, int val);
extern void region_clear_block(region_t *region, int row, int column, int val);

#endif
