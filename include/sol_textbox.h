#ifndef SOL_TEXTBOX_h
#define SOL_TEXTBOX_h

#include <stdint.h>
#include <stdlib.h>

#include "sprite.h"

typedef struct sol_textbox_s {
    char *text;
    sol_sprite_t text_cursor;
    size_t text_len, text_pos;
    uint16_t count_down; // for cursor
    uint16_t xpos, ypos;
    uint16_t cursor_pos;
    int16_t cursor_countdown;
    uint8_t in_focus;
} sol_textbox_t;

extern sol_status_t sol_textbox_render(sol_textbox_t *tb);
extern sol_status_t sol_textbox_set_focus(sol_textbox_t *tb, const int focus);
extern sol_status_t sol_textbox_free(sol_textbox_t *tb);
extern sol_status_t sol_textbox_set_text(sol_textbox_t *tb, char *text);
extern sol_status_t sol_textbox_set_current(sol_textbox_t *tb);
extern sol_status_t sol_textbox_is_in(sol_textbox_t *tb, const uint16_t x, const uint16_t y);
extern sol_status_t sol_textbox_create(const size_t len, const uint16_t _xpos, const uint16_t _ypos, sol_textbox_t **tb);

#endif
