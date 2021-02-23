#ifndef NEW_CHARACTER_H
#define NEW_CHARACTER_H

#include <SDL2/SDL.h>
#include "../../src/spells.h"
#include "../screen-manager.h"
#include "../../src/ds-object.h"

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
