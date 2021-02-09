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
    size_t amt = gff_read_chunk(DSLDATA_GFF_INDEX, &chunk, item1rs, chunk.length);
    if (amt != chunk.length) {
        error("error reading item1rs...\n");
        exit(1);
    }

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
    return item1rs + (sizeof(ds_item1r_t) * item_idx);
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
