#include "dsl-object.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gff.h"
#include "gfftypes.h"

#define MAX_COMBATS    (1024)
#define MAX_ITEMS      (1024)
#define MAX_CHARACTERS (1024)
#define MAX_ITEM1R     (1024)
#define MAX_MINIS      (1024)
#define MAX_ITEM_NAMES (1024)

static ds1_combat_t *ds1_combats = NULL; // Confirmed
static int combat_pos = 0;
static ds1_item_t *ds1_items = NULL; // Confirmed
static int item_pos = 0;
static ds1_character_t *ds1_characters = NULL; // Confirmed
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
    ds1_characters = malloc(sizeof(ds1_character_t) * MAX_CHARACTERS);
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
    memset(ds1_characters, 0x00, sizeof(ds1_character_t) * MAX_CHARACTERS);
    memset(ds1_combats, 0x00, sizeof(ds1_combat_t) * MAX_COMBATS);
    memset(ds1_item1rs, 0x00, sizeof(item1r_t) * MAX_ITEM1R);
    memset(minis, 0x00, sizeof(mini_t) * MAX_MINIS);
    // TODO FIXME WARNING, this may not be correct!
    ptr = gff_get_raw_bytes(DSLDATA_GFF_INDEX, GT_IT1R, 1, &len);
    for (int i = 0; i < len/15; i++) {
        memcpy(ds1_item1rs + i, ptr + (15*i), 15);
        //printf("%d\n", ds1_item1rs[i].range);
    }
}

void dsl_object_cleanup() {
    free(ds1_combats);
    free(ds1_items);
    ds1_combats = NULL;
    ds1_items = NULL;
}
