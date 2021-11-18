#ifndef NEW_CHARACTER_H
#define NEW_CHARACTER_H

#include "window-manager.h"
#include "../src/wizard.h"

enum {
	TEXTBOX_NONE = -1,
	TEXTBOX_NAME
};

entity_t*         sol_new_character_get_pc();
psin_t*           sol_new_character_get_psin();
ssi_spell_list_t* sol_new_character_get_spell_list();
psionic_list_t*   sol_new_character_get_psionic_list();
char*             sol_new_character_get_name();

extern sol_wops_t new_character_window;

#endif
