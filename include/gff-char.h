#ifndef GFF_CHAR_H
#define GFF_CHAR_H

#include <stdint.h>

#include "wizard.h"
#include "entity.h"
#include "ssi-object.h"

typedef struct player_character_s {
    ds_character_t pc;
    sol_psin_t psi;
    ssi_spell_list_t spells;
    sol_psionic_list_t psionics;
} player_character_t;

extern int gff_char_delete(const int id);
extern int gff_char_add_character(sol_entity_t *pc, sol_psin_t *psi, ssi_spell_list_t *spells, sol_psionic_list_t *psionics, char *name);

#endif
