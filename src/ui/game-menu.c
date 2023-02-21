#include "audio.h"
#include "game-menu.h"
#include "narrate.h"
#include "gfftypes.h"
#include "gff.h"
#include "settings.h"
#include "port.h"
#include <string.h>

#define NUM_ARROWS (3)

static uint32_t xoffset, yoffset;
static sol_sprite_t background, music_icon, sound_icon;
static sol_sprite_t left_arrow[NUM_ARROWS];
static sol_sprite_t right_arrow[NUM_ARROWS];
static sol_sprite_t bars[NUM_ARROWS];
static sol_sprite_t animations, info, game_return, game_menu, full_bar;
static int32_t mousex = -1, mousey = -1;
static sol_dim_t help_loc;

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y);

enum menu_state {
    MENU_MAIN,
    MENU_SOUND,
};

enum menu_state state = MENU_SOUND;

static void set_locations(sol_sprite_t sprite, const uint32_t x, const uint32_t y) {
    sol_sprite_info_t info;

    sol_status_check(sol_sprite_get_info(sprite, &info), "Unable to get sprite info.");
    int frame = info.current_frame;
    for (uint32_t i = 0; i < info.num_frames; i++) {
        sol_sprite_set_frame(sprite, i);
        sol_sprite_set_location(sprite, x, y);
    }
    sol_sprite_set_frame(sprite, frame);
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
    xoffset = x / zoom;
    yoffset = y / zoom;

    sol_status_check(sol_sprite_new(pal, 0 + xoffset, 0 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_BMP, 10000, &background),
            "Unable load game menu sprite");
    sol_status_check(sol_sprite_new(pal, 48 + xoffset, 24 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 16100, &music_icon),
            "Unable load game menu sprite");
    sol_status_check(sol_sprite_new(pal, 48 + xoffset, 42 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 16101, &sound_icon),
            "Unable load game menu sprite");
    sol_status_check(sol_sprite_new(pal, 48 + xoffset, 78 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 16103, &animations),
            "Unable load game menu sprite");
    sol_status_check(sol_sprite_new(pal, 68 + xoffset, 78 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 16102, &info),
            "Unable load game menu sprite");
    sol_status_check(sol_sprite_new(pal, 138 + xoffset, 78 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 10108, &game_return),
            "Unable load game menu sprite");
    sol_status_check(sol_sprite_new(pal, 108 + xoffset, 78 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 11101, &game_menu),
            "Unable load game menu sprite");
    sol_status_check(sol_sprite_new(pal, 0, 0, zoom,
            RESOURCE_GFF_INDEX, GFF_BMP, 20107, &full_bar),
            "Unable load game menu sprite");
    for (int i = 0; i < NUM_ARROWS; i++) {
        sol_status_check(sol_sprite_new(pal, 0, 0, zoom,
                RESOURCE_GFF_INDEX, GFF_ICON, 16104, &right_arrow[i]),
                "Unable load game menu sprite");
        sol_status_check(sol_sprite_new(pal, 0, 0, zoom,
                RESOURCE_GFF_INDEX, GFF_ICON, 16105, &left_arrow[i]),
                "Unable load game menu sprite");
        sol_status_check(sol_sprite_new(pal, 0, 0, zoom,
                RESOURCE_GFF_INDEX, GFF_BMP, 20106, &bars[i]),
                "Unable load game menu sprite");
    }

    help_loc.x = (xoffset + 43) * zoom;
    help_loc.y = (yoffset + 102) * zoom;
    help_loc.w = (127) * zoom;

    open_sound();
}

static void print_help(const char *msg) {
    sol_font_render_center(FONT_GREYLIGHT, msg, help_loc.x, help_loc.y, help_loc.w);
}

static void print_text_help() {
    sol_sprite_t spr = get_sprite_mouse_is_on(mousex, mousey);

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
    sol_sprite_render(background);
    sol_audio_stats_t *stats = NULL;
    sol_sprite_info_t sinfo;

    switch(state) {
        case MENU_SOUND:
            sol_sprite_render(animations);
            sol_sprite_render(info);
            sol_sprite_render(music_icon);
            sol_sprite_render(sound_icon);
            sol_sprite_render(game_return);
            sol_sprite_render(game_menu);
            for (int i = 0; i < NUM_ARROWS; i++) {
                sol_sprite_render(right_arrow[i]);
                sol_sprite_render(left_arrow[i]);
                sol_sprite_render(bars[i]);
            }
            sol_audio_get(&stats);
            sol_status_check(sol_sprite_get_info(bars[0], &sinfo), "Unable to get bars sprite info");
            sol_sprite_render_box(full_bar, sinfo.x, sinfo.y,
                sinfo.w * stats->xmi_volume, sinfo.h);
            sol_status_check(sol_sprite_get_info(bars[1], &sinfo), "Unable to get bars sprite info");
            sol_sprite_render_box(full_bar, sinfo.x, sinfo.y,
                sinfo.w * stats->voc_volume, sinfo.h);
            break;
        default:
            break;
    }

    print_text_help();
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    switch(state) {
        case MENU_SOUND:
            if (sol_sprite_in_rect(music_icon, x, y) == SOL_SUCCESS) { return music_icon; }
            if (sol_sprite_in_rect(sound_icon, x, y) == SOL_SUCCESS) { return sound_icon; }
            if (sol_sprite_in_rect(animations, x, y) == SOL_SUCCESS) { return animations; }
            if (sol_sprite_in_rect(info, x, y) == SOL_SUCCESS) { return info; }
            if (sol_sprite_in_rect(game_menu, x, y) == SOL_SUCCESS) { return game_menu; }
            if (sol_sprite_in_rect(game_return, x, y) == SOL_SUCCESS) { return game_return; }

            for (int i = 0; i < NUM_ARROWS; i++) {
                if (sol_sprite_in_rect(left_arrow[i], x, y) == SOL_SUCCESS) { return left_arrow[i]; }
                if (sol_sprite_in_rect(right_arrow[i], x, y) == SOL_SUCCESS) { return right_arrow[i]; }
                if (sol_sprite_in_rect(bars[i], x, y) == SOL_SUCCESS) { return bars[i]; }
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
        sol_sprite_increment_frame(cur_sprite, 1);
        if (last_sprite != SPRITE_ERROR) {
            sol_sprite_increment_frame(last_sprite, -1);
        }
    }

    last_sprite = cur_sprite;
    return 1;// did not handle
}

int game_menu_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    return 1; // means I captured the mouse click
}

int game_menu_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    sol_audio_stats_t *stats = NULL;

    sol_audio_get(&stats);

    if (sol_sprite_in_rect(left_arrow[0], x, y) == SOL_SUCCESS) { sol_audio_set_xmi_volume(stats->xmi_volume - .1); }
    if (sol_sprite_in_rect(right_arrow[0], x, y) == SOL_SUCCESS) { sol_audio_set_xmi_volume(stats->xmi_volume + .1); }
    if (sol_sprite_in_rect(left_arrow[1], x, y) == SOL_SUCCESS) { sol_audio_set_voc_volume(stats->voc_volume - .1); }
    if (sol_sprite_in_rect(right_arrow[1], x, y) == SOL_SUCCESS) { sol_audio_set_voc_volume(stats->voc_volume + .1); }

    if (sol_sprite_in_rect(game_return, x, y) == SOL_SUCCESS) { sol_window_pop(); } 

    return 1; // means I captured the mouse click
}

void game_menu_free() {
    sol_status_check(sol_sprite_free(background), "Unable to free sprite");
    sol_status_check(sol_sprite_free(music_icon), "Unable to free sprite");
    sol_status_check(sol_sprite_free(sound_icon), "Unable to free sprite");

    for (int i = 0; i < NUM_ARROWS; i++) {
        sol_status_check(sol_sprite_free(left_arrow[i]), "Unable to free sprite");
        sol_status_check(sol_sprite_free(right_arrow[i]), "Unable to free sprite");
        sol_status_check(sol_sprite_free(bars[i]), "Unable to free sprite");
    }

    sol_status_check(sol_sprite_free(animations), "Unable to free sprite");
    sol_status_check(sol_sprite_free(info), "Unable to free sprite");
    sol_status_check(sol_sprite_free(game_return), "Unable to free sprite");
    sol_status_check(sol_sprite_free(game_menu), "Unable to free sprite");
    sol_status_check(sol_sprite_free(full_bar), "Unable to free sprite");
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
