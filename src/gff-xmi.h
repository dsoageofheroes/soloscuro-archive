#ifndef DS_XMI_H
#define DS_XMI_H

unsigned char* xmi_to_midi(const unsigned char *xmi_data, const unsigned int xmi_len, unsigned int *midi_len);
unsigned char* xmi_to_midi_type(const unsigned char *xmi_data, const unsigned int xmi_len, unsigned int *midi_len,
    const int type);

#endif
