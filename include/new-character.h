#ifndef NEW_CHARACTER_H
#define NEW_CHARACTER_H

#include "window-manager.h"
#include "wizard.h"

enum {
	TEXTBOX_NONE = -1,
	TEXTBOX_NAME
};

sol_status_t sol_new_character_get_pc(sol_entity_t **);
sol_status_t sol_new_character_get_psin(psin_t **);
sol_status_t sol_new_character_get_spell_list(ssi_spell_list_t **);
sol_status_t sol_new_character_get_psionic_list(psionic_list_t **);
sol_status_t sol_new_character_get_name(char **);

extern sol_wops_t new_character_window;

#endif
