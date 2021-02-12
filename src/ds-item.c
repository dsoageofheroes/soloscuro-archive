#include "ds-item.h"
#include "dsl.h"
#include "gff-common.h"
#include "gff.h"
#include "gfftypes.h"
#include <stdlib.h>
#include <string.h>

static ds_item1r_t *item1rs = NULL; // 1 record for each item (ie: template)
static size_t num_item1rs = 0;
static char *names = NULL;
static size_t num_names = 0;

void ds_item_init() {
    gff_chunk_header_t chunk = gff_find_chunk_header(DSLDATA_GFF_INDEX, GFF_IT1R, 1);
    if (chunk.length == 0 || (chunk.length % sizeof(ds_item1r_t)) != 0) {
        error("item1r not loadable...\n");
        exit(1);
    }
    item1rs = malloc(chunk.length);
    num_item1rs = chunk.length / sizeof(ds_item1r_t);
    //printf("chunk.length = %d, num_item1rs = %ld\n", chunk.length, num_item1rs);
    //exit(1);
    size_t amt = gff_read_chunk(DSLDATA_GFF_INDEX, &chunk, item1rs, chunk.length);
    if (amt != chunk.length) {
        error("error reading item1rs...\n");
        exit(1);
    }
    //printf("%d: %d\n", 4, item1rs[4].base_AC);
    //exit(1);

    chunk = gff_find_chunk_header(DSLDATA_GFF_INDEX, GFF_NAME, 1);
    names = malloc(chunk.length);
    num_names = chunk.length / 25;
    amt = gff_read_chunk(DSLDATA_GFF_INDEX, &chunk, names, chunk.length);
    if (amt != chunk.length) {
        error("error reading name...\n");
        exit(1);
    }
}

#define BUF_MAX (1<<12)

int ds_item_load(ds1_item_t *item, int32_t id) {
    if (item == NULL) { return 0; }
    if (id < 0) { id *= -1; }
    char buf[BUF_MAX];
    uint8_t slot;
    rdff_disk_object_t *entry = (rdff_disk_object_t *)buf;

    gff_chunk_header_t chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_RDFF, id);
    if (chunk.length >= BUF_MAX) {
        error("item info is larger than BUF_MAX!\n");
        return 0;
    }
    gff_read_chunk(OBJEX_GFF_INDEX, &chunk, buf, chunk.length);

    //if(entry->type != RDFF_OBJECT && entry->type != RDFF_CONTAINER) { return 0; }
    if(entry->type != RDFF_OBJECT) { return 0; } // Containers are not items.

    slot = item->slot;
    memcpy(item, (entry + 1), sizeof(ds1_item_t));
    item->slot = slot;
    //printf("item->id = %d\n", item->id);
    //printf("item->name_idx = %d (%s)\n", item->name_idx, names + 25*item->name_idx);
    return 1;
}

const char *ds_item_name(const int32_t name_idx) {
    if (name_idx < 0 || name_idx >= num_names) { return NULL; }
    return names + 25 * name_idx;
}

const ds_item1r_t *ds_get_item1r(const int32_t item_idx) {
    if (item_idx < 0 || item_idx >= num_item1rs) { return NULL; }
    return item1rs + item_idx;
}

void ds_item_close() {
    if (item1rs) {
        free(item1rs);
        item1rs = NULL;
    }
    if (names) {
        free(names);
        names = NULL;
    }
}

int32_t ds_item_get_bmp_id(ds1_item_t *item) {
    disk_object_t dobj;
    if (!item) { return -1; }

    gff_chunk_header_t chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_OJFF,
            item->id < 0 ? -1 * item->id : item->id);
    if (chunk.length <= 0) { return -1;}

    gff_read_chunk(OBJEX_GFF_INDEX, &chunk, &dobj, sizeof(dobj));

    return dobj.bmp_id;
}

int ds_item_allowed_in_slot(ds1_item_t *item, const int slot) {
    if (!item || slot < 0 || slot > 25) { return 0; }
    ds_item1r_t *it1r = item1rs + item->item_index;

    if (slot > 13) { return 1; } // backpack is always okay!

    switch (it1r->placement) {
        case 1: // Chest
            return slot == SLOT_CHEST;
        case 2: // Waist
            return slot == SLOT_WAIST;
        case 3: // ARM
            return slot == SLOT_ARM;
        case 4: // FOOT
            return slot == SLOT_FOOT;
        case 5: // HAND
            return slot == SLOT_HAND0 || slot == SLOT_HAND1;
        case 6: // HEAD
            return slot == SLOT_HEAD;
        case 7: // NECK
            return slot == SLOT_NECK;
        case 8: // CLOAK
            return slot == SLOT_CLOAK;
        case 9: // FINGER
            return slot == SLOT_FINGER0 || slot == SLOT_FINGER1;
        case 10: // LEGS
            return slot == SLOT_LEGS;
        case 11: // AMMO
            return slot == SLOT_AMMO;
        case 12: // MISSILE
            return slot == SLOT_MISSILE;
    }

    return 0;
}
