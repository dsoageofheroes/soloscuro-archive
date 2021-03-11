#ifndef DS_PLAYER_H
#define DS_PLAYER_H

#include "ds-object.h"
#include "region.h"
#include "combat.h"
#include "spells.h"

#define MAX_PCS (4)

typedef struct player_pos_s {
    uint16_t map;
    uint16_t xpos, ypos, zpos;
    uint8_t active, ai;
} player_pos_t;

void ds_player_init();
ssi_spell_list_t* ds_player_get_spells(const int slot);
player_pos_t* ds_player_get_pos(const int slot);
ds_inventory_t* ds_player_get_inv(const int slot);
void ds_player_set_item(const int slot, ds1_item_t *item, const int item_slot);
void ds_player_set_active(const int slot);

int ds_player_get_active();

// New Interface
extern int player_exists(const int slot);
extern entity_t* player_get_entity(const int slot);
extern void player_cleanup();
extern entity_t* player_get_active();
extern int player_get_slot(entity_t *entity);
extern void player_free(const int slot);

// This one needs cleanup
extern int ds_player_replace(const int slot, entity_t *ch, psin_t *psi, ssi_spell_list_t *spells,
        psionic_list_t *psionics, ds_inventory_t *inv, char *name);
#endif
