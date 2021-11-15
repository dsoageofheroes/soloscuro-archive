/* This is a debug program for testing the various available windows. */
#include <SDL2/SDL.h>
#include <stdio.h>
#include "window-manager.h"
#include "windows/add-load-save.h"
#include "windows/game-menu.h"
#include "windows/inventory.h"
#include "windows/interact.h"
#include "windows/narrate.h"
#include "windows/window-main.h"
#include "windows/view-character.h"
#include "windows/popup.h"
#include "windows/new-character.h"
#include "windows/combat-status.h"
#include "../src/dsl.h"
#include "../src/gameloop.h"
#include "../src/region.h"

static SDL_Renderer *renderer = NULL;
static SDL_Surface *surface = NULL;

void load_window(const char *arg) {
    if (!strcmp(arg, "main")) {
        window_push(renderer, &main_window, 10, 10);
    }
    if (!strcmp(arg, "view")) {
        window_push(renderer, &view_character_window, 0, 10);
        narrate_init(renderer, 0, 0); // to setup print_line
    }
    if (!strcmp(arg, "new")) {
        window_push(renderer, &new_character_window, 0, 0);
    }
    if (!strcmp(arg, "popup")) {
        narrate_init(renderer, 0, 0); // to setup print_line
        window_push(renderer, &popup_window, 10, 10);
        popup_set_message("Exit game?");
        popup_set_option(0, "Save");
        popup_set_option(1, "Load");
        popup_set_option(2, "Exit");
    }
    if (!strcmp(arg, "als")) {
        window_push(renderer, &als_window, 0, 0);
    }
    if (!strcmp(arg, "inv")) {
        window_push(renderer, &inventory_window, 0, 0);
    }
    if (!strcmp(arg, "menu")) {
        window_push(renderer, &game_menu_window, 0, 0);
    }
    if (!strcmp(arg, "interact")) {
        window_push(renderer, &interact_window, 0, 0);
    }
    if (!strcmp(arg, "combat")) {
        combat_status_t* cs = combat_status_get();
        strcpy(cs->name, "Tex");
        cs->current_hp = 10;
        cs->max_hp = 20;
        cs->status = 1;
        cs->move = 12;
        window_push(renderer, &combat_status_window, 295, 5);
    }
}

void window_debug_init(SDL_Surface *sur, SDL_Renderer *rend, const char *arg) {
    surface = sur;
    renderer = rend;

    window_init(renderer);

    load_window(arg);
}
