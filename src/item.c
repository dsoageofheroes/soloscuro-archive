#include <string.h>

#include "item.h"
#include "ds-item.h"
#include "dsl.h"
#include "port.h"

static int32_t get_bmp_id(item_t *item) {
    disk_object_t dobj;
    if (!item) { return -1; }

    gff_chunk_header_t chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_OJFF,
            item->ds_id < 0 ? -1 * item->ds_id : item->ds_id);
    if (chunk.length <= 0) { return -1;}

    gff_read_chunk(OBJEX_GFF_INDEX, &chunk, &dobj, sizeof(dobj));

    return dobj.bmp_id;
}

void item_convert_from_ds1(item_t *item, const ds1_item_t *ds1_item) {
    if (!item || !ds1_item) { return; }
    const ds_item1r_t *ds1_item1r = ds_get_item1r(ds1_item->item_index);

    if (!ds1_item1r) {
        error("Unable to find item1r for %d\n", ds1_item->item_index);
        return;
    }

    item->ds_id = ds1_item->id;
    strncpy(item->name, ds_item_name(ds1_item->name_idx), ITEM_NAME_MAX - 1);

    if (ds1_item1r->weapon_type) {
        item->type = ITEM_WEAPON;
    } else if (ds1_item1r->flags & DS1_ARMOR_FLAG) {
        item->type = ITEM_ARMOR;
    } else {
        item->type = ITEM_CONSUMABLE;
    }

    item->quantity = ds1_item->quantity;
    item->value = ds1_item->value;
    item->charges = ds1_item->charges;
    //item->icon = ds1_item->icon; // TODO:probably should look into this...
    item->legal_class = ds1_item1r->legal_class;
    item->placement = ds1_item1r->placement;
    item->weight = ds1_item1r->weight;
    item->material = ds1_item1r->material;
    item->ac = ds1_item1r->base_AC;
    item->attack.number = ds1_item1r->num_attacks;
    item->attack.num_dice = ds1_item1r->dice;
    item->attack.sides = ds1_item1r->sides;
    item->attack.bonus = ds1_item1r->mod;
    item->attack.range = ds1_item1r->range;
    item->attack.damage_type = ds1_item1r->damage_type;
    item->sprite.bmp_id = get_bmp_id(item);

    port_load_item(item);
}
