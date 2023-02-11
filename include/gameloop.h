#ifndef GAMELOOP_H
#define GAMELOOP_H

#define TICKS_PER_SEC (96)

enum {
    WAIT_FINAL,
    WAIT_NARRATE_CONTINUE,
    WAIT_NARRATE_SELECT,
    WAIT_MAX_SIGNALS,
};

extern sol_status_t sol_gameloop_init();
extern sol_status_t sol_game_loop();
extern sol_status_t sol_game_loop_wait_for_signal(int signal);
extern sol_status_t sol_game_loop_signal(int signal, int _accum);
extern sol_status_t sol_game_loop_is_waiting_for(int signal);
extern sol_status_t sol_player_freeze();
extern sol_status_t sol_still_running();
extern sol_status_t sol_started();

#endif
