#include "gpl-string.h"
#include "gpl.h"
#include <stdio.h>
#include <string.h>

sol_gpl_string_t* sol_gpl_global_strings = NULL;
sol_gpl_string_t* sol_gpl_local_strings = NULL;
uint8_t*          sol_gpl_global_string = NULL;

#define INTRODUCE           (0x01)
#define STRING_UNCOMPRESSED (0x02)
#define STRING_COMPRESSED   (0x05)

static char* read_compressed();
static char* introduce();

extern sol_status_t sol_gpl_read_text(char **ret) {
    uint8_t b;

    if (!ret) { return SOL_NULL_ARGUMENT; }

    sol_gpl_peek_one_byte(&b);
    switch(b) {
        case INTRODUCE:
            sol_gpl_get_byte(&b);
            strcpy((char*)sol_gpl_global_string, introduce());
            //printf("INTRODUCE: gpl_global_string = '%s'\n", gpl_global_string);
            *ret = (char*)sol_gpl_global_string;
            return SOL_SUCCESS;
            break;
        case STRING_UNCOMPRESSED:
            printf("read_text: STRING_UNCOMPRESSED not implemented!\n");
            sol_gpl_get_byte(&b);
            return SOL_NOT_IMPLEMENTED;
        case STRING_COMPRESSED:
            sol_gpl_get_byte(&b);
            read_compressed();
            //printf("STRING_COMPRESSED: gpl_global_string = '%s'\n", sol_gpl_global_string);
            *ret = (char*)sol_gpl_global_string;
            return SOL_SUCCESS;
    }
    return SOL_GPL_UNKNOWN_TYPE;
}

static char* read_compressed() {
    uint16_t i = 0;
    uint32_t buffer = 0, inword = 0;
    uint8_t idx = 1, b;

    while (i < (TEXTSTRINGSIZE - 1)) {
        if (idx > 0) {
            buffer = ( (buffer << 8) & 0xFF00);
            sol_gpl_get_byte(&b);
            buffer |= (uint32_t)b;
        }

        inword = ( (buffer >> idx) & 0x7F);
        sol_gpl_global_string[i] = (uint8_t)inword;
        if (sol_gpl_global_string[i] == 0x03) {
            sol_gpl_global_string[i] = 0x00;
            return (char*)sol_gpl_global_string;
        }
        
        if (sol_gpl_global_string[i] < 0x20 || sol_gpl_global_string[i] > 0x7E) {
            sol_gpl_global_string[i] = 0x20;
        }

        i++;
        idx++;

        if (idx > 7) {
            idx = 0;
        }
    }

    sol_gpl_global_string[i] = 0;

    return (char*)sol_gpl_global_string;
}

static char tempstr[1024];
static char* introduce() {
    sprintf(tempstr, "I'm \" .. gpl.get_active_name() .. \"");
    return tempstr;
}
