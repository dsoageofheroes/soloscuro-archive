#include "narrate.h"
#include "port.h"
#include "portrait.h"
#include "gpl.h"
#include "entity-animation.h"
#include "gff.h"
#include "gfftypes.h"
#include "replay.h"
#include "gpl-manager.h"
#include "gameloop.h"
#include "settings.h"
#include "ssi-gui.h"
#include <string.h>
#include <ctype.h>

#define MAX_PORTRAITS (256)
#define MAX_TEXT (4096)
#define MAX_LINE (128)
#define MAX_OPTIONS (32)
#define MAX_MENUS (16)
#define MAX_LINE (128)

static size_t menu_pos = 0, last_option = 0, menu_start_pos = 0;
static uint32_t menu_addrs[MAX_MENUS];

static int display = 0;
static int display_menu = 0;
static uint32_t portrait_index = 0;
static char narrate_text[MAX_TEXT];
static size_t text_pos = 0;
static char menu_options[MAX_OPTIONS][MAX_LINE];
static sol_window_t *menu = NULL, *narrate = NULL;
static uint32_t      winx = 0, winy = 0;

static void clear() {
    display = display_menu = 0;
    text_pos = menu_start_pos = menu_pos = 0;

    for (int i = 0; i < MAX_OPTIONS; i++) {
        menu_options[i][0] = '\0';
    }
    for (int i = 1; i < 7; i++) {
        sol_button_set_text(menu->buttons + i, menu_options[i], FONT_YELLOW);
    }
}

static void update_menu(int force) {
    if (!force && (!display_menu || !sol_button_is_empty(menu->buttons + 1))) { return; }

    for (size_t i = 1; i < 6; i++) {
        sol_button_set_text(menu->buttons + i, menu_options[menu_start_pos + i], FONT_YELLOW);
    }
    
    sol_button_set_enabled(menu->buttons + 6, menu_start_pos != 0);
    sol_button_set_enabled(menu->buttons + 0, (menu_start_pos + 6) < menu_pos);
}

static void scroll_menu(int diff) {
    if ((diff + (ssize_t) menu_start_pos) < 0
        || (menu_start_pos + 5 + diff) >= menu_pos) {
        return;
    }

    menu_start_pos += diff;
    update_menu(1); // force the update
}

extern sol_status_t sol_narrate_is_open() { return display ? SOL_SUCCESS : SOL_STOPPED; }

void narrate_init(const uint32_t x, const uint32_t y) {
    const float zoom = settings_zoom();
    winx = x; winy = y;

    sol_portrait_load();
    display = 0; // start off as off
    narrate = sol_window_from_gff(3007);
    sol_window_set_pos(narrate, winx, 0);
    sol_button_set_enabled(narrate->buttons + 2, 0);
    sol_button_set_enabled(narrate->buttons + 1, 0);

    menu = sol_window_from_gff(3008);
    sol_window_set_pos(menu, winx, settings_screen_height() - (57 * zoom));
}

void print_text() {
    const uint32_t len = 48;
    uint32_t amt = 48;
    size_t y = 16;
    uint32_t i = 0;
    while (i < text_pos) {
        amt = len;
        while (i + amt < text_pos && amt > 0 && narrate_text[i + amt] != ' ') { amt--; }
        sol_status_check(sol_print_line_len(0, narrate_text + i, 200, y, amt),
                "Unable to print.");
        i += amt;
        while (isspace(narrate_text[i])) { i++; }
        y += 16;
    }
}

void narrate_close_ui() {
    display = 0;
    clear();
}

void narrate_render(void *data) {
    (void) data;
    update_menu(0);
    sol_sprite_info_t info;

    if (display_menu) {
        sol_window_render_base(menu);
        sol_status_check(sol_sprite_get_info(menu->underlays[0].spr, &info), "Unable to get menu options sprite info");
        sol_status_check(sol_print_line_len(FONT_YELLOW, menu_options[0], winx + 3 * settings_zoom(), 
            info.y + 3 * settings_zoom(), 0x7FFFFFFF),
            "Unable to print.");
    }

    if (display) {
        sol_status_check(sol_sprite_get_info(narrate->buttons[3].spr, &info), "Unable to get window button sprite info");
        sol_window_render_base(narrate);
        sol_portrait_display(portrait_index,
            info.x + 8 * settings_zoom(),
            info.y + 7 * settings_zoom());
        print_text();
    }
}

extern sol_status_t sol_narrate_clear() {
    text_pos = 0;
    return SOL_SUCCESS;
}

static int add_text(const char *to_add) {
    if (!to_add) { return 0; }

    int len = strlen(to_add);

    if (text_pos + len >= MAX_TEXT) {
        error("Overflowed text buffer in narrate!");
        exit(1);
    }

    strcpy(narrate_text+text_pos, to_add);
    text_pos += len;

    return len;
}

int8_t sol_ui_narrate_open(int16_t action, const char *text, int16_t index) {
    display = 1; // by default we turn on display
    switch(action) {
        case NAR_ADD_MENU:
            display_menu = 1;
            //warn("I need to add_menu with index %d, text = '%s'\n", index, text);
            strncpy(menu_options[menu_pos], text, MAX_LINE);
            menu_options[menu_pos][MAX_LINE-1] = '\0'; // guard
            break;
        case NAR_PORTRAIT:
            portrait_index = index;
            // Empty text or 0 index on portrait is a clear dialog (no display.)
            display = (!add_text(text) || !index) ? 0 : 1;
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

static int narrate_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    size_t button = sol_window_get_button(menu, x, y);
    static size_t last_button = 9999;

    if (last_button <= 5 && last_button != button) {
        sol_button_set_font(menu->buttons + last_button, FONT_YELLOW);
    }

    if (button >= 1 && button <= 5) {
        sol_button_set_font(menu->buttons + button, FONT_YELLOW_BRIGHT);
        last_button = button;
    }

    return display; // zero means I did not handle the mouse, so another window may.
}

static int narrate_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    (void)button;(void)x;(void)y;
    return display;
}

static int narrate_handle_mouse_up(const uint32_t _button, const uint32_t x, const uint32_t y) {
    (void)_button;
    int option;
    size_t button = sol_window_get_button(menu, x, y);

    if (display_menu && button >= 1 && button <= 5) {
        sol_narrate_select_menu(button, &option);
    }

    if (button == 0) { // UP
        scroll_menu(1);
    }

    if (button == 6) { // DOWN
        scroll_menu(-1);
    }

    return display; // zero means I did not handle the mouse click, so another window may.
}

int narrate_handle_key_down(const enum sol_entity_action_e action) {
    if (!display) { return 0; }

    switch (action) {
        case EA_ACTIVATE:
            if (sol_game_loop_is_waiting_for(WAIT_NARRATE_CONTINUE) == SOL_SUCCESS) {
                sol_game_loop_signal(WAIT_NARRATE_CONTINUE, 0);
            }
        default:
        break;
    }
    return 1; // Handle the key
}

void narrate_free() {
    sol_window_free_base(narrate);
    sol_window_free_base(menu);
}

extern sol_status_t sol_narrate_ask_yes_no() {
    sol_narrate_open(NAR_ADD_MENU, "", 0);
    sol_narrate_open(NAR_ADD_MENU, "  Yes", 1);
    sol_narrate_open(NAR_ADD_MENU, "  No", 2);
    sol_game_loop_wait_for_signal(WAIT_NARRATE_SELECT);

    narrate_close_ui();
    return (last_option == 2) ? SOL_NO : SOL_SUCCESS;
}

extern sol_status_t sol_narrate_open(int16_t action, const char *text, int16_t index) {
    if (action == NAR_SHOW_TEXT) {
        if (strncmp("END", text, 3) == 0 || index == 0) {
            if (text_pos == 0) { return 0; }
            sol_game_loop_wait_for_signal(WAIT_NARRATE_CONTINUE);
            sol_narrate_clear();
            return SOL_SUCCESS;
        }
        if (strncmp("CLOSE", text, 5) == 0) {
            narrate_close_ui();
            return SOL_SUCCESS;
        }
    }
    sol_ui_narrate_open(action, text, index);
    switch(action) {
        case NAR_ADD_MENU:
            strncpy(menu_options[menu_pos], text, MAX_LINE);
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
            return SOL_NOT_FOUND;
    }

    return SOL_SUCCESS;
}

static int option_is_exit(const uint32_t option) {
    if (option >= menu_pos || option < 0) { return 0; }
    if (strncmp("Goodbye.", menu_options[option], 8) == 0) { return 1; }
    if (strncmp("END", menu_options[option], 3) == 0) { return 1; }
    if (strncmp("CLOSE", menu_options[option], 5) == 0) { return 1; }
    return 0;
}

extern sol_status_t sol_narrate_select_menu(uint32_t option, int *noption) {
    option += menu_start_pos;

    int accum = !option_is_exit(option);
    char buf[1024];

    //printf("menu_select: %d\n", option);
    last_option = option;
    if (option >= menu_pos || option < 0) {
        error("select_menu: Menu option %d selected, but only (0 - " PRI_SIZET ") available!\n", option, menu_pos - 1);
        return -1;
    }
    menu_pos = 0;
    clear();
    snprintf(buf, 1024, "func%d()\n", menu_addrs[option]);
    //game_loop_signal(WAIT_NARRATE_SELECT, accum);
    if (sol_gpl_in_exit() != SOL_SUCCESS) {
        sol_game_loop_signal(WAIT_NARRATE_SELECT, option);
        sol_gpl_execute_string(buf);
    }
    if (sol_gpl_in_exit() == SOL_SUCCESS || option >= 0) {
        sol_game_loop_signal(WAIT_NARRATE_SELECT, option);
    }
   
    if (noption) {*noption = accum;}
    return SOL_SUCCESS;
}

static int narrate_key_press (const sol_key_e e) {
    if (!display_menu) { return 0; }

    switch(e) {
        case SOLK_1: sol_narrate_select_menu(1, NULL); break;
        case SOLK_2: sol_narrate_select_menu(2, NULL); break;
        case SOLK_3: sol_narrate_select_menu(3, NULL); break;
        case SOLK_4: sol_narrate_select_menu(4, NULL); break;
        case SOLK_5: sol_narrate_select_menu(5, NULL); break;
        default: break;
    }

    return 1;
}

sol_wops_t narrate_window = {
    .init = narrate_init,
    .cleanup = narrate_free,
    .render = narrate_render,
    .mouse_movement = narrate_handle_mouse_movement,
    .mouse_down = narrate_handle_mouse_down,
    .key_down = narrate_handle_key_down,
    .key_press = narrate_key_press,
    .mouse_up = narrate_handle_mouse_up,
    .grey_out_map = 0,
    .data = NULL
};
