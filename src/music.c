#include <fluidsynth/include/fluidsynth.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "music.h"

fluid_settings_t *settings;
fluid_synth_t *synth;
fluid_player_t* player;
fluid_audio_driver_t *adriver;
int sfont_id;

void music_init(const char *sfpath) {    
    /* Create the settings. */
    settings = new_fluid_settings();
    /* Change the settings if necessary*/
    /* Create the synthesizer. */
    synth = new_fluid_synth(settings);

    player = new_fluid_player(synth);

    /* Create the audio driver. The synthesizer starts playing as soon
       as the driver is created. */
    adriver = new_fluid_audio_driver(settings, synth);
    /* Load a SoundFont and reset presets (so that new instruments
     * get used from the SoundFont) */
    sfont_id = fluid_synth_sfload(synth, sfpath, 1);
}

void music_shutdown() {
    /* Clean up */
    delete_fluid_audio_driver(adriver);
    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

void music_play(const char *midipath) {

    if (fluid_is_midifile(midipath)) {
        fluid_player_add(player, midipath);
    }
    /* play the midi files, if any */
    fluid_player_play(player);
}

void music_play_mem(const const void * buffer, size_t len) {
    fluid_player_add_mem(player, buffer, len);
    fluid_player_play(player);
}

void music_stop(const char *midipath) {
    fluid_player_stop(player);
}


