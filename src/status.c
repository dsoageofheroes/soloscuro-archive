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
