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
#include "replay.h"
#include "trigger.h"

#define DSL_ORDER     (0)
#define DSL_LOS_ORDER (1)

uint8_t command_implemented = 0; // Temporary while I figure out each function.

/* Globals */
void get_parameters(int16_t amt);

gpl_param_t param;
/* End Globals */

void gpl_change_region(const int region_id) {
    //gpl_execute_subroutine(region_id, 0, 1);
    replay_print("rep.change_region(%lld)\n", region_id);
    gpl_lua_execute_script(region_id, 0, 1);
}

static void initialize_gpl_stack() {
    gpl_global_strings = (gpl_string_t*) malloc(GSTRINGVARSIZE);
    memset(gpl_global_strings, 0, GSTRINGVARSIZE);
    gpl_local_strings = (gpl_string_t*) malloc(LSTRINGVARSIZE);
    memset(gpl_local_strings, 0, LSTRINGVARSIZE);
    gpl_global_string = (uint8_t*)malloc(TEXTSTRINGSIZE);
}

extern void gpl_init() {
    info("Initalizing DSL.\n");
    initialize_gpl_stack();
    powers_init();
    gpl_init_vars();
    sol_player_init();
    ssi_item_init();
    sol_trigger_init();
    gpl_manager_init();
    sol_region_manager_init();
    info("Running Master DSL #99.\n");
    gpl_lua_execute_script(99, 0, 1);
}

extern void gpl_cleanup() {
    powers_cleanup();
    free(gpl_global_string);
    free(gpl_local_strings);
    free(gpl_global_strings);
    gpl_cleanup_vars();
    gpl_manager_cleanup();
    ssi_item_close();
    sol_trigger_cleanup();
    sol_region_manager_cleanup();
}
