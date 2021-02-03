#include "gff-char.h"
#include "gfftypes.h"
#include "gff-common.h"
#include "gff.h"
#include "rules.h"
#include "dsl-object.h"
#include <string.h>
#include <stdlib.h>

int gff_char_delete(const int id) {
    int16_t entry = 0;
    gff_chunk_header_t chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_CACT, id);

    size_t amt = gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &entry, sizeof(entry));
    if (amt > 1) {
        entry = 0x0000;
        return (gff_write_chunk(CHARSAVE_GFF_INDEX, chunk, (char*) &entry) > 1);
    }

    return 0; // FAILED
}

static void create_combat(ds_character_t *pc, char *name, ds1_combat_t *combat) {
    combat->hp = pc->high_hp;
    combat->psp = pc->base_psp;
    combat->char_index = 0; // TODO: do we need an index?
    combat->id = pc->id;
    combat->ready_item_index = 0; // TODO: do we need this?
    combat->weapon_index = 0; // TODO: do we need this?
    combat->pack_index = 0;  // TODO: do we need this?
    memset(combat->data_block, 0x0, 8);
    combat->special_attack = 0;
    combat->special_defense = 0;
    combat->icon = 0; // TODO: need to fix this eventually...
    combat->ac = dnd2e_get_ac_pc(pc);
    combat->move = dnd2e_get_move_pc(pc);
    combat->status = 0; // clear
    combat->allegiance = pc->allegiance;
    combat->data = 0; // whatever
    combat->thac0 = dnd2e_get_thac0_pc(pc);
    combat->priority = 0; // clear
    combat->flags = 0; // clear
    combat->stats = pc->stats;
    strncpy(combat->name, name, 18);
}

static void write_CHAR_entry(gff_chunk_header_t chunk, ds_character_t *pc, ds1_combat_t *combat) {
    char *buf = malloc(chunk.length);
    rdff_header_t rdff;
    size_t offset = 0;

    memset(buf, 0x0, chunk.length);

    // Load in ds1_combat_t
    rdff.load_action = RDFF_OBJECT;
    rdff.blocknum = 2; // only combat and pc right now. Add items later.
    rdff.type = COMBAT_OBJECT;
    rdff.index = 1; // TODO: TOTAL GUESS
    rdff.from = rdff.index; // Always seems to be the same.
    rdff.len = sizeof(ds1_combat_t);
    //printf("len = %d\n", rdff.len);

    memcpy(buf, &rdff, sizeof(rdff_header_t));
    offset += sizeof(rdff_header_t);
    memcpy(buf + offset, combat, sizeof(ds1_combat_t));
    offset += sizeof(ds1_combat_t);

    // Load in ds_character_t
    rdff.load_action = RDFF_DATA;
    rdff.blocknum = 0; // sub rdffs should be 0.
    rdff.type = 4; // I guess data?
    rdff.index = 1; // TODO: should be same as above
    rdff.from = sizeof(ds_character_t) - 1; // ??, why is font = len here?
    rdff.len = sizeof(ds_character_t) - 1;
    //printf("len = %d\n", rdff.len);

    memcpy(buf + offset, &rdff, sizeof(rdff_header_t));
    offset += sizeof(rdff_header_t);
    memcpy(buf + offset, pc, sizeof(ds_character_t));
    offset += sizeof(ds_character_t);

    gff_write_chunk(CHARSAVE_GFF_INDEX, chunk, buf);

    free(buf);
}

int gff_char_add_character(ds_character_t *pc, psin_t *psi, spell_list_t *spells, psionic_list_t *psionics, char *name) {
    ds1_combat_t combat;
    uint32_t res_ids[1<<10];
    int16_t max_id = 1;
    int16_t id;
    int16_t replace_id = 1;
    int open_slot = -1;

    printf("Add character time!\n");
    int res_max = gff_get_resource_length(CHARSAVE_GFF_INDEX, GFF_CACT);
    gff_get_resource_ids(CHARSAVE_GFF_INDEX, GFF_CACT, res_ids);

    for (int i = res_max - 1; i >= 0; i--) {
        gff_chunk_header_t chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_CACT, res_ids[i]);
        gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &id, sizeof(id));
        if (res_ids[i] > max_id) { max_id = res_ids[i]; }
        if (id <= 0) {
            replace_id = res_ids[i];
            open_slot = i;
        }
    }

    id = max_id + 1;
    printf("open_slot = %d, replace_id = %d, id = %d\n", open_slot, replace_id, id);
    if (open_slot >= -1) {
        create_combat(pc, name, &combat);
        gff_chunk_header_t chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_CHAR, replace_id);
        write_CHAR_entry(chunk, pc, &combat);
        //gff_write_chunk(CHARSAVE_GFF_INDEX, chunk, (char*) pc);
    }
    return 0;
}
