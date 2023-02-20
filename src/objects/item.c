#include <string.h>

#include "item.h"
#include "ssi-item.h"
#include "gpl.h"
#include "map.h"
#include "port.h"
#include "entity.h"
#include "sprite.h"

extern sol_status_t sol_inventory_create(sol_item_t **d) {
    if (!d) { return SOL_NULL_ARGUMENT; }

    sol_item_t *ret = calloc(1, sizeof(sol_inventory_t));
    if (!ret) { return SOL_MEMORY_ERROR; }

    for (int i = 0; i < 26; i++) {
        ret[i].anim.spr = SPRITE_ERROR;
    }

    *d = ret;
    return SOL_SUCCESS;
}

static int32_t get_bmp_id(sol_item_t *item) {
    disk_object_t dobj;
    if (!item) { return -1; }

    gff_chunk_header_t chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_OJFF,
            item->ds_id < 0 ? -1 * item->ds_id : item->ds_id);
    if (chunk.length <= 0) { return -1;}

    gff_read_chunk(OBJEX_GFF_INDEX, &chunk, &dobj, sizeof(dobj));

    return dobj.bmp_id;
}

extern sol_status_t sol_item_convert_from_ds1(sol_item_t *item, const ds1_item_t *ds1_item) {
    if (!item || !ds1_item) { return SOL_NULL_ARGUMENT; }
    const ds_item1r_t *ds1_item1r = ssi_get_item1r(ds1_item->item_index);

    if (!ds1_item1r) {
        error("Unable to find item1r for %d\n", ds1_item->item_index);
        return SOL_NOT_FOUND;
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
    item->anim.bmp_id = get_bmp_id(item);
    item->item_index = ds1_item->item_index;

    return sol_load_item(item);
}

extern sol_status_t sol_item_allowed_in_slot(sol_item_t *item, const int slot) {
    if (!item)                 { return SOL_NULL_ARGUMENT; }
    if (slot < 0 || slot > 25) { return SOL_OUT_OF_RANGE; }

    if (slot > 13) { return SOL_SUCCESS; } // backpack is always okay!

    switch (item->placement) {
        case 1: // Chest
            return slot == SLOT_CHEST ? SOL_SUCCESS : SOL_ILLEGAL_SLOT;
        case 2: // Waist
            return slot == SLOT_WAIST ? SOL_SUCCESS : SOL_ILLEGAL_SLOT;
        case 3: // ARM
            return slot == SLOT_ARM ? SOL_SUCCESS : SOL_ILLEGAL_SLOT;
        case 4: // FOOT
            return slot == SLOT_FOOT ? SOL_SUCCESS : SOL_ILLEGAL_SLOT;
        case 5: // HAND
            return slot == SLOT_HAND0 || slot == SLOT_HAND1 ? SOL_SUCCESS : SOL_ILLEGAL_SLOT;
        case 6: // HEAD
            return slot == SLOT_HEAD ? SOL_SUCCESS : SOL_ILLEGAL_SLOT;
        case 7: // NECK
            return slot == SLOT_NECK ? SOL_SUCCESS : SOL_ILLEGAL_SLOT;
        case 8: // CLOAK
            return slot == SLOT_CLOAK ? SOL_SUCCESS : SOL_ILLEGAL_SLOT;
        case 9: // FINGER
            return slot == SLOT_FINGER0 || slot == SLOT_FINGER1 ? SOL_SUCCESS : SOL_ILLEGAL_SLOT;
        case 10: // LEGS
            return slot == SLOT_LEGS ? SOL_SUCCESS : SOL_ILLEGAL_SLOT;
        case 11: // AMMO
            return slot == SLOT_AMMO ? SOL_SUCCESS : SOL_ILLEGAL_SLOT;
        case 12: // MISSILE
            return slot == SLOT_MISSILE ? SOL_SUCCESS : SOL_ILLEGAL_SLOT;
    }

    return SOL_UNKNOWN_ERROR;
}

extern sol_status_t sol_item_dup(sol_item_t *item, sol_item_t **d) {
    if (!item || !d) { return SOL_NULL_ARGUMENT; }

    sol_item_t *ret = malloc(sizeof(sol_item_t));
    if (!ret) { return SOL_MEMORY_ERROR; }

    memcpy(ret, item, sizeof(sol_item_t));
    *d = ret;
    return SOL_SUCCESS;
}

extern sol_status_t sol_item_free_except_graphics(sol_item_t *item) {
    if (!item) { return SOL_NULL_ARGUMENT; }

    free(item);
    return SOL_SUCCESS;
}

static void free_item(sol_item_t *item) {
    if (!item) { return; }

    if (item->anim.spr != SPRITE_ERROR) {
        sol_status_check(sol_sprite_free(item->anim.spr), "Unable to free sprite.");
        item->anim.spr = SPRITE_ERROR;
    }
}

extern sol_status_t sol_item_free(sol_item_t *item) {
    if (!item) { return SOL_NULL_ARGUMENT; }

    free_item(item);
    free(item);
    return SOL_SUCCESS;
}

extern sol_status_t sol_item_free_inventory(sol_item_t *inv) {
    if (!inv) { return SOL_NULL_ARGUMENT; }
    sol_item_t *items = (sol_item_t*)inv;

    for (int i = 0; i < 26; i++) {
        free_item(items + i);
    }

    //TODO: Free up the effects!
    return SOL_SUCCESS;
}

// TODO: Implement!
extern sol_status_t sol_item_get_wizard_level(sol_item_t *item, uint8_t *level) {
    if (!item) { return SOL_NULL_ARGUMENT; }

    return SOL_NOT_IMPLEMENTED;
}

extern sol_status_t sol_item_get_priest_level(sol_item_t *item, uint8_t *level) {
    if (!item) { return SOL_NULL_ARGUMENT; }

    return SOL_NOT_IMPLEMENTED;
}

extern sol_status_t sol_item_icon(sol_item_t *item, animate_sprite_t **d) {
    if (!item || !d) { return SOL_NULL_ARGUMENT; }

    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    //if (!port_valid_sprite(&item->sprite)) {
    if (item->anim.spr == SPRITE_ERROR) {
        if (!item->anim.bmp_id) { return SOL_NOT_FOUND; }
        sol_sprite_load(&item->anim, pal, OBJEX_GFF_INDEX, GFF_BMP, item->anim.bmp_id, 1);
    }

    *d = &(item->anim);
    return SOL_SUCCESS;
}

extern sol_status_t sol_give_ds1_item(sol_entity_t *pc, const int slot, const int item_index, const int id) {
    if (!pc) { return SOL_NULL_ARGUMENT; }
    if (!pc->inv) { sol_inventory_create(&pc->inv); }
    ds1_item_t ds1_item;
    ds1_item.slot = SLOT_HAND0;
    ds1_item.item_index = item_index;
    ssi_item_load(&ds1_item, id);
    return sol_item_convert_from_ds1(pc->inv + slot, &ds1_item);
}

extern sol_status_t sol_item_set_starting(sol_dude_t *dude) {
    if (!dude) { return SOL_NULL_ARGUMENT; }
    int has_gladiator = sol_entity_has_class(dude, REAL_CLASS_GLADIATOR) == SOL_SUCCESS;
    int has_fighter = sol_entity_has_class(dude, REAL_CLASS_FIGHTER) == SOL_SUCCESS;
    int has_ranger = sol_entity_has_class(dude, REAL_CLASS_AIR_RANGER) == SOL_SUCCESS
        || sol_entity_has_class(dude, REAL_CLASS_EARTH_RANGER) == SOL_SUCCESS
        || sol_entity_has_class(dude, REAL_CLASS_FIRE_RANGER) == SOL_SUCCESS
        || sol_entity_has_class(dude, REAL_CLASS_WATER_RANGER) == SOL_SUCCESS;
    int has_druid = sol_entity_has_class(dude, REAL_CLASS_AIR_DRUID) == SOL_SUCCESS
        || sol_entity_has_class(dude, REAL_CLASS_EARTH_DRUID) == SOL_SUCCESS
        || sol_entity_has_class(dude, REAL_CLASS_FIRE_DRUID) == SOL_SUCCESS
        || sol_entity_has_class(dude, REAL_CLASS_WATER_DRUID) == SOL_SUCCESS;
    int has_cleric = sol_entity_has_class(dude, REAL_CLASS_AIR_CLERIC) == SOL_SUCCESS
        || sol_entity_has_class(dude, REAL_CLASS_EARTH_CLERIC) == SOL_SUCCESS
        || sol_entity_has_class(dude, REAL_CLASS_FIRE_CLERIC) == SOL_SUCCESS
        || sol_entity_has_class(dude, REAL_CLASS_WATER_CLERIC) == SOL_SUCCESS;
    int has_psionicist = sol_entity_has_class(dude, REAL_CLASS_PSIONICIST) == SOL_SUCCESS;
    int has_thief = sol_entity_has_class(dude, REAL_CLASS_THIEF) == SOL_SUCCESS;
    int has_preserver = sol_entity_has_class(dude, REAL_CLASS_PRESERVER) == SOL_SUCCESS;

    sol_item_free_inventory(dude->inv);
    if (dude->inv) {
        memset(dude->inv, 0x0, sizeof(sol_inventory_t));
        for (int i = 0; i < 26; i++) {
            dude->inv[i].anim.spr = SPRITE_ERROR;
        }
    } else {
        sol_inventory_create(&dude->inv);
    }

    if (dude->race == RACE_THRIKREEN) {
        sol_give_ds1_item(dude, SLOT_MISSILE, 48, -1010);
        return SOL_SUCCESS;
    }

    if (has_gladiator || has_fighter || has_ranger || has_thief) {
        sol_give_ds1_item(dude, SLOT_HAND0, 81, -1012); // bone sword
    } else if (has_druid || has_cleric || has_psionicist) {
        sol_give_ds1_item(dude, SLOT_HAND0, 18, -1185); // club
    } else if (has_preserver) {
        sol_give_ds1_item(dude, SLOT_HAND0, 3, -1019); // quaterstaff
    } else {
        return SOL_NO_CLASS; // Dude has no class!
    }

    if (has_gladiator) {
        sol_give_ds1_item(dude, SLOT_HAND1, 18, -1185); // club
    } else if (has_fighter || has_ranger || has_cleric || has_thief) {
        sol_give_ds1_item(dude, SLOT_HAND1, 4, -1020); // leather shield
    }

    if (has_gladiator || has_fighter || has_ranger || has_cleric) {
        sol_give_ds1_item(dude, SLOT_ARM, 7, -1023); // leather arm armor
    }

    if (has_fighter) {
        sol_give_ds1_item(dude, SLOT_LEGS, 8, -1026); // leather leg armor
    }

    if (has_fighter || has_gladiator || has_ranger || has_cleric || has_thief) {
        sol_give_ds1_item(dude, SLOT_CHEST, 6, -1022); // leather chest armor
    }

    if (has_ranger || has_psionicist) {
        sol_give_ds1_item(dude, SLOT_MISSILE, 1, -1017); // Bow
        sol_give_ds1_item(dude, SLOT_AMMO, 62, -1070); // arrows
    } else if (has_druid || has_thief || has_preserver) {
        sol_give_ds1_item(dude, SLOT_MISSILE, 64, -1015); // sling
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_item_get(sol_inventory_t *inv, const int8_t slot, sol_item_t **d) {
    if (!inv || !d) { return SOL_NULL_ARGUMENT; }

    sol_item_t *item = (sol_item_t*) inv;
    if (!inv || slot < 0 || slot >= SLOT_END) { return SOL_OUT_OF_RANGE; }
    if ((item + slot)->name[0] == 0) { return SOL_NOT_FOUND; }

    *d = item + slot;
    return SOL_SUCCESS;
}
