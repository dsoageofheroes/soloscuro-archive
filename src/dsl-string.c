#include "dsl-string.h"
#include "dsl.h"
#include <stdio.h>
#include <string.h>

dsl_string_t* gGstringvar = NULL;
dsl_string_t* gLstringvar = NULL;
uint8_t* gTextstring = NULL;
uint8_t fudge;

#define INTRODUCE           (0x01)
#define STRING_UNCOMPRESSED (0x02)
#define STRING_COMPRESSED   (0x05)

static char* read_compressed();
static char* introduce();

char * read_text() {
    switch(peek_one_byte()) {
        case INTRODUCE:
            get_byte();
            strcpy((char*)gTextstring, introduce());
            //printf("INTRODUCE: gTextstring = '%s'\n", gTextstring);
            return (char*)gTextstring;
            break;
        case STRING_UNCOMPRESSED:
            printf("read_text: STRING_UNCOMPRESSED not implemented!\n");
            get_byte();
            break;
        case STRING_COMPRESSED:
            get_byte();
            read_compressed();
            //printf("STRING_COMPRESSED: gTextstring = '%s'\n", gTextstring);
            return (char*)gTextstring;
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
            buffer |= (uint32_t)get_byte();
        }

        inword = ( (buffer >> idx) & 0x7F);
        gTextstring[i] = (uint8_t)inword;
        if (gTextstring[i] == 0x03) {
            gTextstring[i] = 0x00;
            return (char*)gTextstring;
        }
        
        if (gTextstring[i] < 0x20 || gTextstring[i] > 0x7E) {
            gTextstring[i] = 0x20;
        }

        i++;
        idx++;

        if (idx > 7) {
            idx = 0;
        }
    }

    gTextstring[i] = 0;
    if (peek_one_byte() == 0) {
        fudge = 1;
    } else {
        fudge = 0;
    }

    return (char*)gTextstring;
}

static char tempstr[1024];
static char* introduce() {
    sprintf(tempstr, "I'm %s\n", "<character name here.>");
    return tempstr;
}
