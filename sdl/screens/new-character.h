#ifndef NEW_CHARACTER_H
#define NEW_CHARACTER_H

#include <SDL2/SDL.h>
#include "../../src/wizard.h"
#include "../screen-manager.h"
#include "../../src/ssi-object.h"
#include "../../src/entity.h"
#include "../font.h"
#include "../label.h"

enum {
	TEXTBOX_NONE = -1,
	TEXTBOX_NAME
};

entity_t *new_character_get_pc();
psin_t* new_character_get_psin();
ssi_spell_list_t* new_character_get_spell_list();
psionic_list_t* new_character_get_psionic_list();
char* new_character_get_name();

extern sops_t new_character_screen;

#endif
