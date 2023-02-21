#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gpl.h"
#include "wizard.h"
#include "gpl-manager.h"
#include "ssi-item.h"
#include "region-manager.h"
#include "gpl-state.h"
#include "gpl-string.h"
#include "gpl-var.h"
//#include "gameloop.h"
#include "player.h"
#include "trigger.h"

#define DSL_ORDER     (0)
#define DSL_LOS_ORDER (1)

uint8_t command_implemented = 0; // Temporary while I figure out each function.

/* Globals */
void get_parameters(int16_t amt);
static sol_entity_t *other = NULL;

sol_gpl_param_t param;
/* End Globals */

extern sol_status_t sol_gpl_get_global(enum sol_gpl_global_e what, sol_entity_t **ent) {
    if (!ent) { return SOL_NULL_ARGUMENT; }

    switch(what) {
        case GPL_OTHER: *ent = other; return SOL_SUCCESS;
        default: warn("unknown type: %d\n", what); break;
    }

    return SOL_GPL_UNKNOWN_TYPE;
}

extern sol_status_t sol_gpl_set_global(enum sol_gpl_global_e what, sol_entity_t *entity) {
    switch(what) {
        case GPL_OTHER: other = entity; return SOL_SUCCESS;
        default: warn("unknown type: %d\n", what); break;
    }
    return SOL_GPL_UNKNOWN_TYPE;
}

extern sol_status_t sol_gpl_change_region(const int region_id) {
    sol_region_t *reg;
    //gpl_execute_subroutine(region_id, 0, 1);
    sol_region_manager_get_region(region_id, 0, &reg);
    return sol_gpl_lua_execute_script(region_id, 0, 1);
}

static void initialize_gpl_stack() {
    sol_gpl_global_strings = (sol_gpl_string_t*) malloc(GSTRINGVARSIZE);
    memset(sol_gpl_global_strings, 0, GSTRINGVARSIZE);
    sol_gpl_local_strings = (sol_gpl_string_t*) malloc(LSTRINGVARSIZE);
    memset(sol_gpl_local_strings, 0, LSTRINGVARSIZE);
    sol_gpl_global_string = (uint8_t*)malloc(TEXTSTRINGSIZE);
}

extern sol_status_t sol_gpl_init() {
    info("Initalizing DSL.\n");
    initialize_gpl_stack();
    sol_gpl_init_vars();
    sol_player_init();
    ssi_item_init();
    sol_trigger_init();
    sol_gpl_manager_init();
    sol_region_manager_init();
    return SOL_SUCCESS;
}

extern sol_status_t sol_gpl_cleanup() {
    sol_powers_cleanup();
    free(sol_gpl_global_string);
    free(sol_gpl_local_strings);
    free(sol_gpl_global_strings);
    sol_gpl_cleanup_vars();
    sol_gpl_manager_cleanup();
    ssi_item_close();
    sol_trigger_cleanup();
    return SOL_SUCCESS;
}
