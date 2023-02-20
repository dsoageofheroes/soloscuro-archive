#include "sol_textbox.h"
#include "gfftypes.h"
#include "gff.h"
#include "settings.h"
#include "port.h"

#include <string.h>

extern sol_status_t sol_textbox_create(const size_t len, const uint16_t _xpos, const uint16_t _ypos, sol_textbox_t **tb) {
    gff_palette_t* pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    sol_textbox_t *ret = calloc(1, sizeof(sol_textbox_t));

    if (!ret) { return SOL_MEMORY_ERROR; }

    ret->xpos = _xpos * settings_zoom();
    ret->ypos = _ypos * settings_zoom();
    ret->text = calloc(1, len);
    if (!ret->text) { return SOL_MEMORY_ERROR; }
    ret->text_len = len;
    sol_status_check(sol_sprite_new(pal, ret->xpos, ret->ypos, // Blinking text cursor (for the player's name)
        settings_zoom(), RESOURCE_GFF_INDEX, GFF_ICON, 100, &ret->text_cursor),
        "Unable to load textbox sprite.");
    sol_sprite_set_frame(ret->text_cursor, 1);

    *tb = ret;
    return SOL_SUCCESS;
}

extern sol_status_t sol_textbox_set_text(sol_textbox_t *tb, char *text) {
    size_t len = strlen(text);

    if (!tb || !text) { return SOL_NULL_ARGUMENT; }
    if (len > tb->text_len) {
        tb->text = realloc(tb->text, len + 1);
        tb->text_len = len;
    }

    strcpy(tb->text, text);
    return SOL_SUCCESS;
}

extern sol_status_t sol_textbox_is_in(sol_textbox_t *tb, const uint16_t x, const uint16_t y) {
    if (!tb) { return SOL_NULL_ARGUMENT; }
    int text_width = 8 * tb->text_len;

    if (x < tb->xpos || x > tb->xpos + (text_width * settings_zoom())) { return SOL_OUT_OF_RANGE;}
    if (y < tb->ypos || y > tb->ypos + (8 * settings_zoom())) { return SOL_OUT_OF_RANGE;}

    tb->cursor_pos = (x - tb->xpos) / (8 * settings_zoom());
    return SOL_SUCCESS;;
}

extern sol_status_t sol_textbox_set_focus(sol_textbox_t *tb, const int focus) {
    if (!tb) { return SOL_NULL_ARGUMENT; }
    tb->in_focus = focus;
    return SOL_SUCCESS;
}

extern sol_status_t sol_textbox_free(sol_textbox_t *tb) {
    if (!tb) { return SOL_NULL_ARGUMENT; }

    sol_status_check(sol_sprite_free(tb->text_cursor), "Unable to free sprite");

    if (tb->text) {
        free(tb->text);
        tb->text = NULL;
    }

    free(tb);
    return SOL_SUCCESS;
}

