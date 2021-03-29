#include <string.h>
#include <stdlib.h>
#include "gff-common.h"
#include "gfftypes.h"
#include "gff.h"
#include "ds-player.h"
#include "ds-item.h"
#include "wizard.h"
#include "rules.h"

static entity_t *players[MAX_PCS] = {NULL, NULL, NULL, NULL};

#define BUF_MAX (1<<12)

static int active = 0;

void player_cleanup() {
    for (int i = 0; i < MAX_PCS; i++) {
        entity_free(players[i]);
    }
}

extern void player_free(const int slot) {
    entity_free(players[slot]);
    players[slot] = NULL;
}

void ds_player_init() {
    // Setup the slots for reading/writing
    for (int i = 0; i < MAX_PCS; i++) {
        if (!players[i]) {
            players[i] = player_get_entity(i);
        }
    }
}

/* Can we delete this?
static void create_combat(entity_t *pc, char *name, ds1_combat_t *combat) {
    combat->hp = pc->stats.hp;
    combat->psp = pc->stats.psp;
    combat->char_index = 0; // TODO: do we need an index?
    combat->id = pc->ds_id;
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
    combat->stats.str = pc->stats.str;
    combat->stats.dex = pc->stats.dex;
    combat->stats.con = pc->stats.con;
    combat->stats.intel = pc->stats.intel;
    combat->stats.wis = pc->stats.wis;
    combat->stats.cha = pc->stats.cha;
    strncpy(combat->name, name, 16);
    combat->name[15] = '\0';
}
*/

entity_t* player_get_entity(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    if (!players[slot]) {
        players[slot] = calloc(1, sizeof(entity_t));
        for (int i = 0; i < 3; i++) {
            players[slot]->class[i].class = -1;
            players[slot]->class[i].level = -1;
        }
        players[slot]->inventory = calloc(1, sizeof(ds_inventory_t));
    }
    return players[slot];
}

extern int player_exists(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return (player_get_entity(slot)->name != NULL);
}

extern entity_t* player_get_active() {
    return player_get_entity(active);
}

extern int player_get_slot(entity_t *entity) {
    for (int i = 0; i < MAX_PCS; i++) {
        if (entity == players[i]) { return i; }
    }

    return -1;
}
