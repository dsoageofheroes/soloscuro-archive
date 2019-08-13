#include <stdlib.h>
#include <string.h>
#include "gff-xmi.h"
#include "xmi2mid.h"

unsigned char* xmi_to_midi(const unsigned char *xmi_data, const unsigned int xmi_len, unsigned int *midi_len) {
    //unsigned char buf[1<<20];
    unsigned char *buf = malloc(1<<20);
    unsigned char *midi_data = NULL;

    _WM_xmi2midi(xmi_data, xmi_len, (unsigned char**)&(buf), midi_len,  XMIDI_CONVERT_MT32_TO_GS);

    midi_data = malloc(*midi_len);
    if (!midi_data) { return NULL; }

    memcpy(midi_data, buf, *midi_len);
    free(buf);
    return midi_data;
}
