#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdint.h>

extern void audio_init();
extern void audio_play_xmi(const int gff_idx, uint32_t type, uint32_t res_id);
extern void audio_play_voc(const int gff_idx, uint32_t type, uint32_t res_id, const float volume);
extern float audio_get_xmi_volume();
extern void audio_set_xmi_volume(const float vol);
extern float audio_get_voc_volume();
extern void audio_set_voc_volume(const float vol);
extern void audio_cleanup();

#endif
