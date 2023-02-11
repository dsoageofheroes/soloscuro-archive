#include "audio.h"
#include "main.h"
#include "gpl.h"
#include "gameloop.h"
#include "adlmidi.h"
#include <SDL2/SDL.h>
#include <sndfile.h>

#define XMI_BUF_SIZE (1<<13)

static void soloscuro_audio_callback(void *midi_player, uint8_t *stream, int len);
static uint8_t xmi_playing = 0;
static uint8_t xmi_buf[XMI_BUF_SIZE];
static SDL_AudioSpec            spec, obtained;
static struct ADL_MIDIPlayer    *midi_player = NULL;
static struct ADLMIDI_AudioFormat audio_format;
static int midi_gff = -1, midi_type, midi_res_id;

// The MIDI and VOC subsystems generate sounds at different volumes.
// Namely voc is super loud and midi is quiet. These factor are applied to equalize the volumes.
#define MIDI_FACTOR (6.0)
#define VOC_FACTOR (0.2)

// The volumes (0.0 - 1.0)
static float midi_volume = 0.8;
static float voc_volume = 0.6;

#define VOC_BUFFER_LEN (1<<16)

typedef struct audio_buffer_s {
    sf_count_t offset, length;
    char buffer[VOC_BUFFER_LEN];
} audio_buffer_t;

#define AUDIO_DEVICE_NUM (4)

static SDL_AudioDeviceID audio_device[AUDIO_DEVICE_NUM];
static SDL_AudioDeviceID music_device;
static int audio_opened[AUDIO_DEVICE_NUM] = {0, 0, 0, 0};

extern sol_status_t sol_audio_get(sol_audio_stats_t **stats_arg) {
    static sol_audio_stats_t stats;
    if (stats_arg == NULL) { return SOL_NULL_ARGUMENT; }

    stats.xmi_volume = midi_volume;
    stats.voc_volume = voc_volume;
    *stats_arg = &stats;

    return SOL_SUCCESS;
}

extern sol_status_t sol_audio_set_xmi_volume(const float vol) {
    if (vol < 0)   { midi_volume = 0.0; return SOL_ILLEGAL_ARGUMENT; }
    if (vol > 1.0) { midi_volume = 1.0; return SOL_ILLEGAL_ARGUMENT; }

    midi_volume = vol;
    return SOL_SUCCESS;
}

extern sol_status_t sol_audio_set_voc_volume(const float vol) {
    if (vol < 0)   { voc_volume = 0.0; return SOL_ILLEGAL_ARGUMENT; }
    if (vol > 1.0) { voc_volume = 1.0; return SOL_ILLEGAL_ARGUMENT;  }

    voc_volume = vol;
    return SOL_SUCCESS;
}

extern sol_status_t sol_audio_init() {
    spec.freq = 44100;
    spec.format = AUDIO_S16SYS;
    spec.channels = 2;
    spec.samples = 2048;

    memset(xmi_buf, 0x0, sizeof(xmi_buf));
    memset(&audio_format, 0x0, sizeof(audio_format));

    midi_player = adl_init(spec.freq);
    if (!midi_player) {
        error("Couldn't initialize ADLMIDI: %s\n", adl_errorString());                                                     
        return SOL_AUDIO_INIT_FAILURE;
    }

    adl_switchEmulator(midi_player, ADLMIDI_EMU_DOSBOX);

    spec.callback = soloscuro_audio_callback;
    spec.userdata = midi_player;

    if ((music_device = SDL_OpenAudio(&spec, &obtained)) < 0) {
        error("Couldn't open audio: %s\n", SDL_GetError());                                                                
        return SOL_AUDIO_INIT_FAILURE;
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

    adl_setVolumeRangeModel(midi_player, ADLMIDI_VolumeModel_AIL);
    adl_setScaleModulators(midi_player, 1);
    //adl_setLoopEnabled(midi_player, 1);
    return SOL_SUCCESS;
}

static sol_status_t load_midi() {
    char *buf = NULL;

    if (midi_gff < 0) { return SOL_ILLEGAL_ARGUMENT; }

    SDL_PauseAudio(1);
    if (xmi_playing) {
        adl_reset(midi_player);
    }

    gff_chunk_header_t chunk = gff_find_chunk_header(midi_gff, midi_type, midi_res_id);
    buf = malloc(chunk.length);
    gff_read_chunk(midi_gff, &chunk, buf, chunk.length);

    if (adl_openData(midi_player, buf, chunk.length)) {
        error("Couldn't open music entry (%d, %d, %d): %s\n", midi_gff, midi_type, midi_res_id, adl_errorInfo(midi_player));
        return SOL_DNE;
    }

    xmi_playing = 1;
    /* Start playing */
    SDL_PauseAudio(0);

    free(buf);
    return SOL_SUCCESS;
}

extern sol_status_t sol_audio_play_xmi(const int gff_idx, uint32_t type, uint32_t res_id) {
    midi_gff = gff_idx;
    midi_type = type;
    midi_res_id = res_id;

    return load_midi();
}

static sf_count_t vfget_filelen (void *user_data) {
    audio_buffer_t *ab = (audio_buffer_t*) user_data;
    return ab->length;
}

static sf_count_t vfseek (sf_count_t offset, int whence, void *user_data) {
    audio_buffer_t *ab = (audio_buffer_t*) user_data;

    switch (whence) {
        case SEEK_SET :
            ab->offset = offset;
            break ;
        case SEEK_CUR :
            ab->offset += offset;
            break ;
        case SEEK_END :
            ab->offset = ab->length + offset;
            break ;
    }

    return ab->offset;
}

static sf_count_t vfread (void *ptr, sf_count_t count, void *user_data) {
    audio_buffer_t *ab = (audio_buffer_t*) user_data;

    if (ab->offset + count > ab->length) {
        count = ab->length - ab->offset;
    }

    memcpy (ptr, ab->buffer + ab->offset, count) ;
    ab->offset += count ;

    return count;
}

static sf_count_t vfwrite (const void *ptr, sf_count_t count, void *user_data) {
    audio_buffer_t *ab = (audio_buffer_t*) user_data;

    if (ab->offset >= sizeof(ab->buffer)) {
        return 0 ;
    }

    if (ab->offset + count > sizeof(ab->buffer)) {
        count = sizeof (ab->buffer) - ab->offset ;
    }

    memcpy (ab->buffer + ab->offset, ptr, (size_t) count) ;
    ab->offset += count ;

    if (ab->offset > ab->length) {
        ab->length = ab->offset ;
    }

    return count;
}

static sf_count_t vftell (void *user_data) {
    audio_buffer_t *ab = (audio_buffer_t*) user_data;
    return ab->offset;
}

static int get_audio_id() {
    uint32_t min = 0x7FFFFFFF;
    int min_id = 0;
    int audio_amt = 0;

    for (int i = 0; i < AUDIO_DEVICE_NUM; i++) {
        if (!audio_opened[i]) { return i; }
        audio_amt = SDL_GetQueuedAudioSize(audio_device[i]);
        if (audio_amt == 0) { return i; }
        if (audio_amt < min) {
            min = audio_amt;
            min_id = i;
        }
    }

    return min_id;
}

extern sol_status_t sol_play_sound_effect(const uint16_t id) {
    //printf("sol_player_sound_effect: %d\n", id);
    return sol_audio_play_voc(RESOURCE_GFF_INDEX, GFF_BVOC, id, voc_volume);
}

extern sol_status_t sol_audio_play_voc(const int gff_idx, uint32_t type, uint32_t res_id, const float volume) {
    static float buffer [VOC_BUFFER_LEN];
    int readcount;
    SF_VIRTUAL_IO vout;
    SF_VIRTUAL_IO vin;
    int audio_id;
    SDL_AudioSpec spec;
    SNDFILE     *infile, *outfile;
    SF_INFO     sfinfo ;

    audio_id = get_audio_id();
    //printf("audio_id = %d\n", audio_id);

    audio_buffer_t audio_data;
    audio_data.offset = 0;
    audio_data.length = VOC_BUFFER_LEN;

    audio_buffer_t audio_data_in;
    audio_data_in.offset = 0;
    audio_data_in.length = VOC_BUFFER_LEN;

    vin.get_filelen = vout.get_filelen = vfget_filelen;
    vin.seek = vout.seek = vfseek;
    vin.read = vout.read = vfread;
    vin.write = vout.write = vfwrite;
    vin.tell = vout.tell = vftell;

    // Read in the chunk.
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, type, res_id);
    if (chunk.length > VOC_BUFFER_LEN) {
        error ("loading %d:%d:%d exceeds buffer (%u vs %d)\n",
                gff_idx, type, res_id, chunk.length, VOC_BUFFER_LEN);
        return SOL_BUFFER_OVERRUN;
    }

    gff_read_chunk(gff_idx, &chunk, audio_data_in.buffer, chunk.length);

    // setup the input "file".
    audio_data_in.length = chunk.length;
    if ((infile = sf_open_virtual (&vin, SFM_READ, &sfinfo, &audio_data_in)) == NULL) {
        printf("error setting up input.\n");
        puts (sf_strerror (NULL));
        return SOL_DEVICE_OPEN_ERROR;
    }

    // setup the output "file"
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    if ((outfile = sf_open_virtual (&vout, SFM_WRITE, &sfinfo, &audio_data)) == NULL) {
        printf("error setting up output.\n");
        return SOL_DEVICE_OPEN_ERROR;
    }

    // read in (do the translation from VOC to WAV.
    while ((readcount = (int) sf_read_float (infile, buffer, VOC_BUFFER_LEN)) > 0) {
        sf_write_float (outfile, buffer, readcount) ;
    }

    // Now setup SDL
    spec.freq = 8000;
    spec.format = AUDIO_S16LSB;
    spec.channels = 1;
    spec.silence = 0;
    spec.samples = 64;
    spec.padding = 0;
    spec.size = 0;
    spec.callback = 0x0;
    spec.userdata = 0x0;

    // Open the device if needed.
    if (!audio_opened[audio_id]) {
        audio_device[audio_id] = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
        audio_opened[audio_id] = 1;
    }

    const float total_factor = volume * VOC_FACTOR;
    if (volume >= 0 && volume <= 1.0) {
        for (sf_count_t i = 0; i < audio_data.offset / 2; i++) {
            ((int16_t*)audio_data.buffer)[i] = ((int16_t*)audio_data.buffer)[i] * total_factor > 32767.0
                ? 32767
                : ((int16_t*)audio_data.buffer)[i] * total_factor;
        }
    }

    int failure = SDL_QueueAudio(audio_device[audio_id], audio_data.buffer, audio_data.offset);
    if (failure) {
        error ("failure queuing up audio: %d:%d:%d\n", gff_idx, type, res_id);
        return SOL_AUDIO_QUEUE_ERROR;
    }

    SDL_PauseAudioDevice(audio_device[audio_id], 0);

    sf_close(infile);
    sf_close(outfile);

    return SOL_SUCCESS;
}

static void soloscuro_audio_callback(void *midi_player, uint8_t *stream, int len) {
    if (sol_still_running() != SOL_SUCCESS) { return; }
    struct ADL_MIDIPlayer* p = (struct ADL_MIDIPlayer*)midi_player;

    /* Convert bytes length into total count of samples in all channels */
    int samples_count = len / audio_format.containerSize;

    //printf("midi_position = %f / %f\n", adl_positionTell(midi_player), adl_totalTimeLength(midi_player));
    if (adl_atEnd(midi_player)) {
        adl_positionRewind(midi_player);
    }

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

    // Assuming ADLMIDI_SampleType_S16
    const float total_factor = midi_volume * MIDI_FACTOR;
    for (int i = 0; i < samples_count; i++) {
        ((int16_t*)xmi_buf)[i] = ((int16_t*)xmi_buf)[i] * total_factor > 32767.0
            ? 32767
            : ((int16_t*)xmi_buf)[i] * total_factor;
    }

    /* Send buffer to the audio device */
    SDL_memcpy(stream, xmi_buf, samples_count * audio_format.containerSize);
}

extern sol_status_t sol_audio_cleanup() {
    for (int i = 0; i < AUDIO_DEVICE_NUM; i++) {
        if (audio_opened[i]) {
            SDL_CloseAudioDevice(audio_device[i]);
            audio_opened[i] = 0;
        }
    }

    if (music_device >= 0) {
        SDL_CloseAudioDevice(music_device);
        music_device = 0;
    }

    adl_close(midi_player);
    SDL_CloseAudio();

    return SOL_SUCCESS;
}
