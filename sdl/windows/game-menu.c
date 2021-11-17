#include "game-menu.h"
#include "narrate.h"
#include "../audio.h"
#include "../sprite.h"
#include "../main.h"
#include "../font.h"
#include "../../src/gff.h"
#include "gfftypes.h"
#include "../../src/settings.h"
#include <string.h>

#define NUM_ARROWS (3)

static uint16_t background;
static uint32_t xoffset, yoffset;
static uint16_t music_icon, sound_icon;
static uint16_t left_arrow[NUM_ARROWS];
static uint16_t right_arrow[NUM_ARROWS];
static uint16_t bars[NUM_ARROWS];
static uint16_t animations, info, game_return, game_menu, full_bar;
static int32_t mousex = -1, mousey = -1;
SDL_Rect help_loc;

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y);

enum menu_state {
    MENU_MAIN,
    MENU_SOUND,
};

enum menu_state state = MENU_SOUND;

static void set_locations(uint16_t sprite, const uint32_t x, const uint32_t y) {
    int frame = sprite_get_frame(sprite);
    for (uint32_t i = 0; i < sprite_num_frames(sprite); i++) {
        sprite_set_frame(sprite, i);
        sprite_set_location(sprite, x, y);
    }
    sprite_set_frame(sprite, frame);
}

static void open_sound() {
    const float zoom = settings_zoom();

    set_locations(right_arrow[0], (xoffset + 159) * zoom, (yoffset + 29) * zoom);
    set_locations(right_arrow[1], (xoffset + 159) * zoom, (yoffset + 46) * zoom);
    set_locations(right_arrow[2], (xoffset + 151) * zoom, (yoffset + 66) * zoom);
    set_locations(left_arrow[0], (xoffset + 66) * zoom, (yoffset + 29) * zoom);
    set_locations(left_arrow[1], (xoffset + 66) * zoom, (yoffset + 46) * zoom);
    set_locations(left_arrow[2], (xoffset + 58) * zoom, (yoffset + 66) * zoom);
    set_locations(bars[0], (xoffset + 75) * zoom, (yoffset + 27) * zoom);
    set_locations(bars[1], (xoffset + 75) * zoom, (yoffset + 44) * zoom);
    set_locations(bars[2], (xoffset + 68) * zoom, (yoffset + 64) * zoom);
}

void game_menu_init(const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = settings_zoom();
    SDL_Renderer *renderer = main_get_rend();
    xoffset = x / zoom;
    yoffset = y / zoom;

    background = sprite_new(renderer, pal, 0 + xoffset, 0 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_BMP, 10000);
    music_icon = sprite_new(renderer, pal, 48 + xoffset, 24 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 16100);
    sound_icon = sprite_new(renderer, pal, 48 + xoffset, 42 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 16101);
    animations = sprite_new(renderer, pal, 48 + xoffset, 78 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 16103);
    info = sprite_new(renderer, pal, 68 + xoffset, 78 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 16102);
    game_return = sprite_new(renderer, pal, 138 + xoffset, 78 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 10108);
    game_menu = sprite_new(renderer, pal, 108 + xoffset, 78 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 11101);
    full_bar = sprite_new(renderer, pal, 0, 0, zoom,
            RESOURCE_GFF_INDEX, GFF_BMP, 20107);
    for (int i = 0; i < NUM_ARROWS; i++) {
        right_arrow[i] = sprite_new(renderer, pal, 0, 0, zoom,
                RESOURCE_GFF_INDEX, GFF_ICON, 16104);
        left_arrow[i] = sprite_new(renderer, pal, 0, 0, zoom,
                RESOURCE_GFF_INDEX, GFF_ICON, 16105);
        bars[i] = sprite_new(renderer, pal, 0, 0, zoom,
                RESOURCE_GFF_INDEX, GFF_BMP, 20106);
    }

    help_loc.x = (xoffset + 43) * zoom;
    help_loc.y = (yoffset + 102) * zoom;
    help_loc.w = (127) * zoom;

    open_sound();
}

static void print_help(const char *msg) {
    font_render_center(main_get_rend(), FONT_GREYLIGHT, msg, help_loc);
}

static void print_text_help() {
    int16_t spr = get_sprite_mouse_is_on(mousex, mousey);

    if (spr == animations) { print_help("ANIMATIONS ON/OFF"); }
    if (spr == info) { print_help("ABOUT"); }
    if (spr == game_menu) { print_help("GAME MENU"); }
    if (spr == game_return) { print_help("GAME RETURN"); }
    if (spr == music_icon) { print_help("MUSIC TOGGLE ON/OFF"); }
    if (spr == sound_icon) { print_help("SOUND EFFECTS ON/OFF"); }
    if (spr == bars[0] || spr == right_arrow[0] || spr == left_arrow[0]) {
        print_help("MUSIC VOLUME");
    }
    if (spr == bars[1] || spr == right_arrow[1] || spr == left_arrow[1]) {
        print_help("SOUND EFFECT VOLUME");
    }
    if (spr == bars[2] || spr == right_arrow[2] || spr == left_arrow[2]) {
        print_help("GAME DIFFICULTY");
    }
}

void game_menu_render(void *data) {
    SDL_Renderer *renderer = main_get_rend();
    sprite_render(renderer, background);

    switch(state) {
        case MENU_SOUND:
            sprite_render(renderer, animations);
            sprite_render(renderer, info);
            sprite_render(renderer, music_icon);
            sprite_render(renderer, sound_icon);
            sprite_render(renderer, game_return);
            sprite_render(renderer, game_menu);
            for (int i = 0; i < NUM_ARROWS; i++) {
                sprite_render(renderer, right_arrow[i]);
                sprite_render(renderer, left_arrow[i]);
                sprite_render(renderer, bars[i]);
            }
            sprite_render_box(renderer, full_bar, sprite_getx(bars[0]), sprite_gety(bars[0]),
                sprite_getw(bars[0]) * audio_get_xmi_volume(), sprite_geth(bars[0]));
            sprite_render_box(renderer, full_bar, sprite_getx(bars[1]), sprite_gety(bars[1]),
                sprite_getw(bars[1]) * audio_get_voc_volume(), sprite_geth(bars[1]));
            break;
        default:
            break;
    }

    print_text_help();
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    switch(state) {
        case MENU_SOUND:
            if (sprite_in_rect(music_icon, x, y)) { return music_icon; }
            if (sprite_in_rect(sound_icon, x, y)) { return sound_icon; }
            if (sprite_in_rect(animations, x, y)) { return animations; }
            if (sprite_in_rect(info, x, y)) { return info; }
            if (sprite_in_rect(game_menu, x, y)) { return game_menu; }
            if (sprite_in_rect(game_return, x, y)) { return game_return; }

            for (int i = 0; i < NUM_ARROWS; i++) {
                if (sprite_in_rect(left_arrow[i], x, y)) { return left_arrow[i]; }
                if (sprite_in_rect(right_arrow[i], x, y)) { return right_arrow[i]; }
                if (sprite_in_rect(bars[i], x, y)) { return bars[i]; }
            }

            break;
        default:
            break;
    }
    
    return SPRITE_ERROR;
}

int game_menu_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;
    mousex = x;
    mousey = y;

    uint16_t cur_sprite = get_sprite_mouse_is_on(x, y);

    if (last_sprite != cur_sprite) {
        sprite_set_frame(cur_sprite, sprite_get_frame(cur_sprite) + 1);
        if (last_sprite != SPRITE_ERROR) {
            sprite_set_frame(last_sprite, sprite_get_frame(last_sprite) - 1);
        }
    }

    last_sprite = cur_sprite;
    return 1;// did not handle
}

int game_menu_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another window may.
}

int game_menu_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(left_arrow[0], x, y)) { audio_set_xmi_volume(audio_get_xmi_volume() - .1); }
    if (sprite_in_rect(right_arrow[0], x, y)) { audio_set_xmi_volume(audio_get_xmi_volume() + .1); }
    if (sprite_in_rect(left_arrow[1], x, y)) { audio_set_voc_volume(audio_get_voc_volume() - .1); }
    if (sprite_in_rect(right_arrow[1], x, y)) { audio_set_voc_volume(audio_get_voc_volume() + .1); }

    if (sprite_in_rect(game_return, x, y)) { sol_window_pop(); } 

    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another window may.
}

void game_menu_free() {
    sprite_free(background);
    sprite_free(music_icon);
    sprite_free(sound_icon);

    for (int i = 0; i < NUM_ARROWS; i++) {
        sprite_free(left_arrow[i]);
        sprite_free(right_arrow[i]);
        sprite_free(bars[i]);
    }

    sprite_free(animations);
    sprite_free(info);
    sprite_free(game_return);
    sprite_free(game_menu);
    sprite_free(full_bar);
}

static uint32_t game_menu_get_width() { return 210 * settings_zoom(); }
static uint32_t game_menu_get_height() { return 116 * settings_zoom(); }

sol_wops_t game_menu_window = {
    .init = game_menu_init,
    .cleanup = game_menu_free,
    .render = game_menu_render,
    .mouse_movement = game_menu_handle_mouse_movement,
    .mouse_down = game_menu_handle_mouse_down,
    .mouse_up = game_menu_handle_mouse_up,
    .grey_out_map = 1,
    .get_width = game_menu_get_width,
    .get_height = game_menu_get_height,
    .data = NULL
};
