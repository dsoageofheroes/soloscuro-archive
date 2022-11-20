#include "arbiter.h"
#include "combat.h"
#include "entity-animation.h"
#include "gpl.h"
#include "player.h"
#include "region.h"
#include "gff-map.h"
#include "gff.h"
#include "gfftypes.h"
#include "port.h"
#include "entity.h"
#include "narrate.h"
#include "background.h"
#include "trigger.h"
#include "region-manager.h"

#include <stdlib.h>
#include <string.h>

static void load_tile_ids(sol_region_t *reg);
static void load_map_flags(sol_region_t *reg);

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

extern sol_region_t* sol_region_create_empty() {
    sol_region_t *reg = calloc(1, sizeof(sol_region_t));
    reg->entities = entity_list_create();

    return reg;
}

extern sol_region_t* sol_region_create(const int gff_file) {
    if (!is_region(gff_file)) { return NULL; } // guard

    uint32_t *tids = NULL;
    sol_region_t *reg = sol_region_create_empty();

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

    //TODO Finish region_create!
    load_tile_ids(reg);
    load_map_flags(reg);

    return reg;
}

extern void sol_region_remove_entity(sol_region_t *reg, entity_t *entity) {
    if (!reg || !entity) { return; }
    entity_list_remove(reg->entities, entity_list_find(reg->entities, entity));
    //animation_list_remove(reg->anims, animation_list_find(reg->anims, &(entity->anim)));
}

extern uint32_t* sol_region_get_tile_ids(sol_region_t *reg) {
    if (!reg) { return NULL; }
    if (!reg->tile_ids) {
        load_tile_ids(reg);
    }

    return reg->tile_ids;
}

extern void sol_region_gui_free(sol_region_t *reg) {
    if (!reg) { return; }
    sol_region_manager_remove_players();
    dude_t *dude = NULL;

    entity_list_for_each(reg->entities, dude)  {
        sol_entity_gui_free(dude);
    }

    if (reg->entry_table) {
        free(reg->entry_table);
        reg->entry_table = NULL;
    }

    if (reg->tile_ids) {
        free(reg->tile_ids);
        reg->tile_ids = NULL;
    }

    entity_animation_list_free(&(reg->actions));

    sol_background_free();
}

extern void sol_region_free(sol_region_t *reg) {
    if (!reg) { return; }

    sol_region_gui_free(reg);

    if (reg->entities) {
        entity_list_free_all(reg->entities);
        reg->entities = NULL;
    }

    free(reg);
}

extern int sol_region_get_tile(const sol_region_t *reg, const uint32_t image_id,
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

extern entity_t* sol_region_find_entity_by_id(const sol_region_t *reg, const int id) {
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

extern int sol_region_is_block(sol_region_t *region, int row, int column) {
    uint8_t *block = get_block(region, row, column);

    return block ? (*block & MAP_BLOCK) : -1;
}

extern void sol_region_set_block(sol_region_t *region, int row, int column, int val) {
    uint8_t *block = get_block(region, row, column);

    if (block) { *block |= val; }
}

extern void sol_region_clear_block(sol_region_t *region, int row, int column, int val) {
    uint8_t *block = get_block(region, row, column);

    if (block) { region->flags[row][column] &= ~val; }
}

// Should be a quad tree if too slow.
extern entity_t* sol_region_find_entity_by_location(const sol_region_t *reg, const int x, const int y) {
    dude_t *dude = NULL;

    entity_list_for_each(reg->entities, dude) {
        if (dude->mapx == x && dude->mapy == y && dude->stats.hp > 0) {
            return dude;
        }
    }

    return NULL;
}

extern int sol_region_location_blocked(sol_region_t *reg, const int32_t x, const int32_t y) {
    if (sol_region_is_block(reg, y, x)) { return 1; }

    return sol_region_find_entity_by_location(reg, x, y) ? 1 : 0;
}

extern void sol_region_add_entity(sol_region_t *reg, entity_t *entity) {
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
            entity_t *dude = sol_player_get_active();
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

static int move_entity(entity_t *entity, const int x, const int y) {
    int xdiff = x - entity->mapx;
    int ydiff = y - entity->mapy;
    int posx = entity->mapx;
    int posy = entity->mapy;
    sol_region_t *reg = sol_region_manager_get_current();
    xdiff = (xdiff < 0) ? -1 : (xdiff > 0) ? 1 : 0;
    ydiff = (ydiff < 0) ? -1 : (ydiff > 0) ? 1 : 0;

    //printf("need to go to %d, %d @ (%d, %d)\n", x, y, entity->mapx, entity->mapy);
    if (sol_region_location_blocked(reg, posx + xdiff, posy + ydiff)
        ){
        if (!sol_region_location_blocked(reg, posx, posy + ydiff)) {
            xdiff = 0;
        } else if (!sol_region_location_blocked(reg, posx + xdiff, posy)) {
            // TODO: Hack, fix RMAP and then add BFS for go.
            if (!sol_region_location_blocked(reg, posx + 1, posy + ydiff)) {
                xdiff = 1;
            } else {
                ydiff = 0;
            }
        } else {
            xdiff = ydiff = 0;
            xdiff = 1;
        }
    }

    if ((xdiff == 0) && (ydiff == 0)) { return 0; }

    int action =
      (xdiff == 1 && ydiff == 1) ? EA_WALK_DOWNRIGHT
    : (xdiff == 1 && ydiff == -1) ? EA_WALK_UPRIGHT
    : (xdiff == -1 && ydiff == -1) ? EA_WALK_UPLEFT
    : (xdiff == -1 && ydiff == 1) ? EA_WALK_DOWNLEFT
    : (xdiff == 1) ? EA_WALK_RIGHT
    : (xdiff == -1) ? EA_WALK_LEFT
    : (ydiff == 1) ? EA_WALK_DOWN
    : (ydiff == -1) ? EA_WALK_UP
    : EA_NONE;

    entity_animation_list_add(&(entity->actions), action, entity, NULL, NULL, 30);

    return 1;
}

extern void sol_region_tick(sol_region_t *reg) {
    dude_t *bad_dude = NULL;
    int xdiff, ydiff;
    int posx, posy, in_combat = 0;
    enum entity_action_e action;

    if (!reg || sol_map_is_paused()) { return; }
    in_combat = sol_combat_active(sol_arbiter_combat_region(reg));

    if (reg->actions.head) {
        if (entity_animation_region_execute(reg)) {
            return;
        }
    }

    entity_list_for_each(reg->entities, bad_dude) {
        //printf("thing! %s %p %d, %d\n", bad_dude->name, bad_dude->anim.scmd, bad_dude->mapx, bad_dude->mapy);
        //if (bad_dude->ds_id == -2264) {
            //printf("HERE: %d, (%d, %d)\n", bad_dude->anim.bmp_id, bad_dude->mapx, bad_dude->mapy);
        //}
        if (entity_animation_execute(bad_dude)) {
            if (bad_dude->name) {
                //printf("ACTION! %s %d, %d\n", bad_dude->name, bad_dude->mapx, bad_dude->mapy);
            }
            continue;
        }
        if (bad_dude->abilities.must_go && !in_combat && !sol_narrate_is_open()) {
            move_entity(bad_dude, bad_dude->abilities.args.pos.x, bad_dude->abilities.args.pos.y);
            if (bad_dude->mapx == bad_dude->abilities.args.pos.x
                && bad_dude->mapy == bad_dude->abilities.args.pos.y) {
                bad_dude->abilities.must_go = 0;
                printf("Made it.\n");
                sol_trigger_noorders_entity_check(bad_dude);
            }
        }
        if (bad_dude->abilities.hunt && !in_combat) {
            move_entity(bad_dude, sol_player_get_active()->mapx, sol_player_get_active()->mapy);

            if (calc_distance_to_player(bad_dude) < 5) {
                sol_arbiter_enter_combat(reg, bad_dude->mapx, bad_dude->mapy);
                //combat_initiate(reg, bad_dude->mapx, bad_dude->mapy);
                //return;
            }

            continue;
        }

        // Not in animation, not in hunt mode. Lets check scmd.
        if (bad_dude->anim.scmd != ssi_scmd_empty()) {
            //entity_animation_list_add(&bad_dude->actions, EA_SCMD, NULL, NULL, NULL, 30);
            //printf("Need to animate...\n");
        }
    }
}

extern void sol_region_move_to_nearest(sol_region_t *reg, entity_t *entity) {
    if (!sol_region_location_blocked(reg, entity->mapx, entity->mapy + 1)) {
        entity->mapy = entity->mapy + 1;
        return;
    }
    if (!sol_region_location_blocked(reg, entity->mapx - 1, entity->mapy + 1)) {
        entity->mapx = entity->mapx - 1;
        entity->mapy = entity->mapy + 1;
        return;
    }
    if (!sol_region_location_blocked(reg, entity->mapx - 1, entity->mapy)) {
        entity->mapx = entity->mapx - 1;
        return;
    }
    if (!sol_region_location_blocked(reg, entity->mapx - 1, entity->mapy - 1)) {
        entity->mapx = entity->mapx - 1;
        entity->mapy = entity->mapy - 1;
        return;
    }
    if (!sol_region_location_blocked(reg, entity->mapx, entity->mapy - 1)) {
        entity->mapy = entity->mapy - 1;
        return;
    }
    if (!sol_region_location_blocked(reg, entity->mapx + 1, entity->mapy - 1)) {
        entity->mapx = entity->mapx + 1;
        entity->mapy = entity->mapy - 1;
        return;
    }
    if (!sol_region_location_blocked(reg, entity->mapx + 1, entity->mapy)) {
        entity->mapx = entity->mapx + 1;
        return;
    }
    if (!sol_region_location_blocked(reg, entity->mapx + 1, entity->mapy + 1)) {
        entity->mapx = entity->mapx + 1;
        entity->mapy = entity->mapy + 1;
        return;
    }

    error("Unable to place object, please update region_move_to_nearest!\n");
    exit(1);
}

typedef struct action_node_s {
    uint16_t x, y, pos;
    enum entity_action_e actions[MAX_COMBAT_ACTIONS];
    struct action_node_s *next;
} action_node_t;

static void queue_add(action_node_t **head, action_node_t **tail, action_node_t *current, const enum entity_action_e action) {
    action_node_t *next = (action_node_t*)malloc(sizeof(action_node_t));
    memcpy(next, current, sizeof(action_node_t));
    next->actions[next->pos] = action;
    next->pos++;
    next->next = NULL;

    switch (action) {
        case EA_WALK_LEFT:      next->x -= 1; break;
        case EA_WALK_RIGHT:     next->x += 1; break;
        case EA_WALK_UP:        next->y -= 1; break;
        case EA_WALK_DOWN:      next->y += 1; break;
        case EA_WALK_UPLEFT:    next->x -= 1; next->y -= 1; break;
        case EA_WALK_UPRIGHT:   next->x += 1; next->y -= 1; break;
        case EA_WALK_DOWNLEFT:  next->x -= 1; next->y += 1; break;
        case EA_WALK_DOWNRIGHT: next->x += 1; next->y += 1; break;
        default: 
            break; // Do nothing right now...
    }

    if (!*tail) {
        *head = *tail = next;
    } else {
        (*tail)->next = next;
        *tail = next;
    }
}

// Just goes to the position.
extern void sol_region_generate_move(sol_region_t *reg, entity_t *dude, const int x, const int y, const int speed) {
    static uint8_t visit_flags[MAP_ROWS][MAP_COLUMNS];
    action_node_t *queue = NULL, *tail = NULL;
    memset(visit_flags, 0x0, sizeof(uint8_t) * MAP_ROWS * MAP_COLUMNS);
    action_node_t *rover = (action_node_t*) malloc(sizeof(action_node_t));
    memset(rover, 0x0, sizeof(action_node_t));
    rover->x = dude->mapx;
    rover->y = dude->mapy;

    queue_add(&queue, &tail, rover, EA_WALK_LEFT);
    queue_add(&queue, &tail, rover, EA_WALK_RIGHT);
    queue_add(&queue, &tail, rover, EA_WALK_UP);
    queue_add(&queue, &tail, rover, EA_WALK_DOWN);
    queue_add(&queue, &tail, rover, EA_WALK_UPLEFT);
    queue_add(&queue, &tail, rover, EA_WALK_UPRIGHT);
    queue_add(&queue, &tail, rover, EA_WALK_DOWNLEFT);
    queue_add(&queue, &tail, rover, EA_WALK_DOWNRIGHT);
    free(rover);

    // BFS
    while (queue) {
        rover = queue;
        //printf("HERE: (%d, %d)\n", rover->x, rover->y);
        queue = queue->next;
        if (!queue) { tail = NULL; }
        if (rover->x >= MAP_ROWS || rover->y >= MAP_COLUMNS) {
            free(rover);
            continue;
        }
        if (visit_flags[rover->x][rover->y]) { free(rover); continue; }
        if (sol_region_location_blocked(reg, rover->x, rover->y)) { free(rover); continue; }
        visit_flags[rover->x][rover->y] = 1;// Mark as visited.

        if (rover->x == x && rover->y == y) {
            goto found;
        }

        queue_add(&queue, &tail, rover, EA_WALK_LEFT);
        queue_add(&queue, &tail, rover, EA_WALK_RIGHT);
        queue_add(&queue, &tail, rover, EA_WALK_UP);
        queue_add(&queue, &tail, rover, EA_WALK_DOWN);
        queue_add(&queue, &tail, rover, EA_WALK_UPLEFT);
        queue_add(&queue, &tail, rover, EA_WALK_UPRIGHT);
        queue_add(&queue, &tail, rover, EA_WALK_DOWNLEFT);
        queue_add(&queue, &tail, rover, EA_WALK_DOWNRIGHT);

        free(rover);
    }
    // Did not find...
    return;

found:
    for (int i = 0; i < rover->pos; i++) {
        entity_animation_list_add_speed(&dude->actions, rover->actions[i], dude, NULL, NULL, 30, speed, 0);
    }
    free(rover);
    rover = queue;
    while (rover) {
        queue = rover;
        rover = rover->next;
        free(queue);
    }
}
