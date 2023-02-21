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
#include "window-manager.h"

static int done = 0, started = 0;
static uint8_t wait_flags[WAIT_MAX_SIGNALS];
static int accum = 0;

static void sol_render() {
    sol_region_t *reg;

    sol_region_manager_get_current(&reg);
    sol_region_tick(reg);
    sol_combat_update(reg);
    sol_window_render(sol_get_camerax(), sol_get_cameray());
}

extern sol_status_t sol_still_running() { return done ? SOL_STOPPED : SOL_SUCCESS; }
extern sol_status_t sol_started() { return started ? SOL_SUCCESS : SOL_STOPPED; }

extern sol_status_t sol_player_freeze() {
    return 
        (wait_flags[WAIT_NARRATE_CONTINUE]
        || wait_flags[WAIT_NARRATE_SELECT])
        ? SOL_SUCCESS
        : SOL_NOT_FOUND;
}

extern sol_status_t sol_game_loop_is_waiting_for(int signal) {
    if (signal < 0 || signal >= WAIT_MAX_SIGNALS) {
        return SOL_OUT_OF_RANGE;
    }

    return wait_flags[signal] == 0 ? SOL_NOT_FOUND : SOL_SUCCESS;
}

extern sol_status_t sol_game_loop_signal(int signal, int _accum) {
    if (signal < 0 || signal >= WAIT_MAX_SIGNALS) {
        error("Received signal %d!\n", signal);
        return SOL_OUT_OF_RANGE;
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
    return SOL_SUCCESS;
}

extern sol_status_t sol_game_loop_wait_for_signal(int signal) {
    if (signal < 0 || signal >= WAIT_MAX_SIGNALS) {
        error("Received signal %d!\n", signal);
        return SOL_OUT_OF_RANGE;
    }
    wait_flags[signal]++;
    while (wait_flags[signal]) {
        port_handle_input();
        //Logic here...
        sol_render();
        port_tick();
    }
    done = 0;
    return SOL_SUCCESS;
}

extern sol_status_t sol_gameloop_init() {
    memset(wait_flags, 0x0, sizeof(uint8_t) * WAIT_MAX_SIGNALS);
    wait_flags[WAIT_FINAL] = 1;
    return SOL_SUCCESS;
}

#define TICKS_PER_ROUND (300) // TODO: Is this really 60?
extern sol_status_t sol_game_loop() {
    int16_t gn;
    int ticks_to_increment_time = TICKS_PER_ROUND;
    sol_combat_region_t *cr = NULL;
    sol_region_t *reg;
    while (!done) {
        port_handle_input();
        //Logic here...
        sol_render();
        sol_input_tick();
        port_tick();

        sol_region_manager_get_current(&reg);
        sol_arbiter_combat_region(reg, &cr);
        if (sol_combat_active(cr) != SOL_SUCCESS) {
            ticks_to_increment_time--;
        }
        if (ticks_to_increment_time <= 0) {
            ticks_to_increment_time = TICKS_PER_ROUND;
            // TODO: this is a guess...
            sol_gpl_get_gname(GNAME_TIME, &gn);
            gn += 1;
            sol_gpl_set_gname(GNAME_TIME, gn);
            // Time to check for noorders
            sol_trigger_noorders_event();
        }

        started = 1;
        if (reg && !reg->actions.head) {
            sol_lua_run_function("idle");
        }
    }
    port_close();

    return SOL_SUCCESS;
}
