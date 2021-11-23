#include "narrate.h"
#include "../../src/port.h"
#include "portrait.h"
#include "gpl.h"
#include "entity-animation.h"
#include "gff.h"
#include "gfftypes.h"
#include "../src/replay.h"
#include "gpl-manager.h"
#include "gameloop.h"
#include "settings.h"
#include <string.h>
#include <ctype.h>

#define MAX_PORTRAITS (256)
#define MAX_TEXT (4096)
#define MAX_LINE (128)
#define MAX_OPTIONS (32)
#define MAX_MENUS (16)
#define MAX_LINE (128)

static size_t menu_pos = 0;
static uint32_t menu_addrs[MAX_MENUS];
static char menu_text[MAX_MENUS][MAX_LINE];
static sol_sprite_t background;
static uint32_t xoffset, yoffset;
static uint16_t border;

static int display = 0;
static int display_menu = 0;
static int end_received = 0, close_received = 0;
static uint32_t portrait_index = 0;
static char narrate_text[MAX_TEXT];
static size_t text_pos = 0;
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
    const float zoom = settings_zoom();
    xoffset = 0;//x / zoom;
    yoffset = 0;//y / zoom;
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    sol_portrait_load();
    background = sol_sprite_new(pal, 0 + xoffset, 0 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_BMP, 3007);
    sol_sprite_set_location(background,
        sol_sprite_getx(background) + (settings_screen_width() - sol_sprite_getw(background)) / 2,
        sol_sprite_gety(background));
    border = sol_sprite_new(pal, 0, 0, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 12000);
    sol_sprite_set_location(border,
            sol_sprite_getx(background) + 5 * zoom,
            sol_sprite_gety(background) + 5 * zoom);
    sol_sprite_set_alpha(background, 192);
    display = 0; // start off as off
}

void print_text() {
    const uint32_t len = 48;
    uint32_t amt = 48;
    size_t y = 16;
    uint32_t i = 0;
    while (i < text_pos) {
        amt = len;
        while (i + amt < text_pos && amt > 0 && narrate_text[i + amt] != ' ') { amt--; }
        sol_print_line_len(0, narrate_text + i, 200, y, amt);
        i += amt;
        while (isspace(narrate_text[i])) { i++; }
        y += 16;
    }
}

void print_menu() {
    size_t x = 140, y = 490;
    uint32_t sx = sol_sprite_getx(background);
    uint32_t sy = sol_sprite_gety(background);

    sol_sprite_set_location(background, sx, y - 5);
    sol_sprite_render(background);

    for (int i = 0; i < MAX_OPTIONS; i++) {
        sol_print_line_len(0, menu_options[i], x, y, 0x7FFFFFFF);
        y += 20;
    }

    sol_sprite_set_location(background, sx, sy);
}

void sol_ui_narrate_clear() {
    clear();
}

void sol_ui_narrate_close() {
    display = 0;
    clear();
}

void narrate_render(void *data) {
    if (display) {
        sol_sprite_render(background);
        sol_sprite_render(border);
        sol_portrait_display(portrait_index, sol_sprite_getx(border) + 8 * settings_zoom(), 12 * settings_zoom());
        print_text();
        if (display_menu) {
            print_menu();
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

int8_t sol_ui_narrate_open(int16_t action, const char *text, int16_t index) {
    display = 1; // start off as off
    switch(action) {
        case NAR_ADD_MENU:
            display_menu = 1;
            //warn("I need to add_menu with index %d, text = '%s'\n", index, text);
            strncpy(menu_options[menu_pos], text, MAX_LINE);
            menu_options[menu_pos][MAX_LINE-1] = '\0'; // guard
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
                sol_ui_narrate_clear();
                sol_game_loop_signal(WAIT_NARRATE_CONTINUE, 0);
            }
        default:
        break;
    }
    return 1; // Handle the key
}

void narrate_free() {
    sol_sprite_free(background);
    sol_sprite_free(border);
}

int port_ask_yes_no() {
    error("MUST ASK YES NO, for now NO\n");
    return 0;
}

extern int8_t narrate_open(int16_t action, const char *text, int16_t index) {
    if (action == NAR_SHOW_TEXT) {
        if (strncmp("END", text, 3) == 0) {
            sol_game_loop_wait_for_signal(WAIT_NARRATE_CONTINUE);
            return 0;
        }
        if (strncmp("CLOSE", text, 5) == 0) {
            sol_ui_narrate_close();
            return 0;
        }
    }
    sol_ui_narrate_open(action, text, index);
    switch(action) {
        case NAR_ADD_MENU:
            strncpy(menu_text[menu_pos], text, MAX_LINE);
            menu_addrs[menu_pos++] = index;
            //warn("I need to add_menu with index %d, text = '%s'\n", index, text);
            break;
        case NAR_PORTRAIT:
            menu_pos = 0;
            //warn("I need to create narrate_box with portrait index %d, text = '%s'\n", index, text);
            break;
        case NAR_SHOW_TEXT:
            //warn("I need to show text (index =  %d), text = '%s'\n", index, text);
            break;
        case NAR_SHOW_MENU:
            //warn("I need to show menu (index =  %d), text = '%s'\n", index, text);
            break;
        case NAR_EDIT_BOX:
            //warn("I need to show edit box (index =  %d), text = '%s'\n", index, text);
            break;
        default:
            error("narrate_open: ERROR unknown action %d\n", action);
            exit(1);
    }

    return 0;
}

static int option_is_exit(const int option) {
    if (option >= menu_pos || option < 0) { return 0; }
    if (strncmp("Goodbye.", menu_text[option], 8) == 0) { return 1; }
    if (strncmp("END", menu_text[option], 3) == 0) { return 1; }
    if (strncmp("CLOSE", menu_text[option], 5) == 0) { return 1; }
    return 0;
}

extern int narrate_select_menu(int option) {
    int accum = !option_is_exit(option);
    char buf[1024];

    replay_print("rep.select_menu(%d)\n", option);
    if (option >= menu_pos || option < 0) {
        error("select_menu: Menu option %d selected, but only (0 - " PRI_SIZET ") available!\n", option, menu_pos - 1);
        return -1;
    }
    menu_pos = 0;
    sol_ui_narrate_clear();
    snprintf(buf, 1024, "func%d()\n", menu_addrs[option]);
    gpl_execute_string(buf);
    //game_loop_signal(WAIT_NARRATE_SELECT, accum);
    return accum;
}

sol_wops_t narrate_window = {
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
