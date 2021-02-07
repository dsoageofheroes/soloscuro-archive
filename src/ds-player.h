#ifndef DS_PLAYER_H
#define DS_PLAYER_H

#include "dsl-object.h"
#include "spells.h"

typedef struct player_pos_s {
    uint16_t map;
    uint16_t xpos, ypos, zpos;
} player_pos_t;

void ds_player_init();
int ds_player_exists(const int slot);
int ds_player_load_character_charsave(const int slot, const int res_id);
int ds_player_replace(const int slot, ds_character_t *ch, psin_t *psi, spell_list_t *spells,
        psionic_list_t *psionics, char *name);
ds1_combat_t* ds_player_get_combat(const int slot);
ds_character_t* ds_player_get_char(const int slot);
psin_t* ds_player_get_psi(const int slot);
spell_list_t* ds_player_get_spells(const int slot);
psionic_list_t* ds_player_get_psionics(const int slot);
player_pos_t* ds_player_get_pos(const int slot);
int ds_player_get_active();
void ds_player_set(const int slot);

#endif
