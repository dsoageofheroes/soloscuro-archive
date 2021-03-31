#ifndef NEW_CHARACTER_H
#define NEW_CHARACTER_H

#include <SDL2/SDL.h>
#include "../../src/wizard.h"
#include "../screen-manager.h"
#include "../../src/ds-object.h"
#include "../../src/entity.h"
#include "../font.h"
#include "../label.h"

enum {
    SCREEN_NONE = -1,
    SCREEN_ADD_LOAD_SAVE,
    SCREEN_INVENTORY,
    SCREEN_NARRATE,
    SCREEN_NEW_CHARACTER,
    SCREEN_POPUP,
    SCREEN_MAIN,
    SCREEN_VIEW_CHARACTER,
    SCREEN_END
};

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
