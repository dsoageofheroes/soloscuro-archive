#include <SDL2/SDL.h>
#include <stdio.h>
#include "map.h"
#include "screen-manager.h"
#include "../src/dsl.h"
#include "../src/gameloop.h"

static char *ds1_gffs = NULL;
static uint32_t last_tick = 0;
static const uint32_t TICK_AMT = 1000 / TICKS_PER_SEC;// Not fully correct...
static SDL_Window *win = NULL;
static SDL_Surface *screen = NULL;
static SDL_Renderer *renderer = NULL;
static void free_everything();

static int xmappos = 560;
static int ymappos = 32;

void parse_args(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--ds1") && i < (argc-1)) {
            ds1_gffs = argv[++i];
        }
    }

    if (!ds1_gffs) {
        error("Unable to get the location of the DarkSun 1 GFFs, please pass with '--dsl1 <location>'\n");
        exit(1);
    }
}


void handle_input() {
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            signal_exit();
        }

        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                signal_exit();
            }
        }
    }

    if(key_state[SDL_SCANCODE_DOWN])  { ymappos += 2; }
    if(key_state[SDL_SCANCODE_UP])    { ymappos -= 2; }
    if(key_state[SDL_SCANCODE_LEFT])  { xmappos -= 2; }
    if(key_state[SDL_SCANCODE_RIGHT]) { xmappos += 2; }
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

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    // Order matters.
    gff_init();
    gff_load_directory(ds1_gffs);
    dsl_init();

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

    game_loop();

    free_everything();

    return 0;
}

void free_everything() {
    screen_free();

    //Destroy window
    SDL_DestroyWindow( win );

    //Quit SDL subsystems
    SDL_Quit();

}
