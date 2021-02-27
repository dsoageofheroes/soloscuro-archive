#include "ds-region-manager.h"
#include "gff.h"
#include "gfftypes.h"
#include "ds-player.h"
#include "ds-state.h"
#include "port.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_REGIONS (100)

ds_region_t *regions[MAX_REGIONS];

ds_region_t* ds_region_get_region(const int region_id) {
    if (region_id < 0 || region_id >= MAX_REGIONS) { return NULL; }
    return regions[region_id];
}

ds_region_t* ds_region_load_region(const int region_id) {
    if (region_id < 0 || region_id >= MAX_REGIONS) { return NULL; }
    char gff_name[32];

    snprintf(gff_name, 32, "rgn%x.gff", region_id);
    int index = gff_find_index(gff_name);
    if (index < 0 ) { return 0; }

    regions[region_id] = dsl_load_region(index);

    return regions[region_id];
}

void ds_region_load_region_from_save(const int id, const int region_id) {
    char gff_name[32];
    char *buf = NULL;
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;

    snprintf(gff_name, 32, "rgn%x.gff", id);
    int gff_index = gff_find_index(gff_name);
    if (gff_index < 0 ) { return; }

    dsl_region_t *reg = NULL;
    //player_pos_t *player = ds_player_get_pos(ds_player_get_active());

    gff_chunk_header_t chunk = gff_find_chunk_header(id, GFF_ROBJ, region_id);
    reg = dsl_load_region(gff_index);

    if (gff_read_chunk(id, &chunk, reg->list, chunk.length) < chunk.length) {
        printf("ERROR READING!\n");
        return ;
    }

    // TODO: clean this up.
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

    chunk = gff_find_chunk_header(id, GFF_RDAT, region_id);
    buf = malloc(chunk.length);
    if (!gff_read_chunk(id, &chunk, buf, chunk.length)) {
        printf("Error loading file.\n");
        exit(1);
    }
    memcpy(reg->flags, buf, sizeof(reg->flags));
    buf += sizeof(reg->flags);
    memcpy(&(reg->cr.hunt), &(((combat_region_t*)buf)->hunt), sizeof(reg->cr.hunt));
    buf -= sizeof(reg->flags);
    free(buf);

    chunk = gff_find_chunk_header(id, GFF_GDAT, 99);
    buf = malloc(chunk.length);
    if (!gff_read_chunk(id, &chunk, buf, chunk.length)) {
        printf("Error loading file.\n");
        exit(1);
    }
    dsl_deserialize_globals(buf);
    free(buf);

    chunk = gff_find_chunk_header(id, GFF_GDAT, region_id);
    buf = malloc(chunk.length);
    if (!gff_read_chunk(id, &chunk, buf, chunk.length)) {
        printf("Error loading file.\n");
        exit(1);
    }
    dsl_deserialize_locals(buf);
    free(buf);

    regions[region_id] = reg;
}

void ds_region_manager_init() {
    memset(regions, 0x0, sizeof(regions));
}

void ds_region_manager_cleanup() {
}

