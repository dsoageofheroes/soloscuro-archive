#include "arbiter.h"
#include "combat.h"
#include "alignment.h"
#include "item.h"
#include "gpl.h"
#include "entity.h"
#include "innate.h"
#include "gff.h"
#include "rules.h"
#include "port.h"
#include "gfftypes.h"
#include "gff-map.h"
#include "region-manager.h"
#include <string.h>

extern char *strdup(const char *s); // Not in standard.

static void apply_combat(sol_dude_t *dude, ds1_combat_t *combat) {
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

static void apply_character(sol_dude_t *dude, ds_character_t *ch) {
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

extern sol_status_t sol_entity_create_default_human(sol_entity_t **ret) {
    sol_entity_t *dude = malloc(sizeof(sol_entity_t));
    if (!dude) { return SOL_MEMORY_ERROR; }

    memset(dude, 0x0, sizeof(sol_entity_t));
    dude->stats.hp = 4;
    dude->stats.high_hp = 4;
    dude->race = RACE_HUMAN;
    dude->gender = GENDER_MALE;
    dude->alignment = TRUE_NEUTRAL;
    dude->stats.str = dude->stats.dex = dude->stats.con = dude->stats.intel = dude->stats.wis = dude->stats.cha = 10;
    dude->stats.base_ac = 10;
    dude->stats.base_move = 12;
    dude->stats.saves.paralysis = 20;
    dude->stats.saves.wand = 20;
    dude->stats.saves.petrify = 20;
    dude->stats.saves.breath = 20;
    dude->stats.saves.spell = 20;
    dude->stats.attacks[0].number = 1;
    dude->stats.attacks[0].num_dice = 1;
    dude->stats.attacks[0].sides = 4;
    dude->stats.attacks[1].number = 1;
    dude->stats.attacks[1].num_dice = 1;
    dude->stats.attacks[1].sides = 4;
    dude->class[0].level = -1;
    dude->class[1].level = -1;
    dude->class[2].level = -1;
    dude->anim.spr = SPRITE_ERROR;
    debug("Need to set default size!\n");
    dude->size = 0;
    sol_inventory_create(&dude->inv);

    *ret = dude;
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_create(const int add_inventory, sol_entity_t **ret) {
    sol_entity_t *dude = malloc(sizeof(sol_entity_t));
    if (!dude) { return SOL_MEMORY_ERROR; }
    memset(dude, 0x0, sizeof(sol_entity_t));
    dude->stats.str = dude->stats.dex = dude->stats.con = dude->stats.intel = dude->stats.wis = dude->stats.cha = 10;
    dude->stats.base_ac = 10;
    dude->stats.base_move = 12;
    dude->stats.saves.paralysis = 20;
    dude->stats.saves.wand = 20;
    dude->stats.saves.petrify = 20;
    dude->stats.saves.breath = 20;
    dude->stats.saves.spell = 20;
    dude->anim.spr = SPRITE_ERROR;
    if (add_inventory) {
        sol_inventory_create(&dude->inv);
    }

    *ret = dude;
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_create_from_objex(const int id, sol_entity_t **ret) {
    sol_dude_t *dude = malloc(sizeof(sol_dude_t));
    char *buf = NULL;
    rdff_disk_object_t *rdff = NULL;
    disk_object_t dobj;
    size_t rdff_pos = 0;

    if (!dude) { return SOL_MEMORY_ERROR; }
    memset(dude, 0x0, sizeof(sol_dude_t));
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
    dude->anim.bmp_id = dobj.bmp_id;
    dude->anim.xoffset = dobj.xoffset;
    dude->anim.yoffset = dobj.yoffset;
    dude->mapx = dobj.xpos;
    dude->mapy = dobj.ypos;
    dude->mapz = dobj.zpos;
    dude->ds_id = id;

    rdff = (rdff_disk_object_t*) (buf + rdff_pos);

    while (rdff->load_action != RDFF_END) {
        switch(rdff->type) {
            case ITEM_OBJECT:
                warn("Item loading from rdff not implemented.\n");
                break;
            case COMBAT_OBJECT:
                sol_combat_set_scmd(dude, COMBAT_SCMD_STAND_DOWN);
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

    *ret = dude;
    return SOL_SUCCESS;
error:
    if (dude) { free(dude); }
    if (buf) { free(buf); }
    return SOL_GFF_ERROR;
}

extern sol_status_t sol_entity_create_clone(sol_entity_t *clone, sol_entity_t **dude) {
    sol_status_t status;
    sol_entity_t *ret;
    if ((status = sol_entity_create_from_objex(-1 * abs(clone->ds_id), &ret)) != SOL_SUCCESS) {
        return status;
    }
    if (!ret) { return SOL_NULL_ARGUMENT; }

    ret->abilities = clone->abilities;
    ret->mapx = clone->mapx;
    ret->mapy = clone->mapy;
    ret->mapz = clone->mapz;

    *dude = ret;
    return SOL_SUCCESS;
}

//TODO: Flesh this out. It is currently used for player, but will be use for future entitys;
extern sol_status_t sol_entity_load_from_object(sol_entity_t *entity, const char *data) {
    sol_item_t *inv = entity->inv;
    sol_effect_node_t *effects = entity->effects;
    memcpy(entity, data, sizeof(sol_entity_t));
    entity->inv = inv;
    entity->effects = effects;

    if (!entity->inv) {
        sol_inventory_create(&entity->inv);
    }

    return SOL_SUCCESS;
}

#define BUF_MAX (1<<14)
extern sol_status_t sol_entity_load_from_gff(sol_entity_t *entity, const int gff_idx, const int player, const int res_id) {
    char buf[BUF_MAX];
    rdff_header_t *rdff;
    size_t offset = 0;
    int num_items;
    //ds1_item_t *pc_items = (ds1_item_t*)(entity->inventory);
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_CHAR, res_id);
    if (gff_read_chunk(gff_idx, &chunk, &buf, sizeof(buf)) < 34) { return SOL_GFF_ERROR; }

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
        sol_inventory_create(&entity->inv);
    }

    for (int i = 0; i < num_items; i++) {
        rdff = (rdff_disk_object_t*) (buf + offset);
        offset += sizeof(rdff_disk_object_t);
        int slot = ((ds1_item_t*)(buf + offset))->slot;
        sol_item_convert_from_ds1(entity->inv + slot, (ds1_item_t*)(buf + offset));
        //memcpy(pc_items + slot, buf + offset, sizeof(ds1_item_t));
        offset += rdff->len;
    }

    chunk = gff_find_chunk_header(gff_idx, GFF_PSIN, res_id);
    //if (!gff_read_chunk(gff_idx, &chunk, ds_player_get_psi(player), sizeof(psin_t))) { return; }

    chunk = gff_find_chunk_header(gff_idx, GFF_SPST, res_id);
    //if (!gff_read_chunk(gff_idx, &chunk, ds_player_get_spells(player), sizeof(spell_list_t))) { return; }

    chunk = gff_find_chunk_header(gff_idx, GFF_PSST, res_id);
    //if (!gff_read_chunk(gff_idx, &chunk, ds_player_get_psionics(player), sizeof(psionic_list_t))) { return; }
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_create_from_etab(gff_map_object_t *entry_table, uint32_t id, sol_entity_t **ret) {
    sol_dude_t *dude = calloc(1, sizeof(sol_dude_t));
    if (!dude) { return SOL_MEMORY_ERROR; }
    const gff_map_object_t *gm = entry_table + id;
    disk_object_t disk_object;

    gff_read_object(gm->index, &disk_object);
    dude->object_flags = disk_object.flags;
    dude->anim.bmp_id = disk_object.bmp_id;
    dude->anim.spr = SPRITE_ERROR;
    dude->ds_id = gm->index;

    dude->mapx = (gm->xpos) / 16;
    dude->mapy = (gm->ypos) / 16;
    dude->anim.xoffset = -disk_object.xoffset + ((gm->xpos) % 16);
    dude->anim.yoffset = -disk_object.yoffset  - disk_object.zpos + ((gm->ypos)) % 16;
    dude->mapz = gm->zpos;
    dude->map_flags = gm->flags;

    *ret = dude;
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_copy_item(sol_entity_t *entity, sol_item_t *item, const size_t slot) {
    if (!entity || !item) { return SOL_NULL_ARGUMENT; }
    if (slot < 0 || slot >= ITEM_SLOT_MAX) {return SOL_OUT_OF_RANGE;}
    //TODO: Take care of effects!
    memcpy(entity->inv + slot, item, sizeof(sol_item_t));

    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_clear_item(sol_entity_t *entity, const size_t slot) {
    if (!entity) { return SOL_NULL_ARGUMENT; }
    if (slot < 0 || slot >= ITEM_SLOT_MAX) {return SOL_OUT_OF_RANGE;}
    memset(entity->inv + slot, 0x0, sizeof(sol_item_t));
    entity->inv[slot].anim.spr = SPRITE_ERROR;
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_get_total_exp(sol_entity_t *entity, int32_t *exp) {
    if (!entity) { return SOL_NULL_ARGUMENT; }
    uint32_t total_exp = 0;

    for (int i = 0; i < 3; i++) {
        if (entity->class[i].level > -1) {
            total_exp += entity->class[i].current_xp;
        }
    }

    *exp = total_exp;
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_has_class(const sol_entity_t *dude, const uint16_t class) {
    if (!dude) { return SOL_NULL_ARGUMENT; }
    return (dude &&
        (dude->class[0].class == class
          || dude->class[1].class == class
          || dude->class[2].class == class
          )) ? SOL_SUCCESS : SOL_NO_CLASS;
}

extern sol_status_t sol_entity_gui_free(sol_entity_t *entity) {
    if (!entity) { return SOL_NULL_ARGUMENT; }

    if (entity->anim.spr != SPRITE_ERROR) {
        sol_status_check(sol_sprite_free(entity->anim.spr), "Unable to free sprite.");
        entity->anim.spr = SPRITE_ERROR;
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_free(sol_entity_t *dude) {
    if (!dude) { return SOL_NULL_ARGUMENT; }
    sol_region_t *reg;
    sol_entity_list_node_t *node;

    sol_region_manager_get_current(&reg);
    sol_entity_animation_list_free(&(dude->actions));
    sol_item_free_inventory(dude->inv);

    if (reg) {
        sol_entity_animation_list_remove_references(&reg->actions, dude);
        if (sol_entity_list_find(reg->entities, dude, &node) == SOL_SUCCESS) {
            sol_entity_list_remove(reg->entities, node);
        }
    }

    sol_entity_gui_free(dude);

    if (dude->name) {
        free(dude->name);
        dude->name = NULL;
    }
    if (dude->inv) {
        // TODO: Once effects are added free the effects as well!
        free(dude->inv);
    }

    free(dude); // Dude has got to be free!
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_update_scmd(sol_entity_t *entity) {
    return sol_sprite_set_frame(entity->anim.spr, entity->anim.scmd[entity->anim.pos].bmp_idx);
}

extern sol_status_t sol_entity_attempt_move(sol_dude_t *dude, const int xdiff, const int ydiff, const int speed) {
    sol_combat_region_t          *cr = NULL;
    enum sol_entity_action_e  action;
    sol_region_t             *region;
    sol_entity_t             *target;
    sol_entity_list_node_t   *node;
    sol_status_t              status;

    if (!dude) { return SOL_NULL_ARGUMENT; }
    action =
          (xdiff == 1 && ydiff == 1) ? EA_WALK_DOWNRIGHT
        : (xdiff == 1 && ydiff == -1) ? EA_WALK_UPRIGHT
        : (xdiff == -1 && ydiff == -1) ? EA_WALK_UPLEFT
        : (xdiff == -1 && ydiff == 1) ? EA_WALK_DOWNLEFT
        : (xdiff == 1) ? EA_WALK_RIGHT
        : (xdiff == -1) ? EA_WALK_LEFT
        : (ydiff == 1) ? EA_WALK_DOWN
        : (ydiff == -1) ? EA_WALK_UP
        : EA_NONE;

    // If we are in combat and it isn't our turn, do nothing.
    sol_region_manager_get_current(&region);
    status = sol_arbiter_combat_region(region, &cr);
    if (cr && (sol_combat_get_current(cr, &target) != SOL_SUCCESS || target != dude)) {
        return SOL_NOT_TURN;
    }

    printf("%d, %d, %d\n", xdiff, ydiff, sol_region_is_block(region, dude->mapy + ydiff, dude->mapx + xdiff));
    printf("%d, %d, %d\n", xdiff, ydiff, sol_region_location_blocked(region, dude->mapy + ydiff, dude->mapx + xdiff));
    if (sol_region_is_block(region, dude->mapy + ydiff, dude->mapx + xdiff) == SOL_SUCCESS
        || sol_region_location_blocked(region, dude->mapx + xdiff, dude->mapy + ydiff) == SOL_SUCCESS) {

        if (cr && (xdiff != 0 || ydiff != 0)) {
            if ((status = sol_combat_attempt_action(cr, dude)) != SOL_SUCCESS) { return status; }
            sol_region_find_entity_by_location(
                    region, dude->mapx + xdiff, dude->mapy + ydiff, &target);
            if (target) {
                sol_combat_add_attack_animation(region, dude,
                    target, NULL, EA_MELEE);
            }
            // We did a combat action.
            return SOL_SUCCESS;
        }
        return SOL_BLOCKED;
    }

    // We aren't moving...
    if (action == EA_NONE) {
        dude->anim.movex = dude->anim.movey = 0.0;
        if (dude->actions.head == NULL) {
            sol_entity_animation_list_add(&(dude->actions), EA_NONE, dude, NULL, NULL, 1);
        }
        return SOL_SUCCESS;
    }

    if (sol_entity_animation_list_empty(&(dude->actions)) != SOL_SUCCESS) {
        // We need to wait for the rest of my actions.
        return SOL_WAIT_ACTIONS;
    }

    if (cr && (status = sol_combat_attempt_action(cr, dude)) != SOL_SUCCESS) { return status; }

    sol_status_check(sol_entity_animation_list_add_speed(&(dude->actions), action, dude, NULL,
            NULL, settings_ticks_per_move(), speed, 0), "failed to add animation.");

    //dude->mapx += xdiff;
    //dude->mapy += ydiff;
    //dude->anim.destx += (xdiff * 32);
    //dude->anim.desty += (ydiff * 32);

    if (region) {
        if ((status = sol_entity_list_find(region->entities, dude, &node)) != SOL_SUCCESS) {
            return status;
        }
    printf("HERE\n");
        return sol_animate_shift_entity(region->entities, node);
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_has_wizard_slot(sol_entity_t *entity, const int level) {
    if (!entity) { return SOL_NULL_ARGUMENT; }
    if (level < 0 || level > 10) { return SOL_OUT_OF_RANGE; }
    return entity->stats.wizard[level].amt ? SOL_SUCCESS : SOL_NO_CLASS;
}

extern sol_status_t sol_entity_has_priest_slot(sol_entity_t *entity, const int level) {
    if (!entity) { return SOL_NULL_ARGUMENT; }
    if (level < 0 || level > 10) { return SOL_OUT_OF_RANGE; }
    return entity->stats.priest[level].amt ? SOL_SUCCESS : SOL_NO_CLASS;
}

extern sol_status_t sol_entity_take_wizard_slot(sol_entity_t *entity, const int level) {
    if (!entity) { return SOL_NULL_ARGUMENT; }
    if (level < 0 || level > 10) { return SOL_OUT_OF_RANGE; }
    if (entity->stats.wizard[level].amt < 1) { return SOL_NO_SLOTS_LEFT; }
    entity->stats.wizard[level].amt--;
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_take_priest_slot(sol_entity_t *entity, const int level) {
    if (!entity) { return SOL_NULL_ARGUMENT; }
    if (level < 0 || level > 10) { return SOL_OUT_OF_RANGE; }
    if (entity->stats.priest[level].amt < 1) { return SOL_NO_SLOTS_LEFT; }
    entity->stats.wizard[level].amt--;
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_get_level(sol_entity_t *entity, const int class, uint8_t *level) {
    sol_status_t status = SOL_NOT_FOUND;
    if (!entity) { return SOL_NULL_ARGUMENT; }
    if (class < 0 || class > 20) { return SOL_OUT_OF_RANGE; }
    if (entity->class[0].class == class) { *level = entity->class[0].level; status = SOL_SUCCESS; }
    if (entity->class[1].class == class) { *level = entity->class[1].level; status = SOL_SUCCESS; }
    if (entity->class[2].class == class) { *level = entity->class[2].level; status = SOL_SUCCESS; }
    return status;
}

extern sol_status_t sol_entity_get_wizard_level(sol_entity_t *entity, uint8_t *level) {
    uint8_t max = 0, lvl;
    sol_status_t status, ret_status = SOL_NOT_FOUND;

    if (!entity) { return SOL_NULL_ARGUMENT; }

    if ((status = sol_entity_get_level(entity, REAL_CLASS_PRESERVER, &lvl)) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        ret_status = SOL_SUCCESS;
    }
    if ((status = sol_entity_get_level(entity, REAL_CLASS_DEFILER, &lvl)) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        ret_status = SOL_SUCCESS;
    }

    *level = max;
    return ret_status;
}

extern sol_status_t sol_entity_get_priest_level(sol_entity_t *entity, uint8_t *level) {
    uint8_t max = 0, lvl;
    sol_status_t status = SOL_NOT_FOUND;
    if (sol_entity_get_level(entity, REAL_CLASS_AIR_DRUID, &lvl) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        status = SOL_SUCCESS;
    }
    if (sol_entity_get_level(entity, REAL_CLASS_EARTH_DRUID, &lvl) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        status = SOL_SUCCESS;
    }
    if (sol_entity_get_level(entity, REAL_CLASS_FIRE_DRUID, &lvl) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        status = SOL_SUCCESS;
    }
    if (sol_entity_get_level(entity, REAL_CLASS_WATER_DRUID, &lvl) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        status = SOL_SUCCESS;
    }
    if (sol_entity_get_level(entity, REAL_CLASS_AIR_CLERIC, &lvl) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        status = SOL_SUCCESS;
    }
    if (sol_entity_get_level(entity, REAL_CLASS_EARTH_CLERIC, &lvl) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        status = SOL_SUCCESS;
    }
    if (sol_entity_get_level(entity, REAL_CLASS_FIRE_CLERIC, &lvl) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        status = SOL_SUCCESS;
    }
    if (sol_entity_get_level(entity, REAL_CLASS_WATER_CLERIC, &lvl) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        status = SOL_SUCCESS;
    }
    *level = max;
    return status;
}

extern sol_status_t sol_entity_get_ranger_level(sol_entity_t *entity, uint8_t *level) {
    uint8_t max = 0, lvl;
    sol_status_t status = SOL_NOT_FOUND;

    if(sol_entity_get_level(entity, REAL_CLASS_AIR_RANGER, &lvl) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        status = SOL_SUCCESS;
    }
    if(sol_entity_get_level(entity, REAL_CLASS_EARTH_RANGER, &lvl) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        status = SOL_SUCCESS;
    }
    if(sol_entity_get_level(entity, REAL_CLASS_FIRE_RANGER, &lvl) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        status = SOL_SUCCESS;
    }
    if(sol_entity_get_level(entity, REAL_CLASS_WATER_RANGER, &lvl) == SOL_SUCCESS) {
        max = lvl > max ? lvl : max;
        status = SOL_SUCCESS;
    }
    *level = max;
    return status;
}

extern sol_status_t sol_entity_create_fake(const int mapx, const int mapy, sol_entity_t **ret) {
    sol_dude_t *dude = calloc(1, sizeof(sol_dude_t));
    if (!dude) { return SOL_MEMORY_ERROR; }
    dude->mapx = mapx;
    dude->mapy = mapy;
    dude->anim.spr = SPRITE_ERROR;
    *ret = dude;
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_is_fake(sol_entity_t *entity) {
    return (entity->ds_id == 0) ? SOL_SUCCESS : SOL_IS_REAL;
}

extern sol_status_t sol_entity_distance(const sol_entity_t *source, const sol_entity_t *dest, int16_t *dist) {
    if (!source || !dest) { return SOL_NULL_ARGUMENT; }

    int dx = abs(source->mapx - dest->mapx);
    int dy = abs(source->mapy - dest->mapy);

    *dist = dx > dy ? dx : dy;
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_go(sol_entity_t *dude, const uint16_t x, uint16_t y) {
    if (!dude) { return SOL_NULL_ARGUMENT; }

    dude->abilities.must_go = 1;
    dude->abilities.args.pos.x = x;
    dude->abilities.args.pos.y = y;
    debug("%s given go order to (%d, %d)\n", dude->name, x, y);

    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_debug(sol_entity_t *dude) {
    if (!dude) { return SOL_NULL_ARGUMENT; }
    printf("entity ('%s): \n", dude->name);
    printf("    .ds_id = %d\n", dude->ds_id);
    printf("    .size = %d\n", dude->size);
    printf("    .race = %d\n", dude->race);
    printf("    .gender = %d\n", dude->gender);
    printf("    .alignment = %d\n", dude->alignment);
    printf("    .allegiance = %d\n", dude->allegiance);
    printf("    .object_flags = %x\n", dude->object_flags);
    printf("    .direction = %d\n", dude->direction);
    printf("    .region = %d\n", dude->region);
    printf("    pos = (%d, %d, %d)\n", dude->mapx, dude->mapy, dude->mapz);
    printf("    .sound_fx = %d\n", dude->sound_fx);
    printf("    .attack_sound = %d\n", dude->attack_sound);
    printf("    .combat_status = %d\n", dude->combat_status);
    printf("    stats = (%d, %d %d, %d, %d, %d)\n", dude->stats.str, dude->stats.dex, dude->stats.con, dude->stats.intel, dude->stats.wis, dude->stats.cha);
    printf("    .class[0] = (current_xp = %d, high_xp = %d, class = %d, level = %d, high_level = %d\n",
            dude->class[0].current_xp,
            dude->class[0].high_xp,
            dude->class[0].class,
            dude->class[0].level,
            dude->class[0].high_level);
    printf("    .class[1] = (current_xp = %d, high_xp = %d, class = %d, level = %d, high_level = %d\n",
            dude->class[1].current_xp,
            dude->class[1].high_xp,
            dude->class[1].class,
            dude->class[1].level,
            dude->class[1].high_level);
    printf("    .class[0] = (current_xp = %d, high_xp = %d, class = %d, level = %d, high_level = %d\n",
            dude->class[2].current_xp,
            dude->class[2].high_xp,
            dude->class[2].class,
            dude->class[2].level,
            dude->class[2].high_level);
    printf("    .anim = (spr = %d, scmd = %p)\n", dude->anim.spr, dude->anim.scmd);
    return SOL_SUCCESS;
}
