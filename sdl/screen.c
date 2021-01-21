/* This is a debug program for testing the various available screens. */
#include <SDL2/SDL.h>
#include <stdio.h>
#include "gameloop.h"
#include "screen-manager.h"
#include "screens/screen-main.h"
#include "../src/dsl.h"

static SDL_Renderer *renderer = NULL;
static SDL_Surface *surface = NULL;

void load_screen(const char *arg) {
    if (!strcmp(arg, "main")) {
        screen_load_screen(renderer, 1, &main_screen, 10, 10);
    }
}

void screen_debug_init(SDL_Surface *sur, SDL_Renderer *rend, const char *arg) {
    surface = sur;
    renderer = rend;

    screen_init(renderer);

    load_screen(arg);
}
