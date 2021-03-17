#include "audio.h"
#include "../src/dsl.h"
#include "../ext/libadlmidi/adlmidi.h"
#include <SDL2/SDL.h>

static void soloscuro_audio_callback(void *midi_player, uint8_t *stream, int len);
static uint8_t xmi_playing = 0;
static uint8_t xmi_buf[1<<13];
static SDL_AudioSpec            spec, obtained;
static struct ADL_MIDIPlayer    *midi_player = NULL;
static struct ADLMIDI_AudioFormat audio_format;

extern void audio_init() {
    SDL_Init(SDL_INIT_AUDIO);
    spec.freq = 44100;
    spec.format = AUDIO_S16SYS;
    spec.channels = 2;
    spec.samples = 2048;

    midi_player = adl_init(spec.freq);
    if (!midi_player) {
        error("Couldn't initialize ADLMIDI: %s\n", adl_errorString());                                                     
    }

    adl_switchEmulator(midi_player, ADLMIDI_EMU_NUKED);

    spec.callback = soloscuro_audio_callback;
    spec.userdata = midi_player;

    if (SDL_OpenAudio(&spec, &obtained) < 0) {
        error("Couldn't open audio: %s\n", SDL_GetError());                                                                
    }

    switch(obtained.format) {
        case AUDIO_S8:
            audio_format.type = ADLMIDI_SampleType_S8;
            audio_format.containerSize = sizeof(int8_t);
            audio_format.sampleOffset = sizeof(int8_t) * 2;
        break;
        case AUDIO_U8:
            audio_format.type = ADLMIDI_SampleType_U8;
            audio_format.containerSize = sizeof(uint8_t);
            audio_format.sampleOffset = sizeof(uint8_t) * 2;
            break;
        case AUDIO_S16:
            audio_format.type = ADLMIDI_SampleType_S16;
            audio_format.containerSize = sizeof(int16_t);
            audio_format.sampleOffset = sizeof(int16_t) * 2;
            break;
        case AUDIO_U16:
            audio_format.type = ADLMIDI_SampleType_U16;
            audio_format.containerSize = sizeof(uint16_t);
            audio_format.sampleOffset = sizeof(uint16_t) * 2;
            break;
        case AUDIO_S32:
            audio_format.type = ADLMIDI_SampleType_S32;
            audio_format.containerSize = sizeof(int32_t);
            audio_format.sampleOffset = sizeof(int32_t) * 2;
            break;
        case AUDIO_F32:
            audio_format.type = ADLMIDI_SampleType_F32;
            audio_format.containerSize = sizeof(float);
            audio_format.sampleOffset = sizeof(float) * 2;
            break;
    }

/*
    //if (adl_openFile(midi_player, music_path) < 0) {
    if (adl_openFile(midi_player, "/home/pwest/repos/soloscuro/xmi/gseq002.xmi") < 0) {
        error("Couldn't open music file: %s\n", adl_errorInfo(midi_player));
        SDL_CloseAudio();
        adl_close(midi_player);
    }
//extern ADLMIDI_DECLSPEC int adl_openData(struct ADL_MIDIPlayer *device, const void *mem, unsigned long size);

    xmi_playing = 1;
    SDL_PauseAudio(0);
    */
    //audio_load_xmi(RESOURCE_GFF_INDEX, GFF_GSEQ, 2);
}

extern void audio_load_xmi(const int gff_idx, uint32_t type, uint32_t res_id) {
    char *buf = NULL;

    SDL_PauseAudio(1);
    if (xmi_playing) {
        adl_reset(midi_player);
    }

    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, type, res_id);
    buf = malloc(chunk.length);
    gff_read_chunk(gff_idx, &chunk, buf, chunk.length);

    if (adl_openData(midi_player, buf, chunk.length)) {
        error("Couldn't open music entry (%d, %d, %d): %s\n", gff_idx, type, res_id, adl_errorInfo(midi_player));
        return;
    }

    xmi_playing = 1;
    /* Start playing */
    SDL_PauseAudio(0);

    free(buf);
}

static void soloscuro_audio_callback(void *midi_player, uint8_t *stream, int len) {
    struct ADL_MIDIPlayer* p = (struct ADL_MIDIPlayer*)midi_player;

    /* Convert bytes length into total count of samples in all channels */
    int samples_count = len / audio_format.containerSize;

    /* Take some samples from the ADLMIDI */
    samples_count = adl_playFormat(p, samples_count,
                                   xmi_buf,
                                   xmi_buf + audio_format.containerSize,
                                   &audio_format);

    if(samples_count <= 0) {
        xmi_playing = 0;
        SDL_memset(stream, 0, len);
        return;
    }

    /* Send buffer to the audio device */
    SDL_memcpy(stream, xmi_buf, samples_count * audio_format.containerSize);
}

extern void audio_cleanup() {
    SDL_CloseAudio();
    adl_close(midi_player);
}
