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
    uint16_t cursor_pos;
    int16_t cursor_countdown;
    uint8_t in_focus;
} textbox_t;

textbox_t* textbox_create(const size_t len, const uint16_t _xpos, const uint16_t _ypos);
char* textbox_get_text(textbox_t *tb);
int textbox_is_in(textbox_t *tb, const uint16_t x, const uint16_t y);
void textbox_set_focus(textbox_t *tb, const int focus);
void textbox_render(textbox_t *tb);
int textbox_handle_keydown(textbox_t *tb, SDL_Keysym);
int textbox_handle_keyup(textbox_t *tb, SDL_Keysym);
void textbox_free(textbox_t *tb);
void textbox_set_text(textbox_t *tb, char *text);

#endif
