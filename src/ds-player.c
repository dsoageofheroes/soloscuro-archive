#include <string.h>
#include <stdlib.h>
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
    ssi_spell_list_t spells;
    psionic_list_t psionics;
    player_pos_t pos;
    ds_inventory_t inv;
} player_t;

static entity_t *players[MAX_PCS] = {NULL, NULL, NULL, NULL};

#define BUF_MAX (1<<12)

static player_t pc[MAX_PCS];
static int active = 0;

void player_cleanup() {
    for (int i = 0; i < MAX_PCS; i++) {
        entity_free(players[i]);
    }
}

void ds_player_init() {
    memset(pc, 0x0, MAX_PCS * sizeof(player_t));

    // Setup the slots for reading/writing
    for (int i = 0; i < MAX_PCS; i++) {
        if (!players[i]) {
            players[i] = player_get_entity(i);
        }
        ds1_item_t *item = (ds1_item_t*)&(pc[i].inv);
        for (int j = 0; j < 26; j++) {
            item[j].slot = j;
        }
    }
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
    combat->ac = dnd2e_get_ac_pc(pc, NULL);
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

int ds_player_replace(const int slot, ds_character_t *ch, psin_t *psi, ssi_spell_list_t *spells,
        psionic_list_t *psionics, ds_inventory_t *inv, char *name) {
    ds1_combat_t combat;
    if (slot < 0 || slot >= MAX_PCS) { return 0; }

    memcpy(&(pc[slot].ch), ch, sizeof(ds_character_t));
    create_combat(ch, name, &combat);
    memcpy(&(pc[slot].combat), &combat, sizeof(ds1_combat_t));
    memcpy(&(pc[slot].psi), psi, sizeof(psin_t));
    memcpy(&(pc[slot].spells), spells, sizeof(ssi_spell_list_t));
    memcpy(&(pc[slot].psionics), psionics, sizeof(psionic_list_t));
    memcpy(&(pc[slot].inv), inv, sizeof(ds_inventory_t));

    return 1;
}

entity_t* player_get_entity(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    if (!players[slot]) {
        players[slot] = calloc(1, sizeof(entity_t));
        players[slot]->inventory = calloc(1, sizeof(ds_inventory_t));
    }
    return players[slot];
}

ds1_item_t* ds_player_remove_item(const int slot, const int pos) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    if (pos < 0 || pos >= 26) { return NULL; }
    ds1_item_t *ret = malloc(sizeof(ds1_item_t));

    ds1_item_t *item = ((ds1_item_t*)&(pc[slot].inv)) + pos;
    memcpy(ret, item, sizeof(ds1_item_t));
    item->id = 0;
    //((ds1_item_t*)&(pc[slot].inv))[pos].id = 0;

    return ret;;
}

void ds_player_set_item(const int slot, ds1_item_t *item, const int item_slot) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    if (item_slot < 0 || item_slot >= 26) { return; }

    // TODO: check we can place there;

    ds1_item_t *player_item = ((ds1_item_t*)&(pc[slot].inv)) + item_slot;
    memcpy(player_item, item, sizeof(ds1_item_t));
}

int player_exists(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return (player_get_entity(slot)->name != NULL);
}

int ds_player_exists(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return (player_get_entity(slot)->name != NULL);
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

ssi_spell_list_t* ds_player_get_spells(const int slot) {
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

void ds_player_set_active(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    active = slot;
}

int ds_player_is_ai(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return pc[slot].pos.ai;
}

void ds_player_set_ai(const int slot, int on) {
    if (slot < 0 || slot >= MAX_PCS) { return; }
    pc[slot].pos.ai = on;
}

int ds_player_get_ac(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 10; }
    return dnd2e_get_ac_pc(&(pc[slot].ch), &(pc[slot].inv));
}
