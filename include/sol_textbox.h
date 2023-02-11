#ifndef SOL_TEXTBOX_h
#define SOL_TEXTBOX_h

#include <stdint.h>
#include <stdlib.h>

#include "sprite.h"

typedef struct textbox_s {
    char *text;
    sol_sprite_t text_cursor;
    size_t text_len, text_pos;
    uint16_t count_down; // for cursor
    uint16_t xpos, ypos;
    uint16_t cursor_pos;
    int16_t cursor_countdown;
    uint8_t in_focus;
} textbox_t;

extern textbox_t* sol_textbox_create(const size_t len, const uint16_t _xpos, const uint16_t _ypos);
extern char*      sol_textbox_get_text(textbox_t *tb);
extern int        sol_textbox_is_in(textbox_t *tb, const uint16_t x, const uint16_t y);
extern void       sol_textbox_set_focus(textbox_t *tb, const int focus);
extern sol_status_t sol_textbox_render(textbox_t *tb);
extern void       sol_textbox_free(textbox_t *tb);
extern void       sol_textbox_set_text(textbox_t *tb, char *text);
extern void       sol_textbox_set_current(textbox_t *tb);

#endif
