#include <stdio.h>

#include "status.h"
#include <stdlib.h>

static const char *status_messages[] = {
    "SUCCESS",
    "Function is not implemented.",
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
    "Unable to allocate another slot for sprites.",
    "The GUI has not been initialized.",
    "The sprite could not be located.",
    "One of the parameters is out of range.",
    "The sprite is not in the given rectangle.",
    "The label was not found.",
    "The entity is real.",
    "There was an error loading/using the GFF.",
    "It is not the entity's turn.",
    "It is blocked (were you trying to move there?)",
    "Need to wait until other actions complete before proceeding.",
    "The entity doesn't have any moves left.",
    "You have no class!",
    "No slots are available.",
    "The structure is not empty.",
    "Unable to get id for given gff.",
    "The resource is closed.",
    "The system has stopped.",
    "Could not execute the requested script.",
    "The request GPL function does not exist.",
    "The request chunk was not found in the requested GFF.",
    "The given resource is still active.",
    "The given gpl type received is unknown.",
    "The key is unknown.",
    "The item/object does not go in that slot.",
    "The user selected no.",
    "The game is over, waiting for restart.",
    "The game is paused.",
    "The stats are not valid.",
    "The entity does not have a sex.",
    "The entity does not have a valid magic resistance.",
    "The entity does not have an allegiance.",
    "The entity does not have a valid size.",
    "The entity does not have the correct psi types.", 
    "The entity does not have any melee attacks left.",
    "Currently the system is in combat and the action cannot be performed.",
    "Currently the system is in narration and the action cannot be performed.",
    "There was an error with the filesystem.",
    "There is no instance of the Lua interpreter.",
    "The Lua function was not found.",
    "There is not target.",
    "The action was not handled by the function.",
    "Unknown error occurred.",
};

extern void sol_status_print(sol_status_t error, const char *level, const char *msg) {
    fprintf(stderr, "%s: %s\n", level, msg);
    fprintf(stderr, "       %s\n", status_messages[error]);
}

extern void sol_status_exit(sol_status_t error, const char *msg) {
    sol_status_print(error, "ERROR", msg);
    exit(1);
}

extern void sol_status_check(sol_status_t status, const char *msg) {
    if (status != SOL_SUCCESS) {
        sol_status_exit(status, msg);
    }
}

extern void sol_status_warn(sol_status_t status, const char *msg) {
    if (status != SOL_SUCCESS) {
        sol_status_print(status, "Warning", msg);
    }
}
