#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <stdlib.h>
#include <stdint.h>

typedef struct textbox_s {
    char *text;
    size_t text_len, text_pos;
    uint16_t count_down; // for cursor
    uint16_t xpos, ypos;
    uint16_t text_cursor;
} textbox_t;

textbox_t* textbox_create(const size_t len, const uint16_t _xpos, const uint16_t _ypos);
void textbox_render(textbox_t *tb);
void textbox_free(textbox_t *tb);
void textbox_set_text(textbox_t *tb, char *text);

#endif
