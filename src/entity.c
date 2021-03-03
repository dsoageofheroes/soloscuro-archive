#include "dsl.h"
#include "entity.h"
#include "gff.h"
#include "gfftypes.h"
#include "gff-map.h"
#include <stdlib.h>
#include <string.h>

extern char *strdup(const char *s); // Not in standard.

static void apply_combat(dude_t *dude, ds1_combat_t *combat) {
    // Not used from combat: char_index, id, read_item_index, weapon_index, pack_index, icon
    //                       ac, move, status, thac0, priority.
    dude->stats.hp = combat->hp;
    dude->stats.psp = combat->psp;
    dude->stats.special_defense = combat->special_defense;
    dude->stats.attacks[0].special = combat->special_attack;
    dude->allegiance = combat->allegiance;
    dude->combat_flags = combat->flags;
    dude->stats.str = combat->stats.str;
    dude->stats.dex = combat->stats.dex;
    dude->stats.con = combat->stats.con;
    dude->stats.intel = combat->stats.intel;
    dude->stats.wis = combat->stats.wis;
    dude->stats.cha = combat->stats.cha;
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

    dude->combat_flags = dobj.flags; // gets overwritten by combat.
    dude->sprite.bmp_id = dobj.bmp_id;
    dude->sprite.xoffset = dobj.xoffset;
    dude->sprite.yoffset = dobj.yoffset;
    dude->mapx = dobj.xpos;
    dude->mapy = dobj.ypos;
    dude->mapz = dobj.zpos;

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
                dude->sprite.scmd = combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
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

entity_t* entity_create_from_etab(gff_map_object_t *entry_table, uint32_t id) {
    dude_t *dude = calloc(1, sizeof(dude_t));
    const gff_map_object_t *gm = entry_table + id;
    disk_object_t disk_object;

    gff_read_object(gm->index, &disk_object);
    //dude->disk_idx = gm->index;
    dude->combat_flags = disk_object.flags;
    //dude->gt_idx = disk_object.object_index;
    dude->sprite.bmp_id = disk_object.bmp_id;
    //dude->bmpx = gm->xpos - disk_object.xoffset;
    //dude->bmpy = gm->ypos - disk_object.yoffset - disk_object.zpos;
    dude->sprite.xoffset = disk_object.xoffset;
    dude->sprite.yoffset = disk_object.yoffset;
    dude->mapx = (gm->xpos + disk_object.xoffset) / 16;
    dude->mapy = (gm->ypos + disk_object.yoffset - disk_object.zpos) / 16;
    dude->mapz = gm->zpos;
    //dude->entry_id = id;
    //dude->combat_id = COMBAT_ERROR;
    dude->sprite.flags = gm->flags;
    //dude->obj_id = abs(gm->index);
    //printf("->%d, %d\n", dsl_object->mapx, dsl_object->mapy);

    return dude;
}

void entity_free(entity_t *dude) {
    if (dude->name) {
        free(dude->name);
        dude->name = NULL;
    }

    free(dude); // Dude has got to be free!
}
