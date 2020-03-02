#include "dsl.h"
#include "external.h"
#include "gameloop.h"

static int done = 0;

typedef struct animation_s {
    struct animation_s *next;
} animation_t;

//static animation_t *animations[TICKS_PER_SEC];

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
