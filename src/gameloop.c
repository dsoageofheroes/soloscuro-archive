#include <stdlib.h>
#include <string.h>

#include "gpl.h"
#include "arbiter.h"
#include "gameloop.h"
#include "input.h"
#include "gpl-state.h"
#include "port.h"
#include "region-manager.h"
#include "sol-lua-manager.h"
#include "trigger.h"

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

extern int sol_game_loop_signal(int signal, int _accum) {
    if (signal < 0 || signal >= WAIT_MAX_SIGNALS) {
        error("Received signal %d!\n", signal);
        return 0;
    }
    //replay_print("rep.signal(%d, %d)\n", signal, _accum);
    if (wait_flags[signal]) {
        wait_flags[signal]--;
        accum = _accum;
        done = 1;
        return 1; // someone was waiting
    } 
    warn("signal %d received, but not waiting on it...\n", signal);
    done = 1;
    return 0;
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

#define TICKS_PER_ROUND (300)
extern void sol_game_loop() {
    int ticks_to_increment_time = TICKS_PER_ROUND;
    while (!done) {
        port_handle_input();
        //Logic here...
        sol_render();
        sol_input_tick();
        port_tick();

        if (!sol_combat_active(sol_arbiter_combat_region(sol_region_manager_get_current()))) {
            ticks_to_increment_time--;
        }
        if (ticks_to_increment_time <= 0) {
            ticks_to_increment_time = TICKS_PER_ROUND;
            gpl_set_gname(GNAME_TIME, gpl_get_gname(GNAME_TIME) + 1);
        }
        sol_trigger_tick();
        //if (in_replay_mode() && rep_times > 10) {
            //replay_next();
            //rep_times = 0;
        //}
        started = 1;
        if (sol_region_manager_get_current() && !sol_region_manager_get_current()->actions.head) {
            sol_lua_run_function("idle");
        }
    }
    port_close();
}
