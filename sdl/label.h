#ifndef LABEL_H
#define LABEL_H

#include <stdint.h>
#include "font.h"
#include "../src/entity.h"

typedef struct label_s {
    char* __m_old_text_do_not_use; // updated by label_pixel_width()
    uint32_t __m_pixel_width_do_not_use; // updated by label_pixel_width()

    int parent;
    int id;
    char* text;
    font_t font;
    int32_t x;
    int32_t y;
    uint8_t visible; // 1 = Label is visible, 0 = Not visible
} label_t;

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

enum screen_type {
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


extern void label_create_group();
extern void label_render(struct label_s* label, SDL_Renderer* renderer);
extern void label_set(label_t *label, const char *buf, const int32_t x, const int32_t y);
extern label_t create_label_at_pos(int parent, int id, char* text, font_t font, int16_t x, int16_t y);
extern label_t create_label(int parent, int id, char* text, font_t font);
extern void label_set_text(struct label_s* label, const char* string);
extern uint32_t label_pixel_width(struct label_s* label);
extern label_t* label_point_in(label_t *label, const int32_t x, const int32_t y);

extern void label_create_group();
extern void label_group_set_font(font_t font);
extern void label_set_group(entity_t *dude, enum screen_type _screen_type);
extern void label_set_positions(int32_t oX, int32_t oY, const enum screen_type screen_type);
extern void label_render_full(const int16_t offsetx, const int16_t offsety);
extern void label_render_stats(const int16_t offsetx, const int16_t offsety);
extern void label_render_gra(const int16_t offsetx, const int16_t offsety);
extern void label_render_class_and_combat(const int16_t offsetx, const int16_t offsety);
extern label_t* label_group_point_in(const int32_t x, const int32_t y);

#endif
