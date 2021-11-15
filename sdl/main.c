#include <SDL2/SDL.h>
#include <stdio.h>
#include "map.h"
#include "audio.h"
#include "font.h"
#include "player.h"
#include "../src/lua-inc.h"
#include "../src/settings.h"
#include "mouse.h"
#include "textbox.h"
#include "window-manager.h"
#include "../src/gameloop.h"
#include "windows/narrate.h"
#include "windows/window-main.h"
#include "windows/inventory.h"
#include "windows/add-load-save.h"
#include "windows/view-character.h"
#include "windows/game-menu.h"
#include "../src/combat.h"
#include "../src/dsl.h"
#include "../src/dsl-manager.h"
#include "../src/replay.h"
#include "../src/region-manager.h"
#include "../src/ds-load-save.h"
#include "../src/player.h"
#include "../src/port.h"
#include "../src/sol-lua.h"

void browse_loop(SDL_Surface*, SDL_Renderer *rend);
void window_debug_init(SDL_Surface *sur, SDL_Renderer *rend, const char *arg);
void export_all_images(const char *filename);
void export_all_items(const char *base_path);
void export_all_xmis(const char *base_path);

static uint32_t last_tick = 0;
static const uint32_t TICK_AMT = 1000 / TICKS_PER_SEC;// Not fully correct...
static SDL_Window *win = NULL;
static SDL_Surface *window = NULL;
static SDL_Renderer *renderer = NULL;
//static float zoom = 2.0;
static uint8_t ignore_repeat = 1, browser_mode = 0;
static int show_debug = 0;

static uint32_t xmappos, ymappos;
static int32_t xmapdiff, ymapdiff;

static uint8_t player_directions[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static textbox_t *textbox = NULL;

void main_set_textbox(textbox_t *tb) {
    textbox = tb;
}

SDL_Renderer *main_get_rend() { return renderer; }
SDL_Surface *main_get_window() { return window; }

extern uint32_t getCameraX() { return xmappos; }
extern uint32_t getCameraY() { return ymappos; }
extern int main_get_debug() { return show_debug; }

static void main_toggle_debug() { show_debug = !show_debug; }

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

    window_handle_mouse(x, y);
}

void handle_mouse_down(uint32_t button) {
    int x, y;

    SDL_GetMouseState(&x, &y);

    window_handle_mouse_down(button, x, y);
}

void handle_mouse_up(uint32_t button) {
    int x, y;

    SDL_GetMouseState(&x, &y);

    window_handle_mouse_up(button, x, y);
}

void main_exit_game() {
    sol_game_loop_signal(WAIT_FINAL, 0);
}

static void main_combat_update() {
    entity_action_t player_action;
    player_action.action = EA_NONE;
    switch(combat_player_turn()) {
        case PLAYER1_TURN:
        case PLAYER2_TURN:
        case PLAYER3_TURN:
        case PLAYER4_TURN:
            if (player_directions[1]) { player_action.action = EA_WALK_DOWNLEFT; }
            if (player_directions[2]) { player_action.action = EA_WALK_DOWN; }
            if (player_directions[3]) { player_action.action = EA_WALK_DOWNRIGHT; }
            if (player_directions[4]) { player_action.action = EA_WALK_LEFT; }
            if (player_directions[6]) { player_action.action = EA_WALK_RIGHT; }
            if (player_directions[7]) { player_action.action = EA_WALK_UPLEFT; }
            if (player_directions[8]) { player_action.action = EA_WALK_UP; }
            if (player_directions[9]) { player_action.action = EA_WALK_UPRIGHT; }
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
                if (sol_lua_keyup(event.key.keysym.sym)) { break; }
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
                    window_push(renderer, &als_window, 0, 0);
                }
                if (event.key.keysym.sym == SDLK_F12) {
                    add_load_save_set_mode(ACTION_LOAD);
                    window_push(renderer, &als_window, 0, 0);
                }
                break;
            case SDL_KEYDOWN:
                if (window_handle_key_down(event.key.keysym)) { break; }
                if (ignore_repeat && event.key.repeat != 0) { break; }
                if (textbox_handle_keydown(textbox, event.key.keysym)) { return; }
                if (sol_lua_keydown(event.key.keysym.sym)) { break; }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    sol_game_loop_signal(WAIT_FINAL, 0);
                }
                if (event.key.keysym.sym == SDLK_TAB) {
                    window_toggle(renderer, &game_menu_window, 0, 0);
                }
                if (event.key.keysym.sym == SDLK_i) {
                    window_toggle(renderer, &inventory_window, 0, 0);
                }
                if (event.key.keysym.sym == SDLK_c) {
                    window_toggle(renderer, &view_character_window, 0, 0);
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
                if (event.key.keysym.sym == SDLK_SPACE) { main_toggle_debug(); }
                break;
            case SDL_MOUSEMOTION:
                handle_mouse_motion();
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (sol_game_loop_is_waiting_for(WAIT_NARRATE_CONTINUE)) {
                    narrate_clear();
                    sol_game_loop_signal(WAIT_NARRATE_CONTINUE, 0);
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

void port_handle_input() {
    handle_input();
}

void main_set_xscroll(int amt) { xmapdiff = amt; }
void main_set_yscroll(int amt) { ymapdiff = amt; }
void main_set_ignore_repeat(int repeat) { ignore_repeat = repeat; }

void main_center_on_player() {
    int w, h;

    SDL_GetRendererOutputSize(renderer, &w, &h);
    dude_t *dude = player_get_active();

    xmappos = dude->mapx * 16 * settings_zoom() - w / 2;
    ymappos = dude->mapy * 16 * settings_zoom() - h / 2;
}

void port_window_render() {
    window_render(renderer, xmappos, ymappos);
}

void render() {
    region_tick(region_manager_get_current());
    combat_update(region_manager_get_current());
    window_render(renderer, xmappos, ymappos);
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

void port_tick() { tick(); }

void port_cleanup() {
}

static int sdl_init(const int what) {
    return SDL_Init(what);
}

static void gui_init() {
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
    window = SDL_GetWindowSurface( win );

    renderer = SDL_CreateRenderer(win, -1, 0);

    last_tick = SDL_GetTicks();

    window_init(renderer);
}

void port_init() {
    audio_init();
    gui_init();

    font_init(renderer);

    sol_gameloop_init();

    player_init();
    mouse_init(renderer);
}

void port_close() {
    // Order matters.
    audio_cleanup();
    player_close();
    window_free();

    dsl_cleanup();
    gff_cleanup();
    mouse_free();

    SDL_DestroyRenderer(renderer);
    //SDL_DestroySurface(window);
    SDL_DestroyWindow( win );

    //Quit SDL subsystems
    SDL_Quit();
}

static void cleanup() {
    port_close();
    sol_lua_close();
}

extern void port_game_loop() {
    sol_game_loop();
}

extern void port_start() {
    port_init();
    map_load_region(region_manager_get_current(), renderer);

    port_game_loop();

    cleanup();
    replay_cleanup();
}

static void init(int args, char *argv[]) {
    int run_lua = 1;

    for (int i = 0; i < args; i++) {
        if (!strcmp(argv[i], "--lua") && i < (args - 1)) {
            sol_lua_load(argv[i + 1]);
            cleanup();
            replay_cleanup();
            exit(0);
        }
    }

    gui_init();

    font_init(renderer);

    sol_gameloop_init();

    player_init();
    mouse_init(renderer);
    audio_init();

    if (browser_mode) {
        browse_loop(window, renderer);
        exit(1);
    }

    for (int i = 0; i < args; i++) {
        if (!strcmp(argv[i], "--browse") && i < (args)) {
            printf("Entering browsing mode!\n");
            browse_loop(window, renderer);
            exit(1);
        }
        if (!strcmp(argv[i], "--window") && i < (args - 1)) {
            printf("Entering window mode!\n");
            window_debug_init(window, renderer, argv[i + 1]);
            return;
        }
        if (!strcmp(argv[i], "--extract-images") && i < (args - 1)) {
            export_all_images(argv[i + 1]);
            exit(0);
        }
        if (!strcmp(argv[i], "--extract-xmis") && i < (args - 1)) {
            export_all_xmis(argv[i + 1]);
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

    if (run_lua && sol_lua_load("lua/main.lua") ) {
        printf("Init being handled by lua.\n");
        return;
    }

    ls_load_save_file("save00.sav");
    main_center_on_player();
    // Start the main game.
    //window_push(renderer, &main_window, 0, 0);
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
    sol_lua_load_preload("lua/settings.lua");
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

    sol_game_loop();

    cleanup();
    replay_cleanup();

    return 0;
}

typedef struct animation_s {
    struct animation_s *next;
} animation_t;

void main_exit_system() {
    sol_game_loop_signal(WAIT_FINAL, 0);
}

extern void port_set_config(game_config_t gc, ssize_t val) {
    switch(gc) {
        case CONFIG_REPEAT: main_set_ignore_repeat(val); break;
        case CONFIG_XSCROLL: main_set_xscroll(val); break;
        case CONFIG_YSCROLL: main_set_yscroll(val); break;
        case CONFIG_PLAYER_FRAME_DELAY: player_set_delay(val); break;
        case CONFIG_PLAYER_SET_MOVE: player_set_move(val); break;
        case CONFIG_PLAYER_MOVE: player_move(val); break;
        case CONFIG_PLAYER_UNMOVE: player_unmove(val); break;
        case CONFIG_SET_QUIET: dsl_set_quiet(val); break;
        case CONFIG_EXIT: main_exit_game(); break;
        case CONFIG_RUN_BROWSER: main_set_browser_mode(); break;
    }
}
