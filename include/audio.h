#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdint.h>
#include "status.h"

typedef struct sol_audio_stats_s {
    float xmi_volume;
    float voc_volume;
} sol_audio_stats_t;

extern sol_status_t sol_audio_init();
extern sol_status_t sol_audio_play_xmi(const int gff_idx, uint32_t type, uint32_t res_id);
extern sol_status_t sol_audio_play_voc(const int gff_idx, uint32_t type, uint32_t res_id, const float volume);
extern sol_status_t sol_audio_set_xmi_volume(const float vol);
extern sol_status_t sol_audio_set_voc_volume(const float vol);
extern sol_status_t sol_audio_cleanup();
extern sol_status_t sol_audio_get(sol_audio_stats_t **stats);

#endif
