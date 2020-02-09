#include "dsl.h"
#include "external.h"
#include "gameloop.h"

static int done = 0;

void signal_exit() {
    done = 1;
}

void game_loop() {
    while (!done) {
        handle_input();
        //Logic here...
        render();
        tick();
    }
}
