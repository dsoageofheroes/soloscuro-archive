#ifndef LABEL_H
#define LABEL_H

#include <stdint.h>
#include "font.h"
#include "entity.h"

typedef struct sol_label_s {
    char* __m_old_text_do_not_use; // updated by label_pixel_width()
    uint32_t __m_pixel_width_do_not_use; // updated by label_pixel_width()

    int parent;
    int id;
    char* text;
    sol_font_t font;
    int32_t x;
    int32_t y;
    uint8_t visible; // 1 = Label is visible, 0 = Not visible
} sol_label_t;

enum {
    LABEL_NONE = -1,
    LABEL_NAME,
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

typedef enum sol_screen_type_e {
    SCREEN_NONE = -1,
    SCREEN_ADD_LOAD_SAVE,
    SCREEN_INVENTORY,
    SCREEN_NARRATE,
    SCREEN_NEW_CHARACTER,
    SCREEN_POPUP,
    SCREEN_MAIN,
    SCREEN_VIEW_CHARACTER,
    SCREEN_END
} sol_screen_type_t;

extern sol_status_t sol_label_create_group();
extern sol_status_t sol_label_free(sol_label_t *l);
extern sol_status_t sol_label_render(sol_label_t *label);
extern sol_status_t sol_label_set(sol_label_t *label, const char *buf, const int32_t x, const int32_t y);
extern sol_status_t sol_label_set_text(sol_label_t  *label, const char* string);
extern sol_status_t sol_label_create_group();
extern sol_status_t sol_label_group_set_font(sol_font_t font);
extern sol_status_t sol_label_set_group(sol_entity_t *dude, sol_screen_type_t _screen_type);
extern sol_status_t sol_label_set_positions(int32_t oX, int32_t oY, const sol_screen_type_t screen_type);
extern sol_status_t sol_label_render_full(const int16_t offsetx, const int16_t offsety);
extern sol_status_t sol_label_render_stats(const int16_t offsetx, const int16_t offsety);
extern sol_status_t sol_label_render_gra(const int16_t offsetx, const int16_t offsety);
extern sol_status_t sol_label_render_class_and_combat(const int16_t offsetx, const int16_t offsety);
extern sol_status_t sol_label_create_at_pos(int parent, int id, char* text, sol_font_t font, int16_t x, int16_t y, sol_label_t *l);
extern sol_status_t sol_label_point_in(sol_label_t *label, const int32_t x, const int32_t y, sol_label_t **l);
extern sol_status_t sol_label_group_point_in(const int32_t x, const int32_t y, sol_label_t **l);
extern sol_status_t sol_label_pixel_width(sol_label_t  *label, uint32_t *width);

#endif
