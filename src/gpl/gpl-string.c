#include "gpl-string.h"
#include "gpl.h"
#include <stdio.h>
#include <string.h>

gpl_string_t* gpl_global_strings = NULL;
gpl_string_t* gpl_local_strings = NULL;
uint8_t*      gpl_global_string = NULL;
uint8_t fudge;

#define INTRODUCE           (0x01)
#define STRING_UNCOMPRESSED (0x02)
#define STRING_COMPRESSED   (0x05)

static char* read_compressed();
static char* introduce();

extern char* gpl_read_text() {
    switch(gpl_peek_one_byte()) {
        case INTRODUCE:
            gpl_get_byte();
            strcpy((char*)gpl_global_string, introduce());
            //printf("INTRODUCE: gpl_global_string = '%s'\n", gpl_global_string);
            return (char*)gpl_global_string;
            break;
        case STRING_UNCOMPRESSED:
            printf("read_text: STRING_UNCOMPRESSED not implemented!\n");
            gpl_get_byte();
            break;
        case STRING_COMPRESSED:
            gpl_get_byte();
            read_compressed();
            //printf("STRING_COMPRESSED: gpl_global_string = '%s'\n", gpl_global_string);
            return (char*)gpl_global_string;
            break;
    }
    return NULL;
}

static char* read_compressed() {
    uint16_t i = 0;
    uint32_t buffer = 0, inword = 0;
    uint8_t idx = 1;

    while (i < (TEXTSTRINGSIZE - 1)) {
        if (idx > 0) {
            buffer = ( (buffer << 8) & 0xFF00);
            buffer |= (uint32_t)gpl_get_byte();
        }

        inword = ( (buffer >> idx) & 0x7F);
        gpl_global_string[i] = (uint8_t)inword;
        if (gpl_global_string[i] == 0x03) {
            gpl_global_string[i] = 0x00;
            return (char*)gpl_global_string;
        }
        
        if (gpl_global_string[i] < 0x20 || gpl_global_string[i] > 0x7E) {
            gpl_global_string[i] = 0x20;
        }

        i++;
        idx++;

        if (idx > 7) {
            idx = 0;
        }
    }

    gpl_global_string[i] = 0;
    if (gpl_peek_one_byte() == 0) {
        fudge = 1;
    } else {
        fudge = 0;
    }

    return (char*)gpl_global_string;
}

static char tempstr[1024];
static char* introduce() {
    sprintf(tempstr, "I'm %s\n", "<character name here.>");
    return tempstr;
}
