/* This is a debug program for testing the various available windows. */
#include <SDL2/SDL.h>
#include <stdio.h>
#include "window-manager.h"
#include "add-load-save.h"
#include "game-menu.h"
#include "inventory.h"
#include "interact.h"
#include "narrate.h"
#include "window-main.h"
#include "view-character.h"
#include "popup.h"
#include "new-character.h"
#include "combat-status.h"
#include "gpl.h"
#include "gameloop.h"
#include "region.h"
#include "sprite.h"

static SDL_Renderer *renderer = NULL;
static SDL_Surface *surface = NULL;

void load_window(const char *arg) {
    if (!strcmp(arg, "main")) {
        sol_window_push(&main_window, 10, 10);
    }
    if (!strcmp(arg, "view")) {
        sol_window_push(&view_character_window, 0, 10);
        narrate_init(0, 0); // to setup print_line
    }
    if (!strcmp(arg, "new")) {
        sol_window_push(&new_character_window, 0, 0);
    }
    if (!strcmp(arg, "popup")) {
        narrate_init(0, 0); // to setup print_line
        sol_window_push(&popup_window, 10, 10);
        sol_popup_set_message("Exit game?");
        sol_popup_set_option(0, "Save");
        sol_popup_set_option(1, "Load");
        sol_popup_set_option(2, "Exit");
    }
    if (!strcmp(arg, "als")) {
        sol_window_push(&als_window, 0, 0);
    }
    if (!strcmp(arg, "inv")) {
        sol_window_push(&inventory_window, 0, 0);
    }
    if (!strcmp(arg, "menu")) {
        sol_window_push(&game_menu_window, 0, 0);
    }
    if (!strcmp(arg, "interact")) {
        sol_window_push(&interact_window, 0, 0);
    }
    if (!strcmp(arg, "combat")) {
        combat_status_t* cs = sol_combat_status_get();
        strcpy(cs->name, "Tex");
        cs->current_hp = 10;
        cs->max_hp = 20;
        cs->status = 1;
        cs->move = 12;
        sol_window_push(&combat_status_window, 295, 5);
    }
}
