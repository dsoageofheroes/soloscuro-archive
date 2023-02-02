// A GFF map is the map/region data in the gff file.
// A DSL region is the *running* data to support the map/region.
#ifndef DSL_REGION_H
#define DSL_REGION_H

#include <stdint.h>
#include "gff-map.h"
#include "ssi-object.h"
#include "entity.h"
#include "statics.h"
#include "common.h"
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
    // entities are to be in display order.
    struct entity_list_s *entities;
    // actions from several entities, right now one at a time.
    entity_animation_list_t actions;
    // Temporary while we figure things out...
    struct {
        uint16_t mid;// map to grab a tile from
        uint16_t tid;// tile id from the map
    } sol;
    sol_static_list_t statics;
    struct sol_triggers_s triggers;
    uint32_t walls[32];
} sol_region_t;

// external functions...
extern void             sol_combat_update(sol_region_t *reg);

extern sol_region_t*    sol_region_create(const int gff_file);
extern sol_region_t*    sol_region_create_empty();
extern void             sol_region_free(sol_region_t *region);
extern void             sol_region_gui_free(sol_region_t *reg);
extern int              sol_region_get_tile(const sol_region_t *reg, const uint32_t image_id,
        uint32_t *w, uint32_t *h, unsigned char **data);
extern uint32_t*        sol_region_get_tile_ids(sol_region_t *reg);
extern struct entity_s* sol_region_find_entity_by_id(const sol_region_t *reg, const int id);
extern struct entity_s* sol_region_find_entity_by_location(const sol_region_t *reg, const int x, const int y);
extern void             sol_region_generate_move(sol_region_t *reg, entity_t *monster, const int x, const int y, const int speed);
extern void             sol_region_move_to_nearest(sol_region_t *reg, struct entity_s *entity);
extern int              sol_region_location_blocked(sol_region_t *reg, const int32_t x, const int32_t y);
extern void             sol_region_remove_entity(sol_region_t *reg, struct entity_s *entity);
extern sol_status_t     sol_region_add_entity(sol_region_t *reg, struct entity_s *entity);
extern void             sol_region_tick(sol_region_t *reg);
extern int              sol_region_is_block(sol_region_t *region, int row, int column);
extern void             sol_region_set_block(sol_region_t *region, int row, int column, int val);
extern void             sol_region_clear_block(sol_region_t *region, int row, int column, int val);

#endif
