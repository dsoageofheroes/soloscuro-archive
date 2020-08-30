#ifndef REPLAY_H
#define REPLAY_H

void replay_init(const char *path);
void replay_cleanup();

void replay_print(const char *str, ...);
void replay_game(const char *file);
int replay_next();
int in_replay_mode();

#endif
