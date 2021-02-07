#include <string.h>
#include "gff-common.h"
#include "gfftypes.h"
#include "gff.h"
#include "ds-player.h"
#include "ds-item.h"
#include "spells.h"
#include "rules.h"

typedef struct player_s {
    ds_character_t ch;
    ds1_combat_t combat;
    psin_t psi;
    spell_list_t spells;
    psionic_list_t psionics;
    player_pos_t pos;
    ds_inventory_t inv;
} player_t;

static int active = 0;

#define MAX_PCS (4)
#define BUF_MAX (1<<12)

static player_t pc[MAX_PCS];

void ds_player_init() {
    memset(pc, 0x0, MAX_PCS * sizeof(player_t));
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
    strncpy(combat->name, name, 16);
    combat->name[15] = '\0';
}

int ds_player_replace(const int slot, ds_character_t *ch, psin_t *psi, spell_list_t *spells,
        psionic_list_t *psionics, ds_inventory_t *inv, char *name) {
    ds1_combat_t combat;
    if (slot < 0 || slot >= MAX_PCS) { return 0; }

    memcpy(&(pc[slot].ch), ch, sizeof(ds_character_t));
    create_combat(ch, name, &combat);
    memcpy(&(pc[slot].combat), &combat, sizeof(ds1_combat_t));
    memcpy(&(pc[slot].psi), psi, sizeof(psin_t));
    memcpy(&(pc[slot].spells), spells, sizeof(spell_list_t));
    memcpy(&(pc[slot].psionics), psionics, sizeof(psionic_list_t));
    memcpy(&(pc[slot].inv), inv, sizeof(ds_inventory_t));

    return 1;
}

int ds_player_load_character_charsave(const int slot, const int res_id) {
    char buf[BUF_MAX];
    rdff_header_t *rdff;
    size_t offset = 0;

    if (slot < 0 || slot >= MAX_PCS) { return 0; }

    gff_chunk_header_t chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_CHAR, res_id);
    if (gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &buf, sizeof(buf)) < 34) { return 0; }

    rdff = (rdff_disk_object_t*) (buf);
    memcpy(&(pc[slot].combat), buf + 10, sizeof(ds1_combat_t));
    offset += 10 + rdff->len;

    memcpy(&(pc[slot].ch), buf + 0x4E, sizeof(ds_character_t));

    chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_PSIN, res_id);
    if (!gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &(pc[slot].psi), sizeof(psin_t))) { return 0; }

    chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_SPST, res_id);
    if (!gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &(pc[slot].spells), sizeof(spell_list_t))) { return 0;}

    chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_PSST, res_id);
    if (!gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &(pc[slot].psionics), sizeof(psionic_list_t))) { return 0;}

    return 1;
}

int ds_player_exists(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return pc[slot].combat.name[0];
}

ds1_combat_t* ds_player_get_combat(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    return &(pc[slot].combat);
}

ds_character_t* ds_player_get_char(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    return &(pc[slot].ch);
}

psin_t* ds_player_get_psi(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    return &(pc[slot].psi);
}

spell_list_t* ds_player_get_spells(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    return &(pc[slot].spells);
}

psionic_list_t* ds_player_get_psionics(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    return &(pc[slot].psionics);
}

player_pos_t* ds_player_get_pos(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    return &(pc[slot].pos);
}

ds_inventory_t* ds_player_get_inv(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    return &(pc[slot].inv);
}

int ds_player_get_active() {
    return active;
}

void ds_player_set(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    active = slot;
}
