#include "dsl.h"
#include "dsl-object.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gff.h"
#include "gfftypes.h"

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
static item1r_t *ds1_item1rs = NULL; // Confirmed
static int item1r_pos = 0;
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
    unsigned long len;
    char *ptr;
    ds1_combats = malloc(sizeof(ds1_combat_t) * MAX_COMBATS);
    ds1_items = malloc(sizeof(ds1_item_t) * MAX_ITEMS);
    ds_characters = malloc(sizeof(ds_character_t) * MAX_CHARACTERS);
    ds1_item1rs = malloc(sizeof(item1r_t) * MAX_ITEM1R);
    minis = malloc(sizeof(mini_t) * MAX_MINIS);
    item_names = malloc(sizeof(item_name_t) * MAX_ITEM_NAMES);
    combat_pos = 0;
    item_pos = 0;
    character_pos = 0;
    item1r_pos = 0;
    mini_pos = 0;
    item_names_pos = 0;
    memset(ds1_items, 0x00, sizeof(ds1_item_t) * MAX_ITEMS);
    memset(ds_characters, 0x00, sizeof(ds_character_t) * MAX_CHARACTERS);
    memset(ds1_combats, 0x00, sizeof(ds1_combat_t) * MAX_COMBATS);
    memset(ds1_item1rs, 0x00, sizeof(item1r_t) * MAX_ITEM1R);
    memset(minis, 0x00, sizeof(mini_t) * MAX_MINIS);
    // TODO FIXME WARNING, this may not be correct!
    ptr = gff_get_raw_bytes(DSLDATA_GFF_INDEX, GT_IT1R, 1, &len);
    for (int i = 0; i < len/15; i++) {
        memcpy(ds1_item1rs + i, ptr + (15*i), 15);
        //printf("%d\n", ds1_item1rs[i].range);
    }
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
    free(ds1_item1rs);
    free(minis);
    free(item_names);
    ds1_combats = NULL;
    ds1_items = NULL;
    ds_characters = NULL;
    ds1_item1rs = NULL;
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
    //dsl_object->mapx = gm->xpos;
    //dsl_object->mapy = gm->ypos;
    dsl_object->mapx = gm->xpos - disk_object.xoffset;
    dsl_object->mapy = gm->ypos - disk_object.yoffset;
    dsl_object->mapz = gm->zpos;
    dsl_object->entry_id = id;
}

region_object_t* __region_list_get_next(region_list_t *rl, int *i) {
    while (*i < MAX_REGION_OBJS) {
        if (rl->objs[*i].entry_id) { return rl->objs + *i; }
        (*i)++;
    }

    return NULL;
}

region_object_t* region_list_create_from_objex(region_list_t *rl, const int id, const int32_t x, const int32_t y) {
    unsigned long len;
    region_object_t *robj = NULL;
    disk_object_t *dobj = (disk_object_t*)gff_get_raw_bytes(OBJEX_GFF_INDEX, GT_OJFF, -1*id, &len);

    if (!dobj) {
        printf("unable to get obj from id: %d\n", id);
        return NULL;
    }

    if (rl->pos >= MAX_REGION_OBJS) {
        error("Ran out of region objects!\n");
        exit(1);
    }

    robj = rl->objs + rl->pos++;
    memset(robj, 0x0, sizeof(region_object_t));
    robj->disk_idx = id;
    robj->flags = dobj->flags;
    robj->gt_idx = dobj->object_index;
    robj->btc_idx = dobj->bmp_id;
    robj->bmpx = x - dobj->xoffset;
    robj->bmpy = y - dobj->yoffset - dobj->zpos;
    robj->xoffset = dobj->xoffset;
    robj->yoffset = dobj->yoffset;
    robj->mapx = x - dobj->xoffset;
    robj->mapy = y - dobj->yoffset;
    //robj->mapz = gm->zpos;
    robj->mapz = 0;
    robj->entry_id = -1 * id;
    return robj;
}

void region_list_load_objs(region_list_t *rl, const int gff_file, const int map_id) {
    unsigned long len;
    gff_map_object_t *entry_table = (gff_map_object_t*) gff_get_raw_bytes(gff_file, GT_ETAB, map_id, &len);
    rl->pos = gff_map_get_num_objects(gff_file, map_id);
    memset(&rl->objs, 0x0, sizeof(region_object_t) * MAX_REGION_OBJS);

    for (int i = 0; i < rl->pos; i++) {
        load_object_from_etab(rl->objs + i, entry_table, i);
        rl->objs[i].scmd = gff_map_get_object_scmd(gff_file, map_id, i, 0);
    }
}
