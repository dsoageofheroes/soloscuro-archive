#include <SDL2/SDL.h>
#include <stdio.h>
#include "map.h"
#include "player.h"
#include "screen-manager.h"
#include "../src/port.h"
#include "../src/dsl.h"
#include "../src/gameloop.h"

static uint32_t last_tick = 0;
static const uint32_t TICK_AMT = 1000 / TICKS_PER_SEC;// Not fully correct...
static SDL_Window *win = NULL;
static SDL_Surface *screen = NULL;
static SDL_Renderer *renderer = NULL;

static uint32_t xmappos, ymappos;

const uint32_t getCameraX() { return xmappos; }
const uint32_t getCameraY() { return ymappos; }


void handle_mouse_motion() {
    int x, y;

    SDL_GetMouseState(&x, &y);

    screen_handle_mouse(x, y);
}

void handle_mouse_click() {
    int x, y;

    SDL_GetMouseState(&x, &y);

    screen_handle_mouse_click(x, y);
}

void handle_input() {
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch(event.type) {
            case SDL_QUIT:
                game_loop_signal(WAIT_FINAL, 0);
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    game_loop_signal(WAIT_FINAL, 0);
                }
                break;
            case SDL_MOUSEMOTION:
                handle_mouse_motion();
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (game_loop_is_waiting_for(WAIT_NARRATE_CONTINUE)) {
                    game_loop_signal(WAIT_NARRATE_CONTINUE, 0);
                }
                handle_mouse_click();
                break;
        }
    }

    if(key_state[SDL_SCANCODE_DOWN])  { ymappos += 2; handle_mouse_motion(); }
    if(key_state[SDL_SCANCODE_UP])    { ymappos -= 2; handle_mouse_motion(); }
    if(key_state[SDL_SCANCODE_LEFT])  { xmappos -= 2; handle_mouse_motion(); }
    if(key_state[SDL_SCANCODE_RIGHT]) { xmappos += 2; handle_mouse_motion(); }
    if(key_state[SDL_SCANCODE_D])     { player_move(PLAYER_DOWN); }
    if(key_state[SDL_SCANCODE_E])     { player_move(PLAYER_UP); }
    if(key_state[SDL_SCANCODE_S])     { player_move(PLAYER_LEFT); }
    if(key_state[SDL_SCANCODE_F])     { player_move(PLAYER_RIGHT); }
}

void render() {
    //map_render(&cmap, renderer, xmappos, ymappos);
    screen_render(renderer, xmappos, ymappos);
    //map_blit(&cmap, screen);
    //SDL_UpdateWindowSurface(win);
}

// Simple timing for now...
void tick() {
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

void port_init(int args, char *argv[]) {
    xmappos = 560;
    ymappos = 50;

    if (SDL_Init( SDL_INIT_VIDEO) ) {
        error( "Unable to init video!\n");
        exit(1);
    }
    win = SDL_CreateWindow( "Dark Sun: Shattered Lands", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN );
    if( win == NULL ) {
        error( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        exit(1);
    }

    //Get window surface
    screen = SDL_GetWindowSurface( win );

    renderer = SDL_CreateRenderer(win, -1, 0);

    last_tick = SDL_GetTicks();

    screen_init(renderer);
}

int main(int argc, char *argv[]) {
    pmain(argc, argv);

    return 0;
}

void port_cleanup() {
    screen_free();

    //Destroy window
    SDL_DestroyWindow( win );

    //Quit SDL subsystems
    SDL_Quit();

}
