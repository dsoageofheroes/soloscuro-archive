#include "region-manager.h"
#include "gff.h"
#include "gfftypes.h"
#include "player.h"
#include "gpl-state.h"
#include "port.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gpl-manager.h"
#include "gpl.h"

#define MAX_REGIONS (0xFF)

static sol_region_t *ssi_regions[MAX_REGIONS];
static sol_region_t *sol_regions[MAX_REGIONS];
static int current_region = -1; // will need to eliminate for server code.

void region_manager_init() {
    memset(ssi_regions, 0x0, sizeof(ssi_regions));
    memset(sol_regions, 0x0, sizeof(sol_regions));
}

void region_manager_cleanup() {
    for (int i = 0; i < MAX_REGIONS; i++) {
        if (ssi_regions[i]) {
            region_free(ssi_regions[i]);
            ssi_regions[i] = NULL;
        }
        if (sol_regions[i]) {
            region_free(sol_regions[i]);
            ssi_regions[i] = NULL;
        }
    }
    memset(ssi_regions, 0x0, sizeof(ssi_regions));
    memset(sol_regions, 0x0, sizeof(sol_regions));
}

sol_region_t* region_manager_get_region(const int region_id) {
    char gff_name[32];
    entity_t *dude = NULL;

    if (region_id >= MAX_REGIONS && region_id < 2 * MAX_REGIONS) { return sol_regions[region_id - MAX_REGIONS]; }

    if (!ssi_regions[region_id]) {
        snprintf(gff_name, 32, "rgn%02x.gff", region_id);
        int gff_index = gff_find_index(gff_name);
        if (gff_index < 0 ) { return NULL; }

        ssi_regions[region_id] = region_create(gff_index);
        entity_list_load_etab(ssi_regions[region_id]->entities, gff_index, region_id);
        //entity_list_for_each(ssi_regions[region_id]->entities, dude) {
            //animation_list_add(ssi_regions[region_id]->anims, &dude->anim);
        //}

/*
        entity_list_for_each(ssi_regions[region_id]->entities, dude) {
            animation_list_add(ssi_regions[region_id]->anims, dude->sprite.anim);
        }
        dude = player_get_active();
        if (dude) {
            dude->region = region_id;
            animation_list_add(ssi_regions[region_id]->anims, dude->sprite.anim);
        }
        printf("->%p\n", ssi_regions[region_id]->anims->head);
        */
    }

    current_region = region_id;
	for (int i = 0; i < MAX_PCS; i++) {
        dude = player_get(i);
        if (dude) {
            dude->region = region_id;
            //player_get_entity(i)->region = region_id;
        }
    }

    return ssi_regions[region_id];
}

sol_region_t* region_manager_get_current() {
    if (current_region < 0) { return NULL; }
    if (current_region < MAX_REGIONS) {
        return ssi_regions[current_region];
    }

    return sol_regions[current_region - MAX_REGIONS];
}

// Deprecated API:

void ds_region_load_region_from_save(const int id, const int region_id) {
    char gff_name[32];
    char *buf = NULL;
    //gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;

    snprintf(gff_name, 32, "rgn%x.gff", id);
    int gff_index = gff_find_index(gff_name);
    if (gff_index < 0 ) { return; }

    //dsl_sol_region_t *reg = NULL;
    //entity_t *player = player_get_active();

    gff_chunk_header_t chunk = gff_find_chunk_header(id, GFF_ROBJ, region_id);
    //reg = dsl_load_region(gff_index);

    //if (gff_read_chunk(id, &chunk, reg->list, chunk.length) < chunk.length) {
        //printf("ERROR READING!\n");
        //return ;
    //}

    // TODO: clean this up.
    /*
    for (int i = 0; i < MAX_REGION_OBJS; i++) {
        //printf("reg->list->objs[i].entry_id = %d, %d\n", reg->list->objs[i].entry_id, reg->list->objs[i].combat_id);
        region_object_t *robj = reg->list->objs + i;
        if (robj->combat_id == 9999) {
            robj->scmd = gff_map_get_object_scmd(gff_index, id,
                robj->entry_id, 0);
            if (robj->scmd == 0) {
                robj->scmd = ds_scmd_empty();
            }
        } else {
            if (robj->entry_id > 0) {
                robj->scmd = combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
                gff_chunk_header_t chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_RDFF, robj->entry_id);
                buf = malloc(chunk.length);
                if (!gff_read_chunk(OBJEX_GFF_INDEX, &chunk, buf, chunk.length)) {
                    printf("ERROR can't read %d\n", robj->entry_id);
                    exit(1);
                }
                robj->combat_id = combat_add(&(reg->cr), robj,
                (ds1_combat_t *) (buf + sizeof(rdff_disk_object_t)));
                //robj->mapy -= 16;//sprite_geth(anims[0].spr) - (8);
                robj->bmpx = robj->mapx;
                robj->bmpy = robj->mapy;
                port_add_obj(reg->list->objs + i, pal);
                free(buf);
            }
        }
        robj->data = NULL;
    }
    */

    chunk = gff_find_chunk_header(id, GFF_RDAT, region_id);
    buf = malloc(chunk.length);
    if (!gff_read_chunk(id, &chunk, buf, chunk.length)) {
        printf("Error loading file.\n");
        exit(1);
    }
    //memcpy(reg->flags, buf, sizeof(reg->flags));
    //buf += sizeof(reg->flags);
    //memcpy(&(reg->cr.hunt), &(((combat_sol_region_t*)buf)->hunt), sizeof(reg->cr.hunt));
    //buf -= sizeof(reg->flags);
    free(buf);

    chunk = gff_find_chunk_header(id, GFF_GDAT, 99);
    buf = malloc(chunk.length);
    if (!gff_read_chunk(id, &chunk, buf, chunk.length)) {
        printf("Error loading file.\n");
        exit(1);
    }
    gpl_deserialize_globals(buf);
    free(buf);

    chunk = gff_find_chunk_header(id, GFF_GDAT, region_id);
    buf = malloc(chunk.length);
    if (!gff_read_chunk(id, &chunk, buf, chunk.length)) {
        printf("Error loading file.\n");
        exit(1);
    }
    gpl_deserialize_locals(buf);
    free(buf);

    //ds_regions[region_id] = reg;
}

extern int region_manager_add_region(sol_region_t *region) {
    int pos = 0;
    for (pos = 0; pos < MAX_REGIONS && sol_regions[pos]; pos++) { ; }

    if (pos >= MAX_REGIONS) { return -1; }

    sol_regions[pos] = region;
    if (region) {
        region->map_id = pos;
    }
    return pos;
}

extern void region_manager_set_current(sol_region_t *region) {
    if (!region) { return; }
    region_manager_remove_players();
    for (int i = 0; i < MAX_REGIONS; i++) {
        if (ssi_regions[i] == region) { current_region = i; }
        if (sol_regions[i] == region) { current_region = MAX_REGIONS + i; }
    }
    entity_list_add(region->entities, player_get_active());
}

extern void region_manager_remove_players() {
    sol_region_t* reg = region_manager_get_current();
    if (!reg) { return; }

    for (int i = 0; i < MAX_PCS; i++) {
        entity_list_remove(reg->entities, entity_list_find(reg->entities, player_get(i)));
    }
}
