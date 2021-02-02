#include "popup.h"
#include "narrate.h"
#include "../sprite.h"
#include "../font.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"
#include <string.h>

static uint16_t background, popup_return, option[3];
static char main_text[32], option_text[3][32];
static uint8_t selection = POPUP_NOTHING;
static font_t option_font[3];

SDL_Rect main_loc, option_loc[3];

static SDL_Rect initial_locs[] = {{ 10, 4, 0, 0 }, // main
                                  { 10, 17, 0, 0 }, // option 0
                                  { 10, 29, 0, 0 }, // option 1
                                  { 10, 41, 0, 0 }, // option 2
};

void popup_set_message(const char *msg) {
    strncpy(main_text, msg, 31);
    main_text[31] = '\0';
}

void popup_set_option(const int idx, const char *msg) {
    if (idx < 0 || idx > 2) { return; }
    strncpy(option_text[idx], msg, 31);
    option_text[idx][31] = '\0';
}

uint16_t popup_sprite_create(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_create(renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

static SDL_Rect setup_loc(const SDL_Rect rect, const uint32_t x, const uint32_t y, const float zoom) {
    SDL_Rect ret = {rect.x + x, rect.y + y, rect.w, rect.h};
    ret.x *= zoom;
    ret.y *= zoom;
    ret.w *= zoom;
    ret.h *= zoom;
    return ret;
}

void popup_init(SDL_Renderer *renderer, const uint32_t x, const uint32_t y, const float zoom) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    background = popup_sprite_create(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 14000);
    popup_return = popup_sprite_create(renderer, pal, 103 + x, 36 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 5012);
    option[0] = popup_sprite_create(renderer, pal, 8 + x, 17 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 14101);
    option[1] = popup_sprite_create(renderer, pal, 8 + x, 29 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 14102);
    option[2] = popup_sprite_create(renderer, pal, 8 + x, 41 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 14103);

    memset(main_text, 0x0, sizeof(main_text));
    memset(option_text, 0x0, sizeof(option_text));

    main_loc = setup_loc(initial_locs[0], x, y, zoom);
    option_loc[0] = setup_loc(initial_locs[1], x, y, zoom);
    option_loc[1] = setup_loc(initial_locs[2], x, y, zoom);
    option_loc[2] = setup_loc(initial_locs[3], x, y, zoom);

    option_font[0] = FONT_BLACK;
    option_font[1] = FONT_BLACK;
    option_font[2] = FONT_BLACK;

    selection = POPUP_NOTHING;
}

void popup_render(void *data, SDL_Renderer *renderer) {
    sprite_render(renderer, background);
    sprite_render(renderer, popup_return);
    sprite_render(renderer, option[0]);
    sprite_render(renderer, option[1]);
    sprite_render(renderer, option[2]);

    print_line_len(renderer, FONT_GREY, main_text, main_loc.x, main_loc.y, sizeof(main_text));
    print_line_len(renderer, option_font[0], option_text[0], option_loc[0].x, option_loc[0].y, sizeof(option_text[0]));
    print_line_len(renderer, option_font[1], option_text[1], option_loc[1].x, option_loc[1].y, sizeof(option_text[1]));
    print_line_len(renderer, option_font[2], option_text[2], option_loc[2].x, option_loc[2].y, sizeof(option_text[2]));
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(popup_return, x, y)) { return popup_return; }
    if (sprite_in_rect(option[0], x, y)) { return option[0]; }
    if (sprite_in_rect(option[1], x, y)) { return option[1]; }
    if (sprite_in_rect(option[2], x, y)) { return option[2]; }
    
    return SPRITE_ERROR;
}

int popup_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;

    uint16_t cur_sprite = get_sprite_mouse_is_on(x, y);

    if (last_sprite != cur_sprite) {
        if (cur_sprite == option[0]) { option_font[0] = FONT_RED; }
        if (cur_sprite == option[1]) { option_font[1] = FONT_RED; }
        if (cur_sprite == option[2]) { option_font[2] = FONT_RED; }
        sprite_set_frame(cur_sprite, sprite_get_frame(cur_sprite) + 1);
        if (last_sprite != SPRITE_ERROR) {
            sprite_set_frame(last_sprite, sprite_get_frame(last_sprite) - 1);
            if (last_sprite == option[0]) { option_font[0] = FONT_BLACK; }
            if (last_sprite == option[1]) { option_font[1] = FONT_BLACK; }
            if (last_sprite == option[2]) { option_font[2] = FONT_BLACK; }
        }
    }
    
    last_sprite = cur_sprite;
    return 1;// did not handle
}

int popup_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(option[0], x, y)) { return option[0]; }
    if (sprite_in_rect(option[1], x, y)) { return option[1]; }
    if (sprite_in_rect(option[2], x, y)) { return option[2]; }

    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another screen may.
}

int popup_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(option[0], x, y)) { selection = POPUP_0; }
    if (sprite_in_rect(option[1], x, y)) { selection = POPUP_1; }
    if (sprite_in_rect(option[2], x, y)) { selection = POPUP_2; }
    if (sprite_in_rect(popup_return, x, y)) { selection = POPUP_CANCEL; }

    if (selection != POPUP_NOTHING) {
        screen_pop();
    }

    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another screen may.
}

void popup_free() {
    sprite_free(background);
    sprite_free(popup_return);
    sprite_free(option[0]);
    sprite_free(option[1]);
    sprite_free(option[2]);
}

uint8_t popup_get_selection() { return selection; }
void popup_clear_selection() { selection = POPUP_NOTHING; }

sops_t popup_screen = {
    .init = popup_init,
    .cleanup = popup_free,
    .render = popup_render,
    .mouse_movement = popup_handle_mouse_movement,
    .mouse_down = popup_handle_mouse_down,
    .mouse_up = popup_handle_mouse_up,
    .data = NULL
};
