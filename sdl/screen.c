/* This is a debug program for testing the various available screens. */
#include <SDL2/SDL.h>
#include <stdio.h>
#include "gameloop.h"
#include "screen-manager.h"
#include "screens/add-load-save.h"
#include "screens/game-menu.h"
#include "screens/inventory.h"
#include "screens/narrate.h"
#include "screens/screen-main.h"
#include "screens/view-character.h"
#include "screens/popup.h"
#include "screens/new-character.h"
#include "screens/combat-status.h"
#include "../src/dsl.h"
#include "../src/region.h"

static SDL_Renderer *renderer = NULL;
static SDL_Surface *surface = NULL;

void load_screen(const char *arg) {
    if (!strcmp(arg, "main")) {
        screen_push_screen(renderer, &main_screen, 10, 10);
    }
    if (!strcmp(arg, "view")) {
        screen_push_screen(renderer, &view_character_screen, 0, 10);
        narrate_init(renderer, 0, 0); // to setup print_line
    }
    if (!strcmp(arg, "new")) {
        screen_push_screen(renderer, &new_character_screen, 0, 0);
    }
    if (!strcmp(arg, "popup")) {
        narrate_init(renderer, 0, 0); // to setup print_line
        screen_push_screen(renderer, &popup_screen, 10, 10);
        popup_set_message("Exit game?");
        popup_set_option(0, "Save");
        popup_set_option(1, "Load");
        popup_set_option(2, "Exit");
    }
    if (!strcmp(arg, "als")) {
        screen_push_screen(renderer, &als_screen, 0, 0);
    }
    if (!strcmp(arg, "inv")) {
        screen_push_screen(renderer, &inventory_screen, 0, 0);
    }
    if (!strcmp(arg, "menu")) {
        screen_push_screen(renderer, &game_menu_screen, 0, 0);
    }
    if (!strcmp(arg, "combat")) {
        combat_status_t* cs = combat_status_get();
        strcpy(cs->name, "Tex");
        cs->current_hp = 10;
        cs->max_hp = 20;
        cs->status = 1;
        cs->move = 12;
        screen_push_screen(renderer, &combat_status_screen, 295, 5);
    }
}

void screen_debug_init(SDL_Surface *sur, SDL_Renderer *rend, const char *arg) {
    surface = sur;
    renderer = rend;

    screen_init(renderer);

    load_screen(arg);
}
