#ifndef GFF_CHAR_H
#define GFF_CHAR_H

#include <stdint.h>

#include "../src/wizard.h"
#include "../src/entity.h"
#include "ssi-object.h"

typedef struct player_character_s {
    ds_character_t pc;
    psin_t psi;
    ssi_spell_list_t spells;
    psionic_list_t psionics;
} player_character_t;

extern int gff_char_delete(const int id);
extern int gff_char_add_character(entity_t *pc, psin_t *psi, ssi_spell_list_t *spells, psionic_list_t *psionics, char *name);

#endif
