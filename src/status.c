#include <stdio.h>

#include "status.h"
#include <stdlib.h>

static const char *status_messages[] = {
    "SUCCESS",
    "Function is not implemented.",
    "Unknown error occurred.",
    "Alignment combination not allowed.",
    "No custom action available.",
    "Memory error",
    "There is no examine trigger for given entity.",
    "There was a null parameter passed into a function.",
    "An argument was out of range or not valid.",
    "An attack is not allowed.",
    "The entity is in combat.",
    "The entity is NOT in combat",
    "Could not initialize audio subsystem.",
    "The particular item does not exist.",
    "A buffer was overrun.",
    "Could not open the device.",
    "Could not queue up the audio device.",
    "Could not find the resource.",
    "The resource has already been freed.",
    "A resource has not been loaded.",
    "The class is not allowed for the character.",
    "The level is out of range.",
    "There is not available attack for given item and class.",
};

extern void sol_status_print(sol_status_t error) {
    printf("%s\n", status_messages[error]);
}

extern void sol_status_exit(sol_status_t error, const char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    fprintf(stderr, "       %s\n", status_messages[error]);
    exit(1);
}

extern void sol_status_check(sol_status_t status, const char *msg) {
    if (status != SOL_SUCCESS) {
        sol_status_exit(status, msg);
    }
}
