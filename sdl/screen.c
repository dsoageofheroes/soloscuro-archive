/* This is a debug program for testing the various available screens. */
#include <SDL2/SDL.h>
#include <stdio.h>
#include "gameloop.h"
#include "screen-manager.h"
#include "screens/main.h"
#include "../src/dsl.h"

static int done = 0;
static uint32_t last_tick = 0;
static const uint32_t TICK_AMT = 1000 / TICKS_PER_SEC;// Not fully correct...
static SDL_Renderer *renderer = NULL;
static SDL_Surface *surface = NULL;

static void screen_tick();
static void screen_handle_input();
static void screen_test_render();

void load_screen(const char *arg) {
    if (!strcmp(arg, "main")) {
        screen_load_screen(renderer, 1, &main_screen);
    }
}

void screen_loop(SDL_Surface *sur, SDL_Renderer *rend, const char *arg) {
    surface = sur;
    renderer = rend;

    screen_init(renderer);

    load_screen(arg);
    
    screen_test_render();

    while (!done) {
        screen_handle_input();
        screen_test_render();
        screen_tick();
    }
}

static void screen_tick() {
    uint32_t cticks = SDL_GetTicks();
    uint32_t amt_to_wait = 0;
    last_tick += TICK_AMT;
    //debug("last_time = %u, cticks = %u\n", last_tick, cticks);
    if (last_tick < cticks) {
        debug("gameloop took too long, not sleeping...\n");
    } else {
        amt_to_wait = last_tick - cticks;
        //debug("going to wait %ums\n", amt_to_wait);
        SDL_Delay(amt_to_wait);
    }
}

static void screen_handle_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch(event.type) {
            case SDL_QUIT:
                done = 1;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: done = 1; break;
                }
                break;
                /*
                    case SDLK_PAGEDOWN: move_gff_cursor(1); break;
                    case SDLK_PAGEUP: move_gff_cursor(-1); break;
                    case SDLK_DOWN: move_entry_cursor(1); break;
                    case SDLK_UP: move_entry_cursor(-1); break;
                    case SDLK_RIGHT: move_res_cursor(1); break;
                    case SDLK_LEFT: move_res_cursor(-1); break;
                    case SDLK_w: write_blob(); break;
                    case SDLK_f: move_frame_cursor(1); mapy += 1; cobj++;break;
                    case SDLK_d: mapx += 1; break;
                    case SDLK_s: mapy -= 1; cobj--; break;
                    case SDLK_e: mapx -= 1; break;
                    case SDLK_KP_MINUS: if (zoom > 1.0) {zoom -= 0.25;} break;
                    case SDLK_KP_PLUS: zoom += 0.25; break;
                }
                break;
            case SDL_MOUSEMOTION:
                //handle_mouse_motion();
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (game_loop_is_waiting_for(WAIT_NARRATE_CONTINUE)) {
                    game_loop_signal(WAIT_NARRATE_CONTINUE, 0);
                }
                //handle_mouse_click();
                break;
                */
        }
    }
}

static void screen_test_render() {
    SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear(renderer);

    screen_render(renderer, 0, 0);

    SDL_RenderPresent(renderer);
}
