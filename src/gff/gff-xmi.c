#include <stdlib.h>
#include <string.h>
#include "gff-xmi.h"
#include "xmi2mid.h"

unsigned char* gff_xmi_to_midi_type(const unsigned char *xmi_data, const unsigned int xmi_len, unsigned int *midi_len,
    const int type) {
    unsigned char *buf = malloc(1<<20);
    unsigned char *midi_data = NULL;

    // I think MT32 to GS is the best, followed by MT32 to GM.
    // No conversion and GS127 to GS are not authentic.

    //_WM_xmi2midi(xmi_data, xmi_len, (unsigned char**)&(buf), midi_len, XMIDI_CONVERT_NOCONVERSION);
    //_WM_xmi2midi(xmi_data, xmi_len, (unsigned char**)&(buf), midi_len, XMIDI_CONVERT_MT32_TO_GM );
    _WM_xmi2midi(xmi_data, xmi_len, (unsigned char**)&(buf), midi_len, type );
    //_WM_xmi2midi(xmi_data, xmi_len, (unsigned char**)&(buf), midi_len, XMIDI_CONVERT_GS127_TO_GS );

    midi_data = malloc(*midi_len);
    if (!midi_data) { return NULL; }

    memcpy(midi_data, buf, *midi_len);
    free(buf);
    return midi_data;
}

unsigned char* gff_xmi_to_midi(const unsigned char *xmi_data, const unsigned int xmi_len, unsigned int *midi_len) {
    //unsigned char buf[1<<20];
    unsigned char *buf = malloc(1<<20);
    unsigned char *midi_data = NULL;

    // I think MT32 to GS is the best, followed by MT32 to GM.
    // No conversion and GS127 to GS are not authentic.

    //_WM_xmi2midi(xmi_data, xmi_len, (unsigned char**)&(buf), midi_len, XMIDI_CONVERT_NOCONVERSION);
    //_WM_xmi2midi(xmi_data, xmi_len, (unsigned char**)&(buf), midi_len, XMIDI_CONVERT_MT32_TO_GM );
    _WM_xmi2midi(xmi_data, xmi_len, (unsigned char**)&(buf), midi_len, XMIDI_CONVERT_MT32_TO_GS );
    //_WM_xmi2midi(xmi_data, xmi_len, (unsigned char**)&(buf), midi_len, XMIDI_CONVERT_GS127_TO_GS );

    midi_data = malloc(*midi_len);
    if (!midi_data) { return NULL; }

    memcpy(midi_data, buf, *midi_len);
    free(buf);
    return midi_data;
}
