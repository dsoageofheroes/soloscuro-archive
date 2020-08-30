#include <string.h>
#include "dsl.h"
#include "port.h"
#include "gameloop.h"
#include "replay.h"

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
    }
}

int game_loop_wait_for_signal(int signal) {
    if (signal < 0 || signal >= WAIT_MAX_SIGNALS) {
        error("Received signal %d!\n", signal);
        return 0 ;
    }
    wait_flags[signal]++;
    game_loop();
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
