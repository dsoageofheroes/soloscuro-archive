#include "region-manager.h"
#include "gff.h"
#include "gfftypes.h"
#include "player.h"
#include "gpl-state.h"
#include "port.h"
#include "trigger.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gpl-manager.h"
#include "gpl.h"

static sol_region_t *ssi_regions[MAX_REGIONS];
static sol_region_t *sol_regions[MAX_REGIONS];
static int current_region = -1; // will need to eliminate for server code.

extern void sol_region_manager_init() {
    memset(ssi_regions, 0x0, sizeof(ssi_regions));
    memset(sol_regions, 0x0, sizeof(sol_regions));
}

static void free_entities(sol_region_t *reg) {
    dude_t *dude;

    // Don't free the players, so we will get rid of them first.
    for (int i = 0; i < MAX_PCS; i++) {
        entity_list_remove(reg->entities, entity_list_find(reg->entities, sol_player_get(i)));
    }

    while (reg->entities->head) {
        dude = reg->entities->head->entity;
        entity_list_remove_entity(reg->entities, dude);
        entity_free(dude);
    }
}

extern void sol_region_manager_cleanup(int _free_entities) {
    for (int i = 0; i < MAX_REGIONS; i++) {
        if (ssi_regions[i]) {
            if (_free_entities) {
                free_entities(ssi_regions[i]);
            }
            sol_region_free(ssi_regions[i]);
            ssi_regions[i] = NULL;
        }
        if (sol_regions[i]) {
            if (_free_entities) {
                free_entities(sol_regions[i]);
            }
            sol_region_free(sol_regions[i]);
            ssi_regions[i] = NULL;
        }
    }
    memset(ssi_regions, 0x0, sizeof(ssi_regions));
    memset(sol_regions, 0x0, sizeof(sol_regions));
}

// private: we assume entity and reg are valid.
static int entity_is_in_region(const entity_t *entity, const sol_region_t *reg) {
    dude_t *dude;

    entity_list_for_each(reg->entities, dude) {
        if (dude == entity) { return 1; }
    }

    return 0;
}

extern sol_region_t* sol_region_manager_get_region_with_entity(const entity_t *entity) {
    sol_region_t *reg;
    if (!entity) { return NULL; }

    for (int i = 0; i < MAX_REGIONS; i++) {
        reg = ssi_regions[i];
        if (reg && entity_is_in_region(entity, reg)) { return reg; }
        reg = sol_regions[i];
        if (reg && entity_is_in_region(entity, reg)) { return reg; }
    }

    return NULL;
}

// NOTE: only set assume_loaded on creation!
extern sol_region_t* sol_region_manager_get_region(const int region_id, const int assume_loaded) {
    char gff_name[32];
    entity_t *dude = NULL;

    if (region_id >= MAX_REGIONS && region_id < 2 * MAX_REGIONS) { return sol_regions[region_id - MAX_REGIONS]; }

    if (!ssi_regions[region_id]) {
        snprintf(gff_name, 32, "rgn%02x.gff", region_id);
        int gff_index = gff_find_index(gff_name);
        if (gff_index < 0 ) { return NULL; }

        ssi_regions[region_id] = sol_region_create(gff_index);
        //ssi_regions[region_id]->assume_loaded = assume_loaded;
        if (!assume_loaded) {
            entity_list_load_etab(ssi_regions[region_id]->entities,
                    &ssi_regions[region_id]->statics, gff_index, region_id);
        }
    }

    current_region = region_id;
    sol_trigger_set_region(ssi_regions[region_id]);
	for (int i = 0; i < MAX_PCS; i++) {
        dude = sol_player_get(i);
        if (dude) {
            dude->region = region_id;
        }
    }

    return ssi_regions[region_id];
}

extern sol_region_t* sol_region_manager_get_current() {
    if (current_region < 0) { return NULL; }
    if (current_region < MAX_REGIONS) {
        return ssi_regions[current_region];
    }

    return sol_regions[current_region - MAX_REGIONS];
}

extern int sol_region_manager_add_region(sol_region_t *region) {
    int pos = 0;
    for (pos = 0; pos < MAX_REGIONS && sol_regions[pos]; pos++) { ; }

    if (pos >= MAX_REGIONS) { return -1; }

    sol_regions[pos] = region;
    if (region) {
        region->map_id = pos;
    }
    return pos;
}

extern void sol_region_manager_set_current(sol_region_t *region) {
    if (!region) { return; }

    entity_t *player = sol_player_get_active();
    sol_region_manager_remove_players();

    for (int i = 0; i < MAX_REGIONS; i++) {
        if (ssi_regions[i] == region) { current_region = i; }
        if (sol_regions[i] == region) { current_region = MAX_REGIONS + i; }
    }

    // Clear them all!
    for (int i = 0; i < MAX_REGIONS; i++) {
        sol_region_gui_free(ssi_regions[i]);
    }

    if (player) {
        entity_list_add(region->entities, player);
        if (!player->anim.scmd) { player->anim.scmd = ssi_scmd_empty(); }
    }
    gpl_set_gname(GNAME_REGION, region->map_id);
}

extern void sol_region_manager_remove_players() {
    sol_region_t* reg = sol_region_manager_get_current();
    if (!reg) { return; }

    for (int i = 0; i < MAX_PCS; i++) {
        entity_list_remove(reg->entities, entity_list_find(reg->entities, sol_player_get(i)));
    }
}
