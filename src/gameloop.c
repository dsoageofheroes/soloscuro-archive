#include <stdlib.h>
#include <string.h>

#include "gpl.h"
#include "gameloop.h"
#include "port.h"
#include "region-manager.h"
#include "sol-lua-manager.h"

static int done = 0, started = 0;
static uint8_t wait_flags[WAIT_MAX_SIGNALS];
static int accum = 0;

static void sol_render() {
    sol_region_tick(sol_region_manager_get_current());
    sol_combat_update(sol_region_manager_get_current());
    port_window_render();
}

extern int sol_still_running() { return !done; }
extern int sol_started() { return started; }

extern int sol_player_freeze() {
    return wait_flags[WAIT_NARRATE_CONTINUE]
        || wait_flags[WAIT_NARRATE_SELECT];
}

extern int sol_game_loop_is_waiting_for(int signal) {
    if (signal < 0 || signal >= WAIT_MAX_SIGNALS) {
        error("Received signal %d!\n", signal);
        return 0;
    }

    return wait_flags[signal];
}

extern void sol_game_loop_signal(int signal, int _accum) {
    if (signal < 0 || signal >= WAIT_MAX_SIGNALS) {
        error("Received signal %d!\n", signal);
        return;
    }
    //replay_print("rep.signal(%d, %d)\n", signal, _accum);
    if (wait_flags[signal]) {
        wait_flags[signal]--;
        accum = _accum;
        done = 1;
    } else {
        warn("signal %d received, but not waiting on it...\n", signal);
        done = 1;
    }
}

extern int sol_game_loop_wait_for_signal(int signal) {
    if (signal < 0 || signal >= WAIT_MAX_SIGNALS) {
        error("Received signal %d!\n", signal);
        return 0 ;
    }
    wait_flags[signal]++;
    while (wait_flags[signal]) {
        port_handle_input();
        //Logic here...
        sol_render();
        port_tick();
    }
    done = 0;
    return accum;
}

extern void sol_gameloop_init() {
    memset(wait_flags, 0x0, sizeof(uint8_t) * WAIT_MAX_SIGNALS);
    wait_flags[WAIT_FINAL] = 1;
}

extern void sol_game_loop() {
    while (!done) {
        port_handle_input();
        //Logic here...
        sol_render();
        port_tick();

        //if (in_replay_mode() && rep_times > 10) {
            //replay_next();
            //rep_times = 0;
        //}
        started = 1;
        if (sol_region_manager_get_current() && !sol_region_manager_get_current()->actions.head) {
            sol_lua_run_function("idle");
        }
    }
}
