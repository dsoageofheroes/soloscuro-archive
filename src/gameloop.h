#ifndef GAMELOOP_H
#define GAMELOOP_H

#define TICKS_PER_SEC (96)

enum {
    WAIT_FINAL,
    WAIT_NARRATE_CONTINUE,
    WAIT_NARRATE_SELECT,
    WAIT_MAX_SIGNALS,
};

extern void gameloop_init();
extern void game_loop();
extern int game_loop_wait_for_signal(int signal);
extern void game_loop_signal(int signal, int _accum);
extern int game_loop_is_waiting_for(int signal);

#endif
