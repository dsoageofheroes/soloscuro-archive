#ifndef NEW_CHARACTER_H
#define NEW_CHARACTER_H

#include <SDL2/SDL.h>
#include "../../src/spells.h"
#include "../screen-manager.h"
#include "../../src/ds-object.h"

enum {
    LABEL_NONE = -1,
    LABEL_NAME,
    LABEL_NAME_TEXT,
    LABEL_STR,
    LABEL_STR_VAL,
    LABEL_DEX,
    LABEL_DEX_VAL,
    LABEL_CON,
    LABEL_CON_VAL,
    LABEL_INT,
    LABEL_INT_VAL,
    LABEL_WIS,
    LABEL_WIS_VAL,
    LABEL_CHA,
    LABEL_CHA_VAL,
    LABEL_GENDER_RACE,
    LABEL_ALIGNMENT,
    LABEL_CLASSES,
    LABEL_LEVELS,
    LABEL_EXP_TNL,
    LABEL_AC,
    LABEL_DAM,
    LABEL_HP,
    LABEL_PSP,
    LABEL_END,
};

enum {
	TEXTBOX_NONE,
	TEXTBOX_NAME
};

ds_character_t *new_character_get_pc();
psin_t* new_character_get_psin();
spell_list_t* new_character_get_spell_list();
psionic_list_t* new_character_get_psionic_list();
char* new_character_get_name();

extern sops_t new_character_screen;

#endif
