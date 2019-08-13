#ifndef MUSIC_H
#define MUSIC_H

#include <stdint.h>
#include <stdio.h>

extern void music_init(const char *sfpath);
extern void music_play(const char *midipath);
extern void music_play_mem(const void * buffer, size_t len);
extern void music_stop();
extern void music_shutdown();

#endif
