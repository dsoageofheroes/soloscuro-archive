#include <SDL2/SDL.h>
#include <stdio.h>
#include "map.h"
#include "audio.h"
#include "input.h"
#include "font.h"
#include "utils.h"
#include "lua-inc.h"
#include "settings.h"
#include "mouse.h"
#include "textbox.h"
#include "window-manager.h"
#include "gameloop.h"
#include "narrate.h"
#include "window-main.h"
#include "inventory.h"
#include "add-load-save.h"
#include "view-character.h"
#include "game-menu.h"
#include "combat.h"
#include "gpl.h"
#include "gpl-manager.h"
#include "replay.h"
#include "region-manager.h"
#include "ds-load-save.h"
#include "player.h"
#include "port.h"
#include "sol-lua-manager.h"
#include "sol-lua-settings.h"

void browse_loop(SDL_Surface*, SDL_Renderer *rend);
void window_debug_init(SDL_Surface *sur, SDL_Renderer *rend, const char *arg);
void export_all_images(const char *filename);
void export_all_items(const char *base_path);
void export_all_xmis(const char *base_path);
static void set_keys();

static uint32_t last_tick = 0;
static const uint32_t TICK_AMT = 1000 / TICKS_PER_SEC;// Not fully correct...
static SDL_Window *win = NULL;
static SDL_Surface *window = NULL;
static SDL_Renderer *renderer = NULL;
//static float zoom = 2.0;
static uint8_t ignore_repeat = 1, browser_mode = 0;
static int8_t run_lua = 1, quick_load = 0;
static const char *windowed = NULL, *extract_images = NULL, *extract_xmis = NULL,
                  *extract_items = NULL, *extract_gpl = NULL, *lua_script = "lua/main.lua";
static char *ds1_gffs = NULL;
static char *replay = NULL;

static uint32_t xmappos, ymappos;
static int32_t xmapdiff, ymapdiff;

static textbox_t *textbox = NULL;

extern void sol_textbox_set_current(textbox_t *tb) {
    textbox = tb;
}

#define NUM_KEYS (256)
static sol_key_e keys[NUM_KEYS];

SDL_Renderer *main_get_rend() { return renderer; }
SDL_Surface *main_get_window() { return window; }

extern uint32_t sol_get_camerax() { return xmappos; }
extern uint32_t sol_get_cameray() { return ymappos; }

extern void port_start_display_frame() {
    SDL_RenderClear(main_get_rend());
};

extern void port_commit_display_frame() {
    SDL_RenderPresent(main_get_rend());
};

static enum entity_action_e sdl_to_ea(const SDL_Keysym key) {
    return EA_ACTIVATE;
}

void main_set_browser_mode() {
    browser_mode = 1;
}

sol_mouse_button_t convert_mouse_button(uint32_t button) {
    switch(button) {
        case SDL_BUTTON_RIGHT: return SOL_MOUSE_BUTTON_RIGHT;
        case SDL_BUTTON_LEFT: return SOL_MOUSE_BUTTON_LEFT;
        case SDL_BUTTON_MIDDLE: return SOL_MOUSE_BUTTON_MIDDLE;
    }
    return SOL_MOUSE_BUTTON_LEFT;
}

void handle_mouse_motion() {
    int x, y;

    SDL_GetMouseState(&x, &y);

    sol_window_handle_mouse(x, y);
}

void handle_mouse_down(const sol_mouse_button_t button) {
    int x, y;

    SDL_GetMouseState(&x, &y);

    sol_window_handle_mouse_down(button, x, y);
}

void handle_mouse_up(const sol_mouse_button_t button) {
    int x, y;

    SDL_GetMouseState(&x, &y);

    sol_window_handle_mouse_up(button, x, y);
}

void main_exit_game() {
    sol_game_loop_signal(WAIT_FINAL, 0);
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
                if (sol_window_handle_key_press(keys[event.key.keysym.sym & 0xFF])) { return; }
                sol_key_up(keys[event.key.keysym.sym & 0xFF]);
                break;
            case SDL_KEYDOWN:
                if (sol_window_handle_key_down(sdl_to_ea(event.key.keysym))) { break; }
                if (ignore_repeat && event.key.repeat != 0) { break; }
                if (textbox_handle_keydown(textbox, event.key.keysym)) { return; }
                if (sol_lua_keydown(event.key.keysym.sym)) { break; }
                sol_key_down(keys[event.key.keysym.sym & 0xFF]);
                break;
            case SDL_MOUSEMOTION:
                handle_mouse_motion();
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (sol_game_loop_is_waiting_for(WAIT_NARRATE_CONTINUE)) {
                    narrate_clear();
                    sol_game_loop_signal(WAIT_NARRATE_CONTINUE, 0);
                }
                handle_mouse_down(convert_mouse_button(event.button.button));
                break;
            case SDL_MOUSEBUTTONUP:
                handle_mouse_up(convert_mouse_button(event.button.button));
                break;
        }
    }

    handle_mouse_motion();
    if (sol_region_manager_get_current()) {
        sol_player_update();
    }
}

void port_handle_input() {
    handle_input();
}

void main_set_ignore_repeat(int repeat) { ignore_repeat = repeat; }

extern void sol_camera_scrollx(const int amt) { xmappos += amt; }
extern void sol_camera_scrolly(const int amt) { ymappos += amt; }

void sol_center_on_player() {
    int w, h;

    SDL_GetRendererOutputSize(renderer, &w, &h);
    dude_t *dude = sol_player_get_active();

    xmappos = dude->mapx * 16 * settings_zoom() - w / 2;
    ymappos = dude->mapy * 16 * settings_zoom() - h / 2;
}

void port_window_render() {
    sol_window_render(xmappos, ymappos);
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

static int sdl_init(const int what) {
    return SDL_Init(what);
}

static void gui_init() {
    xmappos = 0;
    ymappos = 0;
    xmapdiff = ymapdiff = 0;

    set_keys();
    if (sdl_init( SDL_INIT_VIDEO) ) {
        error( "Unable to init video!\n");
        exit(1);
    }
    win = SDL_CreateWindow( "Dark Sun: Shattered Lands", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, settings_screen_width(), settings_screen_height(), SDL_WINDOW_SHOWN );
    if( win == NULL ) {
        error( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        exit(1);
    }

    //Get window surface
    window = SDL_GetWindowSurface( win );

    renderer = SDL_CreateRenderer(win, -1, 0);

    last_tick = SDL_GetTicks();

    sol_sprite_init();
    sol_window_init();
}

void port_init() {
    sol_audio_init();
    gui_init();

    font_init(renderer);

    sol_gameloop_init();

    sol_mouse_init(renderer);
}

void port_close() {
    // Order matters.
    sol_audio_cleanup();
    sol_player_close();
    sol_window_free();

    gpl_cleanup();
    gff_cleanup();
    sol_mouse_free();

    SDL_DestroyRenderer(renderer);
    //SDL_DestroySurface(window);
    SDL_DestroyWindow( win );

    //Quit SDL subsystems
    SDL_Quit();
}

extern void port_start() {
    port_init();
    map_load_region(sol_region_manager_get_current());

    sol_game_loop();

    replay_cleanup();
}

static void init() {
    if (quick_load) {
        sol_lua_load("quick.sav");
        return;
    }

    if (run_lua) {
        sol_lua_load(lua_script);
        return;
    }

    // rest requires special open.
    gui_init();

    font_init(renderer);

    sol_gameloop_init();

    sol_mouse_init(renderer);
    sol_audio_init();

    if (browser_mode) {
        browse_loop(window, renderer);
    }
    if (extract_images) {
        export_all_images(extract_images);
    }
    if (extract_xmis) {
        export_all_xmis(extract_xmis);
    }
    if (extract_items) {
        export_all_items(extract_items);
    }
    if (extract_gpl) {
        gpl_lua_load_all_scripts();
    }
}

void parse_args(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--ds1") && i < (argc-1)) {
            ds1_gffs = argv[++i];
        }
        if (!strcmp(argv[i], "--replay") && i < (argc-1)) {
            replay = argv[++i];
        }
        if (!strcmp(argv[i], "--browse") && i < (argc)) {
            browser_mode = 1; run_lua = 0;
        }
        if (!strcmp(argv[i], "--window") && i < (argc - 1)) {
            windowed = argv[i + 1];
        }
        if (!strcmp(argv[i], "--extract-images") && i < (argc - 1)) {
            extract_images = argv[i + 1]; run_lua = 0;
        }
        if (!strcmp(argv[i], "--extract-xmis") && i < (argc - 1)) {
            extract_xmis = argv[i + 1]; run_lua = 0;
        }
        if (!strcmp(argv[i], "--extract-items") && i < (argc - 1)) {
            extract_items = argv[i + 1]; run_lua = 0;
        }
        if (!strcmp(argv[i], "--extract-lua") && i < (argc - 1)) {
            extract_gpl = argv[i + 1]; run_lua = 0;
        }
        if (!strcmp(argv[i], "--ignore-lua")) {
            run_lua = 0;
        }
        if (!strcmp(argv[i], "--lua") && i < (argc - 1)) {
            lua_script = argv[i + 1];
        }
        if (!strcmp(argv[i], "--quick-load")) {
            quick_load = 1;
        }
    }

    //replay_init("replay.lua");
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    // Order matters.
    sol_lua_load_preload("lua/settings.lua");
    if (gff_get_game_type() == DARKSUN_UNKNOWN) {
        if (!ds1_gffs) {
            error("Unable to get the location of the DarkSun 1 GFFs, please pass with '--ds1 <location>'\n");
            exit(1);
        }
        // gff_init is not handled in sol_lua_load_preload IF the game_type is DARKSUN_UNKNOWN
        gff_init();
        gff_load_directory(ds1_gffs);
    }
    powers_init();
    gpl_init();
    sol_lua_register_globals();

    init();

    if (!run_lua) { return 0; }

    if (replay) {
        replay_game(replay);
    }

    sol_game_loop();

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
        //case CONFIG_XSCROLL: main_set_xscroll(val); break;
        //case CONFIG_YSCROLL: main_set_yscroll(val); break;
        case CONFIG_PLAYER_FRAME_DELAY: sol_player_set_delay(val); break;
        //case CONFIG_PLAYER_SET_MOVE: sol_player_set_move(val); break;
        case CONFIG_PLAYER_MOVE: sol_player_move(val); break;
        case CONFIG_PLAYER_UNMOVE: sol_player_unmove(val); break;
        case CONFIG_SET_QUIET: sol_set_debug(val); break;
        case CONFIG_EXIT: main_exit_game(); break;
        case CONFIG_RUN_BROWSER: main_set_browser_mode(); break;
        default: printf("Unknown config %d\n", gc); exit(1); break;
    }
}

static void set_keys() {
    memset(keys, 0x0, NUM_KEYS * sizeof(sol_key_e));
    keys[SDLK_RETURN] = SOLK_RETURN;
    keys[SDLK_ESCAPE] = SOLK_ESCAPE;
    keys[SDLK_BACKSPACE] = SOLK_BACKSPACE;
    keys[SDLK_TAB] = SOLK_TAB;
    keys[SDLK_SPACE] = SOLK_SPACE;
    keys[SDLK_EXCLAIM] = SOLK_EXCLAIM;
    keys[SDLK_QUOTEDBL] = SOLK_QUOTEDBL;
    keys[SDLK_HASH] = SOLK_HASH;
    keys[SDLK_PERCENT] = SOLK_PERCENT;
    keys[SDLK_DOLLAR] = SOLK_DOLLAR;
    keys[SDLK_AMPERSAND] = SOLK_AMPERSAND;
    keys[SDLK_QUOTE] = SOLK_QUOTE;
    keys[SDLK_LEFTPAREN] = SOLK_LEFTPAREN;
    keys[SDLK_RIGHTPAREN] = SOLK_RIGHTPAREN;
    keys[SDLK_ASTERISK] = SOLK_ASTERISK;
    keys[SDLK_PLUS] = SOLK_PLUS;
    keys[SDLK_COMMA] = SOLK_COMMA;
    keys[SDLK_MINUS] = SOLK_MINUS;
    keys[SDLK_PERIOD] = SOLK_PERIOD;
    keys[SDLK_SLASH] = SOLK_SLASH;
    keys[SDLK_0] = SOLK_0;
    keys[SDLK_1] = SOLK_1;
    keys[SDLK_2] = SOLK_2;
    keys[SDLK_3] = SOLK_3;
    keys[SDLK_4] = SOLK_4;
    keys[SDLK_5] = SOLK_5;
    keys[SDLK_6] = SOLK_6;
    keys[SDLK_7] = SOLK_7;
    keys[SDLK_8] = SOLK_8;
    keys[SDLK_9] = SOLK_9;
    keys[SDLK_COLON] = SOLK_COLON;
    keys[SDLK_SEMICOLON] = SOLK_SEMICOLON;
    keys[SDLK_LESS] = SOLK_LESS;
    keys[SDLK_EQUALS] = SOLK_EQUALS;
    keys[SDLK_GREATER] = SOLK_GREATER;
    keys[SDLK_QUESTION] = SOLK_QUESTION;
    keys[SDLK_AT] = SOLK_AT;
    keys[SDLK_LEFTBRACKET] = SOLK_LEFTBRACKET;
    keys[SDLK_BACKSLASH] = SOLK_BACKSLASH;
    keys[SDLK_RIGHTBRACKET] = SOLK_RIGHTBRACKET;
    keys[SDLK_CARET] = SOLK_CARET;
    keys[SDLK_UNDERSCORE] = SOLK_UNDERSCORE;
    keys[SDLK_BACKQUOTE] = SOLK_BACKQUOTE;
    keys[SDLK_a] = SOLK_a;
    keys[SDLK_b] = SOLK_b;
    keys[SDLK_c] = SOLK_c;
    keys[SDLK_d] = SOLK_d;
    keys[SDLK_e] = SOLK_e;
    keys[SDLK_f] = SOLK_f;
    keys[SDLK_g] = SOLK_g;
    keys[SDLK_h] = SOLK_h;
    keys[SDLK_i] = SOLK_i;
    keys[SDLK_j] = SOLK_j;
    keys[SDLK_k] = SOLK_k;
    keys[SDLK_l] = SOLK_l;
    keys[SDLK_m] = SOLK_m;
    keys[SDLK_n] = SOLK_n;
    keys[SDLK_o] = SOLK_o;
    keys[SDLK_p] = SOLK_p;
    keys[SDLK_q] = SOLK_q;
    keys[SDLK_r] = SOLK_r;
    keys[SDLK_s] = SOLK_s;
    keys[SDLK_t] = SOLK_t;
    keys[SDLK_u] = SOLK_u;
    keys[SDLK_v] = SOLK_v;
    keys[SDLK_w] = SOLK_w;
    keys[SDLK_x] = SOLK_x;
    keys[SDLK_y] = SOLK_y;
    keys[SDLK_z] = SOLK_z;
    //keys[SDLK_CAPSLOCK] = SOLK_CAPSLOCK;
    keys[SDLK_F1 & 0xFF] = SOLK_F1;
    keys[SDLK_F2 & 0xFF] = SOLK_F2;
    keys[SDLK_F3 & 0xFF] = SOLK_F3;
    keys[SDLK_F4 & 0xFF] = SOLK_F4;
    keys[SDLK_F5 & 0xFF] = SOLK_F5;
    keys[SDLK_F6 & 0xFF] = SOLK_F6;
    keys[SDLK_F7 & 0xFF] = SOLK_F7;
    keys[SDLK_F8 & 0xFF] = SOLK_F8;
    keys[SDLK_F9 & 0xFF] = SOLK_F9;
    keys[SDLK_F10 & 0xFF] = SOLK_F10;
    keys[SDLK_F11 & 0xFF] = SOLK_F11;
    keys[SDLK_F12 & 0xFF] = SOLK_F12;
    //keys[SDLK_PRINTSCREEN] = SOLK_PRINTSCREEN;
    //keys[SDLK_SCROLLLOCK] = SOLK_SCROLLLOCK;
    //keys[SDLK_PAUSE] = SOLK_PAUSE;
    //keys[SDLK_INSERT] = SOLK_INSERT;
    //keys[SDLK_HOME] = SOLK_HOME;
    //keys[SDLK_PAGEUP] = SOLK_PAGEUP;
    keys[SDLK_DELETE] = SOLK_DELETE;
    //keys[SDLK_END] = SOLK_END;
    //keys[SDLK_PAGEDOWN] = SOLK_PAGEDOWN;
    keys[SDLK_RIGHT & 0xFF] = SOLK_RIGHT;
    keys[SDLK_LEFT & 0xFF] = SOLK_LEFT;
    keys[SDLK_DOWN & 0xFF] = SOLK_DOWN;
    keys[SDLK_UP & 0xFF] = SOLK_UP;
    keys[SDLK_NUMLOCKCLEAR & 0xFF] = SOLK_NUMLOCKCLEAR;
    keys[SDLK_KP_DIVIDE & 0xFF] = SOLK_KP_DIVIDE;
    keys[SDLK_KP_MULTIPLY & 0xFF] = SOLK_KP_MULTIPLY;
    keys[SDLK_KP_MINUS & 0xFF] = SOLK_KP_MINUS;
    keys[SDLK_KP_PLUS & 0xFF] = SOLK_KP_PLUS;
    keys[SDLK_KP_ENTER & 0xFF] = SOLK_KP_ENTER;
    //keys[SDLK_KP_1] = SOLK_KP_1;
    //keys[SDLK_KP_2] = SOLK_KP_2;
    //keys[SDLK_KP_3] = SOLK_KP_3;
    //keys[SDLK_KP_4] = SOLK_KP_4;
    //keys[SDLK_KP_5] = SOLK_KP_5;
    //keys[SDLK_KP_6] = SOLK_KP_6;
    //keys[SDLK_KP_7] = SOLK_KP_7;
    //keys[SDLK_KP_8] = SOLK_KP_8;
    //keys[SDLK_KP_9] = SOLK_KP_9;
    //keys[SDLK_KP_0] = SOLK_KP_0;
    //keys[SDLK_KP_PERIOD] = SOLK_KP_PERIOD;
}
