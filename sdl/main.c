#include <SDL2/SDL.h>
#include <stdio.h>
#include "map.h"
#include "player.h"
#include "lua.h"
#include "mouse.h"
#include "textbox.h"
#include "screen-manager.h"
#include "gameloop.h"
#include "screens/narrate.h"
#include "screens/screen-main.h"
#include "screens/inventory.h"
#include "screens/add-load-save.h"
#include "screens/view-character.h"
#include "../src/combat.h"
#include "../src/dsl.h"
#include "../src/dsl-manager.h"
#include "../src/replay.h"
#include "../src/region-manager.h"
#include "../src/ds-load-save.h"
#include "../src/ds-player.h"

void browse_loop(SDL_Surface*, SDL_Renderer *rend);
void screen_debug_init(SDL_Surface *sur, SDL_Renderer *rend, const char *arg);
void export_all_images(const char *filename);
void export_all_items(const char *base_path);

static uint32_t last_tick = 0;
static const uint32_t TICK_AMT = 1000 / TICKS_PER_SEC;// Not fully correct...
static SDL_Window *win = NULL;
static SDL_Surface *screen = NULL;
static SDL_Renderer *renderer = NULL;
static float zoom = 2.0;
static uint8_t ignore_repeat = 1, browser_mode = 0;

static uint32_t xmappos, ymappos;
static int32_t xmapdiff, ymapdiff;

static uint8_t player_directions[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static textbox_t *textbox = NULL;

void main_set_textbox(textbox_t *tb) {
    textbox = tb;
}

SDL_Renderer *main_get_rend() { return renderer; }
SDL_Surface *main_get_screen() { return screen; }
const float main_get_zoom() { return zoom; }

const uint32_t getCameraX() { return xmappos; }
const uint32_t getCameraY() { return ymappos; }

uint32_t main_get_width() {
    return 800;
}

uint32_t main_get_height() {
    return 600;
}

void main_set_browser_mode() {
    browser_mode = 1;
}

void handle_mouse_motion() {
    int x, y;

    SDL_GetMouseState(&x, &y);

    screen_handle_mouse(x, y);
}

void handle_mouse_down(uint32_t button) {
    int x, y;

    SDL_GetMouseState(&x, &y);

    screen_handle_mouse_down(button, x, y);
}

void handle_mouse_up(uint32_t button) {
    int x, y;

    SDL_GetMouseState(&x, &y);

    screen_handle_mouse_up(button, x, y);
}

void main_exit_game() {
    game_loop_signal(WAIT_FINAL, 0);
}

static void main_combat_update() {
    combat_action_t player_action;
    player_action.action = CA_NONE;
    switch(combat_player_turn()) {
        case PLAYER1_TURN:
        case PLAYER2_TURN:
        case PLAYER3_TURN:
        case PLAYER4_TURN:
            if (player_directions[1]) { player_action.action = CA_WALK_DOWNLEFT; }
            if (player_directions[2]) { player_action.action = CA_WALK_DOWN; }
            if (player_directions[3]) { player_action.action = CA_WALK_DOWNRIGHT; }
            if (player_directions[4]) { player_action.action = CA_WALK_LEFT; }
            if (player_directions[6]) { player_action.action = CA_WALK_RIGHT; }
            if (player_directions[7]) { player_action.action = CA_WALK_UPLEFT; }
            if (player_directions[8]) { player_action.action = CA_WALK_UP; }
            if (player_directions[9]) { player_action.action = CA_WALK_UPRIGHT; }
            combat_player_action(player_action);
            break;
        default: break;
    }
}

void handle_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch(event.type) {
            case SDL_QUIT:
                main_exit_game();
                break;
            case SDL_KEYUP:
                if (ignore_repeat && event.key.repeat != 0) { break; }
                if (textbox_handle_keyup(textbox, event.key.keysym)) { return; }
                if (ui_lua_keyup(event.key.keysym.sym)) { break; }
                if (event.key.keysym.sym == SDLK_s) { player_unmove(PLAYER_LEFT); }
                if (event.key.keysym.sym == SDLK_e) { player_unmove(PLAYER_UP); }
                if (event.key.keysym.sym == SDLK_f) { player_unmove(PLAYER_RIGHT); }
                if (event.key.keysym.sym == SDLK_d) { player_unmove(PLAYER_DOWN); }
                if (event.key.keysym.sym == SDLK_UP) { ymapdiff = 0;}
                if (event.key.keysym.sym == SDLK_DOWN) { ymapdiff = 0;}
                if (event.key.keysym.sym == SDLK_LEFT) { xmapdiff = 0;}
                if (event.key.keysym.sym == SDLK_RIGHT) { xmapdiff = 0;}
                if (event.key.keysym.sym == SDLK_KP_1) { player_directions[1] = 0; }
                if (event.key.keysym.sym == SDLK_KP_2) { player_directions[2] = 0; }
                if (event.key.keysym.sym == SDLK_KP_3) { player_directions[3] = 0; }
                if (event.key.keysym.sym == SDLK_KP_4) { player_directions[4] = 0; }
                if (event.key.keysym.sym == SDLK_KP_6) { player_directions[6] = 0; }
                if (event.key.keysym.sym == SDLK_KP_7) { player_directions[7] = 0; }
                if (event.key.keysym.sym == SDLK_KP_8) { player_directions[8] = 0; }
                if (event.key.keysym.sym == SDLK_KP_9) { player_directions[9] = 0; }
                if (event.key.keysym.sym == SDLK_F11) {
                    add_load_save_set_mode(ACTION_SAVE);
                    screen_push_screen(renderer, &als_screen, 0, 0);
                }
                if (event.key.keysym.sym == SDLK_F12) {
                    add_load_save_set_mode(ACTION_LOAD);
                    screen_push_screen(renderer, &als_screen, 0, 0);
                }
                break;
                break;
            case SDL_KEYDOWN:
                if (ignore_repeat && event.key.repeat != 0) { break; }
                if (textbox_handle_keydown(textbox, event.key.keysym)) { return; }
                if (ui_lua_keydown(event.key.keysym.sym)) { break; }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    game_loop_signal(WAIT_FINAL, 0);
                }
                if (event.key.keysym.sym == SDLK_i) {
                    screen_toggle_screen(renderer, &inventory_screen, 0, 0);
                }
                if (event.key.keysym.sym == SDLK_c) {
                    screen_toggle_screen(renderer, &view_character_screen, 0, 0);
                }
                if (event.key.keysym.sym == SDLK_s) { player_move(PLAYER_LEFT); }
                if (event.key.keysym.sym == SDLK_e) { player_move(PLAYER_UP); }
                if (event.key.keysym.sym == SDLK_f) { player_move(PLAYER_RIGHT); }
                if (event.key.keysym.sym == SDLK_d) { player_move(PLAYER_DOWN); }
                if (event.key.keysym.sym == SDLK_UP) { ymapdiff = -2;}
                if (event.key.keysym.sym == SDLK_DOWN) { ymapdiff = 2;}
                if (event.key.keysym.sym == SDLK_LEFT) { xmapdiff = -2;}
                if (event.key.keysym.sym == SDLK_RIGHT) { xmapdiff = 2;}
                if (event.key.keysym.sym == SDLK_KP_1) { player_directions[1] = 1; }
                if (event.key.keysym.sym == SDLK_KP_2) { player_directions[2] = 1; }
                if (event.key.keysym.sym == SDLK_KP_3) { player_directions[3] = 1; }
                if (event.key.keysym.sym == SDLK_KP_4) { player_directions[4] = 1; }
                if (event.key.keysym.sym == SDLK_KP_6) { player_directions[6] = 1; }
                if (event.key.keysym.sym == SDLK_KP_7) { player_directions[7] = 1; }
                if (event.key.keysym.sym == SDLK_KP_8) { player_directions[8] = 1; }
                if (event.key.keysym.sym == SDLK_KP_9) { player_directions[9] = 1; }
                break;
            case SDL_MOUSEMOTION:
                handle_mouse_motion();
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (game_loop_is_waiting_for(WAIT_NARRATE_CONTINUE)) {
                    narrate_clear();
                    game_loop_signal(WAIT_NARRATE_CONTINUE, 0);
                }
                handle_mouse_down(event.button.button);
                break;
            case SDL_MOUSEBUTTONUP:
                handle_mouse_up(event.button.button);
                break;
        }
    }

    xmappos += xmapdiff;
    ymappos += ymapdiff;
    handle_mouse_motion();
    if (region_manager_get_current()) {
        player_update();
        main_combat_update();
    }
}

void main_set_xscroll(int amt) { xmapdiff = amt; }
void main_set_yscroll(int amt) { ymapdiff = amt; }
void main_set_ignore_repeat(int repeat) { ignore_repeat = repeat; }

void main_center_on_player() {
    int w, h;

    SDL_GetRendererOutputSize(renderer, &w, &h);
    dude_t *dude = player_get_active();

    xmappos = dude->mapx * 16 * main_get_zoom() - w / 2;
    ymappos = dude->mapy * 16 * main_get_zoom() - h / 2;
}

void render() {
    region_tick(region_manager_get_current());
    combat_update(region_manager_get_current());
    screen_render(renderer, xmappos, ymappos);
}

// Simple timing for now...
void tick() {
    uint32_t cticks = SDL_GetTicks();
    uint32_t amt_to_wait = 0;
    last_tick += TICK_AMT;
    //debug("last_time = %u, cticks = %u\n", last_tick, cticks);
    if (last_tick < cticks) {
        //debug("gameloop took too long, not sleeping...\n");
    } else {
        amt_to_wait = last_tick - cticks;
        //debug("going to wait %ums\n", amt_to_wait);
        SDL_Delay(amt_to_wait);
    }
}

void port_init(int argc, char *argv[]) {
}

void port_cleanup() {
}

static int sdl_init(const int what) {
    return SDL_Init(what);
}

static void init(int args, char *argv[]) {
    int run_lua = 1;
    xmappos = 560;
    ymappos = 50;
    xmapdiff = ymapdiff = 0;

    if (sdl_init( SDL_INIT_VIDEO) ) {
        error( "Unable to init video!\n");
        exit(1);
    }
    win = SDL_CreateWindow( "Dark Sun: Shattered Lands", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, main_get_width(), main_get_height(), SDL_WINDOW_SHOWN );
    if( win == NULL ) {
        error( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        exit(1);
    }

    //Get window surface
    screen = SDL_GetWindowSurface( win );

    renderer = SDL_CreateRenderer(win, -1, 0);

    last_tick = SDL_GetTicks();

    gameloop_init();

    screen_init(renderer);

    player_init();
    mouse_init(renderer);

    if (browser_mode) {
        browse_loop(screen, renderer);
        exit(1);
    }

    for (int i = 0; i < args; i++) {
        if (!strcmp(argv[i], "--browse") && i < (args)) {
            printf("Entering browsing mode!\n");
            browse_loop(screen, renderer);
            exit(1);
        }
        if (!strcmp(argv[i], "--screen") && i < (args - 1)) {
            printf("Entering screen mode!\n");
            screen_debug_init(screen, renderer, argv[i + 1]);
            return;
        }
        if (!strcmp(argv[i], "--extract-images") && i < (args - 1)) {
            export_all_images(argv[i + 1]);
            exit(0);
        }
        if (!strcmp(argv[i], "--extract-items") && i < (args - 1)) {
            export_all_items(argv[i + 1]);
            exit(0);
        }
        if (!strcmp(argv[i], "--extract-lua") && i < (args - 1)) {
            dsl_lua_load_all_scripts();
            exit(0);
        }
        if (!strcmp(argv[i], "--ignore-lua")) {
            run_lua = 0;
        }
    }

    if (run_lua && ui_lua_load("lua/main.lua") ) {
        printf("Init being handled by lua.\n");
        return;
    }

    ls_load_save_file("save00.sav");
    main_center_on_player();
    // Start the main game.
    //screen_push_screen(renderer, &main_screen, 0, 0);
}

static void cleanup() {
    ui_lua_close();
    // Order matters.
    player_close();
    screen_free();

    dsl_cleanup();
    gff_cleanup();
    mouse_free();

    SDL_DestroyRenderer(renderer);
    //SDL_DestroySurface(screen);
    SDL_DestroyWindow( win );

    //Quit SDL subsystems
    SDL_Quit();
}

static char *ds1_gffs = NULL;
static char *replay = NULL;

void parse_args(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--ds1") && i < (argc-1)) {
            ds1_gffs = argv[++i];
        }
        if (!strcmp(argv[i], "--replay") && i < (argc-1)) {
            replay = argv[++i];
        }
    }

    replay_init("replay.lua");
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    // Order matters.
    gff_init();
    ui_lua_load_preload("lua/settings.lua");
    if (gff_get_game_type() == DARKSUN_UNKNOWN) {
        if (!ds1_gffs) {
            error("Unable to get the location of the DarkSun 1 GFFs, please pass with '--ds1 <location>'\n");
            exit(1);
        }
        gff_load_directory(ds1_gffs);
    }
    dsl_init();

    init(argc, argv);

    if (replay) {
        replay_game(replay);
    }

    game_loop();

    cleanup();
    replay_cleanup();

    return 0;
}

static int done = 0;
static int accum = 0;
static uint8_t wait_flags[WAIT_MAX_SIGNALS];

typedef struct animation_s {
    struct animation_s *next;
} animation_t;

void gameloop_init() {
    memset(wait_flags, 0x0, sizeof(uint8_t) * WAIT_MAX_SIGNALS);
    wait_flags[WAIT_FINAL] = 1;
}

int main_player_freeze() {
    return wait_flags[WAIT_NARRATE_CONTINUE]
        || wait_flags[WAIT_NARRATE_SELECT];
}

//static animation_t *animations[TICKS_PER_SEC];
int game_loop_is_waiting_for(int signal) {
    if (signal < 0 || signal >= WAIT_MAX_SIGNALS) {
        error("Received signal %d!\n", signal);
        return 0;
    }

    return wait_flags[signal];
}

void game_loop_signal(int signal, int _accum) {
    if (signal < 0 || signal >= WAIT_MAX_SIGNALS) {
        error("Received signal %d!\n", signal);
        return;
    }
    replay_print("rep.signal(%d, %d)\n", signal, _accum);
    if (wait_flags[signal]) {
        wait_flags[signal]--;
        accum = _accum;
        done = 1;
    } else {
        warn("signal %d received, but not waiting on it...\n", signal);
        done = 1;
    }
}

int game_loop_wait_for_signal(int signal) {
    if (signal < 0 || signal >= WAIT_MAX_SIGNALS) {
        error("Received signal %d!\n", signal);
        return 0 ;
    }
    wait_flags[signal]++;
    while (wait_flags[signal]) {
        handle_input();
        //Logic here...
        render();
        tick();
    }
    done = 0;
    return accum;
}

void game_loop() {
    int rep_times = 0;

    while (!done) {
        handle_input();
        //Logic here...
        render();
        tick();
        rep_times++;
        if (in_replay_mode() && rep_times > 10) {
            replay_next();
            rep_times = 0;
        }
    }
}

void main_exit_system() {
    /*
    */
    game_loop_signal(WAIT_FINAL, 0);
}
