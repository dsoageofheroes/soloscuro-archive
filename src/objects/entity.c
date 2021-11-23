#include "combat.h"
#include "item.h"
#include "gpl.h"
#include "entity.h"
#include "gff.h"
#include "port.h"
#include "gfftypes.h"
#include "gff-map.h"
#include "region-manager.h"
#include <string.h>

extern char *strdup(const char *s); // Not in standard.

static void apply_combat(dude_t *dude, ds1_combat_t *combat) {
    // Not used from combat: char_index, id, read_item_index, weapon_index, pack_index, icon
    //                       ac, move, status, thac0, priority, flags
    dude->stats.hp = combat->hp;
    dude->stats.psp = combat->psp;
    dude->stats.special_defense = combat->special_defense;
    dude->stats.attacks[0].special = combat->special_attack;
    dude->stats.base_thac0 = combat->thac0;
    dude->stats.base_ac = combat->ac;
    dude->allegiance = combat->allegiance;
    dude->stats.str = combat->stats.str;
    dude->stats.dex = combat->stats.dex;
    dude->stats.con = combat->stats.con;
    dude->stats.intel = combat->stats.intel;
    dude->stats.wis = combat->stats.wis;
    dude->stats.cha = combat->stats.cha;
    if (dude->name) { free (dude->name); }
    dude->name = strdup(combat->name);
}

static void apply_character(dude_t *dude, ds_character_t *ch) {
    // Not used from ch: id, legal_class, num_blows, spell_group, psi_group, palette.
    dude->class[0].current_xp = ch->current_xp;
    dude->class[0].high_xp = ch->high_xp;
    dude->stats.hp = ch->base_hp;
    dude->stats.high_hp = ch->high_hp;
    dude->stats.high_psp = ch->base_psp;// TODO: run some tests to make sure this is right.
    dude->race = ch->race;
    dude->gender = ch->gender;
    dude->alignment = ch->alignment;
    dude->stats.str = ch->stats.str;
    dude->stats.dex = ch->stats.dex;
    dude->stats.con = ch->stats.con;
    dude->stats.intel = ch->stats.intel;
    dude->stats.wis = ch->stats.wis;
    dude->stats.cha = ch->stats.cha;
    for (int i = 0; i < 3; i++) {
        dude->class[i].class = ch->real_class[i];
        dude->class[i].level = ch->level[i];
        dude->class[i].high_level = ch->high_level[i];
        dude->stats.attacks[i].number = ch->num_attacks[i];
        dude->stats.attacks[i].num_dice = ch->num_dice[i];
        dude->stats.attacks[i].sides = ch->num_sides[i];
        dude->stats.attacks[i].bonus = ch->num_bonuses[i];
    }
    dude->stats.base_ac = ch->base_ac;
    dude->stats.base_move = ch->base_move;
    dude->stats.magic_resistance = ch->magic_resistance;
    dude->stats.saves.paralysis = ch->saving_throw.paral;
    dude->stats.saves.wand = ch->saving_throw.wand;
    dude->stats.saves.petrify = ch->saving_throw.petr;
    dude->stats.saves.breath = ch->saving_throw.breath;
    dude->stats.saves.spell = ch->saving_throw.spell;
    for (int i = 0; i < 3; i++) {
        dude->stats.attacks[i].number = ch->num_attacks[i];
        dude->stats.attacks[i].num_dice = ch->num_dice[i];
        dude->stats.attacks[i].sides = ch->num_sides[i];
        dude->stats.attacks[i].bonus = ch->num_bonuses[i];
    }
    dude->allegiance = ch->allegiance;
    dude->size = ch->size;
    dude->sound_fx = ch->sound_fx;
    dude->attack_sound = ch->attack_sound;
}

entity_t* entity_create_from_objex(const int id) {
    dude_t *dude = malloc(sizeof(dude_t));
    char *buf = NULL;
    rdff_disk_object_t *rdff = NULL;
    disk_object_t dobj;
    size_t rdff_pos = 0;

    if (!dude) { goto error; }
    memset(dude, 0x0, sizeof(dude_t));
    dude->anim.spr = SPRITE_ERROR;

    gff_chunk_header_t chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_RDFF, -1 * id);

    buf = malloc(chunk.length);
    if (!buf) { goto error; }
    if (!gff_read_chunk(OBJEX_GFF_INDEX, &chunk, buf, chunk.length)) {
        printf("unable to get obj from id: %d\n", id);
        goto error;
    }

    chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_OJFF, -1 * id);
    if (!gff_read_chunk(OBJEX_GFF_INDEX, &chunk, &(dobj), sizeof(disk_object_t))) {
        printf("unable to get obj from id: %d\n", id);
        goto error;
    }

    dude->object_flags = dobj.flags;
    dude->sprite.bmp_id = dobj.bmp_id;
    dude->sprite.xoffset = dobj.xoffset;
    dude->sprite.yoffset = dobj.yoffset;
    dude->mapx = dobj.xpos;
    dude->mapy = dobj.ypos;
    dude->mapz = dobj.zpos;
    dude->ds_id = id;

    // This should be done in the region, right now we are just manufacturing an entity.
    // Notes for later:
    //place_region_object(reg, robj, robj->mapx, robj->mapy);
    //dude->scmd = dobj.scmd_id;
    //robj->bmpx = robj->mapx * 16;
    //robj->bmpy = robj->mapy * 16;

    rdff = (rdff_disk_object_t*) (buf + rdff_pos);

    while (rdff->load_action != RDFF_END) {

        switch(rdff->type) {
            case ITEM_OBJECT:
                warn("Item loading from rdff not implemented.\n");
                break;
            case COMBAT_OBJECT:
                dude->anim.scmd = sol_combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
                apply_combat(dude, (ds1_combat_t *) (rdff + 1));
                break;
            case CHAR_OBJECT:
                apply_character(dude, (ds_character_t*) (rdff + 1));
                break;
            case ITEM1R_OBJECT:
                warn("Item1r loading from rdff not implemented.\n");
                break;
            case MINI_OBJECT:
                warn("mini loading from rdff not implemented.\n");
                break;
        }

        rdff_pos += sizeof(rdff_disk_object_t) + rdff->len;
        rdff = (rdff_disk_object_t*) (buf + rdff_pos);
    }

    if (buf) { free(buf); }

    return dude;
error:
    if (dude) { free(dude); }
    if (buf) { free(buf); }
    return NULL;
}

extern entity_t* entity_create_clone(entity_t *clone) {
    entity_t *ret = entity_create_from_objex(-1 * abs(clone->ds_id));

    ret->abilities = clone->abilities;
    ret->mapx = clone->mapx;
    ret->mapy = clone->mapy;
    ret->mapz = clone->mapz;

    return ret;
}

//TODO: Flesh this out. It is currently used for player, but will be use for future entitys;
extern void entity_load_from_object(entity_t *entity, const char *data) {
    item_t *inv = entity->inv;
    effect_node_t *effects = entity->effects;
    memcpy(entity, data, sizeof(entity_t));
    entity->inv = inv;
    entity->effects = effects;

    if (!entity->inv) {
        entity->inv = sol_inventory_create();
    }
    entity->sprite.data = NULL;
}

#define BUF_MAX (1<<14)
void entity_load_from_gff(entity_t *entity, const int gff_idx, const int player, const int res_id) {
    char buf[BUF_MAX];
    rdff_header_t *rdff;
    size_t offset = 0;
    int num_items;
    //ds1_item_t *pc_items = (ds1_item_t*)(entity->inventory);
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_CHAR, res_id);
    if (gff_read_chunk(gff_idx, &chunk, &buf, sizeof(buf)) < 34) { return; }

    rdff = (rdff_disk_object_t*) (buf);
    num_items = rdff->blocknum - 2;
    offset += sizeof(rdff_disk_object_t);
    apply_combat(entity, (ds1_combat_t*)(buf + offset));
    offset += rdff->len;

    rdff = (rdff_disk_object_t*) (buf + offset);
    offset += sizeof(rdff_disk_object_t);
    apply_character(entity, (ds_character_t*)(buf + offset));
    offset += rdff->len;

    if (!entity->inv) {
        entity->inv = sol_inventory_create();
    }

    for (int i = 0; i < num_items; i++) {
        rdff = (rdff_disk_object_t*) (buf + offset);
        offset += sizeof(rdff_disk_object_t);
        int slot = ((ds1_item_t*)(buf + offset))->slot;
        item_convert_from_ds1(entity->inv + slot, (ds1_item_t*)(buf + offset));
        //memcpy(pc_items + slot, buf + offset, sizeof(ds1_item_t));
        offset += rdff->len;
    }

    chunk = gff_find_chunk_header(gff_idx, GFF_PSIN, res_id);
    //if (!gff_read_chunk(gff_idx, &chunk, ds_player_get_psi(player), sizeof(psin_t))) { return; }

    chunk = gff_find_chunk_header(gff_idx, GFF_SPST, res_id);
    //if (!gff_read_chunk(gff_idx, &chunk, ds_player_get_spells(player), sizeof(spell_list_t))) { return; }

    chunk = gff_find_chunk_header(gff_idx, GFF_PSST, res_id);
    //if (!gff_read_chunk(gff_idx, &chunk, ds_player_get_psionics(player), sizeof(psionic_list_t))) { return; }
}

entity_t* entity_create_from_etab(gff_map_object_t *entry_table, uint32_t id) {
    dude_t *dude = calloc(1, sizeof(dude_t));
    const gff_map_object_t *gm = entry_table + id;
    disk_object_t disk_object;

    gff_read_object(gm->index, &disk_object);
    dude->object_flags = disk_object.flags;
    dude->sprite.bmp_id = disk_object.bmp_id;
    dude->anim.spr = SPRITE_ERROR;
    dude->ds_id = gm->index;

    dude->mapx = (gm->xpos) / 16;
    dude->mapy = (gm->ypos) / 16;
    dude->sprite.xoffset = -disk_object.xoffset +
        ((gm->xpos) % 16);
    dude->sprite.yoffset = -disk_object.yoffset  - disk_object.zpos +
        ((gm->ypos)) % 16;
    dude->mapz = gm->zpos;
    dude->sprite.flags = gm->flags;

    return dude;
}

extern void entity_copy_item(entity_t *entity, item_t *item, const size_t slot) {
    if (!entity || !item || slot < 0 || slot >= ITEM_SLOT_MAX) {return;}
    //TODO: Take care of effects!
    memcpy(entity->inv + slot, item, sizeof(item_t));
}

extern void entity_clear_item(entity_t *entity, const size_t slot) {
    memset(entity->inv + slot, 0x0, sizeof(item_t));
    entity->inv[slot].anim.spr = SPRITE_ERROR;
}

extern uint32_t entity_get_total_exp(entity_t *entity) {
    uint32_t total_exp = 0;

    for (int i = 0; i < 3; i++) {
        if (entity->class[i].level > -1) {
            total_exp += entity->class[i].current_xp;
        }
    }

    return total_exp;
}

extern int entity_has_class(const entity_t *dude, const uint16_t class) {
    return dude &&
        (dude->class[0].class == class
          || dude->class[1].class == class
          || dude->class[2].class == class
          );
}

void entity_free(entity_t *dude) {
    if (!dude) { return; }

    entity_animation_list_free(&(dude->actions));
    item_free_inventory(dude->inv);
    port_remove_entity(dude);
    if (dude->name) {
        free(dude->name);
        dude->name = NULL;
    }
    if (dude->inv) {
        // TODO: Once effects are added free the effects as well!
        free(dude->inv);
    }

    free(dude); // Dude has got to be free!
}

// TODO: IMPELMENT!!!!
extern int entity_has_wizard_slot(entity_t *entity, const int slot) {
    if (!entity) { return 0; }
    return 0;
}

extern int entity_has_priest_slot(entity_t *entity, const int slot) {
    if (!entity) { return 0; }
    return 0;
}

extern int entity_take_wizard_slot(entity_t *entity, const int slot) {
    if (!entity) { return 0; }
    return 0;
}

extern int entity_take_priest_slot(entity_t *entity, const int slot) {
    if (!entity) { return 0; }
    return 0;
}

extern int entity_get_wizard_level(entity_t *entity) {
    if (!entity) { return 0; }
    return 1;
}

extern int entity_get_priest_level(entity_t *entity) {
    if (!entity) { return 0; }
    return 1;
}

extern entity_t* entity_create_fake(const int mapx, const int mapy) {
    dude_t *dude = calloc(1, sizeof(dude_t));
    dude->mapx = mapx;
    dude->mapy = mapy;
    dude->anim.spr = SPRITE_ERROR;
    return dude;
}

extern int entity_is_fake(entity_t *entity) {
    return (entity->ds_id == 0);
}
