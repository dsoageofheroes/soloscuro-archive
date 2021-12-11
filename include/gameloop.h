#ifndef GAMELOOP_H
#define GAMELOOP_H

#define TICKS_PER_SEC (96)

enum {
    WAIT_FINAL,
    WAIT_NARRATE_CONTINUE,
    WAIT_NARRATE_SELECT,
    WAIT_MAX_SIGNALS,
};

extern void sol_gameloop_init();
extern void sol_game_loop();
extern int  sol_game_loop_wait_for_signal(int signal);
extern void sol_game_loop_signal(int signal, int _accum);
extern int  sol_game_loop_is_waiting_for(int signal);
extern int  sol_player_freeze();
extern void sol_game_loop();
extern int  sol_still_running();
extern int  sol_started();

#endif
