#include <stdio.h>
#include <stdlib.h>
#include "dsl-narrate.h"

int8_t narrate_open(int16_t action, char *text, int16_t index) {
    switch(action) {
        case NAR_ADD_MENU:
            printf("I need to add_menu with index %d, text = '%s'\n", index, text);
            break;
        case NAR_PORTRAIT:
            printf("I need to create narrate_box with portrait index %d, text = '%s'\n", index, text);
            break;
        case NAR_SHOW_TEXT:
            printf("I need to show text (index =  %d), text = '%s'\n", index, text);
            break;
        case NAR_SHOW_MENU:
            printf("I need to show menu (index =  %d), text = '%s'\n", index, text);
            break;
        case NAR_EDIT_BOX:
            printf("I need to show edit box (index =  %d), text = '%s'\n", index, text);
            break;
        default:
            fprintf(stderr, "narrate_open: ERROR unknown action %d\n", action);
            exit(1);
    }
    return 0;
}
