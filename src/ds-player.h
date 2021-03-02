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
int ds_player_exists(const int slot);
int ds_player_replace(const int slot, ds_character_t *ch, psin_t *psi, spell_list_t *spells,
        psionic_list_t *psionics, ds_inventory_t *inv, char *name);
ds1_combat_t* ds_player_get_combat(const int slot);
ds_character_t* ds_player_get_char(const int slot);
psin_t* ds_player_get_psi(const int slot);
spell_list_t* ds_player_get_spells(const int slot);
psionic_list_t* ds_player_get_psionics(const int slot);
player_pos_t* ds_player_get_pos(const int slot);
ds_inventory_t* ds_player_get_inv(const int slot);
region_object_t* ds_player_get_robj(const int slot);
ds1_item_t* ds_player_remove_item(const int slot, const int pos);
void ds_player_set_item(const int slot, ds1_item_t *item, const int item_slot);
int ds_player_exists(const int slot);
int ds_player_get_active();
void ds_player_set_active(const int slot);
int ds_player_is_ai(const int slot);
void ds_player_set_ai(const int slot, int on);
int ds_player_get_ac(const int slot);

int ds_player_get_active();

#endif
