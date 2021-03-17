#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdint.h>

extern void audio_init();
extern void audio_play_xmi(const int gff_idx, uint32_t type, uint32_t res_id);
extern void audio_play_voc(const int gff_idx, uint32_t type, uint32_t res_id);
extern void audio_cleanup();

#endif
