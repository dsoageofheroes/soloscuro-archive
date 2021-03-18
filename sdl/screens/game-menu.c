#include "game-menu.h"
#include "narrate.h"
#include "../sprite.h"
#include "../main.h"
#include "../font.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"
#include <string.h>

#define NUM_ARROWS (3)

uint16_t background;
static uint32_t xoffset, yoffset;
uint16_t music_icon, sound_icon;
uint16_t left_arrow[NUM_ARROWS];
uint16_t right_arrow[NUM_ARROWS];

enum menu_state {
    MENU_MAIN,
    MENU_SOUND,
};

enum menu_state state = MENU_SOUND;

static void open_sound() {
    const float zoom = main_get_zoom();

    sprite_set_location(right_arrow[0], xoffset + 200 * zoom, yoffset + 30 * zoom);
    sprite_set_location(right_arrow[1], xoffset + 200 * zoom, yoffset + 40 * zoom);
    sprite_set_location(right_arrow[2], xoffset + 200 * zoom, yoffset + 50 * zoom);
}

void game_menu_init(SDL_Renderer *renderer, const uint32_t x, const uint32_t y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = main_get_zoom();
    xoffset = x;
    yoffset = y / zoom;

    background = sprite_new(renderer, pal, 0 + xoffset, 0 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_BMP, 10000);
    music_icon = sprite_new(renderer, pal, 24 * zoom + xoffset, 12 * zoom + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 16100);
    sound_icon = sprite_new(renderer, pal, 24 * zoom + xoffset, 21 * zoom + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_ICON, 16101);
    for (int i = 0; i < NUM_ARROWS; i++) {
        right_arrow[i] = sprite_new(renderer, pal, 0, 0, zoom,
                RESOURCE_GFF_INDEX, GFF_ICON, 16104);
        left_arrow[i] = sprite_new(renderer, pal, 0, 0, zoom,
                RESOURCE_GFF_INDEX, GFF_ICON, 16105);
    }

    open_sound();
}

void game_menu_render(void *data, SDL_Renderer *renderer) {
    sprite_render(renderer, background);

    switch(state) {
        case MENU_SOUND:
            sprite_render(renderer, music_icon);
            sprite_render(renderer, sound_icon);
            for (int i = 0; i < NUM_ARROWS; i++) {
                sprite_render(renderer, right_arrow[i]);
            }
            break;
        default:
            break;
    }
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    
    return SPRITE_ERROR;
}

int game_menu_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;

    uint16_t cur_sprite = get_sprite_mouse_is_on(x, y);
    printf("last_sprite = %d\n", last_sprite);

    last_sprite = cur_sprite;
    return 1;// did not handle
}

int game_menu_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another screen may.
}

int game_menu_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    //if (sprite_in_rect(option[0], x, y)) { selection = POPUP_0; }

    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another screen may.
}

void game_menu_free() {
}

sops_t game_menu_screen = {
    .init = game_menu_init,
    .cleanup = game_menu_free,
    .render = game_menu_render,
    .mouse_movement = game_menu_handle_mouse_movement,
    .mouse_down = game_menu_handle_mouse_down,
    .mouse_up = game_menu_handle_mouse_up,
    .grey_out_map = 1,
    .data = NULL
};
