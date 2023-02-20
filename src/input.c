#include "add-load-save.h"
#include "combat.h"
#include "input.h"
#include "player.h"
#include "game-menu.h"
#include "gameloop.h"
#include "inventory.h"
#include "popup.h"
#include "settings.h"
#include "trigger.h"
#include "save-load.h"
#include "view-character.h"
#include <stdio.h>

#define QUICK_SAVE_LOC "quick.sav"

static int xscroll = 0, yscroll = 0, in_debug_mode = 0;

extern sol_status_t sol_input_tick() {
    sol_camera_scrollx(xscroll);
    sol_camera_scrolly(yscroll);
    return SOL_SUCCESS;
}

extern sol_status_t sol_in_debug_mode() {
    return in_debug_mode ? SOL_SUCCESS : SOL_NOT_FOUND;
}

extern sol_status_t sol_key_down(const sol_key_e key) {
    float zoom = settings_zoom();
    switch(key) {
        case SOLK_g:       sol_combat_guard(NULL); break;
        case SOLK_LEFT:    sol_player_move(PLAYER_LEFT); break;
        case SOLK_UP:      sol_player_move(PLAYER_UP); break;
        case SOLK_RIGHT:   sol_player_move(PLAYER_RIGHT); break;
        case SOLK_DOWN:    sol_player_move(PLAYER_DOWN); break;
        case SOLK_e:       yscroll = -1 * zoom; break;
        case SOLK_d:       yscroll = 1 * zoom; break;
        case SOLK_s:       xscroll = -1 * zoom; break;
        case SOLK_f:       xscroll = 1 * zoom; break;
        case SOLK_TAB:     sol_window_toggle(&game_menu_window, 0, 0); break;
        case SOLK_i:       sol_window_toggle(&inventory_window, 0, 0); break;
        case SOLK_c:       sol_window_toggle(&view_character_window, 0, 0); break;
        case SOLK_ESCAPE:  sol_game_loop_signal(WAIT_FINAL, 0); break;
        case SOLK_F11:     
                           sol_add_load_save_set_mode(ACTION_SAVE);
                           sol_window_toggle(&als_window, 0, 0);
                           break;
        case SOLK_F12:     sol_add_load_save_set_mode(ACTION_LOAD);
                           sol_window_toggle(&als_window, 0, 0);
                           break;
        case SOLK_KP_PLUS: if (sol_save_to_file(QUICK_SAVE_LOC, "quick save") == SOL_SUCCESS) {
                               sol_popup_quick_message("Game saved to quick.sav");
                           } else {
                               sol_popup_quick_message("Can't save");
                           }
                           sol_window_push(&popup_window, 0, 0);
                           break;
        case SOLK_KP_ENTER:
                           sol_combat_kill_all_enemies();
                           break;
        case SOLK_KP_MINUS: sol_load_from_file(QUICK_SAVE_LOC);
                           break;
        case SOLK_SLASH:
                           in_debug_mode = !in_debug_mode;
                           break;
        default: return SOL_UNKNOWN_KEY;
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_key_up(const sol_key_e key) {
    //printf("keyup = %d\n", key);
    switch(key) {
        case SOLK_LEFT:   sol_player_unmove(PLAYER_LEFT); break;
        case SOLK_UP:     sol_player_unmove(PLAYER_UP); break;
        case SOLK_RIGHT:  sol_player_unmove(PLAYER_RIGHT); break;
        case SOLK_DOWN:   sol_player_unmove(PLAYER_DOWN); break;
        case SOLK_e:      yscroll = 0; break;
        case SOLK_d:      yscroll = 0; break;
        case SOLK_s:      xscroll = 0; break;
        case SOLK_f:      xscroll = 0; break;
        case SOLK_o:
            sol_trigger_talk_click(-89);
            break;
        default: return SOL_UNKNOWN_KEY;
    }
    return SOL_SUCCESS;
}
