#include "narrate.h"
#include "utils.h"
#include "sprite.h"
#include "portrait.h"
#include "../../src/dsl.h"
#include "../../src/ds-narrate.h"
#include "../../src/entity-animation.h"
#include "../../src/gff.h"
#include "gfftypes.h"
#include "../../src/gameloop.h"
#include "../font.h"
#include "../main.h"
#include "../../src/settings.h"
#include <string.h>
#include <ctype.h>

#define MAX_PORTRAITS (256)
#define MAX_TEXT (4096)
#define MAX_LINE (128)
#define MAX_OPTIONS (32)

static uint16_t background;
static uint32_t xoffset, yoffset;
static uint16_t border;

static int display = 0;
static int display_menu = 0;
static int end_received = 0, close_received = 0;
static uint32_t portrait_index = 0;
static char narrate_text[MAX_TEXT];
static size_t text_pos = 0;
static int menu_pos = 0;
static char menu_options[MAX_OPTIONS][MAX_LINE];

static void clear() {
    int i;
    display = 0;
    display_menu = 0;
    text_pos = 0;
    end_received = 0;
    close_received = 0;
    menu_pos = 0;
    for (i = 0; i < MAX_OPTIONS; i++) {
        menu_options[i][0] = '\0';
    }
}

int narrate_is_open() { return display; }

void narrate_init(const uint32_t x, const uint32_t y) {
    SDL_Renderer *renderer = main_get_rend();
    const float zoom = settings_zoom();
    xoffset = 0;//x / zoom;
    yoffset = 0;//y / zoom;
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    sol_portrait_load();
    background = sprite_new(renderer, pal, 0 + xoffset, 0 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_BMP, 3007);
    sprite_set_location(background, sprite_getx(background) + (main_get_width() - sprite_getw(background)) / 2, sprite_gety(background));
    border = sprite_new(renderer, pal, 0, 0, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 12000);
    sprite_set_location(border,
            sprite_getx(background) + 5 * zoom,
            sprite_gety(background) + 5 * zoom);
    sprite_set_alpha(background, 192);
    display = 0; // start off as off
}

void print_text(SDL_Renderer *renderer) {
    const uint32_t len = 48;
    uint32_t amt = 48;
    size_t y = 16;
    uint32_t i = 0;
    while (i < text_pos) {
        amt = len;
        while (i + amt < text_pos && amt > 0 && narrate_text[i + amt] != ' ') { amt--; }
        print_line_len(renderer, 0, narrate_text + i, 200, y, amt);
        i += amt;
        while (isspace(narrate_text[i])) { i++; }
        y += 16;
    }
}

void print_menu(SDL_Renderer *renderer) {
    size_t x = 140, y = 490;
    uint32_t sx = sprite_getx(background);
    uint32_t sy = sprite_gety(background);

    sprite_set_location(background, sx, y - 5);
    sprite_render(renderer, background);

    for (int i = 0; i < MAX_OPTIONS; i++) {
        print_line_len(renderer, 0, menu_options[i], x, y, 0x7FFFFFFF);
        y += 20;
    }

    sprite_set_location(background, sx, sy);
}

void port_narrate_clear() {
    clear();
}

void port_narrate_close() {
    display = 0;
    clear();
}

void narrate_render(void *data) {
    SDL_Renderer *renderer = main_get_rend();
    if (display) {
        sprite_render(renderer, background);
        sprite_render(renderer, border);
        sol_portrait_display(portrait_index, sprite_getx(border) + 8 * settings_zoom(), 12 * settings_zoom());
        print_text(renderer);
        if (display_menu) {
            print_menu(renderer);
        }
    }
}

void narrate_clear() {
    text_pos = 0;
}

static void add_text(const char *to_add) {
    if (!to_add) { return; }

    int len = strlen(to_add);

    if (text_pos + len >= MAX_TEXT) {
        error("Overflowed text buffer in narrate!");
        exit(1);
    }

    strcpy(narrate_text+text_pos, to_add);
    text_pos += len;
}

int8_t port_narrate_open(int16_t action, const char *text, int16_t index) {
    display = 1; // start off as off
    switch(action) {
        case NAR_ADD_MENU:
            display_menu = 1;
            //warn("I need to add_menu with index %d, text = '%s'\n", index, text);
            strncpy(menu_options[menu_pos], text, MAX_LINE);
            menu_options[menu_pos++][MAX_LINE-1] = '\0'; // guard
            break;
        case NAR_PORTRAIT:
            portrait_index = index;
            add_text(text);
            break;
        case NAR_SHOW_TEXT:
            add_text(text);
            break;
        case NAR_SHOW_MENU:
            display_menu = 1;
            break;
        case NAR_EDIT_BOX:
            warn("I need to show edit box (index =  %d), text = '%s'\n", index, text);
            break;
        default:
            error("narrate_open: ERROR unknown action %d\n", action);
            exit(1);
    }
    //debug("narrate_text = '%s'\n", narrate_text);
    return 0;
}

int narrate_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    return display; // zero means I did not handle the mouse, so another window may.
}

int narrate_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    int const height = 18;
    int y_test = 516;
    int option;
    if (display_menu) {
        if (x >= 150 && x <= 600) {
            for (int i = 1; i < 8; i++) {
                if (y >= y_test  && y < y_test + height) {
                    option = narrate_select_menu(i);
                    if (option >= 0) {
                        sol_game_loop_signal(WAIT_NARRATE_SELECT, option);
                    }
                    return 1;
                }
                y_test += height;
            }
        }
    }

    return display; // zero means I did not handle the mouse click, so another window may.
}

int narrate_handle_key_down(const enum entity_action_e action) {
    if (!display) { return 0; }

    printf("narrate action: %d\n", action);
    switch (action) {
        case EA_ACTIVATE:
            if (sol_game_loop_is_waiting_for(WAIT_NARRATE_CONTINUE)) {
                narrate_clear();
                sol_game_loop_signal(WAIT_NARRATE_CONTINUE, 0);
            }
        default:
        break;
    }
    return 1; // Handle the key
}

void narrate_free() {
    sprite_free(background);
    sprite_free(border);
}

int port_ask_yes_no() {
    error("MUST ASK YES NO, for now NO\n");
    return 0;
}

wops_t narrate_window = {
    .init = narrate_init,
    .cleanup = narrate_free,
    .render = narrate_render,
    .mouse_movement = narrate_handle_mouse_movement,
    .mouse_down = narrate_handle_mouse_down,
    .key_down = narrate_handle_key_down,
    .mouse_up = NULL,
    .grey_out_map = 0,
    .data = NULL
};
