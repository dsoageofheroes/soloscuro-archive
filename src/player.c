#include <string.h>
#include <stdlib.h>
#include "gff-common.h"
#include "gfftypes.h"
#include "gff.h"
#include "player.h"
#include "wizard.h"
#include "rules.h"

static entity_t *players[MAX_PCS] = {NULL, NULL, NULL, NULL};

#define BUF_MAX (1<<12)

static int active = 0;

extern void player_cleanup() {
    for (int i = 0; i < MAX_PCS; i++) {
        player_free(i);
    }
}

extern void player_free(const int slot) {
    if (players[slot]) {
        entity_free(players[slot]);
    }
    players[slot] = NULL;
}

extern void sol_player_init() {
    // Setup the slots for reading/writing
    for (int i = 0; i < MAX_PCS; i++) {
        if (!players[i]) {
            players[i] = player_get(i);
        }
    }
}

extern entity_t* player_get(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return NULL; }
    return players[slot];
}

extern void player_set(const int slot, entity_t *dude) {
    if (slot < 0 || slot >= MAX_PCS || !dude) { return; }
    players[slot] = dude;
}

extern int player_exists(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    dude_t *player = player_get(slot);
    return player && (player->name != NULL);
}

extern entity_t* player_get_active() {
    return player_get(active);
}

extern int player_get_slot(entity_t *entity) {
    for (int i = 0; i < MAX_PCS; i++) {
        if (entity == players[i]) { return i; }
    }

    return -1;
}
