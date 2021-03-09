#ifndef NEW_CHARACTER_H
#define NEW_CHARACTER_H

#include <SDL2/SDL.h>
#include "../../src/spells.h"
#include "../screen-manager.h"
#include "../../src/ds-object.h"
#include "../font.h"

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
    LABEL_GENDER,
    LABEL_RACE,
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
	TEXTBOX_NONE = -1,
	TEXTBOX_NAME
};

typedef struct label_s {
    char* __m_old_text_do_not_use; // updated by label_pixel_width()
    uint32_t __m_pixel_width_do_not_use; // updated by label_pixel_width()

    int parent;
    int id;
    char* text;
    font_t font;
    int16_t x;
    int16_t y;
    uint32_t (*pixel_width)();
    void (*set_text)();
    void (*render)();
    uint8_t visible; // 1 = Label is visible, 0 = Not visible
} label_t;


ds_character_t *new_character_get_pc();
psin_t* new_character_get_psin();
ssi_spell_list_t* new_character_get_spell_list();
psionic_list_t* new_character_get_psionic_list();
char* new_character_get_name();

extern sops_t new_character_screen;

#endif
