// A GFF map is the map/region data in the gff file.
// A DSL region is the *running* data to support the map/region.
#ifndef DSL_REGION_H
#define DSL_REGION_H

#include <stdint.h>
#include "gff-map.h"
#include "ssi-object.h"
#include "passive.h"
#include "entity.h"
#include "entity-list.h"
#include "combat-region.h"

#define MAX_PASSIVES (1<<10)

typedef struct sol_region_s {
    gff_map_object_t *entry_table;
    uint8_t flags[MAP_ROWS][MAP_COLUMNS];
    uint8_t tiles[MAP_ROWS][MAP_COLUMNS];
    uint32_t *tile_ids;
    uint32_t num_tiles;
    uint32_t palette_id;
    uint32_t gff_file;
    uint32_t map_id;
    passive_t passives[MAX_PASSIVES];
    // entities are to be in display order.
    struct entity_list_s *entities;
    combat_region_t cr;
} sol_region_t;

// external functions...
extern void combat_update(sol_region_t *reg);

extern sol_region_t* region_create(const int gff_file);
extern sol_region_t* region_create_empty();
extern void region_free(sol_region_t *region);
extern int region_get_tile(const sol_region_t *reg, const uint32_t image_id,
        uint32_t *w, uint32_t *h, unsigned char **data);
extern struct entity_s* region_find_entity_by_id(sol_region_t *reg, const int id);
extern void region_move_to_nearest(const sol_region_t *reg, struct entity_s *entity);
extern int region_location_blocked(const sol_region_t *reg, const int32_t x, const int32_t y);
extern void region_remove_entity(sol_region_t *reg, struct entity_s *entity);
extern void region_add_entity(sol_region_t *reg, struct entity_s *entity);
extern void sol_region_tick(sol_region_t *reg);
extern int region_is_block(sol_region_t *region, int row, int column);
extern void region_set_block(sol_region_t *region, int row, int column, int val);
extern void region_clear_block(sol_region_t *region, int row, int column, int val);

#endif
