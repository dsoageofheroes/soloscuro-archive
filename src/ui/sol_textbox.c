#include "sol_textbox.h"
#include "gfftypes.h"
#include "gff.h"
#include "settings.h"
#include "../src/port.h"

#include <string.h>

extern textbox_t* sol_textbox_create(const size_t len, const uint16_t _xpos, const uint16_t _ypos) {
    gff_palette_t* pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    textbox_t *ret = calloc(1, sizeof(textbox_t));

    ret->xpos = _xpos * settings_zoom();
    ret->ypos = _ypos * settings_zoom();
    ret->text = calloc(1, len);
    ret->text_len = len;
    ret->text_cursor = sol_sprite_new(pal, ret->xpos, ret->ypos, // Blinking text cursor (for the player's name)
        settings_zoom(), RESOURCE_GFF_INDEX, GFF_ICON, 100);
    sol_sprite_set_frame(ret->text_cursor, 1);

    return ret;
}

extern void sol_textbox_set_text(textbox_t *tb, char *text) {
    size_t len = strlen(text);

    if (len > tb->text_len) {
        tb->text = realloc(tb->text, len + 1);
        tb->text_len = len;
    }

    strcpy(tb->text, text);
}

extern char* sol_textbox_get_text(textbox_t *tb) {
    if (!tb) { return NULL; }
    return tb->text;
}

extern int sol_textbox_is_in(textbox_t *tb, const uint16_t x, const uint16_t y) {
    if (!tb) { return 0; }
    int text_width = 8 * tb->text_len;

    if (x < tb->xpos || x > tb->xpos + (text_width * settings_zoom())) { return 0;}
    if (y < tb->ypos || y > tb->ypos + (8 * settings_zoom())) { return 0;}

    tb->cursor_pos = (x - tb->xpos) / (8 * settings_zoom());
    return 1;
}

extern void sol_textbox_set_focus(textbox_t *tb, const int focus) {
    if (!tb) { return; }
    tb->in_focus = focus;
}

void sol_textbox_free(textbox_t *tb) {
    if (!tb) { return; }

    sol_sprite_free(tb->text_cursor);

    if (tb->text) {
        free(tb->text);
        tb->text = NULL;
    }

    free(tb);
}

