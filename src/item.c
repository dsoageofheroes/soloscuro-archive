#include <string.h>

#include "item.h"
#include "ssi-item.h"
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
    const ds_item1r_t *ds1_item1r = ssi_get_item1r(ds1_item->item_index);

    if (!ds1_item1r) {
        error("Unable to find item1r for %d\n", ds1_item->item_index);
        return;
    }

    item->ds_id = ds1_item->id;
    strncpy(item->name, ssi_item_name(ds1_item->name_idx), ITEM_NAME_MAX - 1);

    if (ds1_item1r->flags & DS1_ARMOR_FLAG) {
        item->type = ITEM_ARMOR;
    } else if (ds1_item1r->weapon_type & DS1_WEAPON_MELEE) {
        item->type = ITEM_MELEE;
    } else if (ds1_item1r->weapon_type & DS1_WEAPON_MISSILE) {
        if (ds1_item1r->weapon_type & DS1_WEAPON_THROWN) {
            item->type = ITEM_MISSILE_THROWN;
        } else {
            item->type = ITEM_MISSILE_USE_AMMO;
        }
    } else {
        item->type = ITEM_CONSUMABLE;
    }

    item->quantity = ds1_item->quantity;
    item->value = ds1_item->value;
    item->charges = ds1_item->charges;
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

int item_allowed_in_slot(item_t *item, const int slot) {
    if (!item || slot < 0 || slot > 25) { return 0; }

    if (slot > 13) { return 1; } // backpack is always okay!

    switch (item->placement) {
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

extern void item_load_from(item_t *item, const char *data) {
    memcpy(item, data, sizeof(item_t));
    item->sprite.data = NULL;
    port_load_item(item);
}

extern item_t* item_dup(item_t *item) {
    item_t *ret = malloc(sizeof(item_t));
    memcpy(ret, item, sizeof(item_t));
    return ret;
}

void item_free(item_t *item) {
    if (item) {
        free(item);
    }
}

extern void item_free_inventory(item_t *inv) {
    if (!inv) { return; }
    item_t *items = (item_t*)inv;

    for (int i = 0; i < 26; i++) {
        //if (items[i].ds_id) {
        if (items[i].sprite.data) {
            port_free_item(items + i);
        }
    }

    //TODO: Free up the effects!
}

// TODO: Implement!
extern int item_get_wizard_level(item_t *item) {
    if (!item) { return 0; }
    return 0;
}

extern int item_get_priest_level(item_t *item) {
    if (!item) { return 0; }
    return 0;
}
