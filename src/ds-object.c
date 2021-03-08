#include "dsl.h"
#include "ds-object.h"
#include "region.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gff.h"
#include "gfftypes.h"
#include "combat.h"

#define MAX_COMBATS    (1024)
#define MAX_ITEMS      (1024)
#define MAX_CHARACTERS (4) // Change later for online characters.
#define MAX_ITEM1R     (1024)
#define MAX_MINIS      (1024)
#define MAX_ITEM_NAMES (1024)

static ds1_combat_t *ds1_combats = NULL; // Confirmed
static int combat_pos = 0;
static ds1_item_t *ds1_items = NULL; // Confirmed
static int item_pos = 0;
static ds_character_t *ds_characters = NULL; // Confirmed
static int character_pos = 0;
static mini_t *minis = NULL;
static int mini_pos = 0;
static item_name_t *item_names = NULL;
static int item_names_pos = 0;

/* DSL *MUST* hold:
 * OT_ITEM
 * OT_COMBAT
 * OT_CHARREC
 * OT_ITEM1R
 * OT_MINI
 * OT_NAMEIX <-- may not be needed
 */
void dsl_object_init() {
    ds1_combats = malloc(sizeof(ds1_combat_t) * MAX_COMBATS);
    ds1_items = malloc(sizeof(ds1_item_t) * MAX_ITEMS);
    ds_characters = malloc(sizeof(ds_character_t) * MAX_CHARACTERS);
    minis = malloc(sizeof(mini_t) * MAX_MINIS);
    item_names = malloc(sizeof(item_name_t) * MAX_ITEM_NAMES);
    combat_pos = 0;
    item_pos = 0;
    character_pos = 0;
    mini_pos = 0;
    item_names_pos = 0;
    memset(ds1_items, 0x00, sizeof(ds1_item_t) * MAX_ITEMS);
    memset(ds_characters, 0x00, sizeof(ds_character_t) * MAX_CHARACTERS);
    memset(ds1_combats, 0x00, sizeof(ds1_combat_t) * MAX_COMBATS);
    memset(minis, 0x00, sizeof(mini_t) * MAX_MINIS);
    printf("DSL_OBJECT LIST INIT\n");
}

ds_character_t* dsl_get_character(const int id) {
    if (!dsl_valid_character_id(id)) { return NULL; }
    return ds_characters + id;
}

int dsl_valid_character_id(const int id) {
    return (id >=0 && id < MAX_CHARACTERS);
}

void dsl_object_cleanup() {
    free(ds1_combats);
    free(ds1_items);
    free(ds_characters);
    free(minis);
    free(item_names);
    ds1_combats = NULL;
    ds1_items = NULL;
    ds_characters = NULL;
    minis = NULL;
    item_names = NULL;
}

region_list_t* region_list_create() {
    region_list_t *rl = malloc(sizeof(region_list_t));
    if (!rl) { return rl; }
    memset(rl, 0x0, sizeof(region_list_t));
    return rl;
}

void region_list_free(region_list_t *rl) {
    free(rl);
}

static void load_object_from_etab(region_object_t *dsl_object, gff_map_object_t *entry_table, uint32_t id) {
    printf("LOAD OBJECT FROM ETAB\n");
    exit(1);
    const gff_map_object_t *gm = entry_table + id;
    disk_object_t disk_object;
    gff_read_object(gm->index, &disk_object);
    memset(dsl_object, 0x0, sizeof(region_object_t));
    dsl_object->disk_idx = gm->index;
    dsl_object->flags = disk_object.flags;
    dsl_object->gt_idx = disk_object.object_index;
    dsl_object->btc_idx = disk_object.bmp_id;
    dsl_object->bmpx = gm->xpos - disk_object.xoffset;
    dsl_object->bmpy = gm->ypos - disk_object.yoffset - disk_object.zpos;
    dsl_object->xoffset = disk_object.xoffset;
    dsl_object->yoffset = disk_object.yoffset;
    dsl_object->mapx = (gm->xpos + disk_object.xoffset) / 16;
    dsl_object->mapy = (gm->ypos + disk_object.yoffset - disk_object.zpos) / 16;
    dsl_object->mapz = gm->zpos;
    dsl_object->entry_id = id;
    dsl_object->combat_id = COMBAT_ERROR;
    dsl_object->scmd_flags = gm->flags;
    dsl_object->obj_id = abs(gm->index);
    //printf("->%d, %d\n", dsl_object->mapx, dsl_object->mapy);
}

region_object_t* __region_list_get_next(region_list_t *rl, int *i) {
    while (*i < MAX_REGION_OBJS) {
        if (rl->objs[*i].entry_id) { return rl->objs + *i; }
        (*i)++;
    }

    return NULL;
}

void region_list_load_objs(region_list_t *rl, const int gff_idx, const int map_id) {
    if (!open_files[gff_idx].entry_table) {
        gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_ETAB, map_id);
        open_files[gff_idx].entry_table = malloc(chunk.length);
        if (!open_files[gff_idx].entry_table) {
            error ("unable to malloc for entry table!\n");
            exit(1);
        }
        gff_read_chunk(gff_idx, &chunk, open_files[gff_idx].entry_table, chunk.length);
    }
    gff_map_object_t *entry_table = open_files[gff_idx].entry_table;
    rl->pos = gff_map_get_num_objects(gff_idx, map_id);
    memset(&rl->objs, 0x0, sizeof(region_object_t) * MAX_REGION_OBJS);

    for (int i = 0; i < rl->pos; i++) {
        load_object_from_etab(rl->objs + i, entry_table, i);
        rl->objs[i].scmd = gff_map_get_object_scmd(gff_idx, map_id, i, 0);
    }
}
